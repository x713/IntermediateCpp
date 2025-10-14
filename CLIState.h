#pragma once

namespace lab {
  namespace util {



    // CLIState
    enum class CLIState {
      OK = 0,
      ERR = 1,         // General Unknown Errror
      ERR_EXCEPTION = -1, // Exception when calling command
      ERR_NOT_FOUND = -2, // Command not foiund in hashmap
      ERR_EMPTY_COMMAND = -3, // Command name is empty
      ERR_UNKN_CAST = -4, // UnknownCast
    };


    //
    //   CLIState Cast
    //     _toInt
    //     _fromInt
    int CLIState_toInt(CLIState p_state) {
      return static_cast<int>(p_state);
    }

    CLIState CLIState_fromInt(int p_state) {
      CLIState result = CLIState::OK;

      switch (p_state)
      {
      case 0:
        result = CLIState::OK;
        break;
      case 1:
        result = CLIState::ERR;
        break;
      case -1:
        result = CLIState::ERR_EXCEPTION;
        break;
      case -2:
        result = CLIState::ERR_NOT_FOUND;
        break;
      case -3:
        result = CLIState::ERR_EMPTY_COMMAND;
        break;
      default:
        result = CLIState::ERR_UNKN_CAST;
      }

      return result;
    }
  }
}