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
#include "sockets/BLNetwork.h"

#include <map>
#include <memory>
#include <sstream>

namespace cxxhook
{

using namespace ipc::ip;

typedef std::shared_ptr<Tcp::Socket>            TcpSocketSP;
typedef std::map<SOCKET, TcpSocketSP>           TcpSocketMap;

typedef std::shared_ptr<Udp::Socket>            UdpSocketSP;
typedef std::map<SOCKET, UdpSocketSP>           UdpSocketMap;

struct SocketState
{
  //  Construction *************************************************************
  //  ****************************************************************************
  SocketState()
    : m_af(0)
    , m_type(0)
    , m_protocol(0)
    , m_addr(sockaddr())
  { }

  //  ****************************************************************************
  ~SocketState()
  { }

  //  ****************************************************************************


  //  ****************************************************************************

  //  Data Members *************************************************************
  int                 m_af;
  int                 m_type;
  int                 m_protocol;

  sockaddr            m_addr;
  std::string         m_name;

  std::stringstream   m_send_buffer;
  std::stringstream   m_recv_buffer;
};



} // namespace cxxhook

#endif

