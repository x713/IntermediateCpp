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

    using lab::processing::IProcessor;
    using lab::data::IDataSource;
    using lab::data::IDataSink;
    using lab::data::FileDataSource;
    using lab::data::FileDataSink;
    using lab::data::IOStatus;

    using lab::util::Utils;

    using std::ofstream;

    class BaseJob {

    protected:
      std::string m_jobName;
      std::shared_ptr<IProcessor> m_processor;

    public:
      BaseJob(const std::string& p_jobName,
        const std::shared_ptr<IProcessor>& p_itc_processor) {
        Utils::LogDebug(" BaseJob ctor: " + p_jobName);

        m_jobName = p_jobName;
        m_processor = p_itc_processor;
      }

      virtual ~BaseJob() = default;

      virtual void process() {
        Utils::LogDebug("FileJob processed");
      };

      const std::string& getJobName() const {
        return m_jobName;
      }

    };


    class ReaderJob : public BaseJob {
      std::shared_ptr<IDataSource> m_source;

    public:

      ReaderJob(const std::string& p_name,
        const std::shared_ptr<IDataSource>& p_source,
        const std::shared_ptr<IProcessor>& p_itc_processor)
        : BaseJob(p_name, p_itc_processor), m_source(p_source) {
        Utils::LogDebug(" ReaderJob ctor");
      }

      void process() override {
        if (!m_source) {
          Utils::LogErr("ReaderJob: Source is null");
          return;
        }

        consume(m_source);

        Utils::Log(" ReaderJob processed: " + m_jobName);
      }


      void consume(std::shared_ptr<IDataSource> p_dataSource) {
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

        Utils::Log(" Reader finished reading. Awaiting for processor to complete...");
        m_processor->wait();
        Utils::Log(" Reader waiting done");
      }
    };



    class WriterJob : public BaseJob {
      std::shared_ptr<IDataSink> m_sink;
    public:
      WriterJob(const std::string& p_name,
        const std::shared_ptr<IDataSink>& p_sink,
        const std::shared_ptr<IProcessor>& p_itc_processor)
        : BaseJob(p_name, p_itc_processor), m_sink(p_sink) {
        Utils::LogDebug(" WriterJob ctor");
      }

      void process() override {
        if (!m_sink) {
          Utils::LogErr("WriterJob: Sink is null");
          return;
        }

        produce(m_sink);

        Utils::Log(" WriterJob processed: " + m_jobName);
      }

      void produce(std::shared_ptr<IDataSink> p_dataSink) {
        bool done = false;
        while (!done) {
          IOStatus result = *m_processor >> p_dataSink;

          if (IOStatus::NextBufferBusy == result) {
            // TODO : wait?
            // std::this_thread::yield();
            continue;
          }

          if (IOStatus::Failed == result) {
            Utils::Log("produce Failed");
            done = true;
            break;
          }
          if (IOStatus::EndOfFile == result) {
            Utils::Log("produce EndOfFile");
            done = true;
            break;
          }
        }

      }

    };

  }
}
