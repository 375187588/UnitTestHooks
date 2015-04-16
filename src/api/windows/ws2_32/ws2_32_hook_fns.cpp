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

cxxhook::TcpSocketMap g_tcp_sockets;      ///< Represents the active state of the
                                          ///  emulated tcp sockets for each test.
cxxhook::UdpSocketMap g_udp_sockets;      ///< Represents the active state of the
                                          ///  emulated udp sockets for each test.
SOCKET                g_next_tcp_id = 1;  ///< Each tcp socket will be given 
                                          ///  a unique id, odd ID.
SOCKET                g_next_udp_id = 2;  ///< Each udp socket will be given 
                                          ///  a unique id, even ID.

} // namespace unnamed

namespace cxxhook
{

using namespace ipc;

//  ****************************************************************************
bool is_udp(SOCKET s)
{
  return (static_cast<unsigned int>(s) % 2) == 0;
}

//  ****************************************************************************
void reset_socket_state()
{
  g_tcp_sockets.clear();
  g_udp_sockets.clear();
}

//  ****************************************************************************
TcpSocketSP get_tcp_socket(SOCKET id)
{
  TcpSocketMap::iterator iter = g_tcp_sockets.find(id);
  if (iter != g_tcp_sockets.end())
  {
    return iter->second;
  }

  return TcpSocketSP();
}

//  ****************************************************************************
UdpSocketSP get_udp_socket(SOCKET id)
{
  UdpSocketMap::iterator iter = g_udp_sockets.find(id);
  if (iter != g_udp_sockets.end())
  {
    return iter->second;
  }

  return UdpSocketSP();
}

//  ****************************************************************************
SOCKET add_socket_state(TcpSocketSP sp_socket)
{
  SOCKET id = g_next_tcp_id += 2;
  g_tcp_sockets[id] = sp_socket;
  return id;
}

//  ****************************************************************************
SOCKET add_socket_state(UdpSocketSP sp_socket)
{
  SOCKET id = g_next_udp_id += 2;
  g_udp_sockets[id] = sp_socket;
  return id;
}

//  ****************************************************************************
void remove_socket_state(SOCKET id)
{
  if (is_udp(id))
  {
    UdpSocketMap::iterator iter = g_udp_sockets.find(id);
    if (iter != g_udp_sockets.end())
    {
      g_udp_sockets.erase(iter);
    }  
  }
  else
  {
    TcpSocketMap::iterator iter = g_tcp_sockets.find(id);
    if (iter != g_tcp_sockets.end())
    {
      g_tcp_sockets.erase(iter);
    }    
  }
}


//  ****************************************************************************
int set_socket_error(int errCode)
{
  WSASetLastError(errCode);
  return  0 == errCode
          ? 0
          : SOCKET_ERROR;
}

//  ****************************************************************************
int get_socket_error()
{
  return WSAGetLastError();
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
  // Only supporting UDP and TCP socket types.
  SOCKET s = -1;
  if (SOCK_DGRAM == type)
  {
    UdpSocketSP sp_socket = std::make_shared<Udp::Socket>();
    s = add_socket_state(sp_socket);
  }
  else if (SOCK_STREAM == type)
  {
    TcpSocketSP sp_socket = std::make_shared<Tcp::Socket>();
    s = add_socket_state(sp_socket);
  }
  else
  {
    return set_socket_error(WSA_INVALID_PARAMETER);
  }

  return s;
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
template <typename T>
int shutdownT(
  std::shared_ptr<T>        sp_socket,
  SocketBase::ShutdownType  how
)
{
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  return sp_socket->Shutdown(how);
}

//  ****************************************************************************
int Hook_shutdown(
  SOCKET  s,
  int     how
)
{
  return  is_udp(s)
          ? shutdownT(get_udp_socket(s), SocketBase::ShutdownType(how))
          : shutdownT(get_tcp_socket(s), SocketBase::ShutdownType(how));
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
  // TODO: Return and add support for the other parameters, most likely have the berkeley like version call this version instead.
  return Hook_socket(af, type, protocol);
}

//  Socket Transfer ************************************************************
//  ****************************************************************************
template <typename T>
int recvT(
  std::shared_ptr<T>  sp_socket,
  char*               buf,
  int                 len,
  int                 flags
)
{
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  return sp_socket->Receive(buf, len, flags);
}

//  ****************************************************************************
int Hook_recv(
  SOCKET  s,
  char*   buf,
  int     len,
  int     flags
)
{
  return  is_udp(s)
          ? recvT(get_udp_socket(s), buf, len, flags)
          : recvT(get_tcp_socket(s), buf, len, flags);

  //TcpSocketSP sp_socket = get_tcp_socket(s);
  //if (!sp_socket)
  //{
  //  return set_socket_error(WSAENOTSOCK);
  //}

  //std::streambuf* pRdBuf = sp_socket->m_recv_buffer.rdbuf();
  //std::streamsize size   = std::min<std::streamsize>(pRdBuf->in_avail(), len);
  //if (size < 0)
  //{
  //  // TODO: Currently, no calls will block to prevent unit-tests from blocking indefinitely. Support is planned to emulate blocking socket calls.
  //  return  sp_socket->IsBlocking()
  //          ? set_socket_error(0)
  //          : set_socket_error(WSAEWOULDBLOCK);
  //}

  //return (int)pRdBuf->sgetn(buf, size);
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
  // filter out TCP type sockets.
  if (!is_udp(s))
  {
    return set_socket_error(error::k_socketPrototype);
  }

  if (!from)
  {
    return set_socket_error(error::k_invalidArguement);
  }

  UdpSocketSP sp_socket = get_udp_socket(s);
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  Address         address((unsigned long)from->sa_data);
  Udp::Endpoint   endpoint(address, 0);

  int result = sp_socket->ReceiveFrom(buf, len, endpoint, flags);

  if (fromlen)
  {
    *fromlen = sizeof(unsigned long);

    unsigned long from_addr = endpoint.Address().ToUlong();
    memcpy(from, &from_addr, sizeof(unsigned long));
  }

  return result;
}

//  ****************************************************************************
template <typename T>
int sendT(
  std::shared_ptr<T>  sp_socket,
  const char*         buf,
  int                 len,
  int                 flags
)
{
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  return sp_socket->Send(buf, len, flags);
}

//  ****************************************************************************
int Hook_send(
  SOCKET      s,
  const char* buf,
  int         len,
  int         flags
)
{
  return  is_udp(s)
          ? sendT(get_udp_socket(s), buf, len, flags)
          : sendT(get_tcp_socket(s), buf, len, flags);
  //if (len < 0)
  //{
  //  // TODO: Find out what actually happens when a negative index is passed in for the length.
  //  return set_socket_error(WSAENOBUFS);
  //}

  //SocketSP sp_socket = get_socket_state(s);
  //if (!sp_socket)
  //{
  //  return set_socket_error(WSAENOTSOCK);
  //}

  //// TODO: Work in configuration for how large the write buffers are for the sockets.
  //std::streambuf* pRdBuf = sp_socket->m_send_buffer.rdbuf();
  //return (int)pRdBuf->sputn(buf, len);
  return 0;
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
  // filter out TCP type sockets.
  if (!is_udp(s))
  {
    return set_socket_error(error::k_socketPrototype);
  }

  if (!to)
  {
    return set_socket_error(error::k_invalidArguement);
  }

  UdpSocketSP sp_socket = get_udp_socket(s);
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  if (tolen != sizeof(unsigned long))
  {
    return set_socket_error(error::k_invalidArguement);
  }

  Address         address((unsigned long)to->sa_data);
  Udp::Endpoint   endpoint(address, 0);

  return sp_socket->SendTo(buf, len, endpoint, flags);
}

//  Socket Configure ***********************************************************
//  ****************************************************************************
template <typename T>
int getsockoptT(
  std::shared_ptr<T>  sp_socket,
  int     level,
  int     optname,
  char*   optval,
  int*    optlen
)
{
  return 0;
}

//  ****************************************************************************
int Hook_getsockopt(
  SOCKET  s,
  int     level,
  int     optname,
  char*   optval,
  int*    optlen
)
{
  return  is_udp(s)
          ? getsockoptT(get_udp_socket(s), level, optname, optval, optlen)
          : getsockoptT(get_tcp_socket(s), level, optname, optval, optlen);
}

//  ****************************************************************************
template <typename T>
int setsockoptT(
  std::shared_ptr<T>  sp_socket,
  int         level,
  int         optname,
  const char* optval,
  int         optlen
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
  return  is_udp(s)
          ? setsockoptT(get_udp_socket(s), level, optname, optval, optlen)
          : setsockoptT(get_tcp_socket(s), level, optname, optval, optlen);
}

//  ****************************************************************************
template <typename T>
int ioctlsocketT(
  std::shared_ptr<T>  sp_socket,
  long    cmd,
  u_long* argp 
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
  return  is_udp(s)
          ? ioctlsocketT(get_udp_socket(s), cmd, argp)
          : ioctlsocketT(get_tcp_socket(s), cmd, argp);
}

//  Socket Actions *************************************************************
//  ****************************************************************************
template <typename T>
int acceptT(
  std::shared_ptr<T>  sp_socket,
  sockaddr *addr,
  int *     addrlen
)
{
  return 0;
}

//  ****************************************************************************
SOCKET Hook_accept(
  SOCKET    s,
  sockaddr *addr,
  int *     addrlen
)
{
  return  is_udp(s)
          ? acceptT(get_udp_socket(s), addr, addrlen)
          : acceptT(get_tcp_socket(s), addr, addrlen);
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
template <typename T>
int bindT(
  std::shared_ptr<T>  sp_socket,
  const sockaddr*     name,
  int                 namelen
)
{
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  if (!name)
  {
    return set_socket_error(error::k_invalidArguement);
  }

  if (namelen != sizeof(unsigned long))
  {
    return set_socket_error(error::k_invalidArguement);
  }

  ip::Address       addr((unsigned long)name);
  T::EndpointType   local(addr, 0);

  return sp_socket->Bind(local);
}

//  ****************************************************************************
int Hook_bind(
  SOCKET          s,
  const sockaddr* name,
  int             namelen
)
{
  return  is_udp(s)
          ? bindT(get_udp_socket(s), name, namelen)
          : bindT(get_tcp_socket(s), name, namelen);
}

//  ****************************************************************************
template <typename T>
int connectT(
  std::shared_ptr<T>  sp_socket,
  const sockaddr*     name,
  int                 namelen
)
{
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  if (!name)
  {
    return set_socket_error(error::k_invalidArguement);
  }

  if (namelen != sizeof(unsigned long))
  {
    return set_socket_error(error::k_invalidArguement);
  }

  ip::Address       addr((unsigned long)name);
  T::EndpointType                 peer(addr, 0);

  return sp_socket->Connect(peer);
}

//  ****************************************************************************
int Hook_connect(
  SOCKET          s,
  const sockaddr* name,
  int             namelen
)
{
  return  is_udp(s)
          ? connectT(get_udp_socket(s), name, namelen)
          : connectT(get_tcp_socket(s), name, namelen);
}

//  ****************************************************************************
template <typename T>
int listenT(
  std::shared_ptr<T>  sp_socket,
  int                 backlog
)
{
  if (!sp_socket)
  {
    return set_socket_error(error::k_socketNotSocket);
  }

  return sp_socket->Listen(backlog);
}

//  ****************************************************************************
int Hook_listen(
  SOCKET  s,
  int     backlog
)
{
  return  is_udp(s)
          ? listenT(get_udp_socket(s), backlog)
          : listenT(get_tcp_socket(s), backlog);
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
