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
      virtual bool fail() abstract;
      virtual ~IData() {};
    };

    class IDataSink {
    public:
      virtual IOStatus write(const char* p_buf, size_t p_count) abstract;
      virtual bool fail() abstract;
      virtual ~IDataSink() {};
    };


    class IDataSource {
    public:
      virtual IOStatus read(char* p_buf, size_t p_count) abstract;
      virtual std::streamsize getCount() abstract;
      virtual bool fail() abstract;
      virtual bool eof() abstract;
      virtual ~IDataSource() {};
    };

    class IProcessor {
    public:
      virtual IOStatus operator>>(IDataSink* p_dataSink) abstract;
      virtual IOStatus operator<<(IDataSource* p_dataSource) abstract;
      virtual void close() = 0;
    };


  }
}