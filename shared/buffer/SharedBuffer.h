#pragma once

#include <vector>
#include <array>

#include <mutex>
#include <fstream>
#include <string>
#include <iostream> 

//
//
//
namespace lab {
  namespace data {

    class IBuffer {
    public:
      virtual void writeLine(std::ofstream& p_ostream) abstract;
      virtual void readLine(std::ifstream& p_istream) = 0;
    };


    class LineBuffer{

      static constexpr std::streamsize c_bufferSize = 256;

      std::array<char, c_bufferSize> m_buffer{};

      std::streamsize cacheBufferSize = 0;
    public:
      void readLine(std::ifstream& p_istream)  {

        p_istream.read(m_buffer.data(), c_bufferSize);

        // actual read
        std::streamsize actual_read_count = p_istream.gcount();
        cacheBufferSize = actual_read_count;

        if (p_istream.eof()) {
          // std::cout << "EOF. Read bytes: " << actual_read_count << '\n';
        }
        else if (p_istream.fail()) {
          // read less than 256, but not EOF
          // std::cout << "Read failed, Read bytes: " << actual_read_count << '\n';
        }
      }

      virtual void writeLine(std::ofstream& p_ostream) {
        if (cacheBufferSize) {
          p_ostream.write(m_buffer.data(), cacheBufferSize);
        }
      }

      void reset() {
        cacheBufferSize = 0;
      }
    };


    class TrippleBuffer : public IBuffer {

      static constexpr size_t c_bufferPool = 3;


      using TrippleArray = std::array<LineBuffer, c_bufferPool>;
      using TrippleVector = std::vector<LineBuffer>;

      std::mutex m_IndexSemaphore{};



      bool m_writing = false;
      bool m_reading = false;

#ifdef TB_USE_VECTOR
      TrippleVector m_pool{};
#else
      TrippleArray m_pool{};

#endif

      size_t m_writeIdx = 0;
      size_t m_readIdx = c_bufferPool - 1;

    public:

      TrippleBuffer() {
        const std::lock_guard<std::mutex> lock(m_IndexSemaphore);
#ifdef TB_USE_VECTOR
        for (size_t i = 0; i < c_bufferPool; ++i) {
          m_pool.push_back({});
        }
#endif
        // array
        // no need to init
      }

      size_t nextIdx(const size_t p_idx) {
        size_t result = p_idx + 1;
        if (result >= c_bufferPool) {
          result = 0;
        }
        return result;
      }

      void readLine(std::ifstream& p_istream) {
        size_t nextWriteIdx;

        { // CRITICAL SECTION 
          const std::lock_guard<std::mutex> lock(m_IndexSemaphore);

          nextWriteIdx = nextIdx(m_writeIdx);
          if (nextWriteIdx == m_readIdx) {
            // Buffer is still full
            return;
          }

        }

        m_pool[m_writeIdx].readLine(p_istream);

        const std::lock_guard<std::mutex> lock(m_IndexSemaphore);
        m_writeIdx = nextWriteIdx;
      }

      void writeLine(std::ofstream& p_ostream) {
        size_t nextReadIdx = nextIdx(m_readIdx);
        { // CRITICAL SECTION 
          // try to move 'read window'
          const std::lock_guard<std::mutex> lock(m_IndexSemaphore);
          if (m_writeIdx == nextReadIdx) {
            // nextBuffer is still writing
            return;
          }
          m_readIdx = nextReadIdx;
        }

        m_pool[m_readIdx].writeLine(p_ostream);


      }
    };
  }
}