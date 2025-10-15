#include "shared/cli_processor/CLIProcessor.h"
#include "app/cmds/MainCommand.h"

using lab::cli::CLIProcessor;
using lab::cli::CLIState;

int main(const int argc, const char* argv[])
{

  CLIProcessor cmdProc(argc, argv);
  cmdProc.TrimProgramName(argv[0]);

  cmdProc.SetHelp("  multi tool \n"  
                  "    --cp to copy files"
                  "    --ncp to copy files through network");

  cmdProc.AddCommand("cp1", MainCommand);
  //cmdProc.AddCommand("ncp", NetworkCommand);

  int result = CLIState::toInt(cmdProc.Run());

  return result;
}

