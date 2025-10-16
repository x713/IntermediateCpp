#pragma once

#include <fstream>
#include <string>
#include <iostream> 
#include <array> 


#include "../cli_processor/ComTypes.h"

#include "../buffer/SharedBuffer.h"
#include "../../util/Utils.h"


namespace lab {
  namespace worker {

    using lab::data::IBuffer;
    using lab::data::TrippleBuffer;

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
        << 1123 << "abc"
        << 2123 << "abc"
        << 3123 << "abc"
        << 4123 << "abc"
        << 5123 << "abc"
        << 6123 << "abc"
        << 7123 << "abc"
        << 8123 << "abc"
        << 9123 << "abc"
        << 0123 << "abc";

    }

    class ReaderJob : public FileJob {

    public:

      ReaderJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer)
        : FileJob(p_filename, ipc_buffer) {
        Utils::LogDebug(" ReaderJob ctor");
      }

      virtual void process() {
        // this->process();
        // open file
        // setbuffer readyforread
        // until !eof
        // read ch
        // send ch to buffer
        // if error exit
        // set buffer endreading

        //m_buffer

        //m_inFilename
        makeTestData();
        // UART
        // RTS requesttosend
        // DTS dataterminalready
        // TX 

        // open file for reading

        std::string filename = "Test.b";
        std::ifstream istrm(filename, std::ios::binary);
        if (!istrm.is_open()) {
          Utils::Log("failed to open " + filename);
          return;
        }

        bool done = false;
        while (!done) {
          m_buffer->readLine(istrm);

          if (istrm.eof()) {
            Utils::LogDebug("EOF. Read bytes:", ' ');
            //Utils::LogDebug(actual_read_count);

            done = true;
            break;
          }
          else if (istrm.fail()) {
            // read less than 256, but not EOF
            Utils::LogDebug("FAIL : Read less than 256, but not EOF", ' ');
            //Utils::LogDebug(actual_read_count);

            done = true;
            break;
          }
        }

        std::cout << " ReaderJob processed" << std::endl;

      }

    };

    class WriterJob : public FileJob {
    public:
      WriterJob(const std::string& p_filename,
        const std::shared_ptr<IBuffer>& ipc_buffer)
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
