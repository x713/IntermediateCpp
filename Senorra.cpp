#include "shared/cli_processor/CLIProcessor.h"
#include "app/cmds/MainCommand.h"

using lab::cli::CLIProcessor;
using lab::cli::CLIState;

int main(const int argc, const char* argv[])
{

  CLIProcessor cmdProc(argc, argv);
  cmdProc.TrimProgramName(argv[0]);

  cmdProc.SetHelp("  multi tool"  
                  "\n    --cp to copy files"
                  "\n    --tps to start copy server and broadcast file"
                  "\n    --tpc to start copy client and receive file");

  cmdProc.AddCommand("cp", CopyCommand);
  cmdProc.AddCommand("tpc", SharedClientCommand);
  cmdProc.AddCommand("tps", SharedServerCommand);

  int result = CLIState::toInt(cmdProc.Run());

  return result;
}

