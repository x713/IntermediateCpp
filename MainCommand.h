#pragma once
 
#include "ComTypes.h"

#include "SharedBuffer.h"
#include "JobWorker.h"

#include "Utils.h"


using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

using lab::data::SharedBuffer;

using lab::worker::FileJob;
using lab::worker::ReaderJob;
using lab::worker::WriterJob;


CLIState::State MainCommand(CLI_Vector args)
{
  if (args.size() != 2) {
    std::cout << " Two filenames expected" << std::endl;
    return CLIState::State::ERR_FMT_COMMAND;
  }

  const std::string in_filename{ args[0] };
  const std::string out_filename{ args[1] };

  std::cout << "Input: " << in_filename << ", Output: " << out_filename << std::endl;

  if (in_filename == out_filename) {
    std::cout << " Different filenames expected" << std::endl;
    return CLIState::State::ERR_FMT_COMMAND;
  }

  std::shared_ptr<SharedBuffer> ipc_buffer = std::make_shared<SharedBuffer>();

  ReaderJob reader{ in_filename, ipc_buffer };
  WriterJob writer{ out_filename, ipc_buffer };

  std::thread readerThread(&FileJob::process, &reader);
  std::thread writerThread(&FileJob::process, &writer);

  readerThread.join();
  writerThread.join();

  std::cout << "Main thread finished.";

  return CLIState::State::OK;
}
