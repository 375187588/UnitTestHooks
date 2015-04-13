/// @file: BLSocketDef.h
///
/// Definitions and Includes that are required for network communication
/// through the bridge.
/// 
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// BLNetwork.h contains additional copyright information. 
///
//  ****************************************************************************
#ifndef BLSOCKETDEF_H_INCLUDED
#define BLSOCKETDEF_H_INCLUDED
//  Includes *******************************************************************
#include "../../../compiler.h"
#include <string>

#ifdef _WIN32
# if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#  error WinSock.h has already been included
# endif // defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
# include <winsock2.h>
# include <ws2tcpip.h>
# include <mswsock.h>
# if defined(BL_DEFAULT_LINKED_LIBS)
#  if defined(UNDER_CE)
#   pragma comment(lib, "ws2.lib")
#  elif defined(_MSC_VER)
#   pragma comment(lib, "ws2_32.lib")
#   pragma comment(lib, "mswsock.lib")
#  endif // defined(_MSC_VER)
# endif // !defined(BL_NO_DEFAULT_LINKED_LIBS)
#else
# include <sys/ioctl.h>
# include <sys/poll.h>
# include <sys/types.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <sys/uio.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <net/if.h>
# include <limits.h>
#endif

namespace cxxhook
{
namespace ipc
{
namespace detail
{

#ifdef _WIN32
typedef SOCKET                      socketType;
typedef sockaddr                    socketAddrType;
typedef in_addr                     in4AddrType;
typedef ip_mreq                     ip4MreqType;
typedef sockaddr_in                 sockaddrIn4Type;

const   SOCKET                      k_invalidSocket     = INVALID_SOCKET;
const   int                         k_socketError       = SOCKET_ERROR;
const   int                         k_maxAddrV4_strLen  = 256;
const   int                         k_maxAddrV6_strLen  = 256;

// Not sure if this will ever come into play, leave with BOOST def for reference.
# if defined(BOOST_ASIO_HAS_OLD_WIN_SDK)
typedef in6_addr_emulation          in6AddrType;
typedef ipv6_mreq_emulation         ip6MreqType;
typedef sockaddr_in6_emulation      sockaddrIn6Type;
typedef sockaddr_storage_emulation  sockaddrStorageType;
typedef addrinfo_emulation          addrinfoType;
# else
typedef in6_addr                    in6AddrType;
typedef ipv6_mreq                   ip6MreqType;
typedef sockaddr_in6                sockaddrIn6Type;
typedef sockaddr_storage            sockaddrStorageType;
typedef addrinfo                    addrInfoType;
# endif

typedef unsigned long               ioctlArgType;
typedef u_long                      ulongType;
typedef u_short                     ushortType;

const int                           k_shutdownReceive   = SD_RECEIVE;
const int                           k_shutdownSend      = SD_SEND;
const int                           k_shutdownBoth      = SD_BOTH;
const int                           k_messagePeek       = MSG_PEEK;
const int                           k_messageOutOfBand  = MSG_OOB;
const int                           k_messageDoNotRoute = MSG_DONTROUTE;

# if defined (_WIN32_WINNT)
const int                           k_maxIovLen = 64;
# else
const int                           k_maxIovLen = 16;
# endif

#else

typedef int                         socketType;
typedef sockaddr                    socketAddrType;
typedef in_addr                     in4AddrType;
typedef ip_mreq                     ip4MreqType;
typedef sockaddr_in                 sockaddrIn4Type;

const int                           k_invalidSocket     = -1;
const int                           k_socketError       = -1;
const int                           k_maxAddrV4_strLen  = INET_ADDRSTRLEN;
const int                           k_maxAddrV6_strLen  = INET6_ADDRSTRLEN + 1 + IF_NAMESIZE;

typedef in6_addr                    in6AddrType;
typedef ipv6_mreq                   ip6MreqType;
typedef sockaddr_in6                sockaddrIn6Type;
typedef sockaddr_storage            sockaddrStorageType;
typedef sockaddr_un                 sockaddrUnType;
typedef addrinfo                    addrInfoType;

typedef int                         ioctlArgType;
typedef uint32_t                    ulongType;
typedef uint16_t                    ushortType;

const int                           k_shutdownReceive   = SHUT_RD;
const int                           k_shutdownSend      = SHUT_WR;
const int                           k_shutdownBoth      = SHUT_RDWR;
const int                           k_messagePeek       = MSG_PEEK;
const int                           k_messageOutOfBand  = MSG_OOB;
const int                           k_messageDoNotRout  = MSG_DONTROUTE;

# if defined(IOV_MAX)
const int                           k_maxIovLen         = IOV_MAX;
# else
// POSIX platforms are not required to define IOV_MAX.
const int                           k_maxIovLen         = 16;
# endif

#endif

const int k_customSocketOptionLevel       = 0xA5100000;
const int k_enableConnectionAbortedOption = 1;
const int k_alwaysFailOption              = 2;

} // namespace detail
} // namespace ipc

typedef unsigned long  status;

const unsigned long k_noError = 0;

/* Socket Based Errors *******************************************************/
namespace error
{
const unsigned long k_socketBaseError          = 0x00002710L;               // 10000 
      
const unsigned long k_blockingCallInterupted   = k_socketBaseError +  4L;   // WSAEINTR
const unsigned long k_badFileHandle            = k_socketBaseError +  9L;   // WSAEBADF
const unsigned long k_socketAccessError        = k_socketBaseError + 13L;   // WSAEACCES
const unsigned long k_socketFault              = k_socketBaseError + 14L;   // WSAEFAULT
const unsigned long k_invalidArguement         = k_socketBaseError + 22L;   // WSAEINVAL
const unsigned long k_socketWouldBlock         = k_socketBaseError + 35L;   // WSAEWOULDBLOCK
const unsigned long k_socketInProgress         = k_socketBaseError + 36L;   // WSAEINPROGRESS
const unsigned long k_socketAlready            = k_socketBaseError + 37L;   // WSAEALREADY
const unsigned long k_socketNotSocket          = k_socketBaseError + 38L;   // WSAENOTSOCK
const unsigned long k_socketDestAddressReq     = k_socketBaseError + 39L;   // WSAEDESTADDRREQ
const unsigned long k_socketMsgSize            = k_socketBaseError + 40L;   // WSAEMSGSIZE
const unsigned long k_socketPrototype          = k_socketBaseError + 41L;   // WSAEPROTOTYPE
const unsigned long k_socketProtoOpt           = k_socketBaseError + 42L;   // WSAENOPROTOOPT
const unsigned long k_socketProtoNoSupport     = k_socketBaseError + 43L;   // WSAEPROTONOSUPPORT
const unsigned long k_socketSocketNoSupport    = k_socketBaseError + 44L;   // WSAESOCKTNOSUPPORT
const unsigned long k_socketOpNotSupported     = k_socketBaseError + 45L;   // WSAEOPNOTSUPP
const unsigned long k_socketPFNoSupport        = k_socketBaseError + 46L;   // WSAEPFNOSUPPORT
const unsigned long k_socketFNNoSupport        = k_socketBaseError + 47L;   // WSAEAFNOSUPPORT
const unsigned long k_socketAddressInUse       = k_socketBaseError + 48L;   // WSAEADDRINUSE
const unsigned long k_socketAddressNotAvail    = k_socketBaseError + 49L;   // WSAEADDRNOTAVAIL
const unsigned long k_socketNetDown            = k_socketBaseError + 50L;   // WSAENETDOWN
const unsigned long k_socketNetUnreachabl      = k_socketBaseError + 51L;   // WSAENETUNREACH
const unsigned long k_socketNetReset           = k_socketBaseError + 52L;   // WSAENETRESET
const unsigned long k_socketConnAborted        = k_socketBaseError + 53L;   // WSAECONNABORTED
const unsigned long k_socketConnReset          = k_socketBaseError + 54L;   // WSAECONNRESET
const unsigned long k_socketNoBuffers          = k_socketBaseError + 55L;   // WSAENOBUFS
const unsigned long k_socketIsConnected        = k_socketBaseError + 56L;   // WSAEISCONN
const unsigned long k_socketNotConnected       = k_socketBaseError + 57L;   // WSAENOTCONN
const unsigned long k_socketShutdown           = k_socketBaseError + 58L;   // WSAESHUTDOWN
const unsigned long k_socketTooManyRefs        = k_socketBaseError + 59L;   // WSAETOOMANYREFS
const unsigned long k_socketTimedOut           = k_socketBaseError + 60L;   // WSAETIMEDOUT
const unsigned long k_socketConnRefused        = k_socketBaseError + 61L;   // WSAECONNREFUSED
const unsigned long k_socketLoop               = k_socketBaseError + 62L;   // WSAELOOP
const unsigned long k_socketNameTooLong        = k_socketBaseError + 63L;   // WSAENAMETOOLONG
const unsigned long k_socketHostDown           = k_socketBaseError + 64L;   // WSAEHOSTDOWN
const unsigned long k_socketHostUnreachable    = k_socketBaseError + 65L;   // WSAEHOSTUNREACH
const unsigned long k_socketNotEmpty           = k_socketBaseError + 66L;   // WSAENOTEMPTY
const unsigned long k_socketProcLimit          = k_socketBaseError + 67L;   // WSAEPROCLIM
const unsigned long k_socketUsers              = k_socketBaseError + 68L;   // WSAEUSERS
const unsigned long k_socketDiskQuota          = k_socketBaseError + 69L;   // WSAEDQUOT
const unsigned long k_socketStale              = k_socketBaseError + 70L;   // WSAESTALE
const unsigned long k_socketRemote             = k_socketBaseError + 71L;   // WSAEREMOTE

// Extended error from Windows Sockets
const unsigned long k_socketSysNotReady        = k_socketBaseError + 91L;   // WSASYSNOTREADY
const unsigned long k_socketVerNotSupported    = k_socketBaseError + 92L;   // WSAVERNOTSUPPORTED
const unsigned long k_socketNotInitialized     = k_socketBaseError + 93L;   // WSANOTINITIALISED
const unsigned long k_socketDisconnected       = k_socketBaseError + 101L;  // WSAEDISCON
const unsigned long k_socketNoMore             = k_socketBaseError + 102L;  // WSAENOMORE
const unsigned long k_socketCancelled          = k_socketBaseError + 103L;  // WSAECANCELLED
const unsigned long k_socketInvalidProcTable   = k_socketBaseError + 104L;  // WSAEINVALIDPROCTABLE
const unsigned long k_socketInvalidProvider    = k_socketBaseError + 105L;  // WSAEINVALIDPROVIDER
const unsigned long k_socketProviderFailedInit = k_socketBaseError + 106L;  // WSAEPROVIDERFAILEDINIT
const unsigned long k_socketSysCallFailure     = k_socketBaseError + 107L;  // WSASYSCALLFAILURE
const unsigned long k_socketServiceNotFound    = k_socketBaseError + 108L;  // WSASERVICE_NOT_FOUND
const unsigned long k_socketTypeNotFound       = k_socketBaseError + 109L;  // WSATYPE_NOT_FOUND
const unsigned long k_socketNoMore2            = k_socketBaseError + 110L;  // WSA_E_NO_MORE
const unsigned long k_socketCancelled2         = k_socketBaseError + 111L;  // WSA_E_CANCELLED
const unsigned long k_socketRefused            = k_socketBaseError + 112L;  // WSAEREFUSED
const unsigned long k_socketHostNotFound       = k_socketBaseError + 1001L; // WSAHOST_NOT_FOUND
const unsigned long k_socketTryAgain           = k_socketBaseError + 1002L; // WSATRY_AGAIN
const unsigned long k_socketRecovery           = k_socketBaseError + 1003L; // WSANO_RECOVERY
const unsigned long k_socketNoData             = k_socketBaseError + 1004L; // WSANO_DATA
} // namespace error
} // namespace cxxhook


#endif //BLSOCKETDEF_H_INCLUDED
