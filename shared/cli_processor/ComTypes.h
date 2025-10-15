#pragma once

#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <map>

#include "CLIState.h"

namespace lab {
  namespace cli {

    using lab::cli::CLIState;

    using CLI_String = std::string;

    using CLI_Vector = std::vector<CLI_String>;
    using CLI_Pair = std::pair<CLI_String, CLI_String>;
    using CLI_Map = std::map<CLI_String, CLI_String>;



    //using CLI_Command = CLIState (*)(CLI_Vector);

    using CLI_Command = std::function< CLIState::State (CLI_Vector) >;
    using CLI_CommandMap = std::map< CLI_String, CLI_Command >;


    static CLI_String StripQuotes(CLI_String m_str, const char p_TrimChar = '\"') {
      CLI_String result{ m_str };

      // trim first
      while (result.size() > 0 && p_TrimChar == result[0])
      {
        result = result.substr(1, result.size() - 1);
      }

      // trim last
      while (result.size() > 0 && p_TrimChar == result[result.size() - 1])
      {
        result = result.substr(0, result.size() - 1);
      }

      return result;
    }
  }
}