#pragma once

#include <iostream>
#include <string>


#include "../cli_processor/ComTypes.h"

#include "../buffer/SharedBuffer.h"


namespace lab {
  namespace worker {
    using lab::data::SharedBuffer;
    class FileJob {

    private:
      FileJob() = delete;

    protected:
      std::string m_inFilename;
      std::shared_ptr<SharedBuffer> m_buffer;

    public:
      FileJob(const std::string& p_filename,
        const std::shared_ptr<SharedBuffer>& ipc_buffer) {
        std::cout << " FileJob ctor" << std::endl;

        m_inFilename = p_filename;
        m_buffer = ipc_buffer;
      }

      virtual ~FileJob() = default;

      virtual void process() {
        std::cout << " FileJob processed" << std::endl;
      };

    };



    class ReaderJob : public FileJob {
    public:
      ReaderJob(const std::string& p_filename,
        const std::shared_ptr<SharedBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        std::cout << " ReaderJob ctor" << std::endl;
      }

      virtual void process() {
        // this->process();
        std::cout << " ReaderJob processed" << std::endl;
        // open file
        // setbuffer readyforread
        // until !eof
        // read ch
        // send ch to buffer
        // if error exit
        // set buffer endreading
      }

    };

    class WriterJob : public FileJob {
    public:
      WriterJob(const std::string& p_filename,
        const std::shared_ptr<SharedBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        std::cout << " WriterJob ctor" << std::endl;
      }

      virtual void process() {
        // this->process();
        // open file
        // while readyforread && !endreading
        // get ch from buffer
        // save ch
        // if error exit
        std::cout << " WriterJob processed" << std::endl;
      }

    };
  }
}
