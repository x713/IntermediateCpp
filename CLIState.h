#pragma once

namespace lab {
  namespace cli {


    class CLIState {

    public:

      // CLIState
      enum class State {
        OK = 0,
        ERR = 1,         // General Unknown Errror
        ERR_EXCEPTION = 2, // Exception when calling command
        ERR_NOT_FOUND = 3, // Command not foiund in hashmap
        ERR_EMPTY_COMMAND = 4, // Command name is empty
        ERR_FMT_COMMAND = 5, // Command format is wrong
        ERR_UNKN_CAST = 6, // UnknownCast
      };


      //
      //   CLIState Cast
      //     _toInt
      //     _fromInt
      static int toInt(State p_state) {
        return static_cast<int>(p_state);
      }

      static State fromInt(int p_state) {
        State result = State::OK;

        switch (p_state)
        {
        case 0:
          result = State::OK;
          break;
        case 1:
          result = State::ERR;
          break;
        case 2:
          result = State::ERR_EXCEPTION;
          break;
        case 3:
          result = State::ERR_NOT_FOUND;
          break;
        case 4:
          result = State::ERR_EMPTY_COMMAND;
          break;
        case 5:
          result = State::ERR_FMT_COMMAND;
          break;
        default:
          result = State::ERR_UNKN_CAST;
        }

        return result;
      }

      static bool isOK(State p_state) {
        return (State::OK == p_state);
      }

    };
  }
}