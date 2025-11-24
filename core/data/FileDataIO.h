#pragma once


//
// 
//
#include <iostream>
#include <iosfwd>
#include <string>
#include <memory>

#include "../../util/Utils.h"
#include "../proc/IProcessor.h"

namespace lab {
  namespace data {

    using lab::util::Utils;
    using lab::processing::IProcessor;


    class FileDataSink : public IDataSink {
      std::string m_outFilename{};
      std::shared_ptr<IProcessor> m_buffer = nullptr;
      std::shared_ptr<std::ofstream> m_ofstr = nullptr;
      bool m_failed = false;

    public:
      FileDataSink(std::string p_outFilename)
        : m_outFilename(p_outFilename)
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
        m_ofstr = std::make_shared<std::ofstream>(m_outFilename, std::ios::binary);
        if (!m_ofstr->is_open()) {
          Utils::Log("Failed to open dest file");

          return IOStatus::OpenFailed;
        }

        return IOStatus::Ok;
      }

      virtual IOStatus write(const char* p_buf, size_t p_count) override {
        if (!m_ofstr) {
          return IOStatus::NullPointer;
        }

        m_ofstr->write(p_buf, static_cast<std::streamsize>(p_count));

        if (m_ofstr->fail()) {
          return IOStatus::Failed;
        }

        return IOStatus::Ok;
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
          return IOStatus::OpenFailed;
        }

        m_eof = m_istrm->eof();

        return IOStatus::Ok;
      }

      IOStatus read(char* p_buf, size_t p_count) override {
        if (!m_istrm) {
          return IOStatus::NullPointer;
        }
        if (m_istrm->eof())
        {
          m_eof = true;
          return IOStatus::EndOfFile;
        }

        m_istrm->read(p_buf, p_count);
        m_eof = m_istrm->eof();

        return IOStatus::Ok;
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