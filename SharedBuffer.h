#pragma once

#include <queue>

#include <mutex>


//
//
//
namespace lab {
  namespace data {
    using SQueue = std::queue<char>;

    class SharedBuffer {
      SQueue m_Buffer{};

      const int c_buffer_size = 256;

      bool m_writing = false;
      bool m_reading = false;

      std::mutex m_BufferMutex{};
      std::mutex m_SemaphoreMutex{};

    public:

      // Send char to buffer, skip if buffer locked or full. 
      void broadcastChar(const char p_char) {
        if (m_BufferMutex.try_lock())
        {
          // skip if full
          if (m_Buffer.size() < c_buffer_size) {
            m_Buffer.push(p_char);

          }
          m_BufferMutex.unlock();
        }
      }
    };
  }
}