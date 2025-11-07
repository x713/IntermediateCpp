#pragma once


//
//
//
namespace lab {
  namespace data {

    enum class IOStatus {
      IOOK = 0,
      IOEOF,
      IOFAIL,
      IORINGFULL, // WRITE BUFF FULL
      IONEXTBUSY, // READ BUFF NOT RDY
      IOFAILPTR,
      IOFAILOPEN,
    };

    class IData{
    public:
      virtual bool fail() = 0;
      virtual ~IData() {};
    };

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

    class IProcessor {
    public:
      virtual IOStatus operator>>(IDataSink* p_dataSink) = 0;
      virtual IOStatus operator<<(IDataSource* p_dataSource) = 0;
      virtual void close() = 0;
    };


  }
}