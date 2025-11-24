#pragma once

#include "../IOStatus.h"
#include "../data/IDataIO.h"

//
//
//
namespace lab {
  namespace processing {

    using lab::data::IOStatus;
    using lab::data::IDataSource;
    using lab::data::IDataSink;

    class IProcessor {
    public:
      virtual IOStatus operator>>(IDataSink* p_dataSink) = 0;
      virtual IOStatus operator<<(IDataSource* p_dataSource) = 0;
      virtual void close() = 0;
      virtual void wait() = 0;
    };


  }
}