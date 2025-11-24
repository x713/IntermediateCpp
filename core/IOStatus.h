#pragma once

namespace lab {
  namespace data {

    enum class IOStatus {
      Ok = 0,
      EndOfFile,
      Failed,
      RingBufferFull,
      NextBufferBusy,
      NullPointer,
      OpenFailed,
      MemoryAllocationFailed,
    };
  }
}