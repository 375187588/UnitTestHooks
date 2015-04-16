/// @file: BLIp.h
///
/// Provides an abstraction for an Internet Protocol Endpoint.  
/// This basically expands for both the TCP and UDP protocols.
/// The Endpoints also contain the types and constants that are used
/// in the implementation for interaction with these sockets.
/// 
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// BLNetwork.h contains additional copyright information. 
///
//  ****************************************************************************
#ifndef BLIP_H_INCLUDED
#define BLIP_H_INCLUDED
//  Includes *******************************************************************
#include "BLSocketDef.h"
#include "BLIpAddress.h"
#include "BLSocketOptions.h"
#include "BLSocket.h"
//#include "BLSocketSelector.h"

namespace cxxhook
{
namespace ipc
{
namespace ip
{

// TODO: A placeholder template, to compile until asynchronous callbacks are required.
template <typename T>
struct BasicSelector
{ };


//  TCP ************************************************************************
//  ****************************************************************************
/// Encapsulates the flags and helpers for TCP sockets.
///
class Tcp
{
  //  Forward Declarations *****************************************************
  class StreamSocket_;
public:

  //  Typedefs *****************************************************************
  typedef BasicEndpoint<Tcp>          Endpoint;
  typedef StreamSocket_               Socket;
  typedef BasicSelector<Socket>       Selector;

  //  Construction *************************************************************
  static  Tcp v4()         {return Tcp(static_cast<int>(PF_INET));}
#if !defined(BL_PROVIDE_IPV6_SUPPORT)
  Tcp() : family_(PF_INET) { }
#else
  static  Tcp v6()          {Tcp(PF_INET6);}
#endif

  //  Methods ******************************************************************
  int     Type()      const {return SOCK_STREAM;}
  int     Protocol()  const {return IPPROTO_TCP;}
  int     Family()    const {return family_;}

  friend bool operator==(const Tcp& lhs, const Tcp& rhs)
                            {
                              return lhs.family_ == rhs.family_;
                            }

  friend bool operator!=(const Tcp& lhs, const Tcp& rhs)
                            {
                              return lhs.family_ != rhs.family_;
                            }

private:
  //  Members ******************************************************************
  int family_;

  //  Construction *************************************************************
  explicit Tcp(int family) : family_(family) { }

  //  Socket Implementation Classes ********************************************
  class StreamSocket_ :
      public StreamSocket<Tcp>
  {
  public:
    //  Typedef ****************************************************************
    typedef Tcp::Endpoint                        EndpointType;
    typedef Tcp::Endpoint::ProtocolType          ProtocolType;
    typedef StreamSocket                         ThisType;

    typedef cxxhook::ipc::detail::socket_option::boolean<IPPROTO_TCP, TCP_NODELAY> 
      NoDelay;

    //  Construction ***********************************************************
    StreamSocket_()
      : StreamSocket()         
    { }

    explicit 
      StreamSocket_  ( const ProtocolType &protocol) 
      : StreamSocket(protocol) 
    { }

    explicit 
      StreamSocket_  ( const EndpointType &endpoint) 
      : StreamSocket(endpoint) 
    { }

    ~StreamSocket_  ( )
    { }

  };
};

//  UDP ************************************************************************
//  ****************************************************************************
/// Encapsulates the flags and helpers for UDP sockets.
///
class Udp
{
  //  Forward Declarations *****************************************************
  class DatagramSocket_;
public:

  //  Typedefs *****************************************************************
  typedef BasicEndpoint<Udp>          Endpoint;
  typedef DatagramSocket_             Socket;
  typedef BasicSelector<Socket>       Selector;

  static  Udp v4()          {return Udp(PF_INET);}
#if !defined(BL_PROVIDE_IPV6_SUPPORT)
  Udp() : family_(PF_INET) { }
#else
  static  Udp v6()          {return Udp(PF_INET6);}
#endif 

  int     Type()      const {return SOCK_DGRAM;}
  int     Protocol()  const {return IPPROTO_UDP;}
  int     Family()    const {return family_;}

  friend bool operator==(const Udp& lhs, const Udp& rhs)
                            {
                              return lhs.family_ == rhs.family_;
                            }

  friend bool operator!=(const Udp& lhs, const Udp& rhs)
                            {
                              return lhs.family_ != rhs.family_;
                            }

private:
  //  Members ******************************************************************
  int family_;

  //  Construction *************************************************************
  explicit Udp(int family) : family_(family) { }

 
  //  Socket Implementation Classes ********************************************
  class DatagramSocket_ :
      public DatagramSocket<Udp>
  {
  public:
    //  Typedef ****************************************************************
    typedef Udp::Endpoint                 EndpointType;
    typedef Udp::Endpoint::ProtocolType   ProtocolType;
    typedef DatagramSocket                ThisType;

    //  Construction ***********************************************************
    DatagramSocket_()                              
      : DatagramSocket()         
    { }

    explicit 
      DatagramSocket_( const ProtocolType &protocol) 
      : DatagramSocket(protocol) 
    { }

    explicit 
      DatagramSocket_( const EndpointType &endpoint) 
      : DatagramSocket(endpoint) 
    { }

    ~DatagramSocket_( )
    { }

  };
};

} // namespace ip
} // namespace ipc
} // namespace cxxhook

typedef cxxhook::ipc::ip::Tcp                   BLTcp;
typedef cxxhook::ipc::ip::Tcp::Endpoint         BLTcpEndpoint;
typedef cxxhook::ipc::ip::Tcp::Socket           BLTcpSocket;
typedef cxxhook::ipc::ip::Tcp::Selector         BLTcpSelector;

typedef cxxhook::ipc::ip::Udp                   BLUdp;
typedef cxxhook::ipc::ip::Udp::Endpoint         BLUdpEndpoint;
typedef cxxhook::ipc::ip::Udp::Socket           BLUdpSocket;
typedef cxxhook::ipc::ip::Udp::Selector         BLUdpSelector;

#endif 
