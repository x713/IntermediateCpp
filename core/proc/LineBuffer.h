#pragma once

#include <array>
#include <fstream>

#include "../../util/Utils.h"
#include "IProcessor.h"

namespace lab {
  namespace data {

    using lab::util::Utils;
    using lab::data::IOStatus;
    using lab::processing::IProcessor;

    class LineBuffer {


      static constexpr std::streamsize c_bufferSize = 16;//256;

      std::array<char, c_bufferSize> m_buffer{};

      std::streamsize cacheBufferSize = 0;

    public:
      IOStatus operator<<(IDataSource *p_dataSource) {
        if(!p_dataSource){
          return IOStatus::NullPointer;
        }
        p_dataSource->read(m_buffer.data(), c_bufferSize);

        // actual read
        std::streamsize actual_read_count = p_dataSource->getCount();
        cacheBufferSize = actual_read_count;

        if (p_dataSource->eof()) {
          Utils::LogDebug("EOF. Read bytes:", ' ');
          Utils::LogDebug(actual_read_count);

          return IOStatus::EndOfFile;
        }
        else if (p_dataSource->fail()) {

          // read less than 256, but not EOF
          Utils::LogDebug("FAIL : Read less than 256, but not EOF", ' ');
          Utils::LogDebug(actual_read_count);

          return IOStatus::Failed;
        }

        //PrintBuffer();

        return IOStatus::Ok;
      }

      void operator>>(IDataSink* p_dataSink) {
        if (!p_dataSink) {
          return;
        }
        if (cacheBufferSize) {
          p_dataSink->write(m_buffer.data(), cacheBufferSize);
        }
        else {
          Utils::LogDebug("WARN : Write buffer 0 zise, skip writing");
        }

        //PrintBuffer();
      }


      void reset() {
        cacheBufferSize = 0;
      }


      void PrintBuffer() {
#ifdef _DEBUG
        size_t bsize = static_cast<size_t>(cacheBufferSize) < m_buffer.size() ? static_cast<size_t>(cacheBufferSize) : m_buffer.size();
        bsize = c_bufferSize < bsize ? bsize : c_bufferSize;

        for (size_t i = 0; i < bsize - 1; ++i) {
          unsigned char ch = static_cast<unsigned char>(m_buffer[i]);

          if (ch && std::isprint(ch)) {
            Utils::Log(m_buffer[i], ' ');
          }
        }
        Utils::Log("");
#endif // DEBUG
      }
    };
  }
}