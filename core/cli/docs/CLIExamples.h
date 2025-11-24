#pragma once

#include "../CLIProcessor.h"
#include "../../../util/Utils.h"

using lab::cli::CLIState;
using lab::cli::CLI_Vector;
using lab::cli::CLIProcessor;

namespace lab {
  namespace util {

    CLIState::State CLIE_EXAMPLE_TEST() {

      CLI_Vector params{ "test2", "subproc2", "flag1" };

      CLIProcessor cmdProc(params);

      cmdProc.AddCommand("subproc1", CLICommandTestFAIL);
      cmdProc.AddCommand("subproc2", CLICommandTestPASS);
      cmdProc.AddCommand("test2", CLICommandTestPASS);

      return cmdProc.Run();
    }

    CLIState::State CLICommandSendStatus(CLI_Vector args) {
      Utils::Log(" CLICommandSend sub cmd called ");
      return CLIState::State::OK;
    }

    CLIState::State CLICommandSend(CLI_Vector args) {
      CLIProcessor subcmd_proc(args);
      subcmd_proc.AddCommand("status", CLICommandSendStatus);
      subcmd_proc.AddCommand("pick", CLICommandSendStatus);
      subcmd_proc.AddCommand("send", CLICommandSendStatus);
      subcmd_proc.AddCommand("run", CLICommandSendStatus);

      auto result = subcmd_proc.Run();

      Utils::LogDebug(CLIState::toInt(result));

      return result;

    }

    CLIState::State CLI_EXAMPLE_RUN(const int argc, const char* argv[]) {

      CLIProcessor cmdProc(argc, argv);

      //cmdProc.AddCommand("send", CLICommandSend);
      //cmdProc.AddCommand("recv", CLICommandRecv);

      return cmdProc.Run();
    }

    CLIState::State CLICommandTestPASS(CLI_Vector args) {
      Utils::Log("CLICommandTestPASS");
      return CLIState::State::OK;
    }

    CLIState::State CLICommandTestFAIL(CLI_Vector args) {
      Utils::Log("CLICommandTestFAIL");
      return CLIState::State::ERR;
    }
  }
}