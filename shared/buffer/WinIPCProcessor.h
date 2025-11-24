#pragma once


#include <vector>
#include <array>

#include <mutex>

#include <string>
#include <iostream> 

#include "IProcessor.h"
#include "LineBuffer.h"


#include "../../shared/ipc/ipc_mutex.h"
#include "../../util/Utils.h"

#include <windows.h>
#include <string>
#include <string_view>
/*
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
*/



namespace lab {
  namespace data {

    using lab::data::NamedWinMutex;
    using lab::util::Utils;

    // IPC names  
    const std::wstring& SHARED_MEM_NAME = L"Global\\TransferMemory";

    // Events names
    const std::wstring& SERVER_WROTE_EVENT_NAME = L"Global\\TransferServerWroteEvent";
    const std::wstring& CLIENT_READ_EVENT_NAME = L"Global\\TransferClientReadEvent";

    // Buffer size
    constexpr size_t BUFFER_SIZE = 64 * 1024;

    // Windows inter-process copy processor
    template<size_t PoolSize>
    class WinIPCProcessor : public IProcessor {
    public:
      // PoolSize
      // Tripple buffer at least
      static constexpr size_t c_minPoolSize = 3;
      static constexpr size_t c_bufferPool = PoolSize < c_minPoolSize ? c_minPoolSize : PoolSize;

      template<size_t BufferPool>
      struct SharedData {
        size_t writeIdx = 0;
        size_t readIdx = BufferPool - 1;

        size_t dataSize[BufferPool] = { 0 };
        //long long fileSize = 0; 

        bool isReadingInput = false;
        bool transferFinished = false;
      };

      // Total shared memory size: Header + Buffer pool
      static constexpr size_t TOTAL_SHARED_SIZE = sizeof(SharedData<c_bufferPool>) + ((sizeof(size_t) + BUFFER_SIZE) * c_bufferPool);

    private:
      SharedData<c_bufferPool> m_sharedData;

      bool m_isServer = true;

      HANDLE m_hMapFile = NULL;
      HANDLE m_hWroteEvent = NULL;
      HANDLE m_hReadEvent = NULL;
      std::shared_ptr<NamedWinMutex> m_IndexMutex;

      // Read / Write indexes
      // Sync mutex

      SharedData<c_bufferPool>* m_pHeader = nullptr;
      BYTE* m_pBufferData = nullptr; // Pointer to the start of the data area

    public:

      WinIPCProcessor(bool p_isServer = true) :
        m_isServer(p_isServer) {

        try {
          if (m_isServer) {
            createSharedMemory();
          }
          else {
            openSharedMemory();
          }
        }
        catch (const std::exception& /*e*/) {
          // Optional: Log the exception message (e.what())
          // Utils::Log("Exception: ", e.what());
          Utils::Log("Uninvited V.");
        }
      }

      size_t nextIdx(const size_t p_idx) {
        size_t result = p_idx + 1;
        if (result >= c_bufferPool) {
          result = 0;
        }
        return result;
      }



      IOStatus operator<< (IDataSource* p_dataSource) {
        if (!p_dataSource) {
          return IOStatus::IOFAILPTR;
        }
        size_t nextWriteIdx;

        const auto& m_readIdx = m_pHeader->readIdx;
        const auto& m_writeIdx = m_pHeader->writeIdx;

        { // CRITICAL SECTION 
          const std::lock_guard<NamedWinMutex> lock(*m_IndexMutex);

          // Check if buffer is full and we can't write
          nextWriteIdx = nextIdx(m_writeIdx);
          if (nextWriteIdx == m_readIdx) {
            // Buffer is still full
            return IOStatus::IORINGFULL; // Status::FullBuffer
          }

        } // END CRITICAL SECTION

        // write data from stream to shared memory
        auto result = write_to_shared(p_dataSource);

        const std::lock_guard<NamedWinMutex> lock(*m_IndexMutex);
        m_pHeader->writeIdx = nextWriteIdx;

        if (IOStatus::IOEOF == result || IOStatus::IOFAIL == result) {
          m_pHeader->isReadingInput = false;
        }

        return result;
      }


      // write data from data source to shared memory
      IOStatus write_to_shared(IDataSource* p_dataSource) {
        // Set pointer to the start of the data area
        auto p_bufferStart = (m_pBufferData + m_pHeader->writeIdx * (BUFFER_SIZE + sizeof(size_t)));
        auto p_bufferSize = (p_bufferStart + BUFFER_SIZE);

        //p_dataSource->read(p_bufferStart, p_bufferSize);
        p_dataSource->read(reinterpret_cast<char*>(p_bufferStart), BUFFER_SIZE);


        // actual read
        std::streamsize actual_read_count = p_dataSource->getCount();
        m_pHeader->dataSize[m_pHeader->writeIdx] = actual_read_count;

        if (p_dataSource->eof()) {
          Utils::LogDebug("EOF. Read bytes:", ' ');
          Utils::LogDebug(actual_read_count);

          return IOStatus::IOEOF;
        }
        else if (p_dataSource->fail()) {
          // read less than p_bufferSize, but not EOF
          Utils::LogDebug("FAIL : Read less than " + std::to_string(BUFFER_SIZE) + " bytes, but not EOF", ' ');
          Utils::LogDebug(actual_read_count);

          return IOStatus::IOFAIL;
        }

        //PrintBuffer();

        return IOStatus::IOOK;
      }

      // read data from shared memory to data sink
      IOStatus operator>> (IDataSink* p_dataSink) {
        if (!p_dataSink) {
          return IOStatus::IOFAILPTR;
        }

        const auto& readIdx = m_pHeader->readIdx;
        const auto& writeIdx = m_pHeader->writeIdx;
        const auto& isReadingInput = m_pHeader->isReadingInput;

        const size_t nextReadIdx = nextIdx(readIdx);

        { // CRITICAL SECTION 
        // try to move 'read window'
          const std::lock_guard<NamedWinMutex> lock(*m_IndexMutex);
          if (writeIdx == nextReadIdx) {
            if (isReadingInput) {
              // nextBuffer is still writing
              return IOStatus::IONEXTBUSY;
            }
            else {
              // reached end of the data
              m_pHeader->transferFinished = true;
              return IOStatus::IOEOF;
            }
          }

          // continue consuming buffer
          // update read index
          m_pHeader->readIdx = nextReadIdx;
        }

        // read data from shared memory to data sink
        //m_pool[readIdx] >> p_dataSink;

        auto result = read_from_shared(p_dataSink);

        return result;
      }

      IOStatus read_from_shared(IDataSink* p_dataSink) {
        if (!p_dataSink) {
          return IOStatus::IOFAILPTR;
        }

        // read data from shared memory to data sink
        const auto& cacheBufferSize = m_pHeader->dataSize[m_pHeader->readIdx];
        const auto& p_bufferStart = (m_pBufferData + m_pHeader->readIdx * (BUFFER_SIZE + sizeof(size_t)));

        if (cacheBufferSize) {
          //p_dataSink->write(p_bufferStart, cacheBufferSize);
          p_dataSink->write(reinterpret_cast<const char*>(p_bufferStart), cacheBufferSize);
        }
        else {
          Utils::LogDebug("WARN : Write buffer 0 zise, skip writing");
        }

        return IOStatus::IOOK;
      }

      void close() override {
        const std::lock_guard<NamedWinMutex> lock(*m_IndexMutex);
        m_pHeader->isReadingInput = false;
        m_pHeader->transferFinished = true;
      };

      ~WinIPCProcessor() {
        if (m_pHeader) {
          UnmapViewOfFile(m_pHeader);
        }
        if (m_hMapFile) {
          CloseHandle(m_hMapFile);
        }
        if (m_hWroteEvent) {
          CloseHandle(m_hWroteEvent);
        }
        if (m_hReadEvent) {
          CloseHandle(m_hReadEvent);
        }

      }


      bool createSharedMemory() {
        // 1. Create shared memory
        m_hMapFile = CreateFileMappingW(
          INVALID_HANDLE_VALUE,    // Use paging file
          NULL,                    // Default security attributes
          PAGE_READWRITE,          // Read/Write access
          (DWORD)(TOTAL_SHARED_SIZE >> 32), // High-order DWORD
          (DWORD)TOTAL_SHARED_SIZE,        // Low-order DWORD
          SHARED_MEM_NAME.c_str()          // Name of the object
        );
        if (m_hMapFile == NULL) {
          Utils::LogDebug("SERVER: Error CreateFileMapping (" + std::to_string(GetLastError()) + ")");
          return false;
        }

        // 2. Map shared memory into process address space
        m_pHeader = (SharedData<c_bufferPool>*)MapViewOfFile(
          m_hMapFile,                // Handle to mapping object
          FILE_MAP_ALL_ACCESS,     // Read/Write access
          0, 0, TOTAL_SHARED_SIZE
        );
        if (m_pHeader == NULL) {
          Utils::LogDebug("SERVER: Error MapViewOfFile (" + std::to_string(GetLastError()) + ")");
          CloseHandle(m_hMapFile);
          return false;
        }

        // Initialize SharedData header
        m_pHeader->writeIdx = 0;
        m_pHeader->readIdx = c_bufferPool - 1;
        //m_pHeader->fileSize = 0;
        m_pHeader->transferFinished = false;
        m_pHeader->isReadingInput = true;

        // Set pointer to the start of the data area
        m_pBufferData = (BYTE*)(m_pHeader + 1);

        memset(m_pHeader->dataSize, 0, sizeof(m_pHeader->dataSize));

        // 3. Create Mutex (p_isServer = true)
        m_IndexMutex = std::make_shared<NamedWinMutex>(true);

        // 4. Events
        m_hWroteEvent = CreateEventW(NULL, FALSE, FALSE, SERVER_WROTE_EVENT_NAME.c_str());
        m_hReadEvent = CreateEventW(NULL, FALSE, TRUE, CLIENT_READ_EVENT_NAME.c_str());
        // Initially ClientReadEvent = Signaled (Buffer is free)
        if (m_hWroteEvent == NULL || m_hReadEvent == NULL) {
          Utils::LogDebug("SERVER: Error creating events (" + std::to_string(GetLastError()) + ")");
          return false;
        }

        return true;
      }


      bool openSharedMemory() {
        // 1. Open shared memory
        m_hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME.c_str());
        if (m_hMapFile == NULL) {
          Utils::LogDebug("CLIENT: Error OpenFileMapping (" + std::to_string(GetLastError()) + "). Server is not running.");
          return false;
        }

        // 2. Map shared memory into process address space
        m_pHeader = (SharedData<c_bufferPool>*)MapViewOfFile(
          m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, TOTAL_SHARED_SIZE
        );
        if (m_pHeader == NULL) {
          Utils::LogDebug("CLIENT: Error MapViewOfFile (" + std::to_string(GetLastError()) + ")");
          CloseHandle(m_hMapFile);
          return false;
        }

        // Set pointer to the start of the data area
        m_pBufferData = (BYTE*)(m_pHeader + 1);

        // 3. Open mutex (p_isServer = false)
        m_IndexMutex = std::make_shared<NamedWinMutex>(false);

        // 4. Open events
        m_hWroteEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVER_WROTE_EVENT_NAME.c_str());
        m_hReadEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, CLIENT_READ_EVENT_NAME.c_str());
        if (m_hWroteEvent == NULL || m_hReadEvent == NULL) {
          Utils::LogDebug("CLIENT: Error opening events (" + std::to_string(GetLastError()) + ")");
          return false;
        }

        m_pHeader->transferFinished = false;

        return true;
      }

      void wait() override {
        if (!m_pHeader) return;

        // wait for data be consumed
        while (!m_pHeader->transferFinished) {
          // or std::this_thread::sleep_for
          Sleep(1);
        }
      }
    };


  }
}