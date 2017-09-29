// apitypes.h - Copyright (c) Tue July 2 15:01:01 1996,  Spectrum HoloByte, Inc.  All Rights Reserved

#ifndef _APITYPES_H_
#define _APITYPES_H_

//#include "vu2/include/vumath.h"
#include "stdlib.h"

#ifdef USE_SH_POOLS
#include "SmartHeap/Include/shmalloc.h"
#include "SmartHeap/Include/smrtheap.hpp"
#endif

typedef int VU_ERRCODE;
#define VU_ERROR	-1
#define VU_NO_OP	0
#define VU_SUCCESS	1

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

// note: BIG_SCALAR and SM_SCALAR are defined in vumath.h

typedef unsigned long VU_DAMAGE;
typedef unsigned long VU_TIME;

#define VU_TICS_PER_SECOND 1000

typedef unsigned char VU_BYTE;
typedef unsigned char VU_BOOL;
typedef signed char VU_TRI_STATE;	// TRUE, FALSE, or DONT_CARE

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef DONT_CARE
#define DONT_CARE -1
#endif

typedef unsigned char VU_MSG_TYPE;
typedef	unsigned long VU_KEY;
typedef	unsigned long VU_ID_NUMBER;

class VU_SESSION_ID {
public:
  VU_SESSION_ID() : value_(0) {}
  VU_SESSION_ID(unsigned long value) : value_((unsigned long)value) {}

  int operator == (VU_SESSION_ID rhs)
    { return (value_ == rhs.value_ ? TRUE : FALSE); }
  int operator != (VU_SESSION_ID rhs)
    { return (value_ != rhs.value_ ? TRUE : FALSE); }
  int operator > (VU_SESSION_ID rhs)
    { return (value_ > rhs.value_ ? TRUE : FALSE); }
  int operator >= (VU_SESSION_ID rhs)
    { return (value_ >= rhs.value_ ? TRUE : FALSE); }
  int operator < (VU_SESSION_ID rhs)
    { return (value_ < rhs.value_ ? TRUE : FALSE); }
  int operator <= (VU_SESSION_ID rhs)
    { return (value_ <= rhs.value_ ? TRUE : FALSE); }

  operator unsigned long() { return (unsigned long) value_; }

// note: these are private to prevent (mis)use
private:
  int operator == (unsigned long rhs);
  int operator != (unsigned long rhs);
  int operator > (unsigned long rhs);
  int operator >= (unsigned long rhs);
  int operator < (unsigned long rhs);
  int operator <= (unsigned long rhs);

// DATA
public:
  unsigned long 	value_;
};

class VU_ID {
public:
  VU_ID() : creator_(0), num_(0) {}
  VU_ID(VU_SESSION_ID sessionpart, VU_ID_NUMBER idpart)
	  : creator_(sessionpart), num_(idpart) {}

  int operator == (VU_ID rhs)
    { return (num_ == rhs.num_ ? (creator_ == rhs.creator_ ? TRUE : FALSE)
					: FALSE); }
  int operator != (VU_ID rhs)
    { return (num_ == rhs.num_ ? (creator_ == rhs.creator_ ? FALSE : TRUE)
					: TRUE); }

  int operator > (VU_ID rhs)
	{
	if (creator_ > rhs.creator_)
		return TRUE;
	if (creator_ == rhs.creator_)
		{
		if (num_ > rhs.num_)
			return TRUE;
		}
	return FALSE;
	}
  int operator >= (VU_ID rhs)
	{
	if (creator_ > rhs.creator_)
		return TRUE;
	if (creator_ == rhs.creator_)
		{
		if (num_ >= rhs.num_)
			return TRUE;
		}
	return FALSE;
	}
  int operator < (VU_ID rhs)
	{
	if (creator_ < rhs.creator_)
		return TRUE;
	if (creator_ == rhs.creator_)
		{
		if (num_ < rhs.num_)
			return TRUE;
		}
	return FALSE;
	}
  int operator <= (VU_ID rhs)
	{
	if (creator_ < rhs.creator_)
		return TRUE;
	if (creator_ == rhs.creator_)
		{
		if (num_ <= rhs.num_)
			return TRUE;
		}
	return FALSE;
	}
  operator VU_KEY()
    { return (VU_KEY)(((unsigned short)creator_ << 16)
		      | ((unsigned short)num_)); }

// note: these are private to prevent (mis)use
private:
  int operator == (VU_KEY rhs);
  int operator != (VU_KEY rhs);
  int operator > (VU_KEY rhs);
  int operator >= (VU_KEY rhs);
  int operator < (VU_KEY rhs);
  int operator <= (VU_KEY rhs);

// DATA
public:
  VU_ID_NUMBER		num_;
  VU_SESSION_ID 	creator_;
};

#endif // _APITYPES_H_
