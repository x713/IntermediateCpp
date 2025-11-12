#pragma once
 
#include "../../shared/cli_processor/ComTypes.h"

#include "../../shared/buffer/IProcessor.h"
#include "../../shared/buffer/RingBuffer.h"
#include "../../shared/workers/JobWorker.h"

#include "../../util/Utils.h"
#include <string>
#include <thread>


using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

using lab::data::CopyProcessorFactory;

using lab::worker::FileJob;
using lab::worker::ReaderJob;
using lab::worker::WriterJob;
using lab::worker::IPCReaderJob;
using lab::worker::IPCWriterJob;

using lab::util::Utils;


CLIState::State ThreadedCopyCommand(CLI_Vector args)
{
  if (args.size() != 2) {
    Utils::Log(" Two filenames expected");
    return CLIState::State::ERR_FMT_COMMAND;
  }

  const std::string in_filename{ args[0] };
  const std::string out_filename{ args[1] };

  Utils::Log("Input: " + in_filename + ", Output: " + out_filename);

  if (in_filename == out_filename) {
    Utils::Log(" Different filenames expected");
    return CLIState::State::ERR_FMT_COMMAND;
  }

  auto thr_buffer = CopyProcessorFactory::createThreadsProcessor();

  ReaderJob reader{ in_filename, thr_buffer };
  WriterJob writer{ out_filename, thr_buffer };

  std::thread readerThread(&FileJob::process, &reader);
  std::thread writerThread(&FileJob::process, &writer);

  readerThread.join();
  writerThread.join();

  Utils::Log("Main thread finished.");

  return CLIState::State::OK;
}


CLIState::State SharedServerCommand(CLI_Vector args)
{
  Utils::Log("SharedServerCommand finished.");


  if (args.size() != 1) {
    Utils::Log(" filename expected");
    return CLIState::State::ERR_FMT_COMMAND;
  }

  const std::string in_filename{ args[0] };
  
  Utils::Log("Input: " + in_filename);

  auto ipc_buffer = CopyProcessorFactory::createIntersystemProcessor();

  IPCReaderJob server_job{ in_filename, ipc_buffer };

  server_job.process();

  Utils::Log("Main thread finished.");

  return CLIState::State::OK;
}

CLIState::State SharedClientCommand(CLI_Vector args)
{
  Utils::Log("SharedClientCommand finished.");


  if (args.size() != 1) {
    Utils::Log(" filename expected");
    return CLIState::State::ERR_FMT_COMMAND;
  }

  const std::string out_filename{ args[0] };

  Utils::Log("Input: " + out_filename);

  auto ipc_buffer = CopyProcessorFactory::createIntersystemProcessor();

  IPCWriterJob client_job{ out_filename, ipc_buffer };

  client_job.process();

  Utils::Log("Main thread finished.");

  return CLIState::State::OK;
}