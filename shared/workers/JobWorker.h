#pragma once

#include <fstream>
#include <string>
#include <iostream> 
#include <array> 


#include "../cli_processor/ComTypes.h"

#include "../buffer/IBuffer.h"
#include "../../util/Utils.h"


namespace lab {
  namespace worker {

    using lab::data::IBuffer;
    using lab::data::IOStatus;

    using lab::util::Utils;

    using std::ofstream;

    class FileJob {

    private:
      FileJob() = delete;

    protected:
      std::string m_inFilename;
      std::shared_ptr<IBuffer> m_buffer;

    public:
      FileJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer) {
        Utils::LogDebug(" FileJob ctor");

        m_inFilename = p_filename;
        m_buffer = ipc_buffer;
      }

      virtual ~FileJob() = default;

      virtual void process() {
        Utils::LogDebug("FileJob processed");
      };

    };



    void makeTestData() {
      std::string filename = "Test.b";

      // prepare a file to read
      double d = 3.14;
      std::ofstream(filename, std::ios::binary)
        .write(reinterpret_cast<char*>(&d), sizeof d)
        << "opening string"
        << 1123 << "abc"
        << 2123 << "abc"
        << 3123 << "abc"
        << 4123 << "abc"
        << 5123 << "abc"
        << 6123 << "abc"
        << 7123 << "abc"
        << 8123 << "abc"
        << 9123 << "abc"
        << 0123 << "abc"
        << "Long 1long 2long 3long 4long closing string";

    }

    class ReaderJob : public FileJob {

    public:

      ReaderJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        Utils::LogDebug(" ReaderJob ctor");
      }

      void process() override {

        // open file for reading
        std::ifstream istrm(m_inFilename, std::ios::binary);
        if (!istrm.is_open()) {
          Utils::Log("failed to open " + m_inFilename);
          m_buffer->close();
          istrm.close();
          return;
        }
        consume(istrm);
        istrm.close();

        Utils::Log(" ReaderJob processed");
      }


      void consume(std::ifstream& p_ifstream) {
        bool done = false;
        while (!done) {
          auto result = m_buffer->readLine(p_ifstream);

          if (IOStatus::IORINGFULL == result) {
            // TODO : sync spin lock with cv
            continue;
          }

          if (IOStatus::IOEOF == result) {

            done = true;
            break;
          }
          else if (p_ifstream.fail()) {
            // read less than 256, but not EOF

            //done = true;
            //break;
          }
        }

      }
    };



    class WriterJob : public FileJob {
    public:
      WriterJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        Utils::LogDebug(" WriterJob ctor");
      }

      void process() override {

        std::ofstream ofstr(m_inFilename, std::ios::binary);
        if (!ofstr.is_open()) {
          Utils::Log("Failed to open dest file");

          return;
        }

        produce(ofstr);
        ofstr.close();

        Utils::Log(" WriterJob processed");

      }

      void produce(std::ofstream& p_ofstream) {
        bool done = false;
        while (!done) {
          IOStatus result = m_buffer->writeLine(p_ofstream);

          if (IOStatus::IONEXTBUSY == result) {
            // TODO : wait?
            continue;
          }

          if (IOStatus::IOEOF == result) {
            done = true;
            break;
          }
        }

      }

    };


    class IPCReaderJob : public FileJob {
    public:
      IPCReaderJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        Utils::LogDebug(" IPCReader ctor");
      }

      void process() override {
        Utils::LogDebug(" IPCReader process ");
      }
    };

    class IPCWriterJob : public FileJob {
    public:
      IPCWriterJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        Utils::LogDebug(" IPCWriterJob ctor");
      }

      void process() override {
        Utils::LogDebug(" IPCWriterJob process ");
      }

    };

  }
}
