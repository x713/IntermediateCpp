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

namespace lab {
	namespace data {

		using lab::data::NamedWinMutex;
		using lab::util::Utils;

		// Windows inter-process copy processor
		// TODO : TEST: PerfTest different buffer sizes
		template<size_t PoolSize>
		class WICopyProcessor : public IProcessor {
			// PoolSize
			// Tripple buffer at least
			static constexpr size_t c_minPoolSize = 3;
			static constexpr size_t c_bufferPool = PoolSize < c_minPoolSize ? c_minPoolSize : PoolSize;

			struct SharedData {
				size_t writeIdx = 0;
				size_t readIdx = c_bufferPool - 1;
				bool isReadingInput = false;
			};

			SharedData m_sharedData;

			// Buffer Pool [3]
			using RingArray = std::array<LineBuffer, c_bufferPool>;
			RingArray m_pool{};

			// Read / Write indexes
			// Sync mutex
			NamedWinMutex m_IndexSemaphore;

		public:

			WICopyProcessor() {
				const std::lock_guard<NamedWinMutex> lock(m_IndexSemaphore);

				// TODO : request shared memory here ?
				m_sharedData.writeIdx = 0;
				m_sharedData.readIdx = c_bufferPool - 1;
				m_sharedData.isReadingInput = true;
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

				const auto& m_readIdx = m_sharedData.readIdx;
				const auto& m_writeIdx = m_sharedData.writeIdx;

				{ // CRITICAL SECTION 
					const std::lock_guard<NamedWinMutex> lock(m_IndexSemaphore);

					nextWriteIdx = nextIdx(m_writeIdx);
					if (nextWriteIdx == m_readIdx) {
						// Buffer is still full
						return IOStatus::IORINGFULL; // Status::FullBuffer
					}

				} // END CRITICAL SECTION

				// write data from stream to pool
				auto result = m_pool[m_writeIdx] << p_dataSource;

				const std::lock_guard<NamedWinMutex> lock(m_IndexSemaphore);
				m_sharedData.writeIdx = nextWriteIdx;

				if (IOStatus::IOEOF == result) {
					m_sharedData.isReadingInput = false;
				}

				return result;
			}

			IOStatus operator>> (IDataSink* p_dataSink) {
				if (!p_dataSink) {
					return IOStatus::IOFAILPTR;
				}

				const auto& readIdx = m_sharedData.readIdx;
				const auto& writeIdx = m_sharedData.writeIdx;
				const auto& isReadingInput = m_sharedData.isReadingInput;

				const size_t nextReadIdx = nextIdx(readIdx);

				{ // CRITICAL SECTION 
				// try to move 'read window'
					const std::lock_guard<NamedWinMutex> lock(m_IndexSemaphore);
					if (writeIdx == nextReadIdx) {
						if (isReadingInput) {
							// nextBuffer is still writing
							return IOStatus::IONEXTBUSY;
						}
						else {
							// reached end of the data
							return IOStatus::IOEOF;
						}
					}

					// continue consuming buffer
					// update read index
					m_sharedData.readIdx = nextReadIdx;
				}

				// write data from pool to stream
				m_pool[readIdx] >> p_dataSink;

				return IOStatus::IOOK;
			}

			void close() override {
				const std::lock_guard<NamedWinMutex> lock(m_IndexSemaphore);
				m_sharedData.isReadingInput = false;
			};

			const bool isDone() override {
				const std::lock_guard<NamedWinMutex> lock(m_IndexSemaphore);
				return m_sharedData.isReadingInput;
			};

		};


	}
}