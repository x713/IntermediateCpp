#include "../../core/data/NetworkDataIO.h"
#include "../../core/data/HeaderDataIO.h"
#include "../../core/data/CryptoDataIO.h"
#include "../../core/proc/ProcessorFactory.h"
#include "../../core/workers/JobWorker.h"

using lab::data::NetworkDataSource;
using lab::data::NetworkDataSink;
using lab::data::HeaderDataSource;
using lab::data::HeaderDataSink;
using lab::data::CryptoDataSource;
using lab::data::CryptoDataSink;
using lab::data::FileDataSource;
using lab::data::FileDataSink;

using lab::processing::ProcessorFactory;
using lab::workers::ReaderJob;
using lab::workers::WriterJob;

#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// Utility function to sanitize filename
std::string sanitizeFilename(const std::string& filename) {
    std::string sanitized = filename;
    // Remove paths and replace strange characters with underscores
    fs::path p(sanitized);
    sanitized = p.filename().string(); 
    
    std::replace_if(sanitized.begin(), sanitized.end(), [](char c) {
        return (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|');
    }, '_');
    
    return sanitized;
}

CLIState::State NetworkFileServerCommand(CLI_Vector args)
{
  Utils::Log("Network FileServer starting...");

  try {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

    Utils::Log("Waiting for client connection...");
    auto socket = std::make_shared<tcp::socket>(io_context);
    acceptor.accept(*socket);
    Utils::Log("Client connected!");

    // 1. Build chain: Socket -> Crypto -> Header
    auto netSrc = std::make_shared<NetworkDataSource>(socket);
    auto cryptoSrc = std::make_shared<CryptoDataSource>(netSrc, lab::data::MOCK_KEY); 
    auto headerSrc = std::make_shared<HeaderDataSource>(cryptoSrc);

    // 2. Create processor for Reader -> Writer
    auto processor = ProcessorFactory::createThreaded();

    // 3. Trigger header reading to get metadata
    char dummy;
    headerSrc->read(&dummy, 0); 
    auto header = headerSrc->getHeader();
    
    std::string clientDirName = sanitizeFilename(header.clientName);
    if (clientDirName.empty()) clientDirName = "unknown_client";

    std::string safeFileName = sanitizeFilename(header.fileName);
    if (safeFileName.empty()) safeFileName = "unnamed_file.dat";

    // Create directory for client
    fs::path storagePath = fs::current_path() / "storage" / clientDirName;
    fs::create_directories(storagePath);

    fs::path finalPath = storagePath / safeFileName;

    Utils::Log("Client: " + clientDirName + " is sending file: " + safeFileName);
    Utils::Log("Saving to: " + finalPath.string());

    auto fileSink = std::make_shared<FileDataSink>(finalPath.string());
    fileSink->open();

    // 4. Start workers
    ReaderJob netReader{ "NetReader", headerSrc, processor };
    WriterJob fileWriter{ "FileWriter", fileSink, processor };

    std::thread t1(&ReaderJob::process, &netReader);
    std::thread t2(&WriterJob::process, &fileWriter);

    t1.join();
    t2.join();

    Utils::Log("File received and saved successfully at " + finalPath.string());
  }
  catch (std::exception& e) {
    Utils::LogErr("Server Error: " + std::string(e.what()));
    return CLIState::State::ERR_EXCEPTION;
  }
  return CLIState::State::OK;
}



CLIState::State NetworkFileClientCommand(CLI_Vector args)
{
  if (args.empty()) {
    Utils::LogErr("Usage: nfc <filename> [client_name]");
    return CLIState::State::ERR;
  }
  
  std::string filename = args[0];
  std::string clientName = (args.size() > 1) ? args[1] : "DefaultClient";

  Utils::Log("Network FileClient starting as '" + clientName + "' for: " + filename);

  try {
    boost::asio::io_context io_context;
    auto socket = std::make_shared<tcp::socket>(io_context);

    tcp::resolver resolver(io_context);
    boost::asio::connect(*socket, resolver.resolve("127.0.0.1", "12345"));

    // 1. Build chain: Header -> Crypto -> Network
    auto netSink = std::make_shared<NetworkDataSink>(socket);
    auto cryptoSink = std::make_shared<CryptoDataSink>(netSink, lab::data::MOCK_KEY);
    auto headerSink = std::make_shared<HeaderDataSink>(cryptoSink, 1, clientName, filename);

    // 2. Source data - file
    auto fileSrc = std::make_shared<FileDataSource>(filename);
    if (fileSrc->open() != lab::data::IOStatus::Ok) {
        Utils::LogErr("Could not open input file: " + filename);
        return CLIState::State::ERR;
    }

    auto processor = ProcessorFactory::createThreaded();

    // 3. Start workers
    ReaderJob fileReader{ "FileReader", fileSrc, processor };
    WriterJob netWriter{ "NetWriter", headerSink, processor };

    std::thread t1(&ReaderJob::process, &fileReader);
    std::thread t2(&WriterJob::process, &netWriter);

    t1.join();
    t2.join();

    Utils::Log("File sent successfully!");
  }
  catch (std::exception& e) {
    Utils::LogErr("Client Error: " + std::string(e.what()));
    return CLIState::State::ERR;
  }

  return CLIState::State::OK;
}
