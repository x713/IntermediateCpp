#pragma once


#include <vector>
#include <array>

#include <mutex>
#include <condition_variable>

#include <string>
#include <iostream> 

#include "IProcessor.h"
#include "LineBuffer.h"


#include "../../core/ipc/ipc_mutex.h"
#include "../../util/Utils.h"

namespace lab {
  namespace processing {

    using lab::util::Utils;
    using lab::data::LineBuffer;

    // Threaded Data Processor with Ring Buffer Pool
    // TODO : TEST: PerfTest different buffer sizes
    template<size_t PoolSize, typename MutexType>
    class ThreadedDataProcessor : public IProcessor {

      // Tripple buffer at least
      static constexpr size_t c_minPoolSize = 3;
      // PoolSize
      static constexpr size_t c_bufferPool = PoolSize < c_minPoolSize ? c_minPoolSize : PoolSize;

      // Buffer Pool [3]
      // TODO : TEST : array vs vector
#define TB_USE_VECTOR
      using RingArray = std::array<LineBuffer, c_bufferPool>;
      using RingVector = std::vector<LineBuffer>;
#ifdef TB_USE_VECTOR
      RingVector m_pool{};
#else
      RingArray m_pool{};
#endif

      // Read / Write indexes
      size_t m_writeIdx = 0;
      size_t m_readIdx = c_bufferPool - 1;
      // Sync mutex
      MutexType m_IndexMutex;

      // Transmission signaling
      bool m_isReadingInput = false;
			// Bffer available condition variable
      std::condition_variable_any m_bufferAvailable;

    public:

      ThreadedDataProcessor() {
        const std::lock_guard<MutexType> lock(m_IndexMutex);
#ifdef TB_USE_VECTOR
        for (size_t i = 0; i < c_bufferPool; ++i) {
          m_pool.push_back({});
        }
#endif
        // ifdef array : no need to init, should be allocated already

        m_writeIdx = 0;
        m_readIdx = c_bufferPool - 1;
        m_isReadingInput = true;
      }

      size_t nextIdx(const size_t p_idx) noexcept {
        size_t result = p_idx + 1;
        if (result >= c_bufferPool) {
          result = 0;
        }
        return result;
      }



      IOStatus operator<< (std::shared_ptr<IDataSource> p_dataSource) {
        if (!p_dataSource) {
          return IOStatus::NullPointer;
        }
        size_t nextWriteIdx;

        { // CRITICAL SECTION 
          std::unique_lock<MutexType> lock(m_IndexMutex);

          nextWriteIdx = nextIdx(m_writeIdx);

          if (nextWriteIdx == m_readIdx) {
            // Buffer is still full
            // return IOStatus::RingBufferFull; // Status::FullBuffer
          
            // grab nextWriteIdx to update it in the loop
            m_bufferAvailable.wait(lock, [this, &nextWriteIdx] {
              nextWriteIdx = nextIdx(m_writeIdx);
              return nextWriteIdx != m_readIdx;
            });
          }

        } // END CRITICAL SECTION

        // write data from stream to pool
        auto result = m_pool[m_writeIdx] << p_dataSource;

       std::lock_guard<MutexType> lock(m_IndexMutex);
        m_writeIdx = nextWriteIdx;

        if (IOStatus::EndOfFile == result) {
          m_isReadingInput = false;
        }
        m_bufferAvailable.notify_one();
        return result;
      }

      IOStatus operator>> (std::shared_ptr<IDataSink> p_dataSink) {
        if (!p_dataSink) {
          return IOStatus::NullPointer;
        }

        const size_t nextReadIdx = nextIdx(m_readIdx);

        { // CRITICAL SECTION 
          // try to move 'read window'
          std::unique_lock<MutexType> lock(m_IndexMutex);
          if (m_writeIdx == nextReadIdx) {
            if (m_isReadingInput) {
              // nextBuffer is still writing
              //return IOStatus::NextBufferBusy;

              // grab nextReadIdx to update it in the loop
              m_bufferAvailable.wait(lock, [this, nextReadIdx] {
                //nextReadIdx = nextIdx(m_readIdx);
                return nextReadIdx != m_writeIdx;
              });

              if (m_writeIdx == nextReadIdx && !m_isReadingInput) {
                return IOStatus::EndOfFile;
              }
            }
            else {
              // reached end of the data
              return IOStatus::EndOfFile;
            }
          }

          // continue consuming buffer
          m_readIdx = nextReadIdx;
        }

        m_bufferAvailable.notify_one();

        // write data from pool to stream
        m_pool[m_readIdx] >> p_dataSink;

        return IOStatus::Ok;
      }

      void close() override {
        const std::lock_guard<MutexType> lock(m_IndexMutex);
        m_isReadingInput = false;
      };

      void wait() override {
        // pass
      };
    };


  }
}