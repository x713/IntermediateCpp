#pragma once

#include "../../core/cli/ComTypes.h"

#include "../../core/proc/IProcessor.h"
#include "../../core/proc/ThreadedDataProcessor.h"
#include "../../core/proc/ProcessorFactory.h"
#include "../../core/workers/JobWorker.h"

#include "../../util/Utils.h"
#include <string>
#include <thread>


using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

using lab::processing::IPCProcessorFactory;

using lab::workers::BaseJob;
using lab::workers::ReaderJob;
using lab::workers::WriterJob;
//using lab::worker::IPCServerJob;
//using lab::worker::IPCClientJob;

using lab::util::Utils;


CLIState::State SharedServerCommand(CLI_Vector args)
{
  Utils::Log("SharedServerCommand finished.");


  if (args.size() != 1) {
    Utils::Log(" filename expected");
    return CLIState::State::ERR_FMT_COMMAND;
  }

  const std::string in_filename{ args[0] };

  Utils::Log("Input: " + in_filename);

  auto ipc_processor = IPCProcessorFactory::createServer();

  auto fsd = std::make_shared<lab::data::FileDataSource>(in_filename);
  fsd->open();

  ReaderJob server_job{ "IPCServerReader", fsd, ipc_processor };

  server_job.process();

  Utils::Log("Server process finished.");

  return CLIState::State::OK;
}
