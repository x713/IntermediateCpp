#pragma once

#include <array>
#include <fstream>

#include "../../util/Utils.h"
#include "IBuffer.h"

namespace lab {
  namespace data {

    using lab::util::Utils;
    using lab::data::IOStatus;

    class LineBuffer {


      static constexpr std::streamsize c_bufferSize = 4;//256;

      std::array<char, c_bufferSize> m_buffer{};

      std::streamsize cacheBufferSize = 0;

    public:
      IOStatus operator<<(std::ifstream& p_ifstream) {

        p_ifstream.read(m_buffer.data(), c_bufferSize);

        // actual read
        std::streamsize actual_read_count = p_ifstream.gcount();
        cacheBufferSize = actual_read_count;

        PrintBuffer();

        if (p_ifstream.eof()) {
          Utils::LogDebug("EOF. Read bytes:", ' ');
          Utils::LogDebug(actual_read_count);

          return IOStatus::IOEOF;
        }
        else if (p_ifstream.fail()) {

          // read less than 256, but not EOF
          Utils::LogDebug("FAIL : Read less than 256, but not EOF", ' ');
          Utils::LogDebug(actual_read_count);

          return IOStatus::IOFAIL;
        }

        return IOStatus::IOOK;
      }

      void operator>>(std::ofstream& p_ostream) {
        if (cacheBufferSize) {
          p_ostream.write(m_buffer.data(), cacheBufferSize);
        }
        else {
          Utils::LogDebug("WARN : Write buffer 0 zise, skip writing");
        }

        PrintBuffer();
      }

      void reset() {
        cacheBufferSize = 0;
      }

      void PrintBuffer() {
#ifdef _DEBUG
        for (const auto& el : m_buffer) {
          Utils::Log(el, ' ');
        }
        Utils::Log("");
#endif // DEBUG
      }
    };
  }
}