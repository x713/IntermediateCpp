#pragma once

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
  }
}