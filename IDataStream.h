#pragma once


namespace lab{
namespace data{

  template<class IData>
  class ISyncedStream{
    
    IData m_data;

    // receiving data
    bool m_rx{false};

    bool m_tx{false};

  public:

    ISyncedStream() {
      m_rx = true;
      m_tx = false;
    };

    virtual void add(IData p_data) = 0;

    virtual IData get() = 0;

    virtual ~ISyncedStream() = default;
  };
}
}