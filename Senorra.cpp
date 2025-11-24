#include "core/cli/CLIProcessor.h"
#include "app/commands/CopyCommand.h"
#include "app/commands/IPCServerCommand.h"
#include "app/commands/IPCClientCommand.h"

using lab::cli::CLIProcessor;
using lab::cli::CLIState;
using lab::cli::CLI_CommandSet;

int main(const int argc, const char* argv[])
{

  CLIProcessor cmdProc(argc, argv);
  cmdProc.TrimProgramName(argv[0]);

  cmdProc.SetHelp("  multi tool"  
                  "\n    --cp to copy files"
                  "\n    --tps to start copy server and broadcast file"
                  "\n    --tpc to start copy client and receive file");

  const CLI_CommandSet commandSet = {
    // single call
    {"cp", ThreadedCopyCommand},
    // must be called twice, one for each side separatelly
    {"tps", SharedServerCommand},
    {"tpc", SharedClientCommand}
  };

  cmdProc.AddCommandSet(commandSet);

  auto result = cmdProc.Run();

  return CLIState::toInt(result);
}

