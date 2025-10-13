// Senorra.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <queue>

#include <mutex>
#include <thread>


//
//
//
using SQueue = std::queue<char>;

class SharedBuffer {
  SQueue m_Buffer{};

  const int c_buffer_size = 256;

  bool m_writing = false;
  bool m_reading = false;

  std::mutex m_BufferMutex{};
  std::mutex m_SemaphoreMutex{};

public:

  // Send char to buffer, skip if buffer locked or full. 
  void broadcastChar(const char p_char) {
    if (m_BufferMutex.try_lock())
    {
      // skip if full
      if (m_Buffer.size() < c_buffer_size) {
        m_Buffer.push(p_char);

      }
      m_BufferMutex.unlock();
    }
  }
};

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

int main(const int argc, const char* argv[])
{
  if (argc != 3) {
    std::cout << " Two filenames expected" << std::endl;
    return 1;
  }

  const std::string in_filename{ argv[1] };
  const std::string out_filename{ argv[2] };

  std::cout << "Input: " << in_filename << ", Output: " << out_filename << std::endl;

  if (in_filename == out_filename) {
    std::cout << " Different filenames expected" << std::endl;
    return 1;
  }

  std::shared_ptr<SharedBuffer> ipc_buffer = std::make_shared<SharedBuffer>();

  ReaderJob reader{ in_filename, ipc_buffer };
  WriterJob writer{ out_filename, ipc_buffer };

  std::thread readerThread(&FileJob::process, &reader);
  std::thread writerThread(&FileJob::process, &writer);


  readerThread.join();
  writerThread.join();

  std::cout << "Main thread finished.";


  return 0;
}

