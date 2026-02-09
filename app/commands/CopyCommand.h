#pragma once

#include "../../core/cli/ComTypes.h"

#include "../../core/proc/IProcessor.h"
#include "../../core/proc/ProcessorFactory.h"
#include "../../core/workers/JobWorker.h"

#include "../../util/Utils.h"
#include <string>
#include <thread>


using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

using lab::processing::ProcessorFactory;

using lab::workers::BaseJob;
using lab::workers::ReaderJob;
using lab::workers::WriterJob;

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

  auto thr_processor = ProcessorFactory::createThreaded();

  auto fsd = std::make_shared<lab::data::FileDataSource>(in_filename);
  fsd->open();

  auto fsk = std::make_shared<lab::data::FileDataSink>(out_filename);
  fsk->open();

  ReaderJob reader{ "Reader", fsd, thr_processor };
  WriterJob writer{ "Writer", fsk, thr_processor };

  std::thread readerThread(&ReaderJob::process, &reader);
  std::thread writerThread(&WriterJob::process, &writer);

  readerThread.join();
  writerThread.join();

  Utils::Log("Main thread finished.");

  return CLIState::State::OK;
}