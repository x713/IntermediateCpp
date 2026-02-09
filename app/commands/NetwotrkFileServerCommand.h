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
    auto cryptoSrc = std::make_shared<CryptoDataSource>(netSrc, ); // Mock-encryption key
    auto headerSrc = std::make_shared<HeaderDataSource>(cryptoSrc);

    // 2. Create processor for Reader -> Writer
    auto processor = ProcessorFactory::createThreaded();

    // 3. For server we need to read header first to know the filename
    char dummy;
    headerSrc->read(&dummy, 0); // Trigger header reading
    auto header = headerSrc->getHeader();
    std::string receivedName = "received_" + std::string(header.fileName);
    Utils::Log("Receiving file: " + receivedName + " from " + std::string(header.clientName));

    auto fileSink = std::make_shared<FileDataSink>(receivedName);
    fileSink->open();

    // 4. Start workers
    ReaderJob netReader{ "NetReader", headerSrc, processor };
    WriterJob fileWriter{ "FileWriter", fileSink, processor };

    std::thread t1(&ReaderJob::process, &netReader);
    std::thread t2(&WriterJob::process, &fileWriter);

    t1.join();
    t2.join();

    Utils::Log("File received and saved successfully!");
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
    Utils::LogErr("Filename expected!");
    return CLIState::State::ERR;
  }
  std::string filename = args[0];

  Utils::Log("Network FileClient starting for: " + filename);

  try {
    boost::asio::io_context io_context;
    auto socket = std::make_shared<tcp::socket>(io_context);

    tcp::resolver resolver(io_context);
    boost::asio::connect(*socket, resolver.resolve("127.0.0.1", "12345"));

    // 1. Build chain: Header -> Crypto -> Network
    auto netSink = std::make_shared<NetworkDataSink>(socket);
    auto cryptoSink = std::make_shared<CryptoDataSink>(netSink, 0x42);
    auto headerSink = std::make_shared<HeaderDataSink>(cryptoSink, 1, "Client01", filename);

    // 2. Source data - file
    auto fileSrc = std::make_shared<FileDataSource>(filename);
    if (fileSrc->open() != lab::data::IOStatus::Ok) {
        Utils::LogErr("Could not open input file!");
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
