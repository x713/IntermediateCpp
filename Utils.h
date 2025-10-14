#pragma once

#include <iostream>

#include "ComTypes.h"


namespace lab {
  namespace util {

    class Utils {
    public:


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

      template<class T>
      static inline void LogDebug(T p_str, const char p_endl = '\n') {
#ifdef _DEBUG
        Log(p_str, p_endl);
#endif
      }

      template<class T>
      static void Log(T p_str, const char p_endl = '\n') {
        std::cout << p_str;
        if ('\n' == p_endl) {
          std::cout << std::endl;
        }
        else {
          std::cout << p_endl;
        }
      }
    };
  }
}

