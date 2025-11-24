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

using lab::workers::FileJob;
using lab::workers::ReaderJob;
using lab::workers::WriterJob;
//using lab::worker::IPCServerJob;
//using lab::worker::IPCClientJob;

using lab::util::Utils;

CLIState::State SharedClientCommand(CLI_Vector args)
{
  Utils::Log("SharedClientCommand finished.");


  if (args.size() != 1) {
    Utils::Log(" filename expected");
    return CLIState::State::ERR_FMT_COMMAND;
  }

  const std::string out_filename{ args[0] };

  Utils::Log("Output: " + out_filename);

  auto ipc_buffer = IPCProcessorFactory::createClient();

  WriterJob client_job{ out_filename, ipc_buffer };
  //IPCClientJob client_job{ out_filename, ipc_buffer };

  client_job.process();

  Utils::Log("Client process finished.");

  return CLIState::State::OK;
}