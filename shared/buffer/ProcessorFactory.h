#pragma once


#include <vector>
#include <array>

#include <mutex>

#include <string>
#include <iostream> 

#include "IProcessor.h"
#include "ThreadedDataProcessor.h"
#include "WinIPCProcessor.h"
#include "LineBuffer.h"


#include "../../shared/ipc/ipc_mutex.h"
#include "../../util/Utils.h"

namespace lab {
  namespace data {

    //using lab::util::Utils;
    using lab::data::ThreadedDataProcessor;
    using lab::data::WinIPCProcessor;

		// Default pool size
		constexpr size_t c_defaultPoolSize = 3;

		// Different processor factories
    class CopyProcessorFactory {
    public:

      static std::shared_ptr<ThreadedDataProcessor<c_defaultPoolSize, std::mutex>> createThreadsProcessor() {
        auto result = std::make_shared<ThreadedDataProcessor<c_defaultPoolSize, std::mutex>>();
        return result;
      }

      static std::shared_ptr<ThreadedDataProcessor<c_defaultPoolSize, NamedWinMutex>> createWinThreadsProcessor() {
        auto result = std::make_shared<ThreadedDataProcessor<c_defaultPoolSize, NamedWinMutex>>();
        return result;
      }

      static std::shared_ptr<WinIPCProcessor<c_defaultPoolSize>> createISysProcessorServer() {
        auto result = std::make_shared<WinIPCProcessor<c_defaultPoolSize>>(true);
        return result;
      }
      static std::shared_ptr<WinIPCProcessor<c_defaultPoolSize>> createISysProcessorClient() {
        auto result = std::make_shared<WinIPCProcessor<c_defaultPoolSize>>(false);
        return result;
      }
    };
  }
}