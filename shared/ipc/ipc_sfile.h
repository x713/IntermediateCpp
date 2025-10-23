#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

namespace lab {
  namespace data {

    class SharedData {
    };

    class SharedFile : public SharedData {
    };

    class SharedUnixFile : public SharedFile {
    };

    class SharedWinFile : public SharedFile {
      void makeSharedFile() {
        HANDLE hFile = CreateFile(L"swap.file", GENERIC_READ | GENERIC_WRITE, ...);
        HANDLE hMapFile = CreateFileMapping(
          hFile,                // Дескриптор реального файла
          NULL,
          PAGE_READWRITE,
          0,
          0,                    // Размер (0 означает размер файла)
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

    _Ret_maybenull_

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
          L"MySharedMemoryName" // Имя объекта для доступа из других процессов
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
