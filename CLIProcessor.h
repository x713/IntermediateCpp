#pragma once

#include <string>
#include <vector>


#include "ComTypes.h"
#include "CLIState.h"



namespace lab {
  namespace util {
    class CLIProcessor {

    private:
      //constexpr static const char* c_argsName = "_args_";

      CLI_Vector m_args;

      CLI_CommandMap m_commands;

      CLIProcessor() = delete; // no default constructor

      protected:
      CLI_Vector m_parsedArgs;

      void ParseArgs();

      CLI_String GetNextCommand();

      public:
       CLIProcessor(const int argc, const char** argv);

       explicit CLIProcessor(CLI_Vector p_args);

       // Run processor task, ParseFlags, evaluate chosen commmand and execute it.
       CLIState Run();

       // Add command to map
       void AddCommand(const CLI_String, const CLI_Command);

       // Remove program name from arg list
       void TrimProgramName(CLI_String);

       virtual ~CLIProcessor() = default;

    };

    CLIState CLICommandTestPASS(CLI_Vector args);

    CLIState CLICommandTestFAIL(CLI_Vector args);
  }
}