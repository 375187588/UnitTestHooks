/// @file   WS2_32.cpp
/// 
/// API Hook library for unit-testing with Windows Socket dependencies
///
/// The library provides basic support for the most common behaviors required
/// by application programmers with socket communications.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#include "WS2_32.h"
#include "../../../ApiHook.h"

// This is a library unique to the Win32 API
#if defined(WIN32)

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

namespace cxxhook
{

//  ****************************************************************************
LibraryFunction WS2_32_Hooks[WS2_32::k_api_count] = 
{
  WS2_ENTRY(accept),
  //WS2_ENTRY(AcceptEx), //Mswsock.lib
  WS2_ENTRY(bind),
  WS2_ENTRY(closesocket),
  WS2_ENTRY(connect),
  //WS2_ENTRY(ConnectEx),
  //WS2_ENTRY(DisconnectEx),
  WS2_ENTRY(getaddrinfo),
  WS2_ENTRY(GetAddrInfoW),
  WS2_ENTRY(gethostbyname),
  WS2_ENTRY(gethostname),
  WS2_ENTRY(GetHostNameW),
  WS2_ENTRY(getpeername),
  WS2_ENTRY(getsockname),
  WS2_ENTRY(getsockopt),
  WS2_ENTRY(ioctlsocket),
  WS2_ENTRY(listen),
  WS2_ENTRY(recv),
  WS2_ENTRY(recvfrom),
  WS2_ENTRY(select),
  WS2_ENTRY(send),
  WS2_ENTRY(sendto),
  WS2_ENTRY(setsockopt),
  WS2_ENTRY(shutdown),
  WS2_ENTRY(socket),
  WS2_ENTRY(WSAAccept),
  WS2_ENTRY(WSAAsyncSelect),
  WS2_ENTRY(WSACancelAsyncRequest),
  WS2_ENTRY(WSACleanup),
  WS2_ENTRY(WSACloseEvent),
  WS2_ENTRY(WSAConnect),
  WS2_ENTRY(WSACreateEvent),
  WS2_ENTRY(WSAEventSelect),
  WS2_ENTRY(WSAGetOverlappedResult),
  WS2_ENTRY(WSAIoctl),
  WS2_ENTRY(WSARecv),
  WS2_ENTRY(WSARecvDisconnect),
  //WS2_ENTRY(WSARecvEx),
  WS2_ENTRY(WSARecvFrom),
  WS2_ENTRY(WSAResetEvent),
  WS2_ENTRY(WSASend),
  WS2_ENTRY(WSASendDisconnect),
  WS2_ENTRY(WSASendTo),
  WS2_ENTRY(WSASetEvent),
  WS2_ENTRY(WSASocketA),
  WS2_ENTRY(WSASocketW),
  WS2_ENTRY(WSAStartup),
  WS2_ENTRY(WSAWaitForMultipleEvents)
};


//  ****************************************************************************
WS2_32::WS2_32()
  : k_library_name("ws2_32.dll")
  , m_version(MAKEWORD(2, 2))
  , m_ws_data(WSADATA())
{
  Initialize();
}

//  ****************************************************************************
WS2_32::WS2_32(USHORT major, USHORT minor)
  : k_library_name("ws2_32.dll")
  , m_version(MAKEWORD(major, minor))
  , m_ws_data(WSADATA())
{
  Initialize();
}

//  ****************************************************************************
WS2_32::~WS2_32()
{
  unhook();

  if (m_is_init)
  {
    ::WSACleanup();
    m_is_init = false;
  }
}

//  ****************************************************************************
int WS2_32::Initialize()
{
  int result = 0;
  if (!m_is_init)
  {
    // Success is indicated by zero.
    result = ::WSAStartup(m_version, &m_ws_data);
    m_is_init = (0 == result);
  }

  return result;
}

//  ****************************************************************************
void WS2_32::hook()
{
  // Clear the slate.
  unhook();
  // Now create new hooks for all of the functions.
  for (size_t index = 0; index < k_api_count; ++index)
  {
    m_hooks[API_enum(index)] = 
      std::make_shared<ApiHook>(k_library_name.c_str(),
                                WS2_32_Hooks[index].name,
                                WS2_32_Hooks[index].pfn);
  }
}

//  ****************************************************************************
void WS2_32::unhook()
{
  // Remove every entry from the hook map.
  m_hooks.clear();
  reset();
}

//  ****************************************************************************
bool WS2_32::is_hooked() const
{
  return m_hooks.size() == k_api_count;
}

//  ****************************************************************************
void WS2_32::reset()
{
  cxxhook::reset_socket_state();
}

//  ****************************************************************************
TcpSocketSP WS2_32::get_tcp_socket_state(SOCKET sock)
{
  return cxxhook::get_tcp_socket(sock);
}

//  ****************************************************************************
UdpSocketSP WS2_32::get_udp_socket_state(SOCKET sock)
{
  return cxxhook::get_udp_socket(sock);
}


} // namespace cxxhook

#endif
