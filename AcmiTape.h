#ifndef _ACMITAPE_H_
#define _ACMITAPE_H_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Needed for BOOL type
#include "tchar.h"

#define	MAX_ENTITY_CAMS	50

#define ACMI_VERSION 2

#define ACMI_LABEL_LEN 15

class RViewPoint;
class RenderOTW;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// These are the headers and data that are used internally for the .vhs format.
// These use offsets instead of pointers so that we can memory map them.
// All offsets are from the start of the file!!!.

////////////////////////////////////////////////////////////////////////////////
//
// Header for the tape file.

#pragma pack (push, pack1, 1)
typedef struct 
{
	long		fileID;
	long		fileSize;
	long		numEntities;
	long		numFeat;
	long 		entityBlockOffset;
	long 		featBlockOffset;
	long		numEntityPositions;
	long		timelineBlockOffset;
	long		firstEntEventOffset;
	long		firstGeneralEventOffset;
	long		firstEventTrailerOffset;
	long		firstTextEventOffset;
	long		firstFeatEventOffset;
	long		numEvents;
	long		numEntEvents;
	long		numTextEvents;
	long		numFeatEvents;
	float		startTime;
	float		totPlayTime;
	float 		todOffset;
} ACMITapeHeader;
#pragma pack (pop, pack1)

////////////////////////////////////////////////////////////////////////////////
//
// Entity data.

#pragma pack (push, pack1, 1)
typedef struct 
{
	long		uniqueID;
	long		type;
	long		count;
	long		flags;
	
		#define		ENTITY_FLAG_MISSILE			0x00000001
		#define		ENTITY_FLAG_FEATURE			0x00000002
		#define		ENTITY_FLAG_AIRCRAFT		0x00000004
		#define		ENTITY_FLAG_CHAFF			0x00000008
		#define		ENTITY_FLAG_FLARE			0x00000010

	// for features we may need an index to the lead component and
	// the slot # that was in the camp component list (for bridges, bases...)
	long		leadIndex;
	int			slot;
	int			specialFlags;


	// Offset from the start of the file to the start of my positional data.
	long 		firstPositionDataOffset;
	long 		firstEventDataOffset;

} ACMIEntityData;
#pragma pack (pop, pack1)

////////////////////////////////////////////////////////////////////////////////
//
// Entity position data.

// enum types for position
enum
{
	PosTypePos = 0,
	PosTypeSwitch,
	PosTypeDOF,
};

#pragma pack (push, pack1, 1)
typedef struct
{
	// Time stamp for the positional data
	float		time;
	BYTE		type;

	// dereference based on type
	union
	{
		// Positional data.
		struct posTag
		{
			float		x;
			float		y;
			float		z;
			float		pitch;
			float		roll;
			float		yaw;
			long	    radarTarget;
		} posData;
		// switch change
		struct switchTag
		{
			int			switchNum;
			int			switchVal;
			int			prevSwitchVal;
		} switchData;
		// DOF change
		struct dofTag
		{
			int			DOFNum;
			float		DOFVal;
			float		prevDOFVal;
		} dofData;
	};

	// Although position data is a fixed size, we still want
	// this so that we can organize the data to be friendly for
	// paging.
	long		nextPositionUpdateOffset;
	long		prevPositionUpdateOffset;
} ACMIEntityPositionData;
#pragma pack (pop, pack1)

//
// This raw format is used by the position/event/sfx bundler to
// create a .vhs file (dig that extension), which is the ACMITape playback format.
// This is the format stored in the flight file.

typedef struct 
{
	int			type;			// type of object
	long		uniqueID;		// A unique ID for the object. Many to One correlation to Falcon Entities
	int			flags;			// side

	// for features we may need an index to the lead component and
	// the slot # that was in the camp component list (for bridges, bases...)
	long		leadIndex;
	int			slot;
	int			specialFlags;
	ACMIEntityPositionData entityPosData;
} ACMIRawPositionData;

////////////////////////////////////////////////////////////////////////////////
//
// Header for event data.

#pragma pack (push, pack1, 1)
typedef struct
{
	// type of event this is
	BYTE		eventType;
	long 		index;

	// Time stamp for this event.
	float		time;
	float		timeEnd;

	// data specific to type of event
	long		type;
	long		user;
	long		flags;
	float		scale;
	float		x, y, z;
	float		dx, dy, dz;
	float		roll, pitch, yaw;

} ACMIEventHeader;
#pragma pack (pop, pack1)

//
// Trailer for event data.
//

#pragma pack (push, pack1, 1)
typedef struct
{
	float		timeEnd;
	long 		index;		// into EventHeader
} ACMIEventTrailer;
#pragma pack (pop, pack1)

////////////////////////////////////////////////////////////////////////////////
//
// Feature Status Event

#pragma pack (push, pack1, 1)
typedef struct
{
	// Time stamp for this event.
	float		time;

	// index of feature on tape
	long 		index;

	// data specific to type of event
	long		newStatus;
	long		prevStatus;

} ACMIFeatEvent;
#pragma pack (pop, pack1)

typedef struct
{
	long		uniqueID;
	ACMIFeatEvent data;
} ACMIFeatEventImportData;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// .vhs file format:
//
// |                        |                 |                 |
// |        header          | entity block    | timeline block  |
// | sizeof(ACMITapeHeader) | (variable size) | (variable size) |
//
////////////////////////////////////////////////////////////////////////////////
//
// entity block:
//
// |                    |                                       |
// | number of entities |              entities                 |    
// |  sizeof(long)      | num entities * sizeof(ACMIEntityData) |
//
// entity:
//
// |                        |
// |      ACMIEntityData    |
// | sizeof(ACMIEntityData) |
//
////////////////////////////////////////////////////////////////////////////////
//
// timeline block:
//
// |                              |                    |                     |
// | entity position update block | entity event block | general event block |   
// |     (variable size)          |  (variable size)   |    (variable size)  |
//
// The entity position update block contains all entity position updates.
// The position updates are threaded on a per-entity basis, with a separate doubly linked list
// for each entity.
// The position updates should be chronologically sorted.
// There should be a position update read-head for each entity for traversing its linked list
// of position updates.
//
// The entity event block contains all events which are relevant to entities.
// The events are threaded on a per-entity basis, with a separate doubly linked list
// for each entity.
// The events should be chronologically sorted.  
// There should be an event read-head for each entity for traversing its linked list of events.
//
// The general event block contains all events which are not relevant to a specific entity.
// The events are threaded in doubly linked list.
// The events should be chronologically sorted.
// There should be an event read-head for traversing the linked list of events.
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



typedef struct
{

	long			positionDataOffset;
	long			eventDataOffset;
} ACMIEntityReadHead;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ACMITape
{
public:
	
	// Constructors.
	// Do not put the extension with name.
	// This should be the name of the desired .vcr file.
	ACMITape(char *name, RenderOTW *renderer, RViewPoint *viewPoint);

	// Destructor.
	~ACMITape();

	// Import the current positional, event, and sfx data.
	// The filenames of these files will always be the same 
	// so we don't have to pass them in.
	static BOOL Import(char *inFltFile, char *outTapeFileName);
	static void WriteTapeFile ( char *fname, ACMITapeHeader *tapeHdr );
	
private:

	// These are used for importation.
	static void ParseEntities ( void );
	static void ThreadEntityPositions( ACMITapeHeader *tapeHdr );
	static void ThreadEntityEvents( ACMITapeHeader *tapeHdr );
	static void ImportTextEventList( FILE *fd, ACMITapeHeader *tapeHdr );

};



#endif  // _ACMITAPE_H_

