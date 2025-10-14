#pragma once

#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <map>

#include "CLIState.h"

namespace lab {
  namespace util {

    using CLI_String = std::string;
    using CLI_Vector = std::vector<CLI_String>;
    using CLI_Pair = std::pair<CLI_String, CLI_String>;
    using CLI_Map = std::map<CLI_String, CLI_String>;



    //using CLI_Command = CLIState (*)(CLI_Vector);
    using CLI_Command = std::function<CLIState(CLI_Vector)>;
    using CLI_CommandMap = std::map< CLI_String, CLI_Command>;

  }
}