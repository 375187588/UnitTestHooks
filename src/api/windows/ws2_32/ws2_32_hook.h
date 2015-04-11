/// @file   ws2_32_hook.h 
/// 
/// API Hook library for unit-testing with Windows Socket dependencies.
///
/// From the callers point-of-view, when the hooks are active for this class, 
/// the library calls will behave as the original library behaves.
/// However, all memory management will be handled internally and no socket
/// communication will occur.
///
/// Usage:
///       // Create a single instance of the class for the life of the test-suite.
///       WS2_32_hook   winsock_hook;
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
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "../../../ApiHook.h"

#include <vector>


namespace cxxhook
{

// TODO: Create function pointer declarations for each of the APIs to be hooked.
// TODO: Create a state tracking implementation to emulate data transfer
// TODO: Create a way to selectively activate API hooks
// TODO: Create a way to specify the return value/behavior of an API


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
class WS2_32_hook
{
public:
  //  Construction *************************************************************
  WS2_32_hook();
  ~WS2_32_hook();

  //  Hook Control / Status ****************************************************
  void hook_all();
  void unhook_all();

  void hook();
  void unhook();

  bool is_hooked();

  //  Test Facilities **********************************************************
  void reset();

  //  Library Functions ********************************************************

SOCKET accept(
  _In_     SOCKET s,
  _Out_    struct sockaddr *addr,
  _Inout_  int *addrlen
);

BOOL AcceptEx(
  _In_   SOCKET sListenSocket,
  _In_   SOCKET sAcceptSocket,
  _In_   PVOID lpOutputBuffer,
  _In_   DWORD dwReceiveDataLength,
  _In_   DWORD dwLocalAddressLength,
  _In_   DWORD dwRemoteAddressLength,
  _Out_  LPDWORD lpdwBytesReceived,
  _In_   LPOVERLAPPED lpOverlapped
);


int bind(
  _In_  SOCKET s,
  _In_  const struct sockaddr *name,
  _In_  int namelen
);

int closesocket(
  _In_  SOCKET s
);

int connect(
  _In_  SOCKET s,
  _In_  const struct sockaddr *name,
  _In_  int namelen
);

BOOL PASCAL ConnectEx(
  _In_      SOCKET s,
  _In_      const struct sockaddr *name,
  _In_      int namelen,
  _In_opt_  PVOID lpSendBuffer,
  _In_      DWORD dwSendDataLength,
  _Out_     LPDWORD lpdwBytesSent,
  _In_      LPOVERLAPPED lpOverlapped
);

BOOL DisconnectEx(
  _In_  SOCKET hSocket,
  _In_  LPOVERLAPPED lpOverlapped,
  _In_  DWORD dwFlags,
  _In_  DWORD reserved
);

int WSAAPI getaddrinfo(
  _In_opt_  PCSTR pNodeName,
  _In_opt_  PCSTR pServiceName,
  _In_opt_  const ADDRINFOA *pHints,
  _Out_     PADDRINFOA *ppResult
);

int WSAAPI GetAddrInfoW(
  _In_opt_  PCWSTR pNodeName,
  _In_opt_  PCWSTR pServiceName,
  _In_opt_  const ADDRINFOW *pHints,
  _Out_     PADDRINFOW *ppResult
);

struct hostent* FAR gethostbyname(
  _In_  const char *name
);

int gethostname(
  _Out_  char *name,
  _In_   int namelen
);

int WSAAPI GetHostNameW(
  _Out_  PWSTR *name,
  _In_   int namelen
);

int getpeername(
  _In_     SOCKET s,
  _Out_    struct sockaddr *name,
  _Inout_  int *namelen
);

int getsockname(
  _In_     SOCKET s,
  _Out_    struct sockaddr *name,
  _Inout_  int *namelen
);

int getsockopt(
  _In_     SOCKET s,
  _In_     int level,
  _In_     int optname,
  _Out_    char *optval,
  _Inout_  int *optlen
);

int ioctlsocket(
  _In_     SOCKET s,
  _In_     long cmd,
  _Inout_  u_long *argp
);

int listen(
  _In_  SOCKET s,
  _In_  int backlog
);

int recv(
  _In_   SOCKET s,
  _Out_  char *buf,
  _In_   int len,
  _In_   int flags
);

int recvfrom(
  _In_         SOCKET s,
  _Out_        char *buf,
  _In_         int len,
  _In_         int flags,
  _Out_        struct sockaddr *from,
  _Inout_opt_  int *fromlen
);

int select(
  _In_     int nfds,
  _Inout_  fd_set *readfds,
  _Inout_  fd_set *writefds,
  _Inout_  fd_set *exceptfds,
  _In_     const struct timeval *timeout
);

int send(
  _In_  SOCKET s,
  _In_  const char *buf,
  _In_  int len,
  _In_  int flags
);

int sendto(
  _In_  SOCKET s,
  _In_  const char *buf,
  _In_  int len,
  _In_  int flags,
  _In_  const struct sockaddr *to,
  _In_  int tolen
);

int setsockopt(
  _In_  SOCKET s,
  _In_  int level,
  _In_  int optname,
  _In_  const char *optval,
  _In_  int optlen
);

int shutdown(
  _In_  SOCKET s,
  _In_  int how
);

SOCKET WSAAPI socket(
  _In_  int af,
  _In_  int type,
  _In_  int protocol
);


SOCKET WSAAccept(
  _In_     SOCKET s,
  _Out_    struct sockaddr *addr,
  _Inout_  LPINT addrlen,
  _In_     LPCONDITIONPROC lpfnCondition,
  _In_     DWORD_PTR dwCallbackData
);

int WSAAsyncSelect(
  _In_  SOCKET s,
  _In_  HWND hWnd,
  _In_  unsigned int wMsg,
  _In_  long lEvent
);

int WSACancelAsyncRequest(
  _In_  HANDLE hAsyncTaskHandle
);

int WSACleanup(void);

BOOL WSACloseEvent(
  _In_  WSAEVENT hEvent
);

int WSAConnect(
  _In_   SOCKET s,
  _In_   const struct sockaddr *name,
  _In_   int namelen,
  _In_   LPWSABUF lpCallerData,
  _Out_  LPWSABUF lpCalleeData,
  _In_   LPQOS lpSQOS,
  _In_   LPQOS lpGQOS
);

WSAEVENT WSACreateEvent(void);

int WSADuplicateSocket(
  _In_   SOCKET s,
  _In_   DWORD dwProcessId,
  _Out_  LPWSAPROTOCOL_INFO lpProtocolInfo
);

int WSAEventSelect(
  _In_  SOCKET s,
  _In_  WSAEVENT hEventObject,
  _In_  long lNetworkEvents
);

int WSAGetLastError(void);

BOOL WSAAPI WSAGetOverlappedResult(
  _In_   SOCKET s,
  _In_   LPWSAOVERLAPPED lpOverlapped,
  _Out_  LPDWORD lpcbTransfer,
  _In_   BOOL fWait,
  _Out_  LPDWORD lpdwFlags
);

int WSAIoctl(
  _In_   SOCKET s,
  _In_   DWORD dwIoControlCode,
  _In_   LPVOID lpvInBuffer,
  _In_   DWORD cbInBuffer,
  _Out_  LPVOID lpvOutBuffer,
  _In_   DWORD cbOutBuffer,
  _Out_  LPDWORD lpcbBytesReturned,
  _In_   LPWSAOVERLAPPED lpOverlapped,
  _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSARecv(
  _In_     SOCKET s,
  _Inout_  LPWSABUF lpBuffers,
  _In_     DWORD dwBufferCount,
  _Out_    LPDWORD lpNumberOfBytesRecvd,
  _Inout_  LPDWORD lpFlags,
  _In_     LPWSAOVERLAPPED lpOverlapped,
  _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSARecvDisconnect(
  _In_   SOCKET s,
  _Out_  LPWSABUF lpInboundDisconnectData
);

int PASCAL FAR WSARecvEx(
  _In_     SOCKET s,
  _Out_    char *buf,
  _In_     int len,
  _Inout_  int *flags
);

int WSARecvFrom(
  _In_     SOCKET s,
  _Inout_  LPWSABUF lpBuffers,
  _In_     DWORD dwBufferCount,
  _Out_    LPDWORD lpNumberOfBytesRecvd,
  _Inout_  LPDWORD lpFlags,
  _Out_    struct sockaddr *lpFrom,
  _Inout_  LPINT lpFromlen,
  _In_     LPWSAOVERLAPPED lpOverlapped,
  _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSARecvMsg(
  _In_     SOCKET s,
  _Inout_  LPWSAMSG lpMsg,
  _Out_    LPDWORD lpdwNumberOfBytesRecvd,
  _In_     LPWSAOVERLAPPED lpOverlapped,
  _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

BOOL WSAResetEvent(
  _In_  WSAEVENT hEvent
);

int WSASend(
  _In_   SOCKET s,
  _In_   LPWSABUF lpBuffers,
  _In_   DWORD dwBufferCount,
  _Out_  LPDWORD lpNumberOfBytesSent,
  _In_   DWORD dwFlags,
  _In_   LPWSAOVERLAPPED lpOverlapped,
  _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

int WSASendDisconnect(
  _In_  SOCKET s,
  _In_  LPWSABUF lpOutboundDisconnectData
);

int WSASendTo(
  _In_   SOCKET s,
  _In_   LPWSABUF lpBuffers,
  _In_   DWORD dwBufferCount,
  _Out_  LPDWORD lpNumberOfBytesSent,
  _In_   DWORD dwFlags,
  _In_   const struct sockaddr *lpTo,
  _In_   int iToLen,
  _In_   LPWSAOVERLAPPED lpOverlapped,
  _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

BOOL WSASetEvent(
  _In_  WSAEVENT hEvent
);

void WSASetLastError(
  _In_  int iError
);

SOCKET WSASocket(
  _In_  int af,
  _In_  int type,
  _In_  int protocol,
  _In_  LPWSAPROTOCOL_INFO lpProtocolInfo,
  _In_  GROUP g,
  _In_  DWORD dwFlags
);

int WSAStartup(
  _In_   WORD wVersionRequested,
  _Out_  LPWSADATA lpWSAData
);

DWORD WSAWaitForMultipleEvents(
  _In_  DWORD cEvents,
  _In_  const WSAEVENT *lphEvents,
  _In_  BOOL fWaitAll,
  _In_  DWORD dwTimeout,
  _In_  BOOL fAlertable
);

private:

};

} // namespace cxxhook

#endif

#endif



//SOCKET accept(
//BOOL AcceptEx(
//int bind(
//int closesocket(
//int connect(
//BOOL PASCAL ConnectEx(
//BOOL DisconnectEx(
//int WSAAPI getaddrinfo(
//int WSAAPI GetAddrInfoW(
//struct hostent* FAR gethostbyname(
//int gethostname(
//int WSAAPI GetHostNameW(
//int getpeername(
//int getsockname(
//int getsockopt(
//int ioctlsocket(
//int listen(
//int recv(
//int recvfrom(
//int select(
//int send(
//int sendto(
//int setsockopt(
//int shutdown(
//SOCKET WSAAPI socket(
//SOCKET WSAAccept(
//int WSAAsyncSelect(
//int WSACancelAsyncRequest(
//int WSACleanup(void);
//BOOL WSACloseEvent(
//int WSAConnect(
//WSAEVENT WSACreateEvent(void);
//int WSADuplicateSocket(
//int WSAEventSelect(
//int WSAGetLastError(void);
//BOOL WSAAPI WSAGetOverlappedResult(
//int WSAIoctl(
//int WSARecv(
//int WSARecvDisconnect(
//int PASCAL FAR WSARecvEx(
//int WSARecvFrom(
//int WSARecvMsg(
//BOOL WSAResetEvent(
//int WSASend(
//int WSASendDisconnect(
//int WSASendTo(
//BOOL WSASetEvent(
//void WSASetLastError(
//SOCKET WSASocket(
//int WSAStartup(
//DWORD WSAWaitForMultipleEvents(
