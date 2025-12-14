#pragma once


#include <vector>
#include <array>

#include <mutex>

#include <string>
#include <iostream> 

#include "../IProcessor.h"
#include "../LineBuffer.h"

#include "Wrapper.h"


#include "../../../core/ipc/ipc_mutex.h"
#include "../../../util/Utils.h"

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
	namespace processing {

		using lab::data::NamedWinMutex;
		using lab::util::Utils;

		using lab::util::WinHandle;

		// IPC names  
		const std::wstring& SHARED_MEM_NAME = L"Local\\SnTransferMemory";

		// Events names
		const std::wstring& SERVER_WROTE_EVENT_NAME = L"Local\\SnTransferServerWroteEvent";
		const std::wstring& CLIENT_READ_EVENT_NAME = L"Local\\SnTransferClientReadEvent";

		// Buffer size
		constexpr size_t c_bufferSize = 64 * 1024;
		template<size_t BufferPool>
		struct SharedData {
			size_t dataSize[BufferPool] = { 0 };
			// TODO : share size for monitoring copy progress?
			// long long fileSize = 0;        
			size_t writeIdx = 0;
			size_t readIdx = BufferPool - 1;

			bool isReadingInput = false;
			bool transferFinished = false;
		};

		// Windows inter-process copy processor
		template<size_t PoolSize>
		class WinIPCProcessor : public IProcessor {
		public:
			// PoolSize
			// Tripple buffer at least
			static constexpr size_t c_minPoolSize = 3;
			static constexpr size_t c_bufferPool = PoolSize < c_minPoolSize ? c_minPoolSize : PoolSize;



			// Total shared memory size: Header + Buffer pool 
			// Each buffer is additionally storing number of bytes consumed
			// so for each buufer in pool [buff_size = buf_size + 1*size_t]      
			static constexpr size_t TOTAL_SHARED_SIZE =
				sizeof(SharedData<c_bufferPool>) + ((sizeof(size_t) + c_bufferSize) * c_bufferPool);

		private:
			// Read / Write indexes stored in SharedData structure
			SharedData<c_bufferPool>* m_pHeader = nullptr;

			// Pointer to the start of the tripple buffer area
			BYTE* m_pBufferData = nullptr;

			// Are we sending bytes or receiving
			bool m_isServer = true;

			// Windows OS SharedMemory Handler
			//HANDLE m_hMapFile = NULL;
			std::shared_ptr<WinHandle> m_hMapFile = nullptr;

			// Sync mutex
			std::shared_ptr<NamedWinMutex> m_IndexMutex = nullptr;

			// Events for tripple - buffer signalling (unused)
			// TODO : (tweaks)
			std::shared_ptr<WinHandle> m_hWroteEvent = nullptr;
			std::shared_ptr<WinHandle> m_hReadEvent = nullptr;

		public:

			WinIPCProcessor(bool p_isServer = true) :
				m_isServer(p_isServer) {

				bool result = false;

				try {
					if (m_isServer) {
						result = createSharedMemory();
					}
					else {
						result = openSharedMemory();
					}
				}
				catch (const std::exception& /*e*/) {
					// Optional: Log the exception message (e.what())
					// Utils::Log("Exception: ", e.what());
					Utils::Log("Uninvited V.");
				}
				if (!result) {
					Utils::Log("WinIPCProcessor : Failed to initialize .");
				}
			}

			size_t nextIdx(const size_t p_idx) {
				size_t result = p_idx + 1;
				if (result >= c_bufferPool) {
					result = 0;
				}
				return result;
			}


			// read data from source and save into tripple buffer
			IOStatus operator<< (std::shared_ptr<IDataSource> p_dataSource) {
				if (!p_dataSource) {
					return IOStatus::NullPointer;
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
						return IOStatus::RingBufferFull; // Status::FullBuffer
					}

				} // END CRITICAL SECTION

				// write data from stream to shared memory
				auto result = write_to_shared(p_dataSource);

				const std::lock_guard<NamedWinMutex> lock(*m_IndexMutex);
				m_pHeader->writeIdx = nextWriteIdx;

				if (IOStatus::EndOfFile == result || IOStatus::Failed == result) {
					m_pHeader->isReadingInput = false;
				}

				return result;
			}


			// write data from data source to shared tripple buffer
			IOStatus write_to_shared(std::shared_ptr<IDataSource> p_dataSource) {
				// Set pointer to the start of the data area
				auto p_bufferStart = (m_pBufferData + m_pHeader->writeIdx * (c_bufferSize + sizeof(size_t)));
				auto p_bufferSize = (p_bufferStart + c_bufferSize);

				//p_dataSource->read(p_bufferStart, p_bufferSize);
				p_dataSource->read(reinterpret_cast<char*>(p_bufferStart), c_bufferSize);


				// actual read
				std::streamsize actual_read_count = p_dataSource->getCount();
				m_pHeader->dataSize[m_pHeader->writeIdx] = actual_read_count;

				if (p_dataSource->eof()) {
					Utils::LogDebug("EOF. Read bytes:", ' ');
					Utils::LogDebug(actual_read_count);

					return IOStatus::EndOfFile;
				}
				else if (p_dataSource->fail()) {
					// read less than p_bufferSize, but not EOF
					Utils::LogDebug("FAIL : Read less than " + std::to_string(c_bufferSize) + " bytes, but not EOF", ' ');
					Utils::LogDebug(actual_read_count);

					return IOStatus::Failed;
				}

				//PrintBuffer();

				return IOStatus::Ok;
			}

			// read data from tripple buffer and send to data sink
			IOStatus operator>> (std::shared_ptr<IDataSink> p_dataSink) {
				if (!p_dataSink) {
					return IOStatus::NullPointer;
				}
				if (!m_pHeader) {
					return IOStatus::MemoryAllocationFailed;
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
							return IOStatus::NextBufferBusy;
						}
						else {
							// reached end of the data
							m_pHeader->transferFinished = true;
							return IOStatus::EndOfFile;
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

			// read data from shared memory
			IOStatus read_from_shared(std::shared_ptr<IDataSink> p_dataSink) {
				if (!p_dataSink) {
					return IOStatus::NullPointer;
				}

				// read data from shared memory to data sink
				const auto& cacheBufferSize = m_pHeader->dataSize[m_pHeader->readIdx];
				const auto& p_bufferStart = (m_pBufferData + m_pHeader->readIdx * (c_bufferSize + sizeof(size_t)));

				if (cacheBufferSize) {
					//p_dataSink->write(p_bufferStart, cacheBufferSize);
					p_dataSink->write(reinterpret_cast<const char*>(p_bufferStart), cacheBufferSize);
				}
				else {
					Utils::LogDebug("WARN : Write buffer 0 zise, skip writing");
				}

				return IOStatus::Ok;
			}

			// close data streams and set transmission flags 
			void close() override {
				const std::lock_guard<NamedWinMutex> lock(*m_IndexMutex);
				m_pHeader->isReadingInput = false;
				m_pHeader->transferFinished = true;
			};




			// create Windows OS shared file mem segmets (server side)
			bool createSharedMemory() {
				// 1. Create shared memory
				m_hMapFile = std::make_shared<WinHandle>(CreateFileMappingW(
					INVALID_HANDLE_VALUE,    // Use paging file
					NULL,                    // Default security attributes
					PAGE_READWRITE,          // Read/Write access
					(DWORD)(TOTAL_SHARED_SIZE >> 32), // High-order DWORD
					(DWORD)TOTAL_SHARED_SIZE,        // Low-order DWORD
					SHARED_MEM_NAME.c_str()          // Name of the object
				)
				);
				if (!m_hMapFile) {
					Utils::LogDebug("SERVER: Error CreateFileMapping (" + std::to_string(GetLastError()) + ")");
					return false;
				}

				// 2. Map shared memory into process address space
				m_pHeader = (SharedData<c_bufferPool>*)MapViewOfFile(
					(*m_hMapFile),                // Handle to mapping object
					FILE_MAP_ALL_ACCESS,     // Read/Write access
					0, 0, TOTAL_SHARED_SIZE
				);

				if (m_pHeader == NULL) {
					Utils::LogDebug("SERVER: Error MapViewOfFile (" + std::to_string(GetLastError()) + ")");
					m_hMapFile.reset();

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
				m_hWroteEvent = std::make_shared<WinHandle>(CreateEventW(NULL, FALSE, FALSE, SERVER_WROTE_EVENT_NAME.c_str()));
				m_hReadEvent = std::make_shared<WinHandle>(CreateEventW(NULL, FALSE, TRUE, CLIENT_READ_EVENT_NAME.c_str()));
				// Initially ClientReadEvent = Signaled (Buffer is free)
				if (!m_hWroteEvent || !m_hReadEvent) {
					Utils::LogDebug("SERVER: Error creating events (" + std::to_string(GetLastError()) + ")");
					return false;
				}

				return true;
			}

			// opens Windows OS shared file mem segmets (Client side)
			bool openSharedMemory() {
				// 1. Open shared memory
				m_hMapFile = std::make_shared<WinHandle>(OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME.c_str()));
				if (!m_hMapFile) {
					Utils::LogDebug("CLIENT: Error OpenFileMapping (" + std::to_string(GetLastError()) + "). Server is not running.");
					return false;
				}

				// 2. Map shared memory into process address space
				m_pHeader = (SharedData<c_bufferPool>*)MapViewOfFile(
					*m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, TOTAL_SHARED_SIZE
				);
				if (m_pHeader == NULL) {
					Utils::LogDebug("CLIENT: Error MapViewOfFile (" + std::to_string(GetLastError()) + ")");
					m_hMapFile.reset();
					return false;
				}

				// Set pointer to the start of the data area
				m_pBufferData = (BYTE*)(m_pHeader + 1);

				// 3. Open mutex (p_isServer = false)
				m_IndexMutex = std::make_shared<NamedWinMutex>(false);

				// 4. Open events
				m_hWroteEvent = std::make_shared<WinHandle>(OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVER_WROTE_EVENT_NAME.c_str()));
				m_hReadEvent = std::make_shared<WinHandle>(OpenEvent(EVENT_ALL_ACCESS, FALSE, CLIENT_READ_EVENT_NAME.c_str()));
				if (!m_hWroteEvent || !m_hReadEvent) {
					Utils::LogDebug("CLIENT: Error opening events (" + std::to_string(GetLastError()) + ")");
					return false;
				}

				m_pHeader->transferFinished = false;

				return true;
			}

			// Wait until data consumed by receiver
			void wait() override {
				if (!m_isServer) return;
				if (!m_pHeader) return;

				// wait for data be consumed
				while (!m_pHeader->transferFinished) {
					// or std::this_thread::sleep_for
					// or cumullative deadline
					Sleep(1);
				}
			}

			~WinIPCProcessor() {
				if (m_pHeader) {
					UnmapViewOfFile(m_pHeader);
				}

			}
		};


	}
}