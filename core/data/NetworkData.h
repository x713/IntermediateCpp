#pragma once

// Httplib
//A C++11 single - file header - only cross platform HTTP / HTTPS library.
//This library uses 'blocking' socket I / O.If you are looking for a library with 'non-blocking' socket I / O, this is not the one that you want.

//


//
#include <iostream>
#include <iosfwd>
#include <string>
#include <memory>

#include "../../util/Utils.h"
#include "../proc/IProcessor.h"
#include "../data/FileDataIO.h"

namespace lab {
  namespace data {

    using lab::util::Utils;
    using lab::data::FileDataSink;
    //using lab::processing::IProcessor;


    class NetworkDataSink : public IDataSink {
      std::string m_fileName{};
      std::shared_ptr<IProcessor> m_buffer = nullptr;
      bool m_failed = false;

    public:
      NetworkDataSink(std::string p_outHostname,
        std::string p_fileName,
        std::string p_clientName)
        : m_fileName(p_fileName)
      {

      }


      IOStatus open() {
        FileDataSink fs(m_fileName);

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

        m_ofstr->write(p_buf, static_cast<std::streamsize>(p_count));

        if (m_ofstr->fail()) {
          return IOStatus::IOFAIL;
        }

        return IOStatus::IOOK;
      }

      bool fail() override {
        return m_failed;
      };

    };

    class NetworkDataSource : public IDataSource {

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



    /*
    int main() {
    // 1. Создаем клиент (укажите адрес вашего сервера)
    httplib::Client cli("http://localhost:8080");

    // 2. Считываем содержимое файла (или передаем поток)
    // В данном примере мы просто указываем данные файла вручную для наглядности,
    // но ниже я покажу, как удобно загружать их с диска.
    
    std::string file_content = "Это содержимое моего файла";

    httplib::MultipartFormDataItems items = {
        {
            "file",           // Имя поля (как в HTML форме <input name="file">)
            file_content,     // Контент файла
            "test.txt",       // Имя файла, которое увидит сервер
            "text/plain"      // MIME-тип
        },
        { "user_id", "123", "", "" } // Можно добавить и обычные текстовые поля
    };

    // 3. Отправляем POST запрос
    if (auto res = cli.Post("/upload", items)) {
        if (res->status == 200) {
            std::cout << "Файл успешно отправлен!" << std::endl;
            std::cout << "Ответ сервера: " << res->body << std::endl;
        } else {
            std::cerr << "Ошибка сервера: " << res->status << std::endl;
        }
    } else {
        auto err = res.error();
        std::cerr << "Ошибка подключения: " << httplib::to_string(err) << std::endl;
    }

    return 0;
} 
    */

  }
}