/// @file   WS2_32.h 
/// 
/// API Hook library for unit-testing with Windows Socket dependencies.
///
/// The library provides basic support for the most common behaviors required
/// by application programmers with socket communications.
///
/// From the callers point-of-view, when the hooks are active for this class, 
/// the library calls will behave as the original library behaves.
/// However, all memory management will be handled internally and no socket
/// communication will occur.
///
/// Usage:
///       // Create a single instance of the class for the life of the test-suite.
///       cxxhook::WS2_32   winsock_hook;
///
///       // Enable the hooks for every API.
///       winsock_hook.hook_all();
///
///       // Write your tests.
///       // The capture, management, and return of data are automatically handled.
///
///       // Call reset during the *teardown* phase to reset the state
///       // of the system for the next test.
///       winsock_hook.reset();
///
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#ifndef CXXHOOK_WS2_32_H_INCLUDED
#define CXXHOOK_WS2_32_H_INCLUDED
// This is a library unique to the Win32 API
#if defined(WIN32)
//  Includes *******************************************************************
#include "ws2_32_def.h"
#include "socket_state.h"

namespace cxxhook
{

//  ****************************************************************************
/// API Hook library for unit-testing with Windows Socket dependencies.
///
/// For ease of testing, calls have been added to set the desired response 
/// of an API to force a failure,as well as calls to reset the state of the library.
///
/// This class currently is limited to hooking the most commonly called APIs.
///   - APIs that match the Berkeley socket calls.
///   - Windows Socket versions of these calls.
///   - Basic overlapped I/O and socket events.
///
///
class WS2_32
{
public:
  //  Constants ****************************************************************
  const std::string k_library_name;

  //  Construction *************************************************************
  WS2_32();
  WS2_32(USHORT major, USHORT minor);
  ~WS2_32();

  WORD            get_version() const             { return m_version; }
  const WSADATA&  get_WSADATA() const             { return m_ws_data; }

  //  Hook Control / Status ****************************************************
  void hook();
  void unhook();
  bool is_hooked() const;

  void reset();

  TcpSocketSP get_tcp_socket_state(SOCKET sock);
  UdpSocketSP get_udp_socket_state(SOCKET sock);

  //  Enumerated API List ******************************************************
  enum API_enum
  {
    k_unspecified = -1,

    accept,
    AcceptEx,
    bind,
    closesocket,
    connect,
    ConnectEx,
    DisconnectEx,
    getaddrinfo,
    GetAddrInfoW,
    gethostbyname,

    gethostname,
    GetHostNameW,
    getpeername,
    getsockname,
    getsockopt,
    ioctlsocket,
    listen,
    recv,
    recvfrom,
    select,

    send,
    sendto,
    setsockopt,
    shutdown,
    socket,
    WSAAccept,
    WSAAsyncSelect,
    WSACancelAsyncRequest,
    WSACleanup,
    WSACloseEvent,

    WSAConnect,
    WSACreateEvent,
    WSAEventSelect,
    WSAGetOverlappedResult,
    WSAIoctl,
    WSARecv,
    WSARecvDisconnect,
    WSARecvEx,
    WSARecvFrom,
    WSAResetEvent,

    WSASend,
    WSASendDisconnect,
    WSASendTo,
    WSASetEvent,
    WSASocket,
    WSAStartup,
    WSAWaitForMultipleEvents,

    k_api_count
  };

private:
  //  Typedefs *****************************************************************
  typedef std::shared_ptr<ApiHook>                ApiHookSptr;
  typedef std::map<API_enum, ApiHookSptr>         ApiHookMap;

  //  Data Members *************************************************************
  WORD                  m_version;      ///< The requested winsock version.
  WSADATA               m_ws_data;      ///< Startup data received from winsock
                                        ///  initialization.
  bool                  m_is_init;      ///< Flag indicates if the API has been
                                        ///  successfully initialized.

  ApiHookMap            m_hooks;        ///< The set of APIs that are hooked.
  
  //  Methods ******************************************************************
  int Initialize();

};

} // namespace cxxhook

#endif

#endif

