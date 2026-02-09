#pragma once

#include "../../core/cli/ComTypes.h"
#include "../../core/cli/CLIState.h"


#include "../../util/Utils.h"
#include <string>
#include <thread>


using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLI_Command;

//using lab::worker::IPCServerJob;
//using lab::worker::IPCClientJob;

using lab::util::Utils;


CLIState::State NetworkFileServerCommand(CLI_Vector args)
{
  Utils::Log("Network FileServer Command");
  return CLIState::State::OK;
}

CLIState::State NetworkFileClientCommand(CLI_Vector args)
{
  Utils::Log("Network FileClient Command");
  return CLIState::State::OK;
}
