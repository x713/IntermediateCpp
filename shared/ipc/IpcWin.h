#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <string>
#include <mutex>

#include "ipc_mutex.h"



using lab::data::NamedWinMutex;

int main22()
{
  DWORD ThreadID;
  int i;
  int return_code = 0;
  // Create a mutex with no initial owner
  try {
    NamedWinMutex m{ L"IpcMutex" };

    std::lock_guard<NamedWinMutex> lock{ m };

    printf("Main thread acquired the mutex lock for setup.\n");


    // run worker();

   // Wait for all threads to terminate

   // WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);

   // Close thread and mutex handles
  }
  catch (std::runtime_error& e) {
    printf("Runtime Error during setup: %s\n", e.what());
    return_code = 1; // Indicate failure
  }
  catch (const std::exception& e)
  {
    printf("An unknown exception occurred during setup: %s\n", e.what());
    return_code = 2;
  }
  catch (...)
  {
    printf("An unhandled non-standard exception occurred during setup.\n");
    return_code = 3;
  }

  if (return_code == 0) // Only wait if thread creation was fully successful (no early break)
  {
    // Filter out NULL handles if needed, but WaitForMultipleObjects is usually okay
    // if the number of handles passed is correct.
    // Since we check the return_code and THREADCOUNT is fixed, we wait on the full array.
    printf("Waiting for all worker threads to terminate...\n");
    //WaitForMultipleObjects(THREADCOUNT, aThread.data(), TRUE, INFINITE);
    printf("All worker threads have terminated.\n");
  }
  printf("Closing thread handles.\n");
  /*
    for (HANDLE& thread_handle : aThread)
    {
      if (thread_handle != NULL)
      {
        CloseHandle(thread_handle);
        thread_handle = NULL; // Prevent double-closing if this were in a loop
      }
    }
    */
  return return_code;
}
