#pragma once

#include <iostream>


namespace lab {
  namespace util {

    using lab::cli::CLI_String;

    class Utils {
    public:


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

