#pragma once

#include <fstream>
#include <string>
#include <iostream> 
#include <array> 


#include "../cli/ComTypes.h"

#include "../proc/IProcessor.h"
#include "../data/IDataIO.h"
#include "../data/FileDataIO.h"
#include "../../util/Utils.h"


namespace lab {
  namespace workers {

    using lab::data::IProcessor;
    using lab::data::IOStatus;

    using lab::data::FileDataSource;
    using lab::data::FileDataSink;

    using lab::util::Utils;

    using std::ofstream;

    class FileJob {

    private:
      FileJob() = delete;

    protected:
      std::string m_filename;
      std::shared_ptr<IProcessor> m_processor;

    public:
      FileJob(const std::string& p_filename,
        const std::shared_ptr<IProcessor>& p_itc_processor) {
        Utils::LogDebug(" FileJob ctor");

        m_filename = p_filename;
        m_processor = p_itc_processor;
      }

      virtual ~FileJob() = default;

      virtual void process() {
        Utils::LogDebug("FileJob processed");
      };

      const std::string& getFilename() const{
        return m_filename;
      }

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
        const std::shared_ptr<IProcessor>& p_itc_processor)
        : FileJob(p_filename, p_itc_processor) {
        Utils::LogDebug(" ReaderJob ctor");
      }

      void process() override {

        FileDataSource fsd(getFilename());
        if(fsd.open() != IOStatus::Ok ){
          return;
        }

        // open file for reading

        consume(&fsd);

        Utils::Log(" ReaderJob processed");
      }


      void consume(FileDataSource *p_dataSource) {
        bool done = false;
        while (!done) {

          auto result = *m_processor << p_dataSource;

          if (IOStatus::RingBufferFull == result) {
            // TODO : sync spin lock with cv
            continue;
          }

          if (IOStatus::EndOfFile == result) {

            done = true;
            break;
          }
          else if (p_dataSource->fail()) {
            // read less than buff size, but not EOF
            //done = true;
            //break;
          }
        }

        Utils::Log(" Server have read whole file awaiting for consumer");
        m_processor->wait();

      }
    };



    class WriterJob : public FileJob {
    public:
      WriterJob(const std::string& p_filename,
        const std::shared_ptr<IProcessor>& p_itc_processor)
        : FileJob(p_filename, p_itc_processor) {
        Utils::LogDebug(" WriterJob ctor");
      }
      
      void process() override {

        FileDataSink fsd(getFilename());
        if (fsd.open() != IOStatus::Ok) {
          return;
        }

        produce(&fsd);

        Utils::Log(" WriterJob processed");
      }

      void produce(FileDataSink* p_dataSink) {
        bool done = false;
        while (!done) {
          IOStatus result = *m_processor >> p_dataSink;

          if (IOStatus::NextBufferBusy == result) {
            // TODO : wait?
            continue;
          }

          if (IOStatus::EndOfFile == result) {
            done = true;
            break;
          }
        }

      }

    };

  }
}
