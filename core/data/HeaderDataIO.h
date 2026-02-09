#pragma once

#include "DataFilter.h"
#include <cstring>
#include <cstdint>

namespace lab {
  namespace data {

    const uint32_t MAGIC = 0x53454E52; // "SENR"
    const uint16_t VERSION = 1;

    struct PacketHeader {
      uint32_t magic = MAGIC;
      uint16_t version = VERSION;
      uint16_t commandId = 0;
      char clientName[32] = { 0 };
      char fileName[256] = { 0 };
      uint64_t payloadSize = 0;
    };

    class HeaderDataSink : public DataSinkFilter {
      PacketHeader m_header;
      bool m_headerWritten = false;

    public:
      HeaderDataSink(std::shared_ptr<IDataSink> p_next, uint16_t p_cmd, const std::string& p_client, const std::string& p_file)
        : DataSinkFilter(p_next) 
      {
        m_header.commandId = p_cmd;
        strncpy_s(m_header.clientName, p_client.c_str(), sizeof(m_header.clientName) - 1);
        strncpy_s(m_header.fileName, p_file.c_str(), sizeof(m_header.fileName) - 1);
      }

      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        if (!m_headerWritten) {
          // write header
          IOStatus status = m_next->write(reinterpret_cast<const char*>(&m_header), sizeof(m_header));
          if (status != IOStatus::Ok) {
            return status;
          }
          m_headerWritten = true;
        }
        return m_next->write(p_buf, p_count);
      }
    };

    class HeaderDataSource : public DataSourceFilter {
      PacketHeader m_header;
      bool m_headerRead = false;

    public:
      HeaderDataSource(std::shared_ptr<IDataSource> p_next) : DataSourceFilter(p_next) {}

      virtual IOStatus read(char* p_buf, size_t p_count) override {
        if (!m_headerRead) {
          // read header
          IOStatus status = m_next->read(reinterpret_cast<char*>(&m_header), sizeof(m_header));
          if (status != IOStatus::Ok) {
            return status;
          }
          if (m_header.magic != MAGIC) {
            return IOStatus::Failed; // protocol error
          }
          m_headerRead = true;
        }
        return m_next->read(p_buf, p_count);
      }

      const PacketHeader& getHeader() const { return m_header; }
    };

  }
}
