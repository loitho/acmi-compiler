// vu2.h - Copyright (c) Mon July 15 15:01:01 1996,  Spectrum HoloByte, Inc.  All Rights Reserved

#ifndef _VU_2_H_
#define _VU_2_H_
#pragma warning (disable : 4514)
#define VU_DEFAULT_GROUP_SIZE	6

// -----------------
//    VU options
// -----------------
#define VU_USE_COMMS			1
#define VU_SIMPLE_LATENCY		1
/* #define VU_TRACK_LATENCY  	1 */
/* #define VU_AUTO_UPDATE  		1 */
#define VU_AUTO_KEEPALIVE       1
#define VU_THREAD_SAFE  		1
#define VU_GRID_TREE_Y_MAJOR	1
#define FALCON4					1
#define SMOOTH_ROUGH_POSITIONS	1

// set this to queue dead reckon updates 
#define VU_QUEUE_DR_UPDATES		1

// set this to zero to disable rough updates
#define VU_MAX_SESSION_CAMERAS	256//me123 from 2

// Set this if you don't want to auto-dispatch
#define REALLOC_QUEUES			1

// define this to use quaterions rather than YPR for orientation data
// #define VU_USE_QUATERNION 1

// define this to send 8 byte class info with create msgs
// #define VU_SEND_CLASS_INFO 1

// KCK: Our version of Vu's FTOL
#if defined(_MSC_VER)
extern int FloatToInt32(float);
#endif

#define FTOL FloatToInt32

#ifdef VU_USE_COMMS
#include "comms\capi.h"
#endif // VU_USE_COMMS

#include "apitypes.h"
#include "vu2/include/vuentity.h"
#include "vu2/include/vucoll.h"
#include "vu2/include/vuevent.h"
#include "vu2/include/vudriver.h"
#include "vu2/include/vumath.h"
#include "vu2/include/vusessn.h"
#include "vu2/include/vu.h"

// For Debug
#ifndef NDEBUG
#include "debuggr.h"
#define VU_PRINT   MonoPrint
#else
#define VU_PRINT(a)
#endif
#pragma warning (disable : 4514)
#endif // _VU_2_H_
