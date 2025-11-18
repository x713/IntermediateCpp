#pragma once
#pragma once


#include <vector>
#include <array>

#include <mutex>

#include <string>
#include <iostream> 

#include "IProcessor.h"
#include "ThreadedDataProcessor.h"
#include "SCopyProc.h"
#include "LineBuffer.h"


#include "../../shared/ipc/ipc_mutex.h"
#include "../../util/Utils.h"

namespace lab {
  namespace data {

    //using lab::util::Utils;
    using lab::data::ThreadedDataProcessor;
    using lab::data::WICopyProcessor;

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

      static std::shared_ptr<WICopyProcessor<c_defaultPoolSize>> createIntersystemProcessor() {
        auto result = std::make_shared<WICopyProcessor<c_defaultPoolSize>>();
        return result;
      }
    };
  }
}