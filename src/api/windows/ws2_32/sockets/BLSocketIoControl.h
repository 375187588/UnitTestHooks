/// @file: BLSocketIoControl.h
///
/// Abstraction for the IOControl commands for use with socket communications.  
/// This class will mimic the abstraction of the options class created
/// for sockets as well.  However, the nature of the IOCTLs forces
/// the implementation to be explicit for each property.
/// 
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// BLNetwork.h contains additional copyright information. 
///
//  ****************************************************************************
#ifndef BLSOCKETIOCONTROL_H_INCLUDED
#define BLSOCKETIOCONTROL_H_INCLUDED
//  Includes *******************************************************************
#include "BLSocketDef.h"

namespace cxxhook
{
namespace ipc
{
namespace detail
{
namespace ioctl
{

//  ****************************************************************************
/// Helper abstraction for the non-blocking socket option.
///
class NonBlockingIo
{
public:
            NonBlockingIo           ()       : value_(0)          { }
            NonBlockingIo           (bool v) : value_(v ? 1 : 0)  { }

  operator  bool                    () const    { return Value();}
  bool      operator!               () const    { return !Value();}

  int       Name                    () const    { return FIONBIO;}
  bool      Value                   () const    { return value_ != 0;}
  void      Value                   (bool value){ value_ = value ? 1 : 0;}

  detail::ioctlArgType* Data        ()          { return &value_;}
  const detail::ioctlArgType* Data  () const    { return &value_;}

private:
  detail::ioctlArgType  value_;
};

//  ****************************************************************************
/// Helper abstraction for the getting the number of bytes available.
///
class ReadableBytes
{
public:
              ReadableBytes         ()              
                  : value_(0)                       { }
              ReadableBytes         (std::size_t v) 
                  : value_(static_cast<detail::ioctlArgType>(v))  { }

              operator std::size_t  () const        { return Size();}

  int         Name                  () const        { return FIONREAD;}
  std::size_t Size                  () const        { return static_cast<std::size_t>(value_);}
  void        Size                  (std::size_t v) { static_cast<detail::ioctlArgType>(v);}

  detail::ioctlArgType*       Data  ()              { return &value_;}
  const detail::ioctlArgType* Data  () const        { return &value_;}

private:
  detail::ioctlArgType  value_;
};


} // ioctl
} // detail
} // namespace ipc
} // namespace cxxhook

#endif 
