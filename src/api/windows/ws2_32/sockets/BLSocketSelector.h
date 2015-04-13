/// @file: BLSocketSelector.h
///
/// Provides an asynchronous callback mechanism for socket operations.
/// 
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// BLNetwork.h contains additional copyright information. 
///
//  ****************************************************************************
#ifndef BLSOCKETSELECTOR_H_INCLUDED
#define BLSOCKETSELECTOR_H_INCLUDED
//  Includes *******************************************************************
#include "BLSocketDef.h"
#include <map>

namespace cxxhook
{
namespace ipc
{

namespace // unnamed
{
//  ****************************************************************************
/// Object to manage socket and notification callbacks.
///
template<typename SocketType>
class SocketNotify
{
public:
  SocketNotify() : 
        isSelectRead_(false),
        isSelectWrite_(false),
        isSelectError_(false)  { }

  //  Typedef ******************************************************************
  typedef cxxhook::callback_type::V1<const SocketType&> CallbackType;
  typedef typename std::shared_ptr<SocketType>    SocketPtr;

  //  Status *******************************************************************
  bool IsValid( ) const { return !!spSocket_.get();}

  //  Methods ******************************************************************
  SocketPtr GetSocket   ()                      { return spSocket_;}
  void      SetSocket   (SocketPtr  spSocket)   { spSocket_ = spSocket;}

  // Clear handlers
  void ClearRead   ()                           { read_ = Nil_(); SetSelectRead_();}
  void ClearSend   ()                           { send_ = Nil_(); SetSelectWrite_();}
  void ClearAccept ()                           { accept_ = Nil_(); SetSelectRead_();}
  void ClearClose  ()                           { close_ = Nil_(); SetSelectRead_();}
  void ClearConnect()                           { connect_ = Nil_(); SetSelectWrite_(); SetSelectError_();}
  void ClearOOB    ()                           { OOB_ = Nil_(); SetSelectRead_(); SetSelectError_();}

  // Set handlers
  void SetRead     (CallbackType &notifyRead)   { read_ = notifyRead; SetSelectRead_();}
  void SetSend     (CallbackType &notifySend)   { send_ = notifySend; SetSelectWrite_();}
  void SetAccept   (CallbackType &notifyAccept) { accept_ = notifyAccept; SetSelectRead_();}
  void SetClose    (CallbackType &notifyClose)  { close_ = notifyClose; SetSelectRead_();}
  void SetConnect  (CallbackType &notifyConnect){ connect_ = notifyConnect; SetSelectWrite_(); SetSelectError();}
  void SetOOB      (CallbackType &notifyOOB)    { OOB_ = notifyOOB; SetSelectWrite_(); SetSelectError();}
  
  // Indicates Select Group
  bool IsSelectRead  () const                   { return isSelectRead_;}
  bool IsSelectWrite () const                   { return isSelectWrite_;}
  bool IsSelectError () const                   { return isSelectError_;}

  // Fire Event Notifications
  void OnRead()                                 { if ( IsValid()
                                                    && read_ != Nil_())
                                                    read_(*spSocket_);
                                                } 
  void OnSend()                                 { if ( IsValid()
                                                    && send_ != Nil_())
                                                    send_(*spSocket_);
                                                } 
  void OnAccept()                               { if ( IsValid()
                                                    && accept_ != Nil_())
                                                    accept_(*spSocket_);
                                                } 
  void OnClose()                                { if ( IsValid()
                                                    && close_ != Nil_())
                                                    close_(*spSocket_);
                                                } 
  void OnConnect()                              { if ( IsValid()
                                                    && connect_ != Nil_())
                                                    connect_(*spSocket_);
                                                } 
  void OnOOB()                                  { if ( IsValid()
                                                    && OOB_ != Nil_())
                                                    OOB_(*spSocket_);
                                                } 

private:
  //  Members ******************************************************************
  SocketPtr    spSocket_;
  CallbackType read_;
  CallbackType send_;
  CallbackType accept_;
  CallbackType close_;
  CallbackType connect_;
  CallbackType OOB_;

  bool         isSelectRead_;
  bool         isSelectWrite_;
  bool         isSelectError_;

  //  Methods ******************************************************************
  CallbackType  Nil_() const  {return CallbackType::Nil();}

  void SetSelectRead_() {
                          isSelectRead_   = ( read_ != Nil_() 
                                           || accept_ != Nil_()
                                           || close_ != Nil_()
                                           || OOB_ != Nil_());
                        }
  void SetSelectWrite_(){
                          isSelectWrite_  = ( send_ != Nil_() 
                                           || connect_ != Nil_())
                        }
  void SetSelectError_(){
                          isSelectError_  = ( connect_ != Nil_() 
                                           || OOB_ != Nil_())
                        }
};

} // namespace unnamed

//  ****************************************************************************
template<typename SocketType>
class BasicSelector
{
public:
  //  Typedef ******************************************************************
  typedef typename SocketType::EndpointType         EndpointType;
  typedef typename EndpointType::ProtocolType       ProtocolType;
  typedef BasicSelector<SocketType>                 ThisType;
  typedef cxxhook::callback_type::V1<const SocketType&> CallbackType;
  typedef typename std::shared_ptr<SocketType>      SocketPtr;
  typedef SocketNotify<SocketType>                  SocketNotifyType;

  //  Construction *************************************************************
  BasicSelector ( ) 
  { 
    FD_ZERO(&read_);
    FD_ZERO(&write_);
    FD_ZERO(&error_);
  }

  ~BasicSelector ( ) { }

  //  Status *******************************************************************
  bool   IsValid       ( ) const  { return true;}
  size_t Size          ( ) const  { return sockets_.size();}
  size_t Capacity      ( ) const  { return FD_SETSIZE;}
  void   Clear         ( )        { sockets_.clear();
                                    m_isDirty = true;
                                  }

  //  Methods ******************************************************************
  //  ****************************************************************************
  /// Adds a new socket to the selector for monitoring.
  /// @param spSocket[in]: smartPtr to the socket to monitor.
  ///
  /// @return  Success returns an id to refer back to the socket.  
  ///          k_invalidSocket is returned on an error.
  ///
  int AddSocket     (const SocketPtr &spSocket)
  { 
    if ( !spSocket
      || !spSocket->IsValid())
      return error::k_invalidData;

    if (Size() >= Capacity())
      return error::k_socketNoBuffers;

    // Initialize the notification object.
    SocketNotifyType notify;
    notify.SetSocket(spSocket);

    // Attempt to insert the new socket record.
    notifyMap::value_type entry(spSocket->Native(), notify);
    mapInsertReturnType retVal = sockets_.insert(entry);
    if (!retVal.second)
      return detail::k_invalidSocket;

    return spSocket->Native();
  }

  void RemoveSocket (int id)  { sockets_.erase(id);}

  void ClearRead    (int id)  { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.ClearRead(); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void ClearSend    (int id)  { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.ClearSend(); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void ClearAccept  (int id)  { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.ClearAccept(); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void ClearClose   (int id)  { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.ClearClose(); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void ClearConnect (int id)  { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.ClearConnect(); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void ClearOOB     (int id)  { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.ClearOOB(); 
                                  sockets_[id] = notify; 
                                } 
                              }

  void SetRead      (int id, CallbackType &notifyRead)
                              { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.SetRead(notifyRead); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void SetSend      (int id, CallbackType &notifySend)
                              { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.SetSend(notifySend); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void SetAccept    (int id, CallbackType &notifyAccept)
                              { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.SetAccept(notifyAccept); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void SetClose     (int id, CallbackType &notifyClose)
                              { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.SetClose(notifyClose); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void SetConnect   (int id, CallbackType &notifyConnect)
                              { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.SetConnect(notifyConnect); 
                                  sockets_[id] = notify; 
                                } 
                              }
  void SetOOB       (int id, CallbackType &notifyOOB)
                              { SocketNotifyType notify; 
                                if (GetSocketNotify_(id, notify)) 
                                { 
                                  notify.SetOOB(notifyOOB); 
                                  sockets_[id] = notify; 
                                } 
                              }

  unsigned long Wait(unsigned long timeout = k_infinite);

protected:
  //  Typedef ******************************************************************
  typedef std::map<detail::socketType, SocketNotifyType > notifyMap;
  typedef typename notifyMap::iterator                    notifyMapIter;
  typedef std::pair<notifyMapIter, bool>                  mapInsertReturnType;

  //  Members ******************************************************************
  notifyMap sockets_;
  fd_set    read_;
  fd_set    write_;
  fd_set    error_;

  //  Methods ******************************************************************
  //  **************************************************************************
  bool GetSocketNotify_(int id, SocketNotifyType &notify)  
  { notifyMapIter entry = sockets_.find(id);
    if (entry != sockets_.end())
    {
      notify = entry->second;
      return true;
    }
    
    return false;
  }

  //  Update the FD Arrays for a call to select ********************************
  void UpdateFdSet_(int id, SocketNotifyType &notify)
  {
    if (notify.IsSelectRead())
      FD_SET(id, &read_);
    else
      FD_CLR(id, &read_);

    if (notify.IsSelectWrite())
      FD_SET(id, &write_);
    else
      FD_CLR(id, &write_);

    if (notify.IsSelectError())
      FD_SET(id, &error_);
    else
      FD_CLR(id, &error_);
  }

  //  **************************************************************************
  void ProcessEvents_ ()
  {
    // Cycle through the available sockets and process any events if necessary.
    BYTE buffer = 0;

    notifyMapIter cur = sockets_.begin();
    notifyMapIter end = sockets_.end();
    for (cur; cur != end; ++cur)
    {
      if (FD_ISSET(cur->first, &read_))
      {
        SocketPtr spSocket = cur->second.GetSocket();
        // Differentiate which event occurred for this socket.
        int result = spSocket->Receive(&buffer, 0, SocketType::k_messagePeek);
        if (result == 0)
          cur->second.OnAccept();
        else if (result > 0)
          cur->second.OnRead();
        else if (error::k_socketNotConnected == spSocket->Error())
          cur->second.OnClose();
        else 
          cur->second.OnOOB();
      }

      if (FD_ISSET(cur->first, &write_))
      {
        SocketPtr spSocket = cur->second.GetSocket();
        // Differentiate which event occurred for this socket.
        int result = spSocket->Send(&buffer, 0);
        if (result == error::k_socketWouldBlock)
          cur->second.OnSend();
        else
          cur->second.OnConnect();
      }

      if (FD_ISSET(cur->first, &error_))
      {
        SocketPtr spSocket = cur->second.GetSocket();
        // Differentiate which event occurred for this socket.
        int result = spSocket->Receive(&buffer, 0, SocketType::k_messageOutOfBand);
        if (result > 0)
          cur->second.OnOOB();
        else 
          cur->second.OnConnect();
      }
    }
  }
};

//  ****************************************************************************
/// This function is the equivalent of a select call for sockets.
/// Waits for any of the configured sockets to be triggered by an
/// event registered for the socket.  
///
/// @param timeout[in]: The number of ms to wait for an event to occur.  
///               If the event does not occur in this time, the function will return
///               with a timeout indicated.
///
/// @return		  k_waitSuccess is returned if any of the monitored sockets are signalled.
///             k_timeout is returned of the wait operation timeout expires.
///             The network error code is returned on an error.
/// 
template<typename SocketType>
inline unsigned long BasicSelector<SocketType>::Wait(unsigned long timeout)
{ 
  // Update the selector arrays if necessary.
  notifyMapIter cur = sockets_.begin();
  notifyMapIter end = sockets_.end();
  for (cur; cur != end; ++cur)
  {
    UpdateFdSet_(cur->first, cur->second);
  }

unsigned long timeOut_ = 2000;

  timeval to;
  to.tv_sec = timeOut_ / 1000;            // convert ms to seconds.
  to.tv_usec= (timeOut_ % 1000) / 1000;   // convert remainder ms to us.
  int result = ::select(sockets_.size(), &read_, &write_, &error_, &to);
  // Filter out based on errors, or timeouts.
  if (detail::k_socketError == result)
    return SocketType::Error();
  else if (0 == result)
    return status::k_timeout;

  // Socket events were triggered, process them.
  BLASSERT(result > 0 && static_cast<size_t>(result) < Capacity());

  ProcessEvents_ ();

  return status::k_waitSuccess;
}

} // namespace ipc
} // namespace cxxhook


#endif 
