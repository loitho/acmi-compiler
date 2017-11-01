#ifndef _ACMITAPE_H_
#define _ACMITAPE_H_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// ACMI_CallRec
#include "acmirec.h"

#include <vector>

typedef unsigned char BYTE;

#define	MAX_ENTITY_CAMS	50

#define ACMI_VERSION 2

#define ACMI_LABEL_LEN 15

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
	long		fileID = 0;
	long		fileSize = 0;
	long		numEntities = 0;
	long		numFeat = 0;
	long 		entityBlockOffset = 0;			// 5th
	long 		featBlockOffset = 0;
	long		numEntityPositions = 0;
	long		timelineBlockOffset = 0;
	long		firstEntEventOffset = 0;
	long		firstGeneralEventOffset = 0;	// 10th => 40 Bytes
	long		firstEventTrailerOffset = 0;
	long		firstTextEventOffset = 0;
	long		firstFeatEventOffset = 0;
	long		numEvents = 0;
	long		numEntEvents = 0;				// 15th => 60 Byte
	long		numTextEvents = 0;
	long		numFeatEvents = 0;
	float		startTime = 0;
	float		totPlayTime = 0;
	float 		todOffset = 0;
} ACMITapeHeader;
#pragma pack (pop, pack1)

////////////////////////////////////////////////////////////////////////////////
//
// Entity data.

#pragma pack (push, pack1, 1)
typedef struct 
{
	long		uniqueID = 0;
	long		type = 0;
	long		count = 0;
	long		flags = 0;
	
		#define		ENTITY_FLAG_MISSILE			0x00000001
		#define		ENTITY_FLAG_FEATURE			0x00000002
		#define		ENTITY_FLAG_AIRCRAFT		0x00000004
		#define		ENTITY_FLAG_CHAFF			0x00000008
		#define		ENTITY_FLAG_FLARE			0x00000010

	// for features we may need an index to the lead component and
	// the slot # that was in the camp component list (for bridges, bases...)
	long		leadIndex = 0;
	int			slot = 0;
	int			specialFlags = 0;


	// Offset from the start of the file to the start of my positional data.
	long 		firstPositionDataOffset = 0;
	long 		firstEventDataOffset = 0;

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
	long		nextPositionUpdateOffset = 0;
	long		prevPositionUpdateOffset = 0;
} ACMIEntityPositionData;
#pragma pack (pop, pack1)

//
// This raw format is used by the position/event/sfx bundler to
// create a .vhs file (dig that extension), which is the ACMITape playback format.
// This is the format stored in the flight file.

typedef struct 
{
	int			type = 0;			// type of object
	long		uniqueID = 0;		// A unique ID for the object. Many to One correlation to Falcon Entities
	int			flags = 0;			// side

	// for features we may need an index to the lead component and
	// the slot # that was in the camp component list (for bridges, bases...)
	long		leadIndex = 0;
	int			slot = 0;
	int			specialFlags = 0;
	ACMIEntityPositionData entityPosData = {};
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
	float		time = 0;
	float		timeEnd = 0;

	// data specific to type of event
	long		type = 0;
	long		user = 0;
	long		flags = 0;
	float		scale = 0;
	float		x = 0;
	float		y = 0;
	float		z = 0;
	float		dx = 0;
	float		dy = 0;
	float		dz = 0;
	float		roll = 0;
	float		pitch = 0;
	float		yaw = 0;

} ACMIEventHeader;
#pragma pack (pop, pack1)

//
// Trailer for event data.
//

#pragma pack (push, pack1, 1)
typedef struct
{
	float		timeEnd = 0;
	long 		index = 0;		// into EventHeader
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
	long		newStatus = 0;
	long		prevStatus = 0;

} ACMIFeatEvent;
#pragma pack (pop, pack1)

typedef struct
{
	long		uniqueID = 0;
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
	long			positionDataOffset = 0;
	long			eventDataOffset = 0;
} ACMIEntityReadHead;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ACMITape
{
public:
	
	// Constructors.
	ACMITape();

	// Destructor.
	~ACMITape();

	// Import the current positional, event, and sfx data.
	// The filenames of these files will always be the same 
	// so we don't have to pass them in.
	bool Import(char *inFltFile, char *outTapeFileName);
	
	
private:

	// These are used for importation.
	void ParseEntities(void);
	void ThreadEntityPositions(ACMITapeHeader *tapeHdr);
	void ThreadEntityEvents(ACMITapeHeader *tapeHdr);
	void ImportTextEventList(FILE *fd, ACMITapeHeader *tapeHdr);
	void WriteTapeFile(char *fname, ACMITapeHeader *tapeHdr);


	long tempTarget; // for missile lock.
	
	/*Converted list to vector*/
	std::vector<ACMIEntityData> importEntityVec;
	std::vector<ACMIEntityData> importFeatVec;
	std::vector<ACMIRawPositionData> importPosVec;
	std::vector<ACMIEventHeader> importEventVec;
	std::vector<ACMIRawPositionData> importEntEventVec;
	std::vector<ACMIFeatEventImportData> importFeatEventVec;


	//std::vector<ACMI_CallRec> Import_Callsigns;
	ACMI_CallRec *Import_Callsigns = NULL;
	long import_count = 0;


};



#endif  // _ACMITAPE_H_
