
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <boost/asio.hpp>

// Boost section end

#include "core/cli/CLIProcessor.h"
#include "app/commands/CopyCommand.h"
#include "app/commands/IPCServerCommand.h"
#include "app/commands/IPCClientCommand.h"
#include "app/commands/NetwotrkFileServerCommand.h"

#include "app/commands/samples/SampleTimer.h"

using lab::cli::CLIProcessor;
using lab::cli::CLIState;
using lab::cli::CLI_CommandSet;


int main(const int argc, const char* argv[])
{
  std::set_terminate(Utils::custom_terminate);

  CLIProcessor cmdProc(argc, argv);
  cmdProc.TrimProgramName(argv[0]);

  cmdProc.SetHelp("  multi tool"
    "\n    --cp to copy files"
    "\n    --tps to start copy server and broadcast file"
    "\n    --tpc to start copy client and receive file");

  const CLI_CommandSet commandSet = {
    // single call
    {"cp", ThreadedCopyCommand},
    // Shared cpy - must be called twice, one for each side separatelly
    {"tps", SharedServerCommand},
    {"tpc", SharedClientCommand},
    // Network server & client
    {"nfs", NetworkFileServerCommand}};
  cmdProc.AddCommandSet(commandSet);

  // Boost examples
  const CLI_CommandSet boostSet = {
    // current debug cmd
    {"bs00", BostTimerCommand003},
    // Boost timer example 1
    {"bs01", BostTimerCommand001},
    // Boost timer example 2
    {"bs02", BostTimerCommand002},
    // Boost timer example 3
    {"bs03", BostTimerCommand003}
  };
  cmdProc.AddCommandSet(boostSet);

  auto result = cmdProc.Run();

  return CLIState::toInt(result);
}

