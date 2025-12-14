#pragma once


#include <vector>
#include <array>

#include <mutex>

#include <string>
#include <iostream> 

#include "IProcessor.h"
#include "ThreadedDataProcessor.h"
#include "Win/WinIPCProcessor.h"
#include "LineBuffer.h"


#include "../../core/ipc/ipc_mutex.h"
#include "../../util/Utils.h"

namespace lab {
  namespace processing {

    //using lab::util::Utils;
    using lab::processing::ThreadedDataProcessor;
    using lab::processing::WinIPCProcessor;

    // Default pool size
    constexpr size_t c_defaultPoolSize = 3;

    // Different processor factories for separate threads
    class ProcessorFactory {
    public:

      // Two threads copy, mutex guard
      static std::shared_ptr<ThreadedDataProcessor<c_defaultPoolSize, std::mutex>> createThreaded() {
        auto result = std::make_shared<ThreadedDataProcessor<c_defaultPoolSize, std::mutex>>();
        return result;
      }

      // Two threads copy, Windows named mutex guard
      static std::shared_ptr<ThreadedDataProcessor<c_defaultPoolSize, NamedWinMutex>> createWindowsThreaded() {
        auto result = std::make_shared<ThreadedDataProcessor<c_defaultPoolSize, NamedWinMutex>>();
        return result;
      }
    };

    // Different processor factories for separate processes
    class IPCProcessorFactory {
    public:
      // Server copy process, Windows named mutex guard
      static std::shared_ptr<WinIPCProcessor<c_defaultPoolSize>> createServer() {
        auto result = std::make_shared<WinIPCProcessor<c_defaultPoolSize>>(true);
        return result;
      }
      // Client copy process, Windows named mutex guard
      static std::shared_ptr<WinIPCProcessor<c_defaultPoolSize>> createClient() {
        auto result = std::make_shared<WinIPCProcessor<c_defaultPoolSize>>(false);
        return result;
      }
    };   


    /*
    *  TODO : IEventSignalling
    *         ISharedData (local, shared)
    *         more Solid
    */ 
  }
}