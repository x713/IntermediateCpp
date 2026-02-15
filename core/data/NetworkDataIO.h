#pragma once

#include "IDataIO.h"
#include <boost/asio.hpp>
#include <memory>

namespace lab {
  namespace data {

    using boost::asio::ip::tcp;

    // Network receiver (sending to socket)
    class NetworkDataSink : public IDataSink {
      std::shared_ptr<tcp::socket> m_socket;
      bool m_failed = false;

    public:
      NetworkDataSink(std::shared_ptr<tcp::socket> p_socket) : m_socket(p_socket) {}

      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        if (!m_socket || !m_socket->is_open()) return IOStatus::NullPointer;

        boost::system::error_code ec;
        boost::asio::write(*m_socket, boost::asio::buffer(p_buf, p_count), ec);

        if (ec) {
          m_failed = true;
          return IOStatus::Failed;
        }
        return IOStatus::Ok;
      }

      virtual bool fail() override { return m_failed; }
    };

    // Network source (reading from socket)
    class NetworkDataSource : public IDataSource {
      std::shared_ptr<tcp::socket> m_socket;
      std::streamsize m_lastReadCount = 0;
      bool m_failed = false;
      bool m_eof = false;

    public:
      NetworkDataSource(std::shared_ptr<tcp::socket> p_socket) : m_socket(p_socket) {}

      virtual IOStatus read(char* p_buf, size_t p_count) override {
        if (!m_socket || !m_socket->is_open()) return IOStatus::NullPointer;
        if (m_eof) return IOStatus::EndOfFile;

        boost::system::error_code ec;
        size_t len = m_socket->read_some(boost::asio::buffer(p_buf, p_count), ec);
        
        m_lastReadCount = static_cast<std::streamsize>(len);

        if (ec == boost::asio::error::eof) {
          m_eof = true;
          return IOStatus::EndOfFile;
        } else if (ec) {
          m_failed = true;
          return IOStatus::Failed;
        }

        return IOStatus::Ok;
      }

      virtual std::streamsize getCount() override { return m_lastReadCount; }
      virtual bool fail() override { return m_failed; }
      virtual bool eof() override { return m_eof; }
    };

  }
}
