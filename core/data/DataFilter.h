#pragma once

#include "IDataIO.h"
#include <memory>

namespace lab {
  namespace data {

    // Base filter for writing
    class DataSinkFilter : public IDataSink {
    protected:
      std::shared_ptr<IDataSink> m_next;
      bool m_failed = false;

    public:
      DataSinkFilter(std::shared_ptr<IDataSink> p_next) : m_next(p_next) {}

      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        if (!m_next) return IOStatus::NullPointer;
        return m_next->write(p_buf, p_count);
      }

      virtual bool fail() override {
        return m_failed || (m_next && m_next->fail());
      }
    };

    // Base filter for reading
    class DataSourceFilter : public IDataSource {
    protected:
      std::shared_ptr<IDataSource> m_next;
      bool m_failed = false;

    public:
      DataSourceFilter(std::shared_ptr<IDataSource> p_next) : m_next(p_next) {}

      virtual IOStatus read(char* p_buf, size_t p_count) override {
        if (!m_next) return IOStatus::NullPointer;
        return m_next->read(p_buf, p_count);
      }

      virtual std::streamsize getCount() override {
        return m_next ? m_next->getCount() : 0;
      }

      virtual bool fail() override {
        return m_failed || (m_next && m_next->fail());
      }

      virtual bool eof() override {
        return m_next ? m_next->eof() : true;
      }
    };

  }
}
