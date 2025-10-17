#pragma once


#include <vector>
#include <array>

#include <mutex>

#include <string>
#include <iostream> 

#include "IBuffer.h"
#include "LineBuffer.h"
#include "../../util/Utils.h"

namespace lab {
  namespace data {

    using lab::util::Utils;


    // TODO : TEST: PerfTest different buffer sizes
    template<size_t PoolSize>
    class RingBuffer : public IBuffer {

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
      std::mutex m_IndexSemaphore{};

      // Transmission signaling
      bool m_isReadingInput = false;

    public:

      RingBuffer() {
        const std::lock_guard<std::mutex> lock(m_IndexSemaphore);
#ifdef TB_USE_VECTOR
        for (size_t i = 0; i < c_bufferPool; ++i) {
          m_pool.push_back({});
        }
#endif
        // ifdef array : no need to init, should be allocated already

        size_t m_writeIdx = 0;
        size_t m_readIdx = c_bufferPool - 1;
        m_isReadingInput = true;
      }

      size_t nextIdx(const size_t p_idx) {
        size_t result = p_idx + 1;
        if (result >= c_bufferPool) {
          result = 0;
        }
        return result;
      }


      IOStatus readLine(std::ifstream& p_istream) {
        size_t nextWriteIdx;

        { // CRITICAL SECTION 
          const std::lock_guard<std::mutex> lock(m_IndexSemaphore);

          nextWriteIdx = nextIdx(m_writeIdx);
          if (nextWriteIdx == m_readIdx) {
            // Buffer is still full
            return IOStatus::IORINGFULL; // Status::FullBuffer
          }

        } // END CRITICAL SECTION

        // write data from stream to pool
        auto result = m_pool[m_writeIdx] << p_istream;

        const std::lock_guard<std::mutex> lock(m_IndexSemaphore);
        m_writeIdx = nextWriteIdx;

        if(IOStatus::IOEOF == result){
          m_isReadingInput = false;
        }

        return result; 
      }

      IOStatus writeLine(std::ofstream& p_ostream) {
        const size_t nextReadIdx = nextIdx(m_readIdx);

        { // CRITICAL SECTION 
          // try to move 'read window'
          const std::lock_guard<std::mutex> lock(m_IndexSemaphore);
          if (m_writeIdx == nextReadIdx) {
            if (m_isReadingInput) {
              // nextBuffer is still writing
              return IOStatus::IONEXTBUSY;
            }else{
              // reached end of the data
              return IOStatus::IOEOF;
            }
          }

          // continue consuming buffer
          m_readIdx = nextReadIdx;
        }

        // write data from pool to stream
        m_pool[m_readIdx] >> p_ostream;

        return IOStatus::IOOK;
      }

    };

    class RingBufferFactory {
    public:
      static std::shared_ptr<RingBuffer<3>> createSyncTrippleBuffer() {
        auto buffer = std::make_shared<RingBuffer<3>>();
        return buffer;
      }
    };
  }
}