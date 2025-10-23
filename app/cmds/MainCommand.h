#pragma once
 
#include "../../shared/cli_processor/ComTypes.h"

#include "../../shared/buffer/IBuffer.h"
#include "../../shared/buffer/RingBuffer.h"
#include "../../shared/workers/JobWorker.h"

#include "../../util/Utils.h"
#include <string>
#include <thread>


using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

using lab::data::RingBufferFactory;

using lab::worker::FileJob;
using lab::worker::ReaderJob;
using lab::worker::WriterJob;

using lab::util::Utils;


CLIState::State CopyCommand(CLI_Vector args)
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

  auto thr_buffer = RingBufferFactory::createSyncTrippleBuffer();

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

  auto ipc_buffer = RingBufferFactory::createSyncIPCBuffer();

  return CLIState::State::OK;
}

CLIState::State SharedClientCommand(CLI_Vector args)
{
  Utils::Log("SharedClientCommand finished.");

  return CLIState::State::OK;
}