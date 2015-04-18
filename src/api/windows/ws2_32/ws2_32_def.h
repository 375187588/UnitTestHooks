/// @file   WS2_32_def.h 
/// 
/// Definitions to support the WS2_32 API Hook class.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#ifndef CXXHOOK_WS2_32_DEF_H_INCLUDED
#define CXXHOOK_WS2_32_DEF_H_INCLUDED

// This is a library unique to the Win32 API
#if defined(WIN32)
//  Includes *******************************************************************
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>

#include <map>
#include <memory>
#include <string>

#include "socket_state.h"

//  Forward Declarations *******************************************************
class ApiHook;

namespace cxxhook
{

void        reset_socket_state();
TcpSocketSP get_tcp_socket(SOCKET sock);
UdpSocketSP get_udp_socket(SOCKET sock);

//  Function Pointers for Supported Library Hooks ****************************

typedef SOCKET  (WSAAPI *pfn_accept)           (SOCKET, sockaddr*, int *);
typedef BOOL    (WSAAPI *pfn_AcceptEx)         (SOCKET,SOCKET,PVOID,DWORD,DWORD,DWORD,LPDWORD,LPOVERLAPPED);
typedef int     (WSAAPI *pfn_bind)             (SOCKET,const sockaddr*,int);
typedef int     (WSAAPI  *pfn_closesocket)      (SOCKET);
typedef int     (WSAAPI *pfn_connect)          (SOCKET,const sockaddr*,int);
typedef BOOL    (PASCAL *pfn_ConnectEx) (SOCKET,const sockaddr *,int,PVOID,DWORD,LPDWORD,LPOVERLAPPED);
typedef BOOL    (WSAAPI *pfn_DisconnectEx)     (SOCKET,LPOVERLAPPED,DWORD,DWORD);
typedef int     (WSAAPI *pfn_getaddrinfo)(PCSTR,PCSTR,const ADDRINFOA *,PADDRINFOA *);
typedef int     (WSAAPI *pfn_GetAddrInfoW)(PCWSTR,PCWSTR,const ADDRINFOW *,PADDRINFOW *);
typedef hostent*(FAR *pfn_gethostbyname)(const char *);

typedef int     (WSAAPI *pfn_gethostname)      (char*,int);
typedef int     (WSAAPI *pfn_GetHostNameW)(PWSTR*,int);
typedef int     (WSAAPI *pfn_getpeername)      (SOCKET,sockaddr*,int *);
typedef int     (WSAAPI *pfn_getsockname)      (SOCKET,sockaddr*,int *);
typedef int     (WSAAPI *pfn_getsockopt)       (SOCKET,int,int,char*,int*);
typedef int     (WSAAPI *pfn_ioctlsocket)      (SOCKET,long,u_long*);
typedef int     (WSAAPI *pfn_listen)           (SOCKET,int);
typedef int     (WSAAPI *pfn_recv)             (SOCKET,char*,int,int);
typedef int     (WSAAPI *pfn_recvfrom)         (SOCKET,char*,int,int,sockaddr*,int*);
typedef int     (WSAAPI *pfn_select)           (int,fd_set*,fd_set*,fd_set*,const timeval*);

typedef int     (WSAAPI *pfn_send)             (SOCKET,const char*,int,int);
typedef int     (WSAAPI *pfn_sendto)           (SOCKET,const char*,int,int,const sockaddr*,int);
typedef int     (WSAAPI *pfn_setsockopt)       (SOCKET,int,int,const char*,int);
typedef int     (WSAAPI *pfn_shutdown)         (SOCKET,int);
typedef SOCKET  (WSAAPI *pfn_socket)    (int,int,int);
typedef SOCKET  (WSAAPI *pfn_WSAAccept)        (SOCKET,sockaddr*,LPINT,LPCONDITIONPROC,DWORD_PTR);
typedef int     (WSAAPI *pfn_WSAAsyncSelect)   (SOCKET,HWND,unsigned int,long);
typedef int     (WSAAPI *pfn_WSACancelAsyncRequest)(HANDLE);
typedef int     (WSAAPI *pfn_WSACleanup)       (void);
typedef BOOL    (WSAAPI *pfn_WSACloseEvent)    (WSAEVENT);

typedef int     (WSAAPI *pfn_WSAConnect)       (SOCKET,const sockaddr*,int,LPWSABUF,LPWSABUF,LPQOS,LPQOS);
typedef WSAEVENT(WSAAPI *pfn_WSACreateEvent)   (void);
typedef int     (WSAAPI *pfn_WSADuplicateSocket)(SOCKET,DWORD,LPWSAPROTOCOL_INFO);
typedef int     (WSAAPI *pfn_WSAEventSelect)   (SOCKET,WSAEVENT,long);
typedef int     (WSAAPI *pfn_WSAGetLastError)  (void);
typedef BOOL    (WSAAPI *pfn_WSAGetOverlappedResult)(SOCKET,LPWSAOVERLAPPED,LPDWORD,BOOL,LPDWORD);
typedef int     (WSAAPI *pfn_WSAIoctl)         (SOCKET,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int     (WSAAPI *pfn_WSARecv)          (SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int     (WSAAPI *pfn_WSARecvDisconnect)(SOCKET,LPWSABUF);
typedef int     (PASCAL FAR *pfn_WSARecvEx)(SOCKET,char*,int,int*);

typedef int     (WSAAPI *pfn_WSARecvFrom)      (SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,sockaddr*,LPINT,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef BOOL    (WSAAPI *pfn_WSAResetEvent)    (WSAEVENT);
typedef int     (WSAAPI *pfn_WSASend)          (SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int     (WSAAPI *pfn_WSASendDisconnect)(SOCKET,LPWSABUF);
typedef int     (WSAAPI *pfn_WSASendTo)        (SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,const sockaddr*,int,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef BOOL    (WSAAPI *pfn_WSASetEvent)      (WSAEVENT);
typedef void    (WSAAPI *pfn_WSASetLastError)  (int);
typedef SOCKET  (WSAAPI *pfn_WSASocketA)       (int,int,int,LPWSAPROTOCOL_INFO,GROUP,DWORD);
typedef SOCKET  (WSAAPI *pfn_WSASocketW)       (int,int,int,LPWSAPROTOCOL_INFO,GROUP,DWORD);
typedef int     (WSAAPI *pfn_WSAStartup)       (WORD,LPWSADATA);
typedef DWORD   (WSAAPI *pfn_WSAWaitForMultipleEvents)(DWORD,const WSAEVENT*,BOOL,DWORD,BOOL);

//  ****************************************************************************
/// Combines a function name with a function ptr.
/// 
struct LibraryFunction
{
  const char* name;
  PROC        pfn;
};

//  ****************************************************************************
/// MACRO declares an instance of the pointer mapping struct.
/// Usage:
///     LibraryFunction Hooks[] = 
///     {
///       WS2_ENTRY(FirstCall),
///       WS2_ENTRY(SecondCall)
///     };
///
/// Generates:
///     LibraryFunction Hooks[] = 
///     {
///       {"FirstCall", Hook_FirstCall},
///       {"SecondCall", Hook_SecondCall}
///     };
///
#define WS2_ENTRY(N) {#N, (PROC)Hook_##N}

//  ****************************************************************************
//  Hook Function Declarations *************************************************

SOCKET WSAAPI Hook_accept(
  SOCKET,
  sockaddr *addr,
  int *addrlen
);

BOOL WSAAPI Hook_AcceptEx(
  SOCKET sListenSocket,
  SOCKET sAcceptSocket,
  PVOID lpOutputBuffer,
  DWORD dwReceiveDataLength,
  DWORD dwLocalAddressLength,
  DWORD dwRemoteAddressLength,
  LPDWORD lpdwBytesReceived,
  LPOVERLAPPED lpOverlapped
);

int WSAAPI Hook_bind(
  SOCKET,
  const sockaddr*,
  int
);

int WSAAPI Hook_closesocket(
  SOCKET s
);

int WSAAPI Hook_connect(
  SOCKET,
  const sockaddr*,
  int
);

BOOL PASCAL Hook_ConnectEx(
  SOCKET,
  const sockaddr*,
  int,
  PVOID lpSendBuffer,
  DWORD dwSendDataLength,
  LPDWORD lpdwBytesSent,
  LPOVERLAPPED lpOverlapped
);

BOOL WSAAPI Hook_DisconnectEx(
  SOCKET hSocket,
  LPOVERLAPPED lpOverlapped,
  DWORD dwFlags,
  DWORD reserved
);

int WSAAPI Hook_getaddrinfo(
  PCSTR pNodeName,
  PCSTR pServiceName,
  const ADDRINFOA *pHints,
  PADDRINFOA *ppResult
);

int WSAAPI Hook_GetAddrInfoW(
  PCWSTR pNodeName,
  PCWSTR pServiceName,
  const ADDRINFOW *pHints,
  PADDRINFOW *ppResult
);

hostent* FAR Hook_gethostbyname(
  const char *name
);

int WSAAPI Hook_gethostname(
  char*,
  int
);

int WSAAPI Hook_GetHostNameW(
  PWSTR*,
  int
);

int WSAAPI Hook_getpeername(
  SOCKET,
  sockaddr*,
  int *namelen
);

int WSAAPI Hook_getsockname(
  SOCKET,
  sockaddr*,
  int *namelen
);

int WSAAPI Hook_getsockopt(
  SOCKET,
  int,
  int,
  char*,
  int*
);

int WSAAPI Hook_ioctlsocket(
  SOCKET,
  long,
  u_long*
);

int WSAAPI Hook_listen(
  SOCKET,
  int backlog
);

int WSAAPI Hook_recv(
  SOCKET,
  char*,
  int,
  int
);

int WSAAPI Hook_recvfrom(
  SOCKET,
  char*,
  int,
  int,
  sockaddr*,
  int *fromlen
);

int WSAAPI Hook_select(
  int nfds,
  fd_set *readfds,
  fd_set *writefds,
  fd_set *exceptfds,
  const timeval *timeout
);

int WSAAPI Hook_send(
  SOCKET,
  const char*,
  int,
  int
);

int WSAAPI Hook_sendto(
  SOCKET,
  const char*,
  int,
  int,
  const sockaddr *to,
  int tolen
);

int WSAAPI Hook_setsockopt(
  SOCKET,
  int,
  int,
  const char*,
  int optlen
);

int WSAAPI Hook_shutdown(
  SOCKET,
  int how
);

SOCKET WSAAPI Hook_socket(
  int af,
  int type,
  int protocol
);

SOCKET WSAAPI Hook_WSAAccept(
  SOCKET,
  sockaddr *addr,
  LPINT addrlen,
  LPCONDITIONPROC lpfnCondition,
  DWORD_PTR dwCallbackData
);

int WSAAPI Hook_WSAAsyncSelect(
  SOCKET,
  HWND hWnd,
  unsigned int wMsg,
  long lEvent
);

int WSAAPI Hook_WSACancelAsyncRequest(
  HANDLE hAsyncTaskHandle
);

int WSAAPI Hook_WSACleanup(void);

BOOL WSAAPI Hook_WSACloseEvent(
  WSAEVENT hEvent
);

int WSAAPI Hook_WSAConnect(
  SOCKET,
  const sockaddr*,
  int,
  LPWSABUF lpCallerData,
  LPWSABUF lpCalleeData,
  LPQOS lpSQOS,
  LPQOS lpGQOS
);

WSAEVENT WSAAPI Hook_WSACreateEvent(void);

int Hook_WSADuplicateSocket(
  SOCKET,
  DWORD dwProcessId,
  LPWSAPROTOCOL_INFO lpProtocolInfo
);

int WSAAPI Hook_WSAEventSelect(
  SOCKET,
  WSAEVENT hEventObject,
  long lNetworkEvents
);

int WSAAPI Hook_WSAGetLastError(void);

BOOL WSAAPI Hook_WSAGetOverlappedResult(
  SOCKET,
  LPWSAOVERLAPPED lpOverlapped,
  LPDWORD lpcbTransfer,
  BOOL fWait,
  LPDWORD lpdwFlags
);

int WSAAPI Hook_WSAIoctl(
  SOCKET,
  DWORD dwIoControlCode,
  LPVOID lpvInBuffer,
  DWORD cbInBuffer,
  LPVOID lpvOutBuffer,
  DWORD cbOutBuffer,
  LPDWORD lpcbBytesReturned,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSAAPI Hook_WSARecv(
  SOCKET,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesRecvd,
  LPDWORD lpFlags,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSAAPI Hook_WSARecvDisconnect(
  SOCKET,
  LPWSABUF lpInboundDisconnectData
);

int PASCAL FAR Hook_WSARecvEx(
  SOCKET,
  char*,
  int,
  int *flags
);

int WSAAPI Hook_WSARecvFrom(
  SOCKET,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesRecvd,
  LPDWORD lpFlags,
  sockaddr *lpFrom,
  LPINT lpFromlen,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

BOOL WSAAPI Hook_WSAResetEvent(
  WSAEVENT hEvent
);

int WSAAPI Hook_WSASend(
  SOCKET,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesSent,
  DWORD dwFlags,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSAAPI Hook_WSASendDisconnect(
  SOCKET,
  LPWSABUF lpOutboundDisconnectData
);

int WSAAPI Hook_WSASendTo(
  SOCKET,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesSent,
  DWORD dwFlags,
  const sockaddr *lpTo,
  int iToLen,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

BOOL WSAAPI Hook_WSASetEvent(
  WSAEVENT hEvent
);

void WSAAPI Hook_WSASetLastError(
  int iError
);

SOCKET WSAAPI Hook_WSASocketA(
  int af,
  int type,
  int protocol,
  LPWSAPROTOCOL_INFO lpProtocolInfo,
  GROUP g,
  DWORD dwFlags
);

SOCKET WSAAPI Hook_WSASocketW(
  int af,
  int type,
  int protocol,
  LPWSAPROTOCOL_INFO lpProtocolInfo,
  GROUP g,
  DWORD dwFlags
);

int WSAAPI Hook_WSAStartup(
  WORD wVersionRequested,
  LPWSADATA lpWSAData
);

DWORD WSAAPI Hook_WSAWaitForMultipleEvents(
  DWORD cEvents,
  const WSAEVENT *lphEvents,
  BOOL fWaitAll,
  DWORD dwTimeout,
  BOOL fAlertable
);

} // namespace cxxhook

#endif

#endif

