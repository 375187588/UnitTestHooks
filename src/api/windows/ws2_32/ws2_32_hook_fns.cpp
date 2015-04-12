/// @file   WS2_32_hook_fns.cpp
/// 
/// Implementations for the WS2_32 library hook functions for unit-tests.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#include "WS2_32.h"
#include <istream>
#include <algorithm>

// This is a library unique to the Win32 API
#if defined(WIN32)

namespace // unnamed
{

cxxhook::SocketStateMap g_socket_state; ///< Represents the active state of
                                        ///  the emulated sockets API for each test.
SOCKET                  g_next_id = 1;  ///< Keep a counter to return new ids 
                                        ///  each time a socket is requested.

int                     g_error = 0;    ///< Stores the last socket error 
                                        ///  for this thread.
                                        ///  @note Error state is only recorded
                                        ///  for a single thread. Undefined 
                                        ///  behavior will occur if multiple 
                                        ///  threads make socket calls.
} // namespace unnamed

namespace cxxhook
{

//  ****************************************************************************
void reset_socket_state()
{
  g_socket_state.clear();
}

//  ****************************************************************************
SocketStateSptr get_socket_state(SOCKET id)
{
  SocketStateMap::iterator iter = g_socket_state.find(id);
  if (iter != g_socket_state.end())
  {
    return iter->second;
  }

  return SocketStateSptr();
}

//  ****************************************************************************
SOCKET add_socket_state(SocketStateSptr sp_socket)
{
  SOCKET id = g_next_id++;
  g_socket_state[id] = sp_socket;
  return id;
}

//  ****************************************************************************
void remove_socket_state(SOCKET id)
{
  SocketStateMap::iterator iter = g_socket_state.find(id);
  if (iter != g_socket_state.end())
  {
    g_socket_state.erase(iter);
  }
}


//  ****************************************************************************
int set_socket_error(int errCode)
{
  g_error = errCode;
  return  0 == g_error
          ? 0
          : SOCKET_ERROR;
}

//  ****************************************************************************
int get_socket_error()
{
  return g_error;
}

//  ****************************************************************************
//  Socket Creation ************************************************************
//  ****************************************************************************
SOCKET WSAAPI Hook_socket(
  int af,
  int type,
  int protocol
)
{
  SocketStateSptr sp_socket = std::make_shared<SocketState>();
  sp_socket->m_af       = af;
  sp_socket->m_type     = type;
  sp_socket->m_protocol = protocol;

  return add_socket_state(sp_socket);
}

//  ****************************************************************************
int Hook_closesocket(
  SOCKET s
)
{
  remove_socket_state(s);
  return 0;
}

//  ****************************************************************************
int Hook_shutdown(
  SOCKET  s,
  int     how
)
{
  // TODO: 
  return 0;
}

//  ****************************************************************************
SOCKET Hook_WSASocket(
  int   af,
  int   type,
  int   protocol,
  LPWSAPROTOCOL_INFO lpProtocolInfo,
  GROUP g,
  DWORD dwFlags
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSADuplicateSocket(
  SOCKET              s,
  DWORD               dwProcessId,
  LPWSAPROTOCOL_INFO  lpProtocolInfo
)
{
  // TODO: Not sure how valuable this will be in general, may remove it.
  return 0;
}

//  Socket Transfer ************************************************************
//  ****************************************************************************
int Hook_recv(
  SOCKET  s,
  char*   buf,
  int     len,
  int     flags
)
{
  SocketStateSptr sp_socket = get_socket_state(s);
  if (!sp_socket)
  {
    return set_socket_error(WSAENOTSOCK);
  }

  std::streambuf* pRdBuf = sp_socket->m_recv_buffer.rdbuf();
  std::streamsize size   = std::min<std::streamsize>(pRdBuf->in_avail(), len);
  if (size < 0)
  {
    // TODO: Currently, no calls will block to prevent unit-tests from blocking indefinitely. Support is planned to emulate blocking socket calls.
    return  sp_socket->is_blocking()
            ? set_socket_error(0)
            : set_socket_error(WSAEWOULDBLOCK);
  }

  return (int)pRdBuf->sgetn(buf, size);
}

//  ****************************************************************************
int Hook_recvfrom(
  SOCKET    s,
  char*     buf,
  int       len,
  int       flags,
  sockaddr* from,
  int *     fromlen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_send(
  SOCKET      s,
  const char* buf,
  int         len,
  int         flags
)
{
  if (len < 0)
  {
    // TODO: Find out what actually happens when a negative index is passed in for the length.
    return set_socket_error(WSAENOBUFS);
  }

  SocketStateSptr sp_socket = get_socket_state(s);
  if (!sp_socket)
  {
    return set_socket_error(WSAENOTSOCK);
  }

  // TODO: Work in configuration for how large the write buffers are for the sockets.
  std::streambuf* pRdBuf = sp_socket->m_send_buffer.rdbuf();
  return (int)pRdBuf->sputn(buf, len);
}

//  ****************************************************************************
int Hook_sendto(
  SOCKET          s,
  const char*     buf,
  int             len,
  int             flags,
  const sockaddr *to,
  int             tolen
)
{
  return 0;
}

//  Socket Configure ***********************************************************
//  ****************************************************************************
int Hook_getsockopt(
  SOCKET  s,
  int     level,
  int     optname,
  char*   optval,
  int*    optlen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_setsockopt(
  SOCKET      s,
  int         level,
  int         optname,
  const char* optval,
  int         optlen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_ioctlsocket(
  SOCKET  s,
  long    cmd,
  u_long* argp 
)
{
  return 0;
}

//  Socket Actions *************************************************************
//  ****************************************************************************
SOCKET Hook_accept(
  SOCKET    s,
  sockaddr *addr,
  int *     addrlen
)
{
  return 0;
}

//  ****************************************************************************
SOCKET Hook_WSAAccept(
  SOCKET          s,
  sockaddr *      addr,
  LPINT           addrlen,
  LPCONDITIONPROC lpfnCondition,
  DWORD_PTR       dwCallbackData
)
{
  return 0;
}

//  ****************************************************************************
int Hook_bind(
  SOCKET          s,
  const sockaddr* name,
  int             namelen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_connect(
  SOCKET          s,
  const sockaddr* name,
  int             namelen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_listen(
  SOCKET  s,
  int     backlog
)
{
  return 0;
}

//  ****************************************************************************
int Hook_select(
  int             nfds,
  fd_set *        readfds,
  fd_set *        writefds,
  fd_set *        exceptfds,
  const timeval * timeout
)
{
  return 0;
}

//  Asynchronous ***************************************************************
//  ****************************************************************************
BOOL Hook_AcceptEx(
  SOCKET        sListen,
  SOCKET        sAccept,
  PVOID         lpOutputBuffer,
  DWORD         dwReceiveDataLength,
  DWORD         dwLocalAddressLength,
  DWORD         dwRemoteAddressLength,
  LPDWORD       lpdwBytesReceived,
  LPOVERLAPPED  lpOverlapped
)
{
  return FALSE;
}

//  ****************************************************************************
int Hook_WSAAsyncSelect(
  SOCKET        s,
  HWND          hWnd,
  unsigned int  wMsg,
  long          lEvent
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSACancelAsyncRequest(
  HANDLE hAsyncTaskHandle
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSAConnect(
  SOCKET          s,
  const sockaddr* name,
  int             namelen,
  LPWSABUF        lpCallerData,
  LPWSABUF        lpCalleeData,
  LPQOS           lpSQOS,
  LPQOS           lpGQOS
)
{
  return 0;
}

//  ****************************************************************************
BOOL PASCAL Hook_ConnectEx(
  SOCKET          s,
  const sockaddr* name,
  int             namelen,
  PVOID           lpSendBuffer,
  DWORD           dwSendDataLength,
  LPDWORD         lpdwBytesSent,
  LPOVERLAPPED    lpOverlapped
)
{
  return 0;
}

//  ****************************************************************************
BOOL Hook_DisconnectEx(
  SOCKET        hSOCKET,
  LPOVERLAPPED  lpOverlapped,
  DWORD         dwFlags,
  DWORD         reserved
)
{
  return 0;
}

//  ****************************************************************************
BOOL WSAAPI Hook_WSAGetOverlappedResult(
  SOCKET          s,
  LPWSAOVERLAPPED lpOverlapped,
  LPDWORD         lpcbTransfer,
  BOOL            fWait,
  LPDWORD         lpdwFlags
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSARecv(
  SOCKET    s,
  LPWSABUF  lpBuffers,
  DWORD     dwBufferCount,
  LPDWORD   lpNumberOfBytesRecvd,
  LPDWORD   lpFlags,
  LPWSAOVERLAPPED                     lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSARecvDisconnect(
  SOCKET    s,
  LPWSABUF  lpInboundDisconnectData
)
{
  return 0;
}

//  ****************************************************************************
int PASCAL FAR Hook_WSARecvEx(
  SOCKET  s,
  char*   buf,
  int     len,
  int *   flags
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSARecvFrom(
  SOCKET      s,
  LPWSABUF    lpBuffers,
  DWORD       dwBufferCount,
  LPDWORD     lpNumberOfBytesRecvd,
  LPDWORD     lpFlags,
  sockaddr *  lpFrom,
  LPINT       lpFromlen,
  LPWSAOVERLAPPED                     lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSASend(
  SOCKET    s,
  LPWSABUF  lpBuffers,
  DWORD     dwBufferCount,
  LPDWORD   lpNumberOfBytesSent,
  DWORD     dwFlags,
  LPWSAOVERLAPPED                     lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSASendTo(
  SOCKET          s,
  LPWSABUF        lpBuffers,
  DWORD           dwBufferCount,
  LPDWORD         lpNumberOfBytesSent,
  DWORD           dwFlags,
  const sockaddr *lpTo,
  int             iToLen,
  LPWSAOVERLAPPED                     lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSASendDisconnect(
  SOCKET    s,
  LPWSABUF  lpOutboundDisconnectData
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSAIoctl(
  SOCKET    s,
  DWORD     dwIoControlCode,
  LPVOID    lpvInBuffer,
  DWORD     cbInBuffer,
  LPVOID    lpvOutBuffer,
  DWORD     cbOutBuffer,
  LPDWORD   lpcbBytesReturned,
  LPWSAOVERLAPPED                     lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine
)
{
  return 0;
}

//  WSA Init/Term Calls ********************************************************
//  ****************************************************************************
int Hook_WSAStartup(
  WORD wVersionRequested,
  LPWSADATA lpWSAData
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSACleanup(void)
{
  return 0;
}

//  WSAEvent Related Calls *****************************************************
//  ****************************************************************************
WSAEVENT Hook_WSACreateEvent(void)
{
  return 0;
}

//  ****************************************************************************
BOOL Hook_WSACloseEvent(
  WSAEVENT hEvent
)
{
  return 0;
}

//  ****************************************************************************
int Hook_WSAEventSelect(
  SOCKET s,
  WSAEVENT hEventObject,
  long lNetworkEvents
)
{
  return 0;
}

//  ****************************************************************************
BOOL Hook_WSAResetEvent(
  WSAEVENT hEvent
)
{
  return 0;
}

//  ****************************************************************************
BOOL Hook_WSASetEvent(
  WSAEVENT hEvent
)
{
  return FALSE;
}

//  ****************************************************************************
DWORD Hook_WSAWaitForMultipleEvents(
  DWORD cEvents,
  const WSAEVENT *lphEvents,
  BOOL fWaitAll,
  DWORD dwTimeout,
  BOOL fAlertable
)
{
  return 0;
}

//  Name/Address Functions *****************************************************
//  ****************************************************************************
int WSAAPI Hook_getaddrinfo(
  PCSTR pNodeName,
  PCSTR pServiceName,
  const ADDRINFOA *pHints,
  PADDRINFOA *ppResult
)
{
  return 0;
}

//  ****************************************************************************
int WSAAPI Hook_GetAddrInfoW(
  PCWSTR pNodeName,
  PCWSTR pServiceName,
  const ADDRINFOW *pHints,
  PADDRINFOW *ppResult
)
{
  return 0;
}

//  ****************************************************************************
hostent* FAR Hook_gethostbyname(
  const char *name
)
{
  return 0;
}

//  ****************************************************************************
int Hook_gethostname(
  char*,
  int
)
{
  return 0;
}

//  ****************************************************************************
int WSAAPI Hook_GetHostNameW(
  PWSTR*,
  int
)
{
  return 0;
}

//  ****************************************************************************
int Hook_getpeername(
  SOCKET s,
  sockaddr*,
  int *namelen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_getsockname(
  SOCKET s,
  sockaddr*,
  int *namelen
)
{
  return 0;
}

//  Error Handling *************************************************************
//  ****************************************************************************
int Hook_WSAGetLastError(void)
{
  return get_socket_error();
}

//  ****************************************************************************
void Hook_WSASetLastError(
  int iError
)
{
  set_socket_error(iError);
}


} // namespace cxxhook

#endif
