#pragma once

#include "DataFilter.h"
#include <vector>

namespace lab {
  namespace data {

    const uint8_t MOCK_KEY = 0x42;
    // --- MOCK ENCRYPTION (XOR) ---
    class CryptoDataSink : public DataSinkFilter {
      uint8_t m_key;
    public:
      CryptoDataSink(std::shared_ptr<IDataSink> p_next, uint8_t p_key) 
        : DataSinkFilter(p_next), m_key(p_key) {}

      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        std::vector<char> encrypted(p_count);
        for (size_t i = 0; i < p_count; ++i) {
          encrypted[i] = p_buf[i] ^ m_key;
        }
        return m_next->write(encrypted.data(), p_count);
      }
    };

    class CryptoDataSource : public DataSourceFilter {
      uint8_t m_key;
    public:
      CryptoDataSource(std::shared_ptr<IDataSource> p_next, uint8_t p_key = MOCK_KEY) 
        : DataSourceFilter(p_next), m_key(p_key) {}

      virtual IOStatus read(char* p_buf, size_t p_count) override {
        IOStatus status = m_next->read(p_buf, p_count);
        if (status == IOStatus::Ok) {
          std::streamsize actual = m_next->getCount();
          for (std::streamsize i = 0; i < actual; ++i) {
            p_buf[i] ^= m_key;
          }
        }
        return status;
      }
    };

    // --- MOCK COMPRESSION (Just forward with log) ---
    class CompressDataSink : public DataSinkFilter {
    public:
      CompressDataSink(std::shared_ptr<IDataSink> p_next) : DataSinkFilter(p_next) {}
      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        // Here could be [Lempel-Ziv without dictionary]
        return m_next->write(p_buf, p_count);
      }
    };

    class DecompressDataSource : public DataSourceFilter {
    public:
      DecompressDataSource(std::shared_ptr<IDataSource> p_next) : DataSourceFilter(p_next) {}
      virtual IOStatus read(char* p_buf, size_t p_count) override {
        return m_next->read(p_buf, p_count);
      }
    };

  }
}
