/// @file: BLSocketOperations.h
///
/// Options abstraction for use with socket communications.  
/// The options helper templates simplify the abstraction for the 
/// XXXsockopt functions.  The types of variables in the socket API
/// vary between bools, ints, and structure definitions.
///
/// These class templates provide a protected abstract interface that 
/// will reduce to inline operations for an optimized build.
///
/// This example is for the usage of the keep alive socket option:
///
///   using namespace cxxhook::icp;
///   typedef ::detail::socket_option::boolean<SOL_SOCKET, SO_KEEPALIVE> keep_alive;
///
///   keep_alive keepAlive(true);
///   ip::tcp::Socket socket;
///   socket.SetOption(keepAlive);
///
///   // alternatively, construct option type inline
///   socket.SetOption(keep_alive(true));
///
///   // To read a value, simply define an option var, and Get the option.
///   keep_alive keep;
///   socket.GetOption(keep);
/// 
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// BLNetwork.h contains additional copyright information. 
///
//  ****************************************************************************
#ifndef BLSOCKETOPTIONS_H_INCLUDED
#define BLSOCKETOPTIONS_H_INCLUDED
//  Includes *****************************************************************
#include "BLSocketDef.h"

namespace cxxhook
{
namespace ipc
{
namespace detail
{
namespace socket_option
{

//  ****************************************************************************
/// Helper abstraction for boolean-based options.
///
template <int level, int name>
class boolean
{
public:
            boolean  ()       : m_value(0)          { }
  explicit  boolean  (bool v) : m_value(v ? 1 : 0)  { }
  boolean&  operator=(bool v)                       { m_value = v ? 1 : 0;
                                                      return *this;
                                                    }
  bool      Value    () const                       { return !!m_value;}
  operator  bool     () const                       { return !!m_value;}
  bool      operator!() const                       { return !m_value;}

  template <typename Protocol>
  int       Level    (const Protocol&) const        { return level;}

  template <typename Protocol>
  int       Name     (const Protocol&) const        { return name;}

  template <typename Protocol>
  int*      Data     (const Protocol&)              { return &m_value;}

  template <typename Protocol>
  const int* Data    (const Protocol&) const        { return &m_value;}

  template <typename Protocol>
  size_t    Size     (const Protocol&) const        { return sizeof(m_value);}

  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    // On some platforms (e.g. Windows Vista), the getsockopt function will
    // return the size of a boolean socket option as one byte, even though a
    // four byte integer was passed in.
    switch (s)
    {
    case sizeof(char):
      m_value = *reinterpret_cast<char*>(&m_value) ? 1 : 0;
      break;
    case sizeof(m_value):
      break;
    default:
      BLASSERT(("boolean socket option resize set with invalid size", 0));
    }
  }

private:
  int m_value;
};

//  ****************************************************************************
/// Helper abstraction for integer options.
///
template <int level, int name>
class integer
{
public:
            integer  ()       : m_value(0)          { }
  explicit  integer  (int v)  : m_value(v ? 1 : 0)  { }
  integer&  operator=(int v)                        { m_value = v; return *this;}                                                    
  int       Value    () const                       { return m_value;}

  template <typename Protocol>
  int       Level    (const Protocol&) const        { return level;}

  template <typename Protocol>
  int       Name     (const Protocol&) const        { return name;}

  template <typename Protocol>
  int*      Data     (const Protocol&)              { return &m_value;}

  template <typename Protocol>
  const int* Data    (const Protocol&) const        { return &m_value;}

  template <typename Protocol>
  size_t    Size     (const Protocol&) const        { return sizeof(m_value);}

  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    if (s != sizeof(m_value))
      BLASSERT(("integer socket option resize set with invalid size", 0));
  }

private:
  int m_value;
};


//  ****************************************************************************
/// Helper abstraction for linger options.
///
template <int level, int name>
class linger
{
public:
            linger   ()                             { m_value.l_onoff = 0;
                                                      m_value.l_linger = 0;
                                                    }
            linger(bool isEnabled, int timeout)     { Enabled(isEnabled);
                                                      Timeout(timeout);
                                                    }
  void      Enabled  (bool isEnabled)               { m_value.l_onoff = isEnabled ? 1 : 0;}
  bool      Enabled  () const                       { return m_value.l_onoff != 0;}
  void      Timeout  (int timeout)                  { 
#if defined(WIN32)
                                                      m_value.l_linger = static_cast<u_short>(timeout);
#else
                                                      m_value.l_linger = timeout;
#endif
                                                    }
  int       Timeout  () const                       { return static_cast<int>(m_value.l_linger);}

  template <typename Protocol>
  int       Level    (const Protocol&) const        { return level;}

  template <typename Protocol>
  int       Name     (const Protocol&) const        { return name;}

  template <typename Protocol>
  ::linger* Data     (const Protocol&)              { return &m_value;}

  template <typename Protocol>
  const ::linger* Data (const Protocol&) const      { return &m_value;}

  template <typename Protocol>
  size_t    Size     (const Protocol&) const        { return sizeof(m_value);}

  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    if (s != sizeof(m_value))
      BLASSERT(("linger socket option resize set with invalid size", 0));
  }

private:
  ::linger m_value;
};

} // socket_option
} // detail
} // namespace ipc
} // namespace cxxhook

#endif 
