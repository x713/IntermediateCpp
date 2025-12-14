#pragma once

#include <iostream>


namespace lab {
  namespace util {

    //using lab::cli::CLI_String;

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

      static void custom_terminate() {
        try {
          auto exc = std::current_exception();
          if (exc) {
            std::rethrow_exception(exc);
          }
        }
        catch (const std::exception& e) {
          Log("Terminate called: " + std::string(e.what()));
        }
        // Cleanup shared memory
        std::abort();
      }

    };

    // Logging class for stack unwinding
		// Inject object instance to log entering and leaving scope
    class ScopeLogger {
      std::string m_name;
    public:
      ScopeLogger(std::string name) : m_name(name) {
        Utils::Log("Entering: " + m_name);
      }

      ~ScopeLogger() {
        Utils::Log("Leaving: " + m_name);
      }
    };

  }
}

