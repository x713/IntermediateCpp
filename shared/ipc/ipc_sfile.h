#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

namespace lab {
  namespace data {

    template<size_t BufferPool>
    struct SharedData {
      size_t writeIdx = 0;
      size_t readIdx = BufferPool - 1;

      size_t dataSize[BufferPool] = {0}; 
      long long fileSize = 0; 

      bool isReadingInput = false;
    };

    class SharedFile : public SharedData {
    };

    class SharedUnixFile : public SharedFile {
    };

    class SharedWinFile : public SharedFile {
      void makeSharedFile() {
        HANDLE hFile = CreateFile(L"swap.file", GENERIC_READ | GENERIC_WRITE, ...);
        HANDLE hMapFile = CreateFileMapping(
          hFile,                // Handle to mapping object
          NULL,
          PAGE_READWRITE,
          0,
          0,                    // max page size LW
          L"MySharedFileName"
        );


        if (hMapFile != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
        {
          CloseHandle(hMapFile);
          hMapFile = NULL;
        }
      }

    };

    class SharedMemory : public SharedData {
    };

    class SharedUnixMemory : public SharedMemory {
    };

    //_Ret_maybenull_

    class SharedWinMemory : public SharedMemory {
      void makeMemFile() {


        /*
        *
        *       CreateFileMappingW(
                _In_     HANDLE hFile,
                _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                _In_     DWORD flProtect,
                _In_     DWORD dwMaximumSizeHigh,
                _In_     DWORD dwMaximumSizeLow,
                _In_opt_ LPCWSTR lpName
              )
        */
        HANDLE hMapFile = CreateFileMapping(
          INVALID_HANDLE_VALUE, // System swapfile (SharedMemory)
          NULL,                 // Attributes
          PAGE_READWRITE,       // Page access type
          0,//dwMaximumSizeHigh,    // max page size HW
          0,//dwMaximumSizeLow,     // max page size LW
          L"MySharedMemoryName" // Name of the object
        );


        if (hMapFile != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
        {
          CloseHandle(hMapFile);
          hMapFile = NULL;
        }

      }
    };


  }
}

#include <memory>

namespace lab {
  namespace fab {
    using lab::data::SharedData;
    using lab::data::SharedWinFile;
    using lab::data::SharedUnixFile;
    class SharedDataFactory {
    public:
      static std::shared_ptr<SharedData> createDataObject() {
        std::shared_ptr<SharedData> result = nullptr;
#if defined(__linux__)
        result = std::shared_ptr<SharedUnixFile>();
#elif _WIN32
        result = std::shared_ptr<SharedWinFile>();
#else
        std::cout << "OS unknown!" << '\n';
#endif
        return result;
      }
    };
  }
}
