/*
** Name: ACMIREC.H
** Description:
**		Recorder class for writing an ACMI recording in raw data format.
**		Types of records are defined here.
** History:
**		13-oct-97 (edg)
**			We go dancing in.....
** Here we are in 21 october 2017, 20 years after ! (loitho)
*/
#ifndef _ACMIREC_H_
#define _ACMIREC_H_

#include "tchar.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
** These are the enumerated record types
*/
enum
{
	ACMIRecGenPosition = 0,
	ACMIRecMissilePosition,
	ACMIRecFeaturePosition,
	ACMIRecAircraftPosition,
	ACMIRecTracerStart,
	ACMIRecStationarySfx,
	ACMIRecMovingSfx,
	ACMIRecSwitch,
	ACMIRecDOF,
	ACMIRecChaffPosition,
	ACMIRecFlarePosition,
	ACMIRecTodOffset,
	ACMIRecFeatureStatus,
	ACMICallsignList,
	ACMIRecMaxTypes
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
** Record structure typedefs for each type of record
*/

//
// ACMIRecHeader
// this struct is common thru all record types as a record header
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	BYTE		type;		// one of the ennumerated types
	float		time;		// time stamp
} ACMIRecHeader;
#pragma pack (pop, pack1)

//
// ACMIGenPositionData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	int		type = 0;			// base type for creating simbase object
	long	uniqueID = 0;		// identifier of instance
	float	x = 0;
	float	y = 0;
	float	z = 0;
	float	yaw = 0;
	float	pitch = 0;
	float 	roll = 0;
} ACMIGenPositionData;
#pragma pack (pop, pack1)

//
// ACMIFeaturePositionData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	int		type = 0;			// base type for creating simbase object
	long	uniqueID = 0;		// identifier of instance
	long	leadUniqueID = 0;	// id of lead component (for bridges. bases etc)
	int		slot = 0;			// slot number in component list
	int		specialFlags = 0;   // campaign feature flag
	float	x = 0;
	float	y = 0;
	float	z = 0;
	float	yaw = 0;
	float	pitch = 0;
	float 	roll = 0;
} ACMIFeaturePositionData;
#pragma pack (pop, pack1)

/*
** ACMI Text event (strings parsed from event file)
** Which we don't have, so this is never used
*/
#pragma pack (push, pack1, 1)
typedef struct
{
	long	   intTime;
	_TCHAR timeStr[20];
	_TCHAR msgStr[100];
} ACMITextEvent;
#pragma pack (pop, pack1)

//
// ACMISwitchData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	int		type;			// base type for creating simbase object
	long	uniqueID;		// identifier of instance
	int		switchNum;
	int		switchVal;
	int		prevSwitchVal;
} ACMISwitchData;
#pragma pack (pop, pack1)

//
// ACMIFeatureStatusData
// Feature status change data
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	long	uniqueID;		// identifier of instance
	int		newStatus;
	int		prevStatus;
} ACMIFeatureStatusData;
#pragma pack (pop, pack1)

//
// ACMIDOFData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	int		type;			// base type for creating simbase object
	long	uniqueID;		// identifier of instance
	int		DOFNum;
	float	DOFVal;
	float	prevDOFVal;
} ACMIDOFData;
#pragma pack (pop, pack1)

//
// ACMITracerStartData
// Starting pos and velocity of tracer rounds
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	// initial values
	float	x;
	float	y;
	float	z;
	float	dx;
	float	dy;
	float 	dz;
} ACMITracerStartData;
#pragma pack (pop, pack1)

//
// ACMIStationarySfxData
// Starting pos of a staionay special sfx
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	int		type;		// sfx type
	float	x;			// position
	float	y;
	float	z;
	float	timeToLive;
	float	scale;
} ACMIStationarySfxData;
#pragma pack (pop, pack1)

//
// ACMIMovingSfxData
// Starting pos of a staionay special sfx
//
#pragma pack (push, pack1, 1)
typedef struct 
{
	int		type;		// sfx type
	int		user;		// misc data
	int		flags;
	float	x;			// position
	float	y;
	float	z;
	float	dx;			// vector
	float	dy;
	float	dz;
	float	timeToLive;
	float	scale;
} ACMIMovingSfxData;
#pragma pack (pop, pack1)

// these are the actual I/O records
#pragma pack (push, pack1, 1)

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIMovingSfxData		data;
} ACMIMovingSfxRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIStationarySfxData		data;
} ACMIStationarySfxRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIGenPositionData			data;
} ACMIGenPositionRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIGenPositionData			data;
} ACMIMissilePositionRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
} ACMITodOffsetRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIGenPositionData			data;
} ACMIChaffPositionRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIGenPositionData			data;
} ACMIFlarePositionRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIGenPositionData			data;
	long						RadarTarget;
	
} ACMIAircraftPositionRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIFeaturePositionData			data;
} ACMIFeaturePositionRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIFeatureStatusData			data;
} ACMIFeatureStatusRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMITracerStartData			data;
} ACMITracerStartRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMISwitchData				data;
} ACMISwitchRecord;

typedef struct 
{
	ACMIRecHeader				hdr;
	ACMIDOFData				data;
} ACMIDOFRecord;
#pragma pack (pop, pack1)





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#pragma pack (1)
struct ACMI_CallRec
{
	char label[16];
	long teamColor;
};
#pragma pack()

#endif  // _ACMIREC_H_

