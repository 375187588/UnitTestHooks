/// @file: BLSocket.h
///
/// Declarations for network communication abstraction objects.
/// 
/// The basic design for the sockets implementation is built around
/// an object that manages the socket resource, and will automatically
/// handle all of the platform dependant changes to the API calls.
/// 
/// A socket will need to be assigned to a Selector type object in order
/// to process commands related to the socket.  
/// Both a blocking and non-blocking selector can be created based on
/// the needs of the applicaton.  multiple sockets can be assigned to
/// a single selector in order to allow a single thread to process 
/// multiple sockets.
/// 
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// BLNetwork.h contains additional copyright information. 
///
//  ****************************************************************************
#ifndef BLSOCKET_H_INCLUDED
#define BLSOCKET_H_INCLUDED
//  Includes *******************************************************************
#include "BLSocketDef.h"
#include "BLSocketIOControl.h"
#include "BLSocketOptions.h"
#include "BLSocketOperations.h"

#include <algorithm>
#include <queue>
#include <sstream>

// Define BL_DEF_SOCKET_TIMEOUT before including BLSocket.h to override the
// internal processing timeout value used in the BasicSocket implmentation.
#ifndef BL_DEF_SOCKET_TIMEOUT
# define BL_DEF_SOCKET_TIMEOUT 2000
#endif

namespace cxxhook
{
namespace ipc
{

//  ****************************************************************************
/// Contains socket typedefs, constants, and definitions common to the
/// socket framework.
///
class SocketBase
{
public:
  // Different ways a socket can be shutdown.
  enum ShutdownType
  {
    k_shutdownReceive = detail::k_shutdownReceive,
    k_shutdownSend    = detail::k_shutdownSend,
    k_shutdownBoth    = detail::k_shutdownBoth
  };

  // Bitmask type for flags that can be sent to transmit operations.
  typedef int MsgFlags;

  static const int k_messagePeek        = detail::k_messagePeek;
  static const int k_messageOutOfBand   = detail::k_messageOutOfBand;
  static const int k_messageDoNotRoute  = detail::k_messageDoNotRoute;

  //  Typedef all option types *************************************************
  typedef detail::socket_option::boolean<SOL_SOCKET, SO_BROADCAST> 
    Broadcast;

  typedef detail::socket_option::boolean<SOL_SOCKET, SO_DEBUG> 
    Debug;

  typedef detail::socket_option::boolean<SOL_SOCKET, SO_DONTROUTE> 
    DoNotRoute;

  typedef detail::socket_option::boolean<SOL_SOCKET, SO_KEEPALIVE> 
    KeepAlive;

  typedef detail::socket_option::integer<SOL_SOCKET, SO_SNDBUF> 
    SendBufferSize;

  typedef detail::socket_option::integer<SOL_SOCKET, SO_SNDLOWAT> 
    SendLowWatermark;

  typedef detail::socket_option::integer<SOL_SOCKET, SO_RCVBUF> 
    ReceiveBufferSize;

  typedef detail::socket_option::integer<SOL_SOCKET, SO_RCVLOWAT> 
    ReceiveLowWatermark;

  typedef detail::socket_option::boolean<SOL_SOCKET, SO_REUSEADDR> 
    ReuseAddress;

  typedef detail::socket_option::linger<SOL_SOCKET, SO_LINGER> 
    Linger;

  typedef detail::ioctl::NonBlockingIo NonBlockingIo;
  typedef detail::ioctl::ReadableBytes ReadableBytes;

  static const int k_maxConnections = SOMAXCONN;

protected:
  // Destructor is protected to prevent deletion from this level.
  ~SocketBase () {};
};

//  ****************************************************************************
/// Contains utilities to help prepare and query socket state for unit-tests.
///
class SocketTest
  : public SocketBase

{
public:
  //  **************************************************************************
  void setup_accept()
  {
    m_is_accepting = true;
  }

  //  **************************************************************************
  void setup_connect()
  {
    m_is_connecting = true;
  }

  //  **************************************************************************
  void setup_listen()
  {
    m_is_listening = true;
  }

  //  **************************************************************************
  void setup_wait()
  {
    m_is_waiting = true;
  }

  //  **************************************************************************
  void teardown()
  {
    m_is_accepting  = false;
    m_is_connecting = false;
    m_is_listening  = false;
    m_is_waiting    = false;
  }

  //  **************************************************************************
  bool is_accepting()
  {
    return m_is_accepting;
  }

  //  **************************************************************************
  bool is_connecting()
  {
    return m_is_connecting;
  }

  //  **************************************************************************
  bool is_listening()
  {
    return m_is_listening;
  }

  //  **************************************************************************
  bool is_waiting()
  {
    return m_is_waiting;
  }

protected:
  //  Data Members *************************************************************
  bool    m_is_accepting;
  bool    m_is_connecting;
  bool    m_is_listening;
  bool    m_is_waiting;

  //  **************************************************************************
  ~SocketTest () {};
};


//  ****************************************************************************
/// This is a base implementation for an asynchronous socket that 
/// provides functionality common to both stream and datagram based
/// type sockets.
/// 
/// Template:   Protocol: The communication protocol type associated with the this
///             socket.  The two most common will be ip::Tcp and ip:Udp.
/// 
template<typename Protocol>
class BasicSocket :
    public SocketTest
{
public:
  //  Typedef ******************************************************************
  typedef           Protocol               ProtocolType;
  typedef typename  Protocol::Endpoint     EndpointType;
  typedef           BasicSocket<Protocol>  ThisType;

  //  Construction *************************************************************
  //  **************************************************************************
  BasicSocket() 
    : descriptor_(detail::k_invalidSocket)
    , isAsyncIo_(false)
    , isProhibitBlocking_(true)
    , isBlocking_(false)
    , timeOut_(BL_DEF_SOCKET_TIMEOUT)
  { }

  //  **************************************************************************
  explicit      
    BasicSocket( const ProtocolType &protocol) 
    : descriptor_(detail::k_invalidSocket)
    , isAsyncIo_(false)
    , isProhibitBlocking_(true)
    , isBlocking_(false)
    , timeOut_(BL_DEF_SOCKET_TIMEOUT)
  {
    Open(protocol);
  }

  //  **************************************************************************
  explicit      
    BasicSocket( const EndpointType &endpoint) 
    : descriptor_(detail::k_invalidSocket)
    , isAsyncIo_(false)
    , isProhibitBlocking_(true)
    , isBlocking_(false)
    , timeOut_(BL_DEF_SOCKET_TIMEOUT)
  {
    Open(endpoint.Protocol());
    Bind(endpoint);
  }

  //  Status *******************************************************************
  //  **************************************************************************
  bool IsValid( ) const 
  { 
    return descriptor_ != detail::k_invalidSocket;
  }

  //  **************************************************************************
  detail::socketType Native     ( ) const
  { 
    return descriptor_;
  }

  //  **************************************************************************
  template <typename GetableSocketOption>
  bool GetOption( GetableSocketOption& option) const
  { 
    ProtocolType pt;
    int size = option.Size(pt);
    return (::getsockopt( descriptor_, 
                          option.Level(pt),
                          option.Name(pt), 
                          reinterpret_cast<char*>(option.Data(pt)),
                          &size
                          ) != detail::k_socketError);
  }

  //  **************************************************************************
  template <typename SetableSocketOption>
  bool SetOption( SetableSocketOption& option)
  {
    ProtocolType pt;
    return (::setsockopt( descriptor_, 
                          option.Level(pt),
                          option.Name(pt), 
                          reinterpret_cast<char*>(option.Data(pt)),
                          option.Size(pt)
                        ) != detail::k_socketError);
  }

  //  **************************************************************************
  template <typename IOCtlCommand>
  bool IOControl(IOCtlCommand &command)
  { 
    return (::ioctlsocket( descriptor_, 
                            command.Name(), 
                            command.Data()
                          ) != detail::k_socketError);
  }

  //  **************************************************************************
  bool IOControl(SocketBase::NonBlockingIo &command)
  { 
    isAsyncIo_ = command.Value();
    CancelBlockingCall();
    return true;
  }

  //  **************************************************************************
  bool IsBlockingProhibited( ) const
  { 
    return isProhibitBlocking_;
  }

  //  **************************************************************************
  void AllowBlocking()
  { 
    isProhibitBlocking_ = false;
  }

  //  **************************************************************************
  void ProhibitBlocking()
  { 
    isProhibitBlocking_ = true;
  }

  //  **************************************************************************
  bool IsBlocking( ) const
  { 
    return isBlocking_;
  }

  //  **************************************************************************
  void CancelBlockingCall( ) 
  { 
    if (IsBlocking())
      SetBlockingState_(false);
  }

  //  Methods ******************************************************************
  void Open(const ProtocolType& protocol = ProtocolType())
  { }

  //  **************************************************************************
	bool Accept( ThisType &connSocket)  
  { 
    return Accept(connSocket, EndpointType());
  }

  //  **************************************************************************
	bool Accept( ThisType &connSocket, EndpointType &peer)
  { 
    return Accept_(connSocket, peer);
  }

  //  **************************************************************************
	bool Listen( int connectionBacklog = 5)
  { 
    return (::listen(descriptor_, connectionBacklog) != detail::k_socketError);
  }

  //  **************************************************************************
	bool Bind( const EndpointType &local)
  { 
    return (::bind(descriptor_, local.Data(), local.Size()) != detail::k_socketError);
  }

  //  **************************************************************************
  void Close( ) 
  { 
    Close_();
  }

  //  **************************************************************************
	bool Connect( const EndpointType &peer)
  { 
    return Connect_(peer);
  }

  //  **************************************************************************
	int Receive( void* pBuf, int len, MsgFlags flags = 0)
  { 
    return Receive_(pBuf, len, flags);
  }

  //  **************************************************************************
	int Send( const void* pBuf, int len, MsgFlags flags = 0)
  { 
    return Send_(pBuf, len, flags);
  }

  //  **************************************************************************
  bool Shutdown( ShutdownType mode)
  { 
    return (::shutdown(descriptor_, mode) != detail::k_socketError);
  }

  //  **************************************************************************
  EndpointType  LocalEndpoint   ( ) const
  { EndpointType ep;
    int size = ep.Size();
    if (::getsockname(descriptor_, ep.Data(), &size) == detail::k_socketError)
      ep.Clear();

    return ep;
  }

  //  **************************************************************************
  EndpointType  RemoteEndpoint  ( ) const
  { EndpointType ep;
    int size = ep.Size();
    if (::getpeername(descriptor_, ep.Data(), &size) == detail::k_socketError)
      ep.Clear();

    return ep;
  }

  //  Static Functions *********************************************************
  static int    Error( ) 
  { 
    return detail::GetLastSocketError();
  }

  static void   Error ( int error )
  { 
    detail::SetLastSocketError(error);
  }

protected:
  //  Members ******************************************************************
  detail::socketType  descriptor_;
  bool                isAsyncIo_;
  bool                isProhibitBlocking_;
  bool                isBlocking_;
  unsigned long       timeOut_;

  //  Helper Abstractions ******************************************************
  bool          Assign_         (detail::socketType socket);
  bool          Accept_         ( BasicSocket &connSocket, EndpointType &peer);
  void          Close_          ( );
  bool          Connect_        ( const EndpointType &peer);
  int           Receive_        ( void* pBuf, int len, SocketBase::MsgFlags flags);
  int           Send_           ( const void* pBuf, int len, SocketBase::MsgFlags flags);

  //  Socket Call Impl Methods *************************************************
  virtual 
  bool OnCallAccept_ ( ThisType &connSocket, EndpointType &peer);
  
  virtual 
  void OnCallClose_  ( );

  virtual 
  bool OnCallConnect_( const EndpointType &peer);

  virtual 
  int  OnCallReceive_( void* pBuf, int len, MsgFlags flags) = 0;

  virtual 
  int  OnCallSend_   ( const void* pBuf, int len, MsgFlags flags) = 0;

  //  Blocking Call Helpers ****************************************************
  bool          SetBlockingState_( bool isBlocking);
  int           SendPortion_    ( const void* pBuf, int len, SocketBase::MsgFlags flags);
  bool          WatchSelect_    ( int selectEvent);
  bool          WaitForSelect_  ( int selectEvent);

  ~BasicSocket                  ( ) { Close();}
};

//  ****************************************************************************
/// Helper function to assign a socket to this object and initialize its state.
///
/// @return		  true on success, false otherwise.
///
template<typename Protocol>
inline bool BasicSocket<Protocol>::Assign_(detail::socketType socket)
{ 
  if (detail::k_invalidSocket == socket)
  { 
    Error(error::k_badFileHandle);
    return false;
  }

  if (IsValid())
  { 
    Close();
  }

  descriptor_   = socket;
  // All sockets will communicate in non-blocking mode.  
  // The object will handle blocking control.
  detail::ulongType data = true;
  return (::ioctlsocket(descriptor_, FIONBIO, &data) != detail::k_socketError);
}

//  ****************************************************************************
/// Helper function that handles both the blocking and non-blocking
/// versions to accept socket connections.
/// 			
/// @return		  true on success, false otherwise.
///
template<typename Protocol>
inline bool BasicSocket<Protocol>::Accept_(BasicSocket &connSocket, EndpointType &peer) 
{ 
  if (isAsyncIo_)
    return OnCallAccept_(connSocket, peer);
  else // Blocking implementation
  { 
    if (IsBlocking())
    { 
      Error(error::k_socketInProgress); 
      return false;
    }

    while (!OnCallAccept_(connSocket, peer))
    { 
      if (error::k_socketWouldBlock == Error())
      { 
        if (!WatchSelect_(FD_ACCEPT))
          return false;
      }
      else
        return false;
    }

    return true;
  }
}

//  ****************************************************************************
/// Closes socket connections for all socket types.
///
template<typename Protocol>
inline void BasicSocket<Protocol>::Close_( ) 
{ 
  CancelBlockingCall(); // Benign for async mode.
  OnCallClose_();
}

//  ****************************************************************************
/// Helper function that handles both the blocking and non-blocking
/// versions to connect a socket to an endpoint.
///
/// @return true on success, false otherwise.
///
template<typename Protocol>
inline bool BasicSocket<Protocol>::Connect_(const EndpointType &peer) 
{ 
  if (isAsyncIo_)
    return OnCallConnect_(peer);
  
  // Blocking implementation
  if (IsBlocking())
  { 
    Error(error::k_socketInProgress); 
    return false;
  }

  if (OnCallConnect_(peer))
    return true;

  if ( error::k_socketWouldBlock != Error()
    || IsBlockingProhibited())
    return false;

  while (WatchSelect_(FD_CONNECT))
  { 
    DWORD err = Error();
    if (detail::k_socketError != err)
      return 0 != err;
  }

  return true;
}

//  ****************************************************************************
/// Helper function that handles both the blocking and non-blocking
/// versions to receive data.
/// 
template<typename Protocol>
inline int BasicSocket<Protocol>::Receive_(void* pBuf, int len, SocketBase::MsgFlags flags) 
{  
  if (isAsyncIo_)
    return OnCallReceive_(pBuf, len, flags);
  
  // Blocking implementation
  if (IsBlocking())
  { 
    Error(error::k_socketInProgress); 
    return false;
  }

  int retVal = 0;
  while ((retVal = OnCallReceive_(pBuf, len, flags)) == detail::k_socketError)
  { 
    if ( error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;

    if ( !WatchSelect_(FD_READ)
      && error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;
  }

  return retVal;
}


//  ****************************************************************************
/// Helper function that handles both the blocking and non-blocking
/// versions to send data on a connection.
///
template<typename Protocol>
inline int BasicSocket<Protocol>::Send_(const void* pBuf, int len, SocketBase::MsgFlags flags) 
{  
  if (isAsyncIo_)
    return OnCallSend_(pBuf, len, flags);
  
  // Blocking implementation
  if (IsBlocking())
  { 
    Error(error::k_socketInProgress); 
    return false;
  }

  int   count = len;
  int   written = 0;
  const BYTE *pCur = reinterpret_cast<const BYTE*>(pBuf);
  
  while (count > 0)
  {
    written = SendPortion_(pBuf, count, flags);
    if (detail::k_socketError == written)
      return written;

    count -= written;
    pCur += written;
  }

  return (len - count);
}

//  ****************************************************************************
/// Changes the blocking state of the socket if possible.
/// @param 	  isBlocking[in]: The new state to set for the socket.
/// @return 	The set state is returned.
/// 
template<typename Protocol>
inline bool BasicSocket<Protocol>::SetBlockingState_( bool isBlocking)
{
// TODO: Possibly allow the tester to control if blocking is allowed or not. For now, explicitly prevent it.
  //if (!isAsyncIo_)
  //  isBlocking_ = isBlocking;

  return isBlocking_;
}

//  ****************************************************************************
/// Helper function to send small chunks of data on a connection 
/// and still remain interuptable.
///
template<typename Protocol>
inline int BasicSocket<Protocol>::SendPortion_(const void* pBuf, int len, SocketBase::MsgFlags flags) 
{
  int retVal = 0;
  while ((retVal = OnCallSend_(pBuf, len, flags)) == detail::k_socketError)
  { 
    if ( error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;

    if ( !WatchSelect_(FD_WRITE)
      && error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;
  }

  return retVal;
}

//  ****************************************************************************
/// Helper function for blocking socket calls of a single event.
///
template<typename Protocol>
inline bool BasicSocket<Protocol>::WatchSelect_(int selectEvent)
{
  SetBlockingState_(true);
  while (IsBlocking())
  {
    if (WaitForSelect_(selectEvent))
      break;
  }

  if (!IsBlocking())
  { 
    Error(error::k_blockingCallInterupted);
  }

  SetBlockingState_(false);

  return true;
}

//  ****************************************************************************
/// Helper function for blocking socket calls, This is an abstraction
/// for a wait function for the select call.
/// 
/// @return		  true indicates requested event was triggered, 
///             false indicates a timeout or an error.
/// 
template<typename Protocol>
inline bool BasicSocket<Protocol>::WaitForSelect_(int selectEvent)
{
  fd_set  rs;
  fd_set  ws;

  FD_ZERO(&rs);
  FD_ZERO(&ws);

  // TODO: Investigate the SO_OOBINLINE flag that will change where OOB is signalled.
  // Set the read or write set as required.
  if (selectEvent & (FD_READ | FD_OOB | FD_ACCEPT))
    FD_SET(descriptor_, &rs);
  else if (selectEvent & (FD_WRITE | FD_CONNECT))
    FD_SET(descriptor_, &ws);

  timeval to;
  to.tv_sec = timeOut_ / 1000;            // convert ms to seconds.
  to.tv_usec= (timeOut_ % 1000) / 1000;   // convert remainder ms to us.
  int result = ::select(1, &rs, &ws, NULL, &to);

DWORD err = Error();

  return (result > 0);
}

//  ****************************************************************************
template<typename Protocol>
inline bool BasicSocket<Protocol>::OnCallConnect_( const EndpointType &peer)
{ 
  return (::connect(descriptor_, peer.Data(), peer.Size()) != detail::k_socketError);
}

//  ****************************************************************************
template<typename Protocol>
inline bool BasicSocket<Protocol>::OnCallAccept_( ThisType &connSocket, 
                                                  EndpointType &peer) 
{ 
  connSocket.Close();
  int size = peer.Size();
  detail::socketType connDescriptor = 
      ::accept(descriptor_, peer.Data(), &size);
  if (detail::k_invalidSocket == connDescriptor)
    return false; 

  return connSocket.Assign_(connDescriptor);
}

//  ****************************************************************************
template<typename Protocol>
inline void BasicSocket<Protocol>::OnCallClose_( ) 
{ 
  if (IsValid())
  { 
    ::closesocket(descriptor_);
    descriptor_ = detail::k_invalidSocket;
  }
}



//  ****************************************************************************
//  ****************************************************************************
/// Provide functionality to communicate through a stream type socket.
///
template<typename Protocol>
class StreamSocket :
  public BasicSocket<Protocol>
{
// TODO: Add non-copyable
//  DISALLOW_COPY_AND_ASSIGN(InitSockets);
public:
  //  Typedef ******************************************************************
  typedef           Protocol               ProtocolType;
  typedef typename  Protocol::Endpoint     EndpointType;
  typedef           BasicSocket<Protocol>  BaseType;

  //  Construction *************************************************************
  //  **************************************************************************
  StreamSocket( )
    : BasicSocket()          
  { }

  //  **************************************************************************
  explicit 
    StreamSocket( const ProtocolType &protocol) 
    : BasicSocket(protocol)  
  { }

  //  **************************************************************************
  explicit 
    StreamSocket( const EndpointType &endpoint) 
    : BasicSocket(endpoint)  
  { }

  //  **************************************************************************
  ~StreamSocket( )
  { }

  //  **************************************************************************
  void add_to_recv_buffer(const char* pBuf, size_t len)
  {
    m_recv_buffer.rdbuf()->sputn(pBuf, len);
  }

  //  **************************************************************************
  void add_to_send_buffer(const char* pBuf, size_t len)
  {
    m_send_buffer.rdbuf()->sputn(pBuf, len); 
  }

  //  **************************************************************************
  size_t get_from_send_buffer(char* pBuf, size_t len)
  {
    return (size_t)m_send_buffer.rdbuf()->sgetn(pBuf, len); 
  }

protected:
  //  Data Members *************************************************************
  std::stringstream   m_send_buffer;
  std::stringstream   m_recv_buffer;

  //  Socket Call Impl Methods *************************************************
  int  OnCallReceive_( void* pBuf, int len, MsgFlags flags);
  int  OnCallSend_   ( const void* pBuf, int len, MsgFlags flags);

};


//  ****************************************************************************
template<typename Protocol>
inline 
int StreamSocket<Protocol>::OnCallReceive_( 
  void* pBuf, 
  int len, 
  SocketBase::MsgFlags flags
)
{ 
  int result = (int)m_recv_buffer.rdbuf()->sgetn((char*)(pBuf), len);  

  if (result < len)
  {
    Error(error::k_socketWouldBlock);
    return detail::k_socketError;
  }

  return result;
}

//  ****************************************************************************
template<typename Protocol>
inline 
int StreamSocket<Protocol>::OnCallSend_ ( 
  const void* pBuf, 
  int len, 
  SocketBase::MsgFlags flags
)
{ 
  return (int)m_send_buffer.rdbuf()->sputn((char*)(pBuf), len);  
}



//  ****************************************************************************
//  ****************************************************************************
/// Provide functionality to communicate through a datagram type socket.
///
template<typename Protocol>
class DatagramSocket :
  public BasicSocket<Protocol>
{
// TODO: Add non-copyable
//  DISALLOW_COPY_AND_ASSIGN(InitSockets);
public:
  //  Typedef ******************************************************************
  typedef           Protocol                ProtocolType;
  typedef typename  Protocol::Endpoint      EndpointType;
  typedef           BasicSocket<Protocol>   BaseType;

  typedef std::vector<char>                 datagram_type;
  typedef std::queue<datagram_type>         data_queue;

  //  Construction *************************************************************
  //  **************************************************************************
  DatagramSocket ( )
    : BasicSocket()
  { }

  //  **************************************************************************
  explicit 
    DatagramSocket( const ProtocolType &protocol) 
    : BasicSocket(protocol)  
  { }

  //  **************************************************************************
  explicit 
    DatagramSocket( const EndpointType &endpoint) 
    : BasicSocket(endpoint)  
  { }

  //  **************************************************************************
  ~DatagramSocket( )
  { }

  //  **************************************************************************
  void add_to_recv_buffer(const char* pBuf, size_t len)
  {
    datagram_type buffer(len);
    ::memcpy(&buffer[0], pBuf, len);
    m_recv_queue.push(buffer);
  }

  //  **************************************************************************
  void add_to_send_buffer(const char* pBuf, size_t len)
  {
    datagram_type buffer(len);
    ::memcpy(&buffer[0], pBuf, len);
    m_send_queue.push(buffer);
  }

  //  **************************************************************************
  size_t get_from_send_buffer(char* pBuf, size_t len)
  {
    if (m_send_queue.empty())
    {
      return 0;
    }

    datagram_type& buffer = m_send_queue.front();
    size_t         available = buffer.size();
    ::memcpy(pBuf, &buffer[0], std::min(available, len));
    m_send_queue.pop();

    return available;
  }


  //  Methods ******************************************************************
  //  **************************************************************************
	int ReceiveFrom( void* pBuf, int len, EndpointType &source, MsgFlags flags = 0)
  { 
    return ReceiveFrom_(pBuf, len, source, flags);
  }

  //  **************************************************************************
	int SendTo( const void* pBuf, int len, EndpointType &destination, MsgFlags flags = 0)
  { 
    return SendTo_(pBuf, len, destination, flags);
  }

protected:
  //  Data Members *************************************************************
  data_queue   m_send_queue;
  data_queue   m_recv_queue;

  //  Helper Abstractions ******************************************************
  int   ReceiveFrom_    ( void* pBuf, int len, EndpointType &source, MsgFlags flags);
  int   SendTo_         ( const void* pBuf, int len, EndpointType &destination, MsgFlags flags);

  //  Socket Call Impl Methods *************************************************
  int  OnCallReceive_( void* pBuf, int len, MsgFlags flags);
  int  OnCallSend_   ( const void* pBuf, int len, MsgFlags flags);

  //  **************************************************************************
  int   OnCallReceiveFrom_( void* pBuf, size_t len, EndpointType &source, MsgFlags flags);

  //  **************************************************************************
  int OnCallSendTo_( const void* pBuf, int len, EndpointType &destination, MsgFlags flags);
};

//  ****************************************************************************
template<typename Protocol>
inline 
int DatagramSocket<Protocol>::OnCallReceive_( 
  void* pBuf, 
  int len, 
  SocketBase::MsgFlags flags
)
{ 
  if (m_recv_queue.empty())
  {
    // TODO: Add an error value if necessary
    return 0;
  }

  const datagram_type& msg = m_recv_queue.front();
  int msg_size = static_cast<int>(msg.size());
  int size = std::min(msg_size, len);
  std::memcpy(pBuf, &msg[0], size);

  m_recv_queue.pop();

  if (len < msg_size)
  {
    Error(error::k_socketMsgSize);
    return detail::k_socketError;
  }

  return size;
}

//  ****************************************************************************
template<typename Protocol>
inline 
int DatagramSocket<Protocol>::OnCallSend_ ( 
  const void* pBuf, 
  int len, 
  SocketBase::MsgFlags flags
)
{ 
  datagram_type msg(len);
  std::memcpy(&msg[0], pBuf, len);

  m_send_queue.push(msg);

  return len;
}

//  ****************************************************************************
/// Purpose:	  Helper function that handles both the blocking and non-blocking
///             versions to receive socket datagrams from unconnected sources.
/// 
template<typename Protocol>
inline int DatagramSocket<Protocol>::ReceiveFrom_(void* pBuf, 
                                                  int len, 
                                                  EndpointType &source, 
                                                  MsgFlags flags) 
{ 
  if (isAsyncIo_)
    return OnCallReceiveFrom_(pBuf, len, source, flags);

  // Blocking implementation
  if (IsBlocking())
  { 
    Error(error::k_socketInProgress); 
    return false;
  }

	int retVal;
  while ((retVal = OnCallReceiveFrom_(pBuf, len, source, flags)) == detail::k_socketError)
	{
    if ( error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;

    if ( !WatchSelect_(FD_READ)
      && error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;
  }

	return retVal;
}

//  ****************************************************************************
/// Helper function that handles both the blocking and non-blocking
/// versions to send datagrams on unconnected sockets.
///
template<typename Protocol>
inline int DatagramSocket<Protocol>::SendTo_(const void* pBuf, 
                                             int len, 
                                             EndpointType &destination, 
                                             MsgFlags flags) 
{ 
  if (isAsyncIo_)
    return OnCallSendTo_(pBuf, len, destination, flags);

  // Blocking implementation
  if (IsBlocking())
  { 
    Error(error::k_socketInProgress); 
    return 0;
  }

	int retVal;
  while ((retVal = OnCallSendTo_(pBuf, len, destination, flags)) == detail::k_socketError)
	{
    if ( error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;
    
    if ( !WatchSelect_(FD_WRITE)
      && error::k_socketWouldBlock != Error()
      || IsBlockingProhibited())
      return detail::k_socketError;
  }

	return retVal; 
}


//  **************************************************************************
template<typename Protocol>
inline 
int DatagramSocket<Protocol>::OnCallReceiveFrom_( 
  void* pBuf, 
  size_t len, 
  EndpointType &source, 
  MsgFlags flags
)
{ 
  // No explicit blocking for unit-tests.
  if (m_recv_queue.empty())
  {
    return error::k_socketWouldBlock;
  }

  if (!pBuf)
  {
    return error::k_invalidArgument;
  }

  int size = source.Size();

  // Get the datagram and return it to the caller.
  datagram_type &msg = m_recv_queue.front();
  m_recv_queue.pop();

  // TODO: Possibly add alternate logic to read from the send queue for the source.
  memcpy(pBuf, &msg[0], std::min(len, msg.size()));

  return  len < msg.size()
          ? error::k_socketMsgSize
          : 0;
  //return ::recvfrom(descriptor_, 
  //                  reinterpret_cast<char*>(pBuf), 
  //                  len, 
  //                  flags,
  //                  source.Data(),
  //                  &size);
}


//  **************************************************************************
template<typename Protocol>
inline 
int DatagramSocket<Protocol>::OnCallSendTo_( 
  const void* pBuf, 
  int len, 
  EndpointType &destination, 
  MsgFlags flags
)
{ return ::sendto(descriptor_, 
                  reinterpret_cast<const char*>(pBuf), 
                  len, 
                  flags,
                  destination.Data(),
                  destination.Size());
}

} // namespace ipc
} // namespace cxxhook


#endif //BLSOCKET_H_INCLUDED
