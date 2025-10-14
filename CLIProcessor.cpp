#pragma once

#include <string>
#include <vector>

#include "CLIProcessor.h"

#include "Utils.h"



namespace lab {
  namespace util {
    CLIProcessor::CLIProcessor(const int argc, const char** argv) {
      m_args = std::vector<CLI_String>(argv, argv + argc);
      ParseArgs();
    }

    CLIProcessor::CLIProcessor(CLI_Vector p_args) : m_args(p_args) {
      ParseArgs();
    }

    // Add command to map
    void CLIProcessor::AddCommand(const CLI_String p_cmdName, const CLI_Command p_cmdPtr) {
      m_commands[p_cmdName] = p_cmdPtr;
    }

    CLI_String CLIProcessor::GetNextCommand() {
      if (!m_parsedArgs.empty()) {
        auto l_cmdName = m_parsedArgs.front();

        m_parsedArgs.erase(m_parsedArgs.begin());

        return l_cmdName;
      }

      // Command not found
      return "";
    }


    void CLIProcessor::ParseArgs() {
      for (const auto& arg : m_args) {
        m_parsedArgs.push_back(Utils::StripQuotes(arg));
      }
      m_args.clear();
    }



    // Remove program name from arg list
    void CLIProcessor::TrimProgramName(CLI_String p_appName) {
      if (!m_parsedArgs.empty()) {
        if (m_parsedArgs.front() == p_appName) {
          m_parsedArgs.erase(m_parsedArgs.begin());
        }
      }
    }

    // Run processor task, ParseFlags, evaluate chosen commmand and execute it.
    CLIState CLIProcessor::Run() {

      // Pop cmd
      auto cmd_name = GetNextCommand();

      CLIState result = CLIState::ERR;

      if (!cmd_name.empty()) {
        // evaluate and execute command
        if (m_commands.find(cmd_name) != m_commands.end()) { 

          try {
            return m_commands[cmd_name](m_parsedArgs);
          }
          catch (const std::exception& /*e*/) {
            // Optional: Log the exception message (e.what())
            // Utils::Log("Exception: ", e.what());
            return CLIState::ERR_NOT_FOUND;;
          }


        }else{
          Utils::Log("CLIProcessor:", ' ');
          Utils::Log("Error:", ' ');
          Utils::Log("Command not found", ' ');
          Utils::Log(cmd_name);
          result = CLIState::ERR_NOT_FOUND;
        }


      }
      else {
        Utils::Log("CLIProcessor:", ' ');
        Utils::Log("Error:", ' ');
        Utils::Log("Empty command", ' ');
        Utils::Log(cmd_name);
        result = CLIState::ERR_EMPTY_COMMAND;
      }


       return result;
    }

    CLIState CLICommandTestPASS(CLI_Vector args) {
      Utils::Log("CLICommandTestPASS");
      return CLIState::OK;
    }

    CLIState CLICommandTestFAIL(CLI_Vector args) {
      Utils::Log("CLICommandTestFAIL");
      return CLIState::ERR;
    }

  }
}