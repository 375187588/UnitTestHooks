/// @file   WS2_32.cpp
/// 
/// API Hook library for unit-testing with Windows Socket dependencies
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
WS2_32::WS2_32()
  : k_library_name("ws2_32")
  , m_version(MAKEWORD(2, 2))
  , m_ws_data(WSADATA())
{
  Initialize();
}

//  ****************************************************************************
WS2_32::WS2_32(USHORT major, USHORT minor)
  : k_library_name("ws2_32")
  , m_version(MAKEWORD(major, minor))
  , m_ws_data(WSADATA())
{
  Initialize();
}

//  ****************************************************************************
WS2_32::~WS2_32()
{
  unhook_all();

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
void WS2_32::hook_all()
{

}

//  ****************************************************************************
void WS2_32::unhook_all()
{
  // Remove every entry from the hook map.
  m_hooks.clear();
  m_state.clear();
}

//  ****************************************************************************
bool WS2_32::is_all_hooked() const
{
  return m_hooks.size() == k_api_count;
}

//  ****************************************************************************
void WS2_32::hook(WS2_32::API_enum api)
{
  if (is_hooked(api))
  {
    return;
  }

  // TODO: Add the specified entry to the map.
}

//  ****************************************************************************
void WS2_32::unhook(WS2_32::API_enum api)
{

}

//  ****************************************************************************
bool WS2_32::is_hooked(WS2_32::API_enum api) const
{
  return false;
}

//  ****************************************************************************
void WS2_32::reset()
{
  m_state.clear();
}

//  ****************************************************************************
SocketStateSptr WS2_32::get_socket_state(SOCKET sock)
{
  SocketStateMap::iterator iter = m_state.find(sock);
  if (iter != m_state.end())
  {
    return iter->second;
  }

  return SocketStateSptr();
}


} // namespace cxxhook

#endif
