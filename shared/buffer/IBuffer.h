#pragma once


//
//
//
namespace lab {
  namespace data {

    enum class IOStatus{
      IOOK = 0,
      IOEOF,
      IOFAIL,
      IORINGFULL, // WRITE BUFF FULL
      IONEXTBUSY, // READ BUFF NOT RDY
    };

    class IBuffer {
    public:     

      virtual IOStatus writeLine(std::ofstream& p_ostream) abstract;
      virtual IOStatus readLine(std::ifstream& p_istream) = 0;
      virtual void close() = 0;
    };


  }
}