#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <string>
#include <mutex>


namespace lab {
  namespace data {

    // threads mutex
    class ThreadMutex : public std::mutex {

    };



    // Provides a system-wide, recursive, named lock. 
    // This class satisfies requirements of C++11 concept "Lockable",
    // i.e. can be (and should be) used with unique_lock etc.
    class NamedWinMutex : public ThreadMutex
    {
    private:
      HANDLE m_handle{};
      const std::wstring& m_name = L"IPCWMutex";
    public:


      NamedWinMutex() {
        m_handle = CreateMutexW(nullptr, FALSE, m_name.c_str());
        if (m_handle == NULL) {
          throw std::runtime_error("Creation of mutex failed");
        }
      }

      ~NamedWinMutex() {
        const BOOL result = CloseHandle(m_handle);
        if (result == FALSE) {
          // Error: Failed to close mutex handle (Error ignored since we are in destructor)
        }
      }

      // Moveable, not copyable
      NamedWinMutex(const NamedWinMutex& other) = delete;
      NamedWinMutex(NamedWinMutex&& other) = default;
      NamedWinMutex& operator=(const NamedWinMutex& other) = delete;
      NamedWinMutex& operator=(NamedWinMutex&& other) = default;



      void lock() {
        const auto result = WaitForSingleObject(m_handle, INFINITE);
        if (result == WAIT_ABANDONED) {
          // Warning: Lock obtained, but on an abandoned mutex
          // (was not correctly released before, e.g. due to a crash)
          // The thread got ownership of an abandoned mutex
          // The database is in an indeterminate state
          throw std::runtime_error("Abandoned mutex");;
        }
        else if (result != WAIT_OBJECT_0) {
          throw std::runtime_error("Failed to acquire lock");
        }
      }

      void unlock() {
        const BOOL result = ReleaseMutex(m_handle);
        if (result == FALSE) {
          throw std::runtime_error("Failed to release lock: does not own the mutex");
        }
      }

      bool try_lock();

    };

  }
}