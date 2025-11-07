#pragma once


//
// 
//
#include <iostream>
#include <iosfwd>
#include <string>
#include <memory>

#include "../../util/Utils.h"
#include "../buffer/IProcessor.h"

namespace lab {
  namespace data {

    using lab::util::Utils;
    /*
      enum class IOStatus {
        IOOK = 0,
        IOEOF,
        IOFAIL,
        IORINGFULL, // WRITE BUFF FULL
        IONEXTBUSY, // READ BUFF NOT RDY
      };

      class IData {};
      class IDataSource : public IData {};
      class IDataSink : public IData {};

      // IProcessor
      class IProcessor {
      public:

        // writeLine
        virtual IOStatus put(IDataSink p_data) abstract;
        virtual IOStatus get(IDataSource p_data) = 0;
        virtual void close() = 0;
      };

  */

    class FileDataSink : public IDataSink {
      std::string m_inFilename{};
      std::shared_ptr<IProcessor> m_buffer = nullptr;
      std::shared_ptr<std::ofstream> m_ofstr = nullptr;
      bool m_failed = false;

    public:
      FileDataSink(std::string p_outFilename)
        : m_inFilename(p_outFilename)
      {

      }


      IOStatus open() {
        if (m_ofstr) {
          if (m_ofstr->is_open()) {
            m_ofstr->close();
          }
          m_ofstr = nullptr;
        }
        // open file for reading
        m_ofstr = std::make_shared<std::ofstream>(m_inFilename, std::ios::binary);
        if (!m_ofstr->is_open()) {
          Utils::Log("Failed to open dest file");

          return IOStatus::IOFAILOPEN;
        }

        return IOStatus::IOOK;
      }

      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        if (!m_ofstr) {
          return IOStatus::IOFAILPTR;
        }

        m_ofstr->write(p_buf, p_count);

        if(m_ofstr->fail()){
          return IOStatus::IOFAIL;
        }

        return IOStatus::IOOK;
      }

      bool fail() override {
        return m_failed;
      };

    };

    class FileDataSource : public IDataSource {
      std::string m_inFilename{};
      std::shared_ptr<IProcessor> m_buffer = nullptr;
      std::shared_ptr <std::ifstream> m_istrm = nullptr;
      bool m_failed = false;
      bool m_eof = true;

    public:
      FileDataSource(std::string p_inFilename)
        : m_inFilename(p_inFilename)
      {

      }


      IOStatus open() {

        if (m_istrm) {
          if (m_istrm->is_open()) {
            m_istrm->close();
          }
        }
        // open file for reading

        m_istrm = std::make_shared<std::ifstream>(m_inFilename, std::ios::binary);
        if (!m_istrm->is_open()) {
          Utils::Log("failed to open " + m_inFilename);
          m_buffer->close();
          m_istrm->close();
          return IOStatus::IOFAILOPEN;
        }

        m_eof = m_istrm->eof();

        return IOStatus::IOOK;
      }

      IOStatus read(char* p_buf, size_t p_count) override {
        if (!m_istrm) {
          return IOStatus::IOFAILPTR;
        }
        if (m_istrm->eof())
        {
          m_eof = true;
          return IOStatus::IOEOF;
        }

        m_istrm->read(p_buf, p_count);
        m_eof = m_istrm->eof();

        return IOStatus::IOOK;
      }

      std::streamsize getCount() override {
        return m_istrm->gcount();
      }

      bool fail() override {
        return m_failed;
      };

      bool eof() override {
        return m_eof;
      };
    };


  }
}