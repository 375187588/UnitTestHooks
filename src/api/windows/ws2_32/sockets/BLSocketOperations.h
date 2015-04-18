/// @file   BLSocketOperations.h 
/// 
/// Definitions and Includes that are required for network communication
/// through the bridge.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#ifndef BLSOCKETOPERATIONS_H_INCLUDED
#define BLSOCKETOPERATIONS_H_INCLUDED
//  Includes *******************************************************************
#include "BLSocketDef.h"

namespace cxxhook
{
namespace ipc
{
namespace detail
{

//  ****************************************************************************
inline int GetLastSocketError()
{
#ifdef _WIN32
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

//  ****************************************************************************
inline void SetLastSocketError(int error)
{
#ifdef _WIN32
  ::WSASetLastError(error);
#else
  errno = error;
#endif
}

//  ****************************************************************************
inline ulongType HostToNetworkLong(ulongType hostLong)
{
  return ::htonl(hostLong);
}

//  ****************************************************************************
inline ulongType NetworkToHostLong(ulongType networkLong)
{
  return ::ntohl(networkLong);
}

//  ****************************************************************************
inline ushortType HostToNetworkShort(ushortType hostShort)
{
  return ::htons(hostShort);
}

//  ****************************************************************************
inline ushortType NetworkToHostShort(ushortType networkShort)
{
  return ::ntohs(networkShort);
}

/*****************************************************************************/
inline const char* inet_ntop(int af, 
                             const void* src, 
                             char* dest, 
                             const size_t length,
                             unsigned long scope_id,
                             unsigned long &status)
{
#ifdef _WIN32
  status = cxxhook::k_noError;

  if ( AF_INET != af 
    && AF_INET6 != af)
  {
    status = cxxhook::error::k_socketFNNoSupport;
    SetLastSocketError(status);
    return 0;
  }

  union
  {
    socketAddrType base;
    sockaddrStorageType storage;
    sockaddrIn4Type v4;
    sockaddrIn6Type v6;
  } address;

  DWORD addressLength;
  if (AF_INET == af)
  {
    addressLength = sizeof(sockaddrIn4Type);
    address.v4.sin_family = AF_INET;
    address.v4.sin_port = 0;
    ::memcpy(&address.v4.sin_addr, src, sizeof(in4AddrType));
  }
  else // AF_INET6
  {
    addressLength = sizeof(sockaddrIn6Type);
    address.v6.sin6_family = AF_INET6;
    address.v6.sin6_port = 0;
    address.v6.sin6_flowinfo = 0;
    address.v6.sin6_scope_id = scope_id;
    ::memcpy(&address.v6.sin6_addr, src, sizeof(in6AddrType));
  }

  DWORD stringLength = static_cast<DWORD>(length);
  CHAR buffer[cxxhook::ipc::detail::k_maxAddrV4_strLen];
  int result = ::WSAAddressToString(&address.base, addressLength, 0, buffer, &stringLength);
  if (k_socketError != result)
  {
    SetLastSocketError(cxxhook::k_noError);
    ::strncpy(dest, buffer, length);
  }
  else if (k_socketError == result)
  {
    status = cxxhook::error::k_invalidArgument;
  }

  return result == k_socketError ? 0 : dest;
#else
  const char* result = ::inet_ntop(af, src, dest, length);
  if (result == 0)
    status = cxxhook::error::k_invalidArguement;

  if ( 0 != result
    && AF_INET6 == af
    && 0 != scope_id)
  {
    char if_name[IF_NAMESIZE + 1] = "%";
    const in6_addr_type* ipv6_address = static_cast<const in6AddrType*>(src);
    bool is_link_local = IN6_IS_ADDR_LINKLOCAL(ipv6_address);
    if (!is_link_local || if_indextoname(scope_id, if_name + 1) == 0)
      ::sprintf(if_name + 1, "%lu", scope_id);
    ::strcat(dest, if_name);
  }
  return result;
#endif // _WIN32
}

//  ****************************************************************************
inline int inet_pton(int af, 
                     const char* src, 
                     void* dest,
                     unsigned long* scope_id, 
                     unsigned long &status)
{
#ifdef _WIN32
  status = cxxhook::k_noError;

  if ( AF_INET != af 
    && AF_INET6 != af)
  {
    status = cxxhook::error::k_socketFNNoSupport;
    SetLastSocketError(status);
    return 0;
  }

  union
  {
    socketAddrType base;
    sockaddrStorageType storage;
    sockaddrIn4Type v4;
    sockaddrIn6Type v6;
  } address;

  int addressLength = sizeof(sockaddrStorageType);
  int result = ::WSAStringToAddress(const_cast<char*>(src), af, 0, &address.base, &addressLength);

  if (AF_INET == af)
  {
    if (k_socketError != result)
    {
      ::memcpy(dest, &address.v4.sin_addr, sizeof(in4AddrType));      
    }
    else if (strcmp(src, "255.255.255.255") == 0)
    {
      static_cast<in4AddrType*>(dest)->s_addr = INADDR_NONE;
    }
  }
  else // AF_INET6
  {
    if (k_socketError != result)
    {
      ::memcpy(dest, &address.v6.sin6_addr, sizeof(in6AddrType));
      if (scope_id)
        *scope_id = address.v6.sin6_scope_id;
    }
  }

  // Windows may not set an error code on failure.
  if (k_socketError != result)
    SetLastSocketError(cxxhook::k_noError);
  else if (k_socketError == result)
    status = cxxhook::error::k_invalidArgument;

  return result == k_socketError ? -1 : 1;
#else 
  int result = ::inet_pton(af, src, dest);
  if (result <= 0)
    status = cxxhook::error::k_invalidArguement;

  if ( result > 0 
    && AF_INET6 == af
    && scope_id)
  {
    *scope_id = 0;
    if (const char* if_name = ::strchr(src, '%'))
    {
      in6AddrType* ipv6_address = static_cast<in6AddrType*>(dest);
      bool is_link_local = IN6_IS_ADDR_LINKLOCAL(ipv6_address);
      if (is_link_local)
        *scope_id = if_nametoindex(if_name + 1);
      if (*scope_id == 0)
        *scope_id = ::atoi(if_name + 1);
    }
  }

  return result;
#endif // _WIN32
}

//  ****************************************************************************
inline hostent* gethostbyaddr(const char* pAddr,
                              int length,
                              int af,
                              hostent*  pResult,
                              char* pBuffer, 
                              int bufferLen)
{
  SetLastSocketError(k_noError);
  hostent* pRetVal = NULL;

#if defined _WIN32
  (void)(pBuffer);
  (void)(bufferLen);
  pRetVal = ::gethostbyaddr(pAddr, length, af);
  if (!pRetVal)
    return 0;

  *pResult = *pRetVal;
  return pRetVal;

#elif defined(__MACH__) && defined(__APPLE__)
  (void)(pBuffer);
  (void)(pBufferLen);
  int error = 0;
  pRetVal = ::getipnodebyaddr(pAddr, length, af, &error);
  SetLastSocketError(error);

  if (!pRetval)
    return 0;

  *pResult = *pRetVal;

#else
  int error = 0;
  ::gethostbyaddr_r(pAddr, length, af, pResult, pBuffer, buflength, &pRetval, &error);
  // BOOST translates error messages.   Add implementation if required in future.
  SetLastSocketError(error);

#endif

  return pRetVal;
}

//  ****************************************************************************
inline hostent* gethostbyname(const char* pName,
                              int af,
                              hostent*  pResult,
                              char* pBuffer, 
                              int bufferLen,
                              int ai_flags)
{
  SetLastSocketError(k_noError);
  hostent* pRetVal = NULL;

#if defined _WIN32
  (void)(pBuffer);
  (void)(bufferLen);
  (void)(ai_flags);
  if (af != AF_INET)
  {
    SetLastSocketError(error::k_socketFNNoSupport);
    return 0;
  }

  pRetVal = ::gethostbyname(pName);
  if (!pRetVal)
    return 0;

  *pResult = *pRetVal;

#elif defined(__MACH__) && defined(__APPLE__)
  (void)(buffer);
  (void)(buflength);
  int error = 0;
  pRetVal = ::getipnodebyname(pName, af, ai_flags, &error);
  SetLastSocketError(error);

  if (!pRetVal)
    return 0;

  *pResult = *pRetval;

#else
  (void)(ai_flags);
  if (af != AF_INET)
  {
    SetLastSocketError(error::k_socketFNNoSupport);
    return 0;
  }

  int error = 0;
  ::gethostbyname_r(pName, pResult, pBuffer, bufferLen, &pRetVal, &error);
  SetLastSocketError(error);

#endif

  return pRetVal;
}

//  ****************************************************************************
inline int getnameinfo(const socketAddrType *pAddr,
                       std::size_t addrLen,
                       char* pHost,
                       std::size_t hostLen,
                       char* pServ,
                       std::size_t servLen,
                       int flags)
{
  int error = k_noError;
#ifdef _WIN32
  error = ::getnameinfo(pAddr, 
                        static_cast<socklen_t>(addrLen),
                        pHost, 
                        static_cast<DWORD>(hostLen),
                        pServ, 
                        static_cast<DWORD>(servLen), 
                        flags);
#elif defined(__MACH__) && defined(__APPLE__)
# error The APPLE implementation requires an emulation of the getnameinfo function.  
# error Refer to the boost ASIO implementation.
#else
  error = ::getnameinfo(pAddr, addrLen, pHost, hostLen, pServ, servLen, flags);
#endif
  // Boost performs a translation at this point.
  // wait and determine if that is necessary before adding code.
  return error;
}



#ifdef _WIN32
//  ****************************************************************************
/// Facilitates the initialization of Winsock on windows platforms.
/// This class is adapted from the boost implementation.
///
template <int Major = 2, int Minor = 0>
class InitSockets
{
// TODO: Add non-copyable
//  DISALLOW_COPY_AND_ASSIGN(InitSockets);
private:
  // Structure to perform the actual initialization.
  struct InitWinsock
  {
    InitWinsock()
    {
      WSADATA wsa_data;
      result_ = ::WSAStartup(MAKEWORD(Major, Minor), &wsa_data);
    }

    ~InitWinsock()
    {
      ::WSACleanup();
    }

    int Result() const
    {
      return result_;
    }

    // Helper function to manage an Init singleton. The static instance of the
    // Init object ensures that this function is always called before
    // main, and therefore before any other threads can get started. The Init
    // instance must be static in this function to ensure that it gets
    // initialised before any other global objects try to use it.
    static std::shared_ptr<InitWinsock> Instance()
    {
      static std::shared_ptr<InitWinsock> inst(new InitWinsock);
      return inst;
    }

  private:
    int result_;
  };

public:
  //  Construction **************************************************************
  InitSockets() : ref_(InitWinsock::Instance())  { }
  ~InitSockets()                                 { }

private:
  //  Members ******************************************************************
  // Instance to force initialization of winsock at global scope.
  static InitSockets instance_;

  // Reference to singleton Init object to ensure that winsock does not get
  // cleaned up until the last user has finished with it.
  std::shared_ptr<InitWinsock> ref_;
};

template <int Major, int Minor>
InitSockets<Major, Minor> InitSockets<Major, Minor>::instance_;

#else

template <int Major = 2, int Minor = 2>
//  ****************************************************************************
/// Facilitates the initialization of any socket resources on this platform.
/// This class is adapted from the boost implementation.
///
class InitSockets
{
// TODO: Add non-copyable
//  DISALLOW_COPY_AND_ASSIGN(InitSockets);
public:
  InitSockets()  { }
  ~InitSockets() { }
};

#endif // _WIN32

} // namespace detail
} // namespace ipc
} // namespace cxxhook

typedef cxxhook::ipc::detail::InitSockets<>  BLInitSockets;

#endif 
