#pragma once

#include "../IOStatus.h"

//
//
//
namespace lab {
  namespace data {
    /*
        class IData {
        public:
          virtual bool fail() = 0;
          virtual ~IData() {};
        };

        class IDataSourceSink : public IDataSink, IDataSource {
        public:
        IDataSourceSink() default;
        };
    */

    class IDataSink {
    public:
      virtual IOStatus write(const char* p_buf, size_t p_count) = 0;
      virtual bool fail() = 0;
      virtual ~IDataSink() {};
    };


    class IDataSource {
    public:
      virtual IOStatus read(char* p_buf, size_t p_count) = 0;
      virtual std::streamsize getCount() = 0;
      virtual bool fail() = 0;
      virtual bool eof() = 0;
      virtual ~IDataSource() {};
    };
  }
}