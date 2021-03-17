#ifndef _ACMITAPE_H_
#define _ACMITAPE_H_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// ACMI_CallRec
#include "Acmirec.h"

#include <vector>
#include <memory>

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
typedef struct ACMITapeHeader
{
	int32_t		fileID = 0;
	int32_t		fileSize = 0;
	int32_t		numEntities = 0;
	int32_t		numFeat = 0;
	int32_t 		entityBlockOffset = 0;			// 5th
	int32_t 		featBlockOffset = 0;
	int32_t		numEntityPositions = 0;
	int32_t		timelineBlockOffset = 0;
	int32_t		firstEntEventOffset = 0;
	int32_t		firstGeneralEventOffset = 0;	// 10th => 40 Bytes
	int32_t		firstEventTrailerOffset = 0;
	int32_t		firstTextEventOffset = 0;
	int32_t		firstFeatEventOffset = 0;
	int32_t		numEvents = 0;
	int32_t		numEntEvents = 0;				// 15th => 60 Byte
	int32_t		numTextEvents = 0;
	int32_t		numFeatEvents = 0;
	float		startTime = 0;
	float		totPlayTime = 0;
	float 		todOffset = 0;
} ACMITapeHeader;
#pragma pack (pop, pack1)

////////////////////////////////////////////////////////////////////////////////
//
// Entity data.

#pragma pack (push, pack1, 1)
typedef struct ACMIEntityData
{
	int32_t		uniqueID = 0;
	int32_t		type = 0;
	int32_t		count = 0;
	int32_t		flags = 0;
	
		#define		ENTITY_FLAG_MISSILE			0x00000001
		#define		ENTITY_FLAG_FEATURE			0x00000002
		#define		ENTITY_FLAG_AIRCRAFT		0x00000004
		#define		ENTITY_FLAG_CHAFF			0x00000008
		#define		ENTITY_FLAG_FLARE			0x00000010

	// for features we may need an index to the lead component and
	// the slot # that was in the camp component list (for bridges, bases...)
	int32_t		leadIndex = 0;
	int32_t			slot = 0;
	int32_t			specialFlags = 0;


	// Offset from the start of the file to the start of my positional data.
	int32_t 		firstPositionDataOffset = 0;
	int32_t 		firstEventDataOffset = 0;

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
typedef struct ACMIEntityPositionData
{
	// Time stamp for the positional data
	float		time = 0.0;
	BYTE		type = 0;

	// dereference based on type
	union
	{
		// Positional data.
		struct
		{
			float		x = 0.0;
			float		y = 0.0;
			float		z = 0.0;
			float		pitch = 0.0;
			float		roll = 0.0;
			float		yaw = 0.0;
			int32_t	    radarTarget = 0;
		} posData;
		// switch change
		struct
		{
			int32_t			switchNum = 0;
			int32_t			switchVal = 0;
			int32_t			prevSwitchVal = 0;
		} switchData;
		// DOF change
		struct
		{
			int32_t			DOFNum = 0;
			float		DOFVal = 0.0;
			float		prevDOFVal = 0.0;
		} dofData;
	};

	// Although position data is a fixed size, we still want
	// this so that we can organize the data to be friendly for
	// paging.
	int32_t		nextPositionUpdateOffset = 0;
	int32_t		prevPositionUpdateOffset = 0;
} ACMIEntityPositionData;
#pragma pack (pop, pack1)

//
// This raw format is used by the position/event/sfx bundler to
// create a .vhs file (dig that extension), which is the ACMITape playback format.
// This is the format stored in the flight file.

typedef struct ACMIRawPositionData
{
	int32_t			type = 0;			// type of object
	int32_t		uniqueID = 0;		// A unique ID for the object. Many to One correlation to Falcon Entities
	int32_t			flags = 0;			// side

	// for features we may need an index to the lead component and
	// the slot # that was in the camp component list (for bridges, bases...)
	int32_t		leadIndex = 0;
	int32_t			slot = 0;
	int32_t			specialFlags = 0;
	ACMIEntityPositionData entityPosData = {};
} ACMIRawPositionData;

////////////////////////////////////////////////////////////////////////////////
//
// Header for event data.

#pragma pack (push, pack1, 1)
typedef struct ACMIEventHeader
{
	// type of event this is
	BYTE		eventType = 0;
	int32_t 		index = -42;

	// Time stamp for this event.
	float		time = 0;
	float		timeEnd = 0;

	// data specific to type of event
	int32_t		type = 0;
	int32_t		user = 0;
	int32_t		flags = 0;
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
typedef struct ACMIEventTrailer
{
	float		timeEnd = 0;
	int32_t 		index = 0;		// into EventHeader
} ACMIEventTrailer;
#pragma pack (pop, pack1)

////////////////////////////////////////////////////////////////////////////////
//
// Feature Status Event

#pragma pack (push, pack1, 1)
typedef struct ACMIFeatEvent
{
	// Time stamp for this event.
	float		time = 0.0;

	// index of feature on tape
	int32_t 		index = 0;

	// data specific to type of event
	int32_t		newStatus = 0;
	int32_t		prevStatus = 0;

} ACMIFeatEvent;
#pragma pack (pop, pack1)

typedef struct ACMIFeatEventImportData
{
	int32_t		uniqueID = 0;
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
// |  sizeof(int32_t)      | num entities * sizeof(ACMIEntityData) |
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



typedef struct ACMIEntityReadHead
{
	int32_t			positionDataOffset = 0;
	int32_t			eventDataOffset = 0;
} ACMIEntityReadHead;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ACMITape
{
public:
	
	// Constructors.
	ACMITape() = default;

	// Import the current positional, event, and sfx data.
	// The filenames of these files will always be the same 
	// so we don't have to pass them in.
	bool Import(const char *inFltFile, const char *outTapeFileName);
	
	
private:

	// These are used for importation.
	void ParseEntities(void);
	void ThreadEntityPositions(ACMITapeHeader *tapeHdr);
	void ThreadEntityEvents(ACMITapeHeader *tapeHdr);
	void ImportTextEventList(FILE *fd, ACMITapeHeader *tapeHdr);
	bool WriteTapeFile(const char *fname, ACMITapeHeader *tapeHdr);


	int32_t tempTarget = 0; // for missile lock.
	
	/*Converted list to vector*/
	std::vector<ACMIEntityData> importEntityVec;				// List of entities
	std::vector<ACMIEntityData> importFeatVec;					// List of Features
	std::vector<ACMIRawPositionData> importPosVec;				// List of Positions
	std::vector<ACMIEventHeader> importEventVec;				// List of Event
	std::vector<ACMIRawPositionData> importEntEventVec;			// 
	std::vector<ACMIFeatEventImportData> importFeatEventVec;


	std::unique_ptr<ACMI_CallRec[]> Import_Callsigns;
	int32_t import_count = 0;


};



#endif  // _ACMITAPE_H_

