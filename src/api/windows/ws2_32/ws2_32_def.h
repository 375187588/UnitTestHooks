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

//  Forward Declarations *******************************************************
class ApiHook;

namespace cxxhook
{

//  Function Pointers for Supported Library Hooks ****************************

typedef SOCKET  (*pfn_accept)           (SOCKET, sockaddr*, int *);
typedef BOOL    (*pfn_AcceptEx)         (SOCKET,SOCKET,PVOID,DWORD,DWORD,DWORD,LPDWORD,LPOVERLAPPED);
typedef int     (*pfn_bind)             (SOCKET,const sockaddr*,int);
typedef int     (*pfn_closesocket)      (SOCKET);
typedef int     (*pfn_connect)          (SOCKET,const sockaddr*,int);
typedef BOOL    (PASCAL *pfn_ConnectEx) (SOCKET,const sockaddr *,int,PVOID,DWORD,LPDWORD,LPOVERLAPPED);
typedef BOOL    (*pfn_DisconnectEx)     (SOCKET,LPOVERLAPPED,DWORD,DWORD);
typedef int     (WSAAPI *pfn_getaddrinfo)(PCSTR,PCSTR,const ADDRINFOA *,PADDRINFOA *);
typedef int     (WSAAPI *pfn_GetAddrInfoW)(PCWSTR,PCWSTR,const ADDRINFOW *,PADDRINFOW *);
typedef hostent*(FAR *pfn_gethostbyname)(const char *);

typedef int     (*pfn_gethostname)      (char*,int);
typedef int     (WSAAPI *pfn_GetHostNameW)(PWSTR*,int);
typedef int     (*pfn_getpeername)      (SOCKET,sockaddr*,int *);
typedef int     (*pfn_getsockname)      (SOCKET,sockaddr*,int *);
typedef int     (*pfn_getsockopt)       (SOCKET,int,int,char*,int*);
typedef int     (*pfn_ioctlsocket)      (SOCKET,long,u_long*);
typedef int     (*pfn_listen)           (SOCKET,int);
typedef int     (*pfn_recv)             (SOCKET,char*,int,int);
typedef int     (*pfn_recvfrom)         (SOCKET,char*,int,int,sockaddr*,int*);
typedef int     (*pfn_select)           (int,fd_set*,fd_set*,fd_set*,const timeval*);

typedef int     (*pfn_send)             (SOCKET,const char*,int,int);
typedef int     (*pfn_sendto)           (SOCKET,const char*,int,int,const sockaddr*,int);
typedef int     (*pfn_setsockopt)       (SOCKET,int,int,const char*,int);
typedef int     (*pfn_shutdown)         (SOCKET,int);
typedef SOCKET  (WSAAPI *pfn_socket)    (int,int,int);
typedef SOCKET  (*pfn_WSAAccept)        (SOCKET,sockaddr*,LPINT,LPCONDITIONPROC,DWORD_PTR);
typedef int     (*pfn_WSAAsyncSelect)   (SOCKET,HWND,unsigned int,long);
typedef int     (*pfn_WSACancelAsyncRequest)(HANDLE);
typedef int     (*pfn_WSACleanup)       (void);
typedef BOOL    (*pfn_WSACloseEvent)    (WSAEVENT);

typedef int     (*pfn_WSAConnect)       (SOCKET,const sockaddr*,int,LPWSABUF,LPWSABUF,LPQOS,LPQOS);
typedef WSAEVENT(*pfn_WSACreateEvent)   (void);
typedef int     (*pfn_WSADuplicateSocket)(SOCKET,DWORD,LPWSAPROTOCOL_INFO);
typedef int     (*pfn_WSAEventSelect)   (SOCKET,WSAEVENT,long);
typedef int     (*pfn_WSAGetLastError)  (void);
typedef BOOL    (WSAAPI *pfn_WSAGetOverlappedResult)(SOCKET,LPWSAOVERLAPPED,LPDWORD,BOOL,LPDWORD);
typedef int     (*pfn_WSAIoctl)         (SOCKET,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int     (*pfn_WSARecv)          (SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int     (*pfn_WSARecvDisconnect)(SOCKET,LPWSABUF);
typedef int     (PASCAL FAR *pfn_WSARecvEx)(SOCKET,char*,int,int*);

typedef int     (*pfn_WSARecvFrom)      (SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,sockaddr*,LPINT,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef BOOL    (*pfn_WSAResetEvent)    (WSAEVENT);
typedef int     (*pfn_WSASend)          (SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int     (*pfn_WSASendDisconnect)(SOCKET,LPWSABUF);
typedef int     (*pfn_WSASendTo)        (SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,const sockaddr*,int,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef BOOL    (*pfn_WSASetEvent)      (WSAEVENT);
typedef void    (*pfn_WSASetLastError)  (int);
typedef SOCKET  (*pfn_WSASocket)        (int,int,int,LPWSAPROTOCOL_INFO,GROUP,DWORD);
typedef int     (*pfn_WSAStartup)       (WORD,LPWSADATA);
typedef DWORD   (*pfn_WSAWaitForMultipleEvents)(DWORD,const WSAEVENT*,BOOL,DWORD,BOOL);

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
///     LibraryFunction[] = 
///     {
///       WS2_ENTRY(FirstCall),
///       WS2_ENTRY(SecondCall)
///     };
///
/// Generates:
///     LibraryFunction[] = 
///     {
///       {"FirstCall", pfn_FirstCall},
///       {"SecondCall", pfn_SecondCall}
///     };
///
#define WS2_ENTRY(N) {#N, pfn_##N}

//  ****************************************************************************
//  Original Function Declarations *********************************************

//SOCKET accept(
//  SOCKET,
//   sockaddr *addr,
//  int *addrlen
//);
//
//BOOL AcceptEx(
//  SOCKET sListenSocket,
//  SOCKET sAcceptSocket,
//  PVOID lpOutputBuffer,
//  DWORD dwReceiveDataLength,
//  DWORD dwLocalAddressLength,
//  DWORD dwRemoteAddressLength,
//  LPDWORD lpdwBytesReceived,
//  LPOVERLAPPED lpOverlapped
//);
//
//
//int bind(
//  SOCKET,
//  const struct sockaddr*,
//  int
//);
//
//int closesocket(
//  SOCKET s
//);
//
//int connect(
//  SOCKET,
//  const struct sockaddr*,
//  int
//);
//
//BOOL PASCAL ConnectEx(
//  SOCKET,
//  const struct sockaddr*,
//  int,
//  PVOID lpSendBuffer,
//  DWORD dwSendDataLength,
//  LPDWORD lpdwBytesSent,
//  LPOVERLAPPED lpOverlapped
//);
//
//BOOL DisconnectEx(
//  SOCKET hSocket,
//  LPOVERLAPPED lpOverlapped,
//  DWORD dwFlags,
//  DWORD reserved
//);
//
//int WSAAPI getaddrinfo(
//  PCSTR pNodeName,
//  PCSTR pServiceName,
//  const ADDRINFOA *pHints,
//  PADDRINFOA *ppResult
//);
//
//int WSAAPI GetAddrInfoW(
//  PCWSTR pNodeName,
//  PCWSTR pServiceName,
//  const ADDRINFOW *pHints,
//  PADDRINFOW *ppResult
//);
//
//struct hostent* FAR gethostbyname(
//  const char *name
//);
//
//int gethostname(
//  char*,
//  int
//);
//
//int WSAAPI GetHostNameW(
//  PWSTR*,
//  int
//);
//
//int getpeername(
//  SOCKET,
//  struct sockaddr*,
//  int *namelen
//);
//
//int getsockname(
//  SOCKET,
//  struct sockaddr*,
//  int *namelen
//);
//
//int getsockopt(
//  SOCKET,
//  int,
//  int,
//  char*,
//  int*
//);
//
//int ioctlsocket(
//  SOCKET,
//  long,
//  u_long*
//);
//
//int listen(
//  SOCKET,
//  int backlog
//);
//
//int recv(
//  SOCKET,
//  char*,
//  int,
//  int
//);
//
//int recvfrom(
//  SOCKET,
//  char*,
//  int,
//  int,
//  struct sockaddr*,
//  int *fromlen
//);
//
//int select(
//  int nfds,
//  fd_set *readfds,
//  fd_set *writefds,
//  fd_set *exceptfds,
//  const struct timeval *timeout
//);
//
//int send(
//  SOCKET,
//  const char*,
//  int,
//  int
//);
//
//int sendto(
//  SOCKET,
//  const char*,
//  int,
//  int,
//  const struct sockaddr *to,
//  int tolen
//);
//
//int setsockopt(
//  SOCKET,
//  int,
//  int,
//  const char*,
//  int optlen
//);
//
//int shutdown(
//  SOCKET,
//  int how
//);
//
//SOCKET WSAAPI socket(
//  int af,
//  int type,
//  int protocol
//);
//
//
//SOCKET WSAAccept(
//  SOCKET,
//  struct sockaddr *addr,
//  LPINT addrlen,
//  LPCONDITIONPROC lpfnCondition,
//  DWORD_PTR dwCallbackData
//);
//
//int WSAAsyncSelect(
//  SOCKET,
//  HWND hWnd,
//  unsigned int wMsg,
//  long lEvent
//);
//
//int WSACancelAsyncRequest(
//  HANDLE hAsyncTaskHandle
//);
//
//int WSACleanup(void);
//
//BOOL WSACloseEvent(
//  WSAEVENT hEvent
//);
//
//int WSAConnect(
//  SOCKET,
//  const struct sockaddr*,
//  int,
//  LPWSABUF lpCallerData,
//  LPWSABUF lpCalleeData,
//  LPQOS lpSQOS,
//  LPQOS lpGQOS
//);
//
//WSAEVENT WSACreateEvent(void);
//
//int WSADuplicateSocket(
//  SOCKET,
//  DWORD dwProcessId,
//  LPWSAPROTOCOL_INFO lpProtocolInfo
//);
//
//int WSAEventSelect(
//  SOCKET,
//  WSAEVENT hEventObject,
//  long lNetworkEvents
//);
//
//int WSAGetLastError(void);
//
//BOOL WSAAPI WSAGetOverlappedResult(
//  SOCKET,
//  LPWSAOVERLAPPED lpOverlapped,
//  LPDWORD lpcbTransfer,
//  BOOL fWait,
//  LPDWORD lpdwFlags
//);
//
//int WSAIoctl(
//  SOCKET,
//  DWORD dwIoControlCode,
//  LPVOID lpvInBuffer,
//  DWORD cbInBuffer,
//  LPVOID lpvOutBuffer,
//  DWORD cbOutBuffer,
//  LPDWORD lpcbBytesReturned,
//  LPWSAOVERLAPPED lpOverlapped,
//  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
//);
//
//int WSARecv(
//  SOCKET,
//  LPWSABUF lpBuffers,
//  DWORD dwBufferCount,
//  LPDWORD lpNumberOfBytesRecvd,
//  LPDWORD lpFlags,
//  LPWSAOVERLAPPED lpOverlapped,
//  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
//);
//
//int WSARecvDisconnect(
//  SOCKET,
//  LPWSABUF lpInboundDisconnectData
//);
//
//int PASCAL FAR WSARecvEx(
//  SOCKET,
//  char*,
//  int,
//  int *flags
//);
//
//int WSARecvFrom(
//  SOCKET,
//  LPWSABUF lpBuffers,
//  DWORD dwBufferCount,
//  LPDWORD lpNumberOfBytesRecvd,
//  LPDWORD lpFlags,
//  struct sockaddr *lpFrom,
//  LPINT lpFromlen,
//  LPWSAOVERLAPPED lpOverlapped,
//  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
//);
//
//BOOL WSAResetEvent(
//  WSAEVENT hEvent
//);
//
//int WSASend(
//  SOCKET,
//  LPWSABUF lpBuffers,
//  DWORD dwBufferCount,
//  LPDWORD lpNumberOfBytesSent,
//  DWORD dwFlags,
//  LPWSAOVERLAPPED lpOverlapped,
//  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
//);
//
//int WSASendDisconnect(
//  SOCKET,
//  LPWSABUF lpOutboundDisconnectData
//);
//
//int WSASendTo(
//  SOCKET,
//  LPWSABUF lpBuffers,
//  DWORD dwBufferCount,
//  LPDWORD lpNumberOfBytesSent,
//  DWORD dwFlags,
//  const struct sockaddr *lpTo,
//  int iToLen,
//  LPWSAOVERLAPPED lpOverlapped,
//  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
//);
//
//BOOL WSASetEvent(
//  WSAEVENT hEvent
//);
//
//void WSASetLastError(
//  int iError
//);
//
//SOCKET WSASocket(
//  int af,
//  int type,
//  int protocol,
//  LPWSAPROTOCOL_INFO lpProtocolInfo,
//  GROUP g,
//  DWORD dwFlags
//);
//
//int WSAStartup(
//  WORD wVersionRequested,
//  LPWSADATA lpWSAData
//);
//
//DWORD WSAWaitForMultipleEvents(
//  DWORD cEvents,
//  const WSAEVENT *lphEvents,
//  BOOL fWaitAll,
//  DWORD dwTimeout,
//  BOOL fAlertable
//);

} // namespace cxxhook

#endif

#endif

