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
#include <sstream>

namespace cxxhook
{

struct SocketState
{
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
  int                 m_af;
  int                 m_type;
  int                 m_protocol;

  std::string         m_name;
  sockaddr            m_addr;

  std::stringstream   m_send_buffer;
  std::stringstream   m_recv_buffer;
};

typedef std::shared_ptr<SocketState>            SocketStateSptr;
typedef std::map<SOCKET, SocketStateSptr>       SocketStateMap;


} // namespace cxxhook

#endif

