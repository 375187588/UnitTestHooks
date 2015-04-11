/// @file   socket_state.h 
/// 
/// Manages the state of an emulated socket for unit-tests.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#ifndef CXXHOOK_SOCKET_STATE_H_INCLUDED
#define CXXHOOK_SOCKET_STATE_H_INCLUDED
//  Includes *******************************************************************

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>

#endif


#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cxxhook
{

struct SocketState
{
  //  Typedefs *****************************************************************
  typedef std::vector<char>             buffer_t;

  //  Construction *************************************************************
  SocketState()
    : m_af(0)
    , m_type(0)
    , m_protocol(0)
    , m_addr(sockaddr())
  { }

  ~SocketState()
  { }

  //  Data Members *************************************************************
  int             m_af;
  int             m_type;
  int             m_protocol;

  std::string     m_name;
  sockaddr        m_addr;

  buffer_t        m_send_buffer;
  buffer_t        m_recv_buffer;



};

typedef std::shared_ptr<SocketState>            SocketStateSptr;


} // namespace cxxhook

#endif

