/// @file   BLIpAddress.h 
/// 
/// Implements a version independant IP Address object.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
//  ****************************************************************************
#ifndef BLIPADDRESS_H_INCLUDED
#define BLIPADDRESS_H_INCLUDED
//  Includes *******************************************************************
#include "BLSocketDef.h"
#include "BLSocketOperations.h"

#include <array>
#include <string>

namespace cxxhook
{
namespace ipc
{
namespace ip
{

//  ****************************************************************************
/// Implements a version 4 style address.
///
class AddressV4
{
public:
  //  Typedef ******************************************************************
  typedef std::array<unsigned char, 4>  BytesType;

  //  Construction *************************************************************
                AddressV4()                       { m_addr.s_addr = 0;}
                AddressV4(const AddressV4 &rhs) :
                    m_addr(rhs.m_addr)            { }
  explicit      AddressV4(const BytesType &bytes) { ::memcpy(&m_addr.s_addr, &bytes, 4);}
  explicit      AddressV4(unsigned long addr)     { m_addr.s_addr = detail::NetworkToHostLong(addr);}
  explicit      AddressV4(const std::string& addr){ int retVal = 0;
                                                    unsigned long status;
                                                    retVal = detail::inet_pton( AF_INET,
                                                                                addr.c_str(),
                                                                                &m_addr, 
                                                                                0,
                                                                                status);
                                                    if (retVal <= 0)
                                                      ::memset(&m_addr.s_addr, 0, 4); 
                                                  }

  static        AddressV4 Any()                   { return AddressV4(INADDR_ANY);}
  static        AddressV4 Loopback()              { return AddressV4(static_cast<unsigned long>(INADDR_LOOPBACK));}
  static        AddressV4 Broadcast()             { return AddressV4(INADDR_BROADCAST);}
  static        AddressV4 Broadcast(const AddressV4 &addr, const AddressV4 &mask)             
                                                  { return AddressV4(addr.ToUlong() | ~mask.ToUlong());}

  //  Operators ****************************************************************
  AddressV4&    operator= (const AddressV4 &rhs)  { m_addr = rhs.m_addr; return *this;}

  friend bool   operator==(const AddressV4 &lhs, const AddressV4 &rhs)
                                                  { return lhs.m_addr.s_addr == rhs.m_addr.s_addr;}
  friend bool   operator!=(const AddressV4 &lhs, const AddressV4 &rhs)
                                                  { return lhs.m_addr.s_addr != rhs.m_addr.s_addr;}
  friend bool   operator< (const AddressV4 &lhs, const AddressV4 &rhs)
                                                  { return lhs.ToUlong() <  rhs.ToUlong();}
  friend bool   operator> (const AddressV4 &lhs, const AddressV4 &rhs)
                                                  { return lhs.ToUlong() >  rhs.ToUlong();}
  friend bool   operator<=(const AddressV4 &lhs, const AddressV4 &rhs)
                                                  { return lhs.ToUlong() <= rhs.ToUlong();}
  friend bool   operator>=(const AddressV4 &lhs, const AddressV4 &rhs)
                                                  { return lhs.ToUlong() >= rhs.ToUlong();}
  //  Status *******************************************************************
  BytesType     ToBytes() const                   { BytesType bytes;
                                                    ::memcpy(&bytes, &m_addr.s_addr, 4);
                                                    return bytes;
                                                  }
  unsigned long ToUlong() const                   { return detail::NetworkToHostLong(m_addr.s_addr);}
  std::string   ToString() const                  { char          addrStr[cxxhook::ipc::detail::k_maxAddrV4_strLen];
                                                    unsigned long status;
                                                    detail::inet_ntop( AF_INET, 
                                                                       &m_addr, 
                                                                       addrStr, 
                                                                       cxxhook::ipc::detail::k_maxAddrV4_strLen, 
                                                                       0, 
                                                                       status);
                                                    return std::string(addrStr);
                                                  }
#if !defined(BL_PROVIDE_IPV6_SUPPORT)
  bool          Isv4()                            { return true;}
#endif

private:
  //  Members ******************************************************************
  cxxhook::ipc::detail::in4AddrType  m_addr;
};

#if defined(BL_PROVIDE_IPV6_SUPPORT)
#error Not Yet Implemented, implement when the extra cost is justified.
#error Reference boost::asio for details

//  ****************************************************************************
/// Implements a version 6 style address.
//class AddressV6
//{
//};
//
//  ****************************************************************************
/// Version independent IP Address implementation.
//class Address
//{
//};

#else
typedef AddressV4 Address;
#endif // defined(BL_PROVIDE_IPV6_SUPPORT)

//  ****************************************************************************
/// The Endpoint provides an abstraction for the entire sockaddr struct
/// which holds a connections IP Address, port ID, and protocol.
/// 
/// This class is intended to provide a simple interface that will also
/// hide the complexities of converting between the different forms
/// an IP Address can take.  A further abstract for IP Addresses is
/// implemented in cxxhook::ipc::ip::Address, or BLIPAddress.
/// 
/// In a connected socket, there will be two Endpoints, the 
/// sock or local Endpoint, and the peer or remote endpoint.
/// 
/// The BasicEndpoint is not intended to be used alone, it is a template
/// that will accept a protocol implementation, such as Tcp and Udp.
/// Future possible improvements could accomodate bluetooth socket support.
///
template <typename InternetProtocol>
class BasicEndpoint
{
public:
  //  Construction *************************************************************
  typedef InternetProtocol                  ProtocolType;
  typedef cxxhook::ipc::detail::socketAddrType  DataType;

  BasicEndpoint() : address_()          { address_.v4.sin_family = AF_INET;
                                          address_.v4.sin_port   = 0;
                                          address_.v4.sin_addr.s_addr = INADDR_ANY;
                                        }
  BasicEndpoint(const BasicEndpoint& rhs) : address_(rhs.address_) { } 
  BasicEndpoint(InternetProtocol &protocol, unsigned short port) : address_()
                                        { if (protocol.Family() == PF_INET)
                                          {
                                            address_.v4.sin_family = AF_INET;
                                            address_.v4.sin_port = detail::HostToNetworkShort(port);
                                            address_.v4.sin_addr.s_addr = INADDR_ANY;
                                          }
#if defined(BL_PROVIDE_IPV6_SUPPORT)
                                          else
                                          {
                                            address_.v6.sin6_family = AF_INET6;
                                            address_.v6.sin6_port = detail::HostToNetworkShort(port);
                                            address_.v6.sin6_flowinfo = 0;
                                            cxxhook::ipc::in6AddrType tmpAddr = IN6ADDR_ANY_INIT;
                                            address_.v6.sin6_addr = tmp_addr;
                                            address_.v6.sin6_scope_id = 0;
                                          }
#endif // defined(BL_PROVIDE_IPV6_SUPPORT)
                                        }
  BasicEndpoint(ip::Address &address, unsigned short port) : address_()
                                        { if (address.Isv4())
                                          {
                                            address_.v4.sin_family = AF_INET;
                                            address_.v4.sin_port = detail::HostToNetworkShort(port);
                                            address_.v4.sin_addr.s_addr = detail::HostToNetworkLong(address.ToUlong());
                                          }
#if defined(BL_PROVIDE_IPV6_SUPPORT)
                                          else
                                          {
                                            address_.v6.sin6_family = AF_INET6;
                                            address_.v6.sin6_port = detail::HostToNetworkShort(port);
                                            address_.v6.sin6_flowinfo = 0;
                                            ip::AddressV6 v6Addr = addr.ToV6();
                                            ip::AddressV6::BytesType bytes = v6Addr.ToBytes();
                                            ::memcpy(address_.v6.sin6addr.s6_addr, &bytes, 16);
                                            address_.v6.sin6_scope_id = 0;
                                          }
#endif // defined(BL_PROVIDE_IPV6_SUPPORT)
                                        }

  BasicEndpoint &operator=(const BasicEndpoint &rhs)  
                                        { address_ = rhs.address_; return *this;}

  //  Operators ****************************************************************
  friend bool operator==(const BasicEndpoint<InternetProtocol>& lhs,
                         const BasicEndpoint<InternetProtocol>& rhs)
                                        { return lhs.Address() == rhs.Address()
                                              && lhs.Port() == rhs.Port();
                                        }
  friend bool operator!=(const BasicEndpoint<InternetProtocol>& lhs,
                         const BasicEndpoint<InternetProtocol>& rhs)
                                        { return lhs.Address() != rhs.Address()
                                              || lhs.Port() != rhs.Port();
                                        } 
  friend bool operator< (const BasicEndpoint<InternetProtocol>& lhs,
                         const BasicEndpoint<InternetProtocol>& rhs)
                                        { if (lhs.Address() < rhs.Address())
                                            return true;
                                          if (lhs.Address() != rhs.Address())
                                            return false;
                                          return lhs.Port() < rhs.Port();
                                        } 

  //  Status *******************************************************************
#if defined(BL_PROVIDE_IPV6_SUPPORT)
  ProtocolType    Protocol() const      { if (Isv4())
                                            return InternetProtocol::v4();
                                          return InternetProtocol::v6();
                                        }
  size_t          Size() const          { if (Isv4())
                                            return sizeof(cxxhook::ipc::detail::sockaddrIn4Type);
                                          return sizeof(cxxhook::ipc::detail::sockaddrIn6Type);
                                        }
#else
  ProtocolType    Protocol() const      { return InternetProtocol::v4();}
  size_t          Size() const          { return sizeof(cxxhook::ipc::detail::sockaddrIn4Type);}

#endif

  size_t          Capacity() const      { return sizeof(cxxhook::ipc::detail::sockaddrStorageType);}
  bool            Resize(size_t size)   { return size <= sizeof(cxxhook::ipc::detail::sockaddrStorageType);}
  DataType*       Data()                { return &address_.base;}
  const DataType* Data() const          { return &address_.base;}

  //  Members ******************************************************************
  void Address(const ip::Address &addr) { BasicEndpoint<InternetProtocol> tmpEndPoint(addr, port());
                                          address_ = tmpEndpoint.address_;
                                        }

#if defined(BL_PROVIDE_IPV6_SUPPORT)
  ip::Address Address () const          { if (Isv4()) 
                                          {
                                            return ip::AddressV4(detail::NetworkToHostLong(address_.v4.sin_addr.s_addr));
                                          }
                                          else
                                          {
                                            ip::AddressV6::BytesType bytes;
                                            ::memcpy(&bytes, address_.v6.sin6addr.s6_addr, 16);
                                            return ip::AddressV6(bytes, address_.v6.sin6_scope_id);
                                          }
                                        }

  unsigned short Port() const           { if (Isv4())
                                            return detail::NetworkToHostShort(address_.v4.sin_port);
                                          else
                                            return detail::NetworkToHostShort(address_.v6.sin_port);
                                        }
  void Port(unsigned short portNum)     { if (Isv4())
                                            address_.v4.sin_port = detail::HostToNetworkShort(portNum);
                                          else
                                            address_.v6.sin_port = detail::HostToNetworkShort(portNum);
                                        }
#else
  ip::Address Address () const          { return ip::AddressV4(detail::NetworkToHostLong(address_.v4.sin_addr.s_addr));}

  unsigned short Port() const           { return detail::NetworkToHostShort(address_.v4.sin_port);}
  void Port(unsigned short portNum)     { address_.v4.sin_port = detail::HostToNetworkShort(portNum);}

#endif

  void Clear()                          { ::memset(&address_, 0, sizeof(address_));}

private:
  union AddressUnion
  {
    cxxhook::ipc::detail::socketAddrType       base;
    cxxhook::ipc::detail::sockaddrStorageType  storage;
    cxxhook::ipc::detail::sockaddrIn4Type      v4;
#if defined(BL_PROVIDE_IPV6_SUPPORT)
    cxxhook::ipc::detail::sockaddrIn6Type      v6;
#endif
  } address_;

  bool Isv4() const       {return address_.base.sa_family == AF_INET;}
};

} // namespace ip
} // namespace ipc
} // namespace cxxhook

typedef cxxhook::ipc::ip::AddressV4 BLIpAddress;

#endif 
