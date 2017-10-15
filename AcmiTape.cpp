// ACMI tape class.
// Originally written by Jim DiZoglio (x257) as ACMIView class
// last modified: 9/25/97	Michael P. Songy
// Modified SEVERAL TIMES SINCE... various people
//#pragma optimize( "", off )

// STFU _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <tchar.h>
#include <iostream>

//#include "codelib\tools\lists\lists.h"

#include "AcmiTape.h"

#include "acmirec.h"


//////////////////////////////////////////////////////////////////////////
/// 3-23 BING
//#include "AcmiView.h"
//#include "AcmiUI.h"
				
#include "threading.h"
#include <vector>

#if _DEBUG
	#define MonoPrint  printf
#else
	#define MonoPrint  NULL
#endif 
//extern ACMIView			*acmiView;


long tempTarget; // for missile lock.
				

//////////////////////////////////////////////////////////////////////////


//void CalcTransformMatrix(SimBaseClass* theObject);
//void CreateDrawable (SimBaseClass* theObject, float objectScale);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// these are for raw data import
LIST *importEntityList;
std::vector<ACMIEntityData> importEntityVec;

LIST *importFeatList;
std::vector<ACMIEntityData> importFeatVec;

LIST *importPosList;
std::vector<ACMIRawPositionData> importPosVec;

LIST *importEventList;
std::vector<ACMIEventHeader> importEventVec;

LIST *importEntEventList;
std::vector<ACMIRawPositionData> importEntEventVec;

LIST *importFeatEventList;
std::vector<ACMIFeatEventImportData> importFeatEventVec;


LIST *importEntityListEnd;
LIST *importFeatListEnd;
LIST *importPosListEnd;
LIST *importEventListEnd;
LIST *importEntEventListEnd;
LIST *importFeatEventListEnd;
int importNumPos;
int importNumEnt;
int importNumFeat;
int importNumFeatEvents;
int importNumEvents;
int importNumEntEvents;
int importEntOffset;
int importFeatOffset;
int importFeatEventOffset;
int importPosOffset;
int importEventOffset;
int importEntEventOffset;
ACMIEventTrailer *importEventTrailerList;

//extern long TeamSimColorList[NUM_TEAMS];

LIST * AppendToEndOfList( LIST * list, LIST **end, void * node );
void DestroyTheList( LIST * list );
//extern float CalcKIAS( float, float );

ACMI_CallRec *ACMI_Callsigns=NULL;
ACMI_CallRec *Import_Callsigns=NULL;
long import_count=0;

//extern GLOBAL_SPEED;
//extern GLOBAL_ALTITUDE;
//extern GLOBAL_HEADING;






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DefaultForwardACMIGeneralEventCallback
(
	ACMITape *,
	EventIdData eventId, 
	void *,
	void *
)
{
	MonoPrint
	(
		"General event occured in forward ACMI Tape play --> event type: %d.\n",
		eventId.type
	);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DefaultReverseACMIGeneralEventCallback
(
	ACMITape *,
	EventIdData eventId,
	void *,
	void *
)
{
	MonoPrint
	(
		"General event occured in reverse ACMI Tape play --> event type: %d.\n",
		eventId.type
	);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
** Callback compare function from qsort.
*/
int CompareEventTrailer( const void *p1, const void *p2 )
{
	ACMIEventTrailer *t1 = (ACMIEventTrailer *)p1;
	ACMIEventTrailer *t2 = (ACMIEventTrailer *)p2;

	if ( t1->timeEnd < t2->timeEnd )
		return -1;
	else if ( t1->timeEnd > t2->timeEnd )
		return 1;
	else
		return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DestroyACMIRawPositionDataList(LIST *list)
{
	// LIST_DESTROY (list, (PFV)DeleteACMIRawPositionData);
	DestroyTheList (list);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DeleteACMIRawPositionData(ACMIRawPositionData* rawPositionData)
{
	delete rawPositionData;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DeleteACMIEntityData(ACMIEntityData *data)
{
	delete data;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DeleteACMIEventHeader(ACMIEventHeader *data)
{
	delete data;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DeleteACMIEntityPositionData(ACMIEntityPositionData *data)
{
	delete data;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DeleteACMIFeatEventImportData(ACMIFeatEventImportData *data)
{
	delete data;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

inline int ACMITape::NumEntities()
{
	// F4Assert(_tape != NULL);

	return _tapeHdr.numEntities;
}




ACMITape::ACMITape(char *name, RenderOTW *renderer, RViewPoint *viewPoint )
{ 
	int j;
	int i, numEntities;
	char fullName[MAX_PATH];
	ACMIEntityData *e;
	long length=0;
	char *callsigns=NULL;
	long numcalls=0;

	std::cout << "test acmi tape started" << std::endl;

	// initialize storage for drawable poled objects
	#ifdef USE_SH_POOLS
	DrawablePoled::InitializeStorage();
	#endif

	//F4Assert(name != NULL);

	_tape = NULL;
	_entityReadHeads = NULL;
	_simTapeEntities = NULL;
	_simTapeFeatures = NULL;
	_activeEventHead = NULL;
	_eventList = NULL;
	_screenCapturing = FALSE;
	_wingTrails = FALSE;
	_tapeObjScale = 1.0f;

	// set our render and viewpoint
	_renderer = renderer;
	_viewPoint = viewPoint;
	
	Init();

	// Open up a map file with the given name.

	// edg note on hack: right now, ALWAYS do an import from the acmi.flt
	// file to convert to a tape file.  Later we'll probably want to import
	// right after an ACMIU record session to get into .vhs format

	// LOL, how fun is it to read that nearly 20 years after and the code is still the same x)

	//strcpy( fullName, "campaign\\save\\fltfiles\\" );
	strcpy( fullName, "acmibin\\" );
	strcat( fullName, name );

	// commented out if statement for quick testing....
 	// if ( Import( fullName ) )
	
		// create the memory mapping
		//length=OpenTapeFile( fullName );

		// just test
		
	// If it loaded, do any additional setup.
	
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ACMITape::~ACMITape()
{
	// Delete Callsigns
	if(ACMI_Callsigns)
	{
		delete ACMI_Callsigns;
		ACMI_Callsigns=NULL;
	}
	Init();

	#ifdef USE_SH_POOLS
	DrawablePoled::ReleaseStorage();
	#endif

	OutputDebugString("TEST-DEBUG");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMITape::Init()
{

	if(_entityReadHeads)
	{
		delete [] _entityReadHeads;
		_entityReadHeads = NULL;
	}

	if(_simTapeEntities)
	{
		//CleanupSimTapeEntities();
	}

	if ( _eventList )
	{
		//CleanupEventList( );
	}

	//SetGeneralEventCallbacks
	//(
	//	NULL,
	//	NULL,
	//	NULL
	//);

	if(_tape)
	{
		// close file mapping.
		//CloseTapeFile();
	}

	_playVelocity = 0.0;
	_playAcceleration = 0.0;
	_maxPlaySpeed = 4.0;	

	_simTime = 0.0;
	_stepTrail = 0.0;

	_lastRealTime = 0.0;


	_unpause = FALSE;
	_paused = TRUE;
	_simulateOnly = FALSE;

	_generalEventReadHeadHeader = 0;
	_featEventReadHead = NULL;
	_generalEventReadHeadTrailer = NULL;


}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL ReadRawACMIPositionData(
	FILE *flightFile,
	ACMIRawPositionData &rawPositionData)
{
	int
		result;

	fscanf
	(
		flightFile,
		"%d %d",
		&rawPositionData.type,
		&rawPositionData.uniqueID
	);

	
	// We don't need to check the status of our last two fscanf calls, because
	// if they fail, this one will too.
	result = fscanf
	(
		flightFile,
		"%f %f %f %f %f %f\n",
		&rawPositionData.entityPosData.posData.x,
		&rawPositionData.entityPosData.posData.y,
		&rawPositionData.entityPosData.posData.z,
		&rawPositionData.entityPosData.posData.pitch,
		&rawPositionData.entityPosData.posData.roll,
		&rawPositionData.entityPosData.posData.yaw
	);

	// insure pitch roll and yaw are positive (edg:?)
	// or in 0 - 2PI range
	/* nah, this ain't right....  need to fix songy's stuff
	if ( rawPositionData.entityPosData.pitch < 0.0f )
		rawPositionData.entityPosData.pitch += 2.0f * PI;
	if ( rawPositionData.entityPosData.roll < 0.0f )
		rawPositionData.entityPosData.roll += 2.0f * PI;
	if ( rawPositionData.entityPosData.yaw < 0.0f )
		rawPositionData.entityPosData.yaw += 2.0f * PI;
	*/

	return (!result || result == EOF ? FALSE : TRUE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CleanupACMIImportPositionData
(
	FILE *flightFile,
	ACMIRawPositionData *rawPositionData
)
{
	if(flightFile != NULL)
	{
		fclose(flightFile);
	}

	if(rawPositionData != NULL)
	{
		delete rawPositionData;
	}

	if ( importEntityList != NULL )
	{
		DestroyTheList (importEntityList);
		importEntityList = NULL;
	}

	if ( importFeatList != NULL )
	{
		DestroyTheList (importFeatList);
		importFeatList = NULL;
	}

	if ( importPosList != NULL )
	{
		DestroyTheList (importPosList );
		importPosList = NULL;
	}

	if ( importEntEventList != NULL )
	{
		DestroyTheList (importEntEventList );
		importEntEventList = NULL;
	}

	if ( importEventList != NULL )
	{
		DestroyTheList (importEventList );
		importEventList = NULL;
	}

	if ( importEventTrailerList != NULL )
	{
		delete [] importEventTrailerList;
		importEventTrailerList = NULL;
	}

	if ( importFeatEventList != NULL )
	{
		DestroyTheList (importFeatEventList );
		importFeatEventList = NULL;
	}
	if(Import_Callsigns)
	{
		delete Import_Callsigns;
		Import_Callsigns=NULL;
		import_count=0;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL ACMITape::Import(char *inFltFile, char *outTapeFileName)
{
	FILE
		*flightFile;

	ACMIRawPositionData
		*rawPositionData = NULL;

	ACMIEventHeader
		*ehdr = NULL;

	ACMIFeatEventImportData
		*fedata = NULL;

	float
		begTime,
		endTime;

	ACMITapeHeader tapeHdr;
	ACMIRecHeader  hdr;
	ACMIGenPositionData genpos;
	ACMIFeaturePositionData featpos;
	ACMITracerStartData tracer;
	ACMIStationarySfxData sfx;
	ACMIMovingSfxData msfx;
	ACMISwitchData sd;
	ACMIDOFData dd;
	ACMIFeatureStatusData fs;
		
	
	// zero our counters
	importNumFeat = 0;
	importNumPos = 0;
	importNumEnt = 0;
	importNumEvents = 0;
	importNumFeatEvents = 0;
	importNumEntEvents = 0;

	// zero out position list
	importFeatList = NULL;
	//vector <> importFeatVec;
	importFeatEventList = NULL;
	importPosList = NULL;
	importEventList = NULL;
	importEntEventList = NULL;
	importEventTrailerList = NULL;

	// this value comes from tod type record
	tapeHdr.todOffset =  0.0f;


	// Load flight file for positional data.
	//flightFile = fopen("campaign\\save\\fltfiles\\acmi.flt", "rb");
	flightFile = fopen(inFltFile, "rb");
						
	if (flightFile == NULL)
	{
		MonoPrint("Error opening acmi flight file");
		return FALSE;
	}

	begTime = -1.0;
	endTime = 0.0;
	//OutputDebugString("TEST-DEBUG\n");
	MonoPrint("ACMITape Import: Reading Raw Data ....\n");
	while( fread(&hdr, sizeof( ACMIRecHeader ), 1, flightFile ) )
	{
		
		// now read in the rest of the record depending on type
		switch( hdr.type )
		{
			case ACMIRecTodOffset:
				tapeHdr.todOffset =  hdr.time;
				break;

			case ACMIRecGenPosition:
			case ACMIRecMissilePosition:
			case ACMIRecChaffPosition:
			case ACMIRecFlarePosition:
			case ACMIRecAircraftPosition:
		

				//std::cout << "boop" << std::endl;

				// Read the data
				if ( !fread( &genpos, sizeof( ACMIGenPositionData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}
				if (hdr.type == ACMIRecAircraftPosition)
					fread(&tempTarget, sizeof(tempTarget),1,flightFile);
				else
					tempTarget = -1;
				// Allocate a new data node.
				//F4Assert(rawPositionData == NULL);
				rawPositionData = new ACMIRawPositionData;
				//F4Assert(rawPositionData != NULL);

				//std::cout << "boop" << std::endl;
				// fill in raw position data
				rawPositionData->uniqueID = genpos.uniqueID;
				rawPositionData->type = genpos.type;
				if ( hdr.type == ACMIRecMissilePosition )
					rawPositionData->flags = ENTITY_FLAG_MISSILE;
				else if ( hdr.type == ACMIRecAircraftPosition )
					rawPositionData->flags = ENTITY_FLAG_AIRCRAFT;
				else if ( hdr.type == ACMIRecChaffPosition )
					rawPositionData->flags = ENTITY_FLAG_CHAFF;
				else if ( hdr.type == ACMIRecFlarePosition )
					rawPositionData->flags = ENTITY_FLAG_FLARE;
				else
					rawPositionData->flags = 0;

				rawPositionData->entityPosData.time = hdr.time;
				rawPositionData->entityPosData.type = PosTypePos;
// remove				rawPositionData->entityPosData.teamColor = genpos.teamColor;
// remove				strcpy((char*)rawPositionData->entityPosData.label, (char*)genpos.label);
				rawPositionData->entityPosData.posData.x = genpos.x;
				rawPositionData->entityPosData.posData.y = genpos.y;
				rawPositionData->entityPosData.posData.z = genpos.z;
				rawPositionData->entityPosData.posData.roll = genpos.roll;
				rawPositionData->entityPosData.posData.pitch = genpos.pitch;
				rawPositionData->entityPosData.posData.yaw = genpos.yaw;
				rawPositionData->entityPosData.posData.radarTarget= tempTarget;

																		
				importPosVec.push_back(*rawPositionData);
				// Append our new position data.
				importPosList = AppendToEndOfList(importPosList, &importPosListEnd, rawPositionData);
				rawPositionData = NULL;
		
				// bump counter
				importNumPos++;

				break;
			case ACMIRecTracerStart:

				// Read the data
				if ( !fread( &tracer, sizeof( ACMITracerStartData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(ehdr == NULL);
				ehdr = new ACMIEventHeader;
				//F4Assert(ehdr != NULL);

				// fill in data
				ehdr->eventType = hdr.type;
				ehdr->time = hdr.time;
				ehdr->timeEnd = hdr.time + 2.5F;
				ehdr->index = importNumEvents;
				ehdr->x = tracer.x;
				ehdr->y = tracer.y;
				ehdr->z = tracer.z;
				ehdr->dx = tracer.dx;
				ehdr->dy = tracer.dy;
				ehdr->dz = tracer.dz;

				importEventVec.push_back(*ehdr);
				// Append our new data.
				importEventList = AppendToEndOfList(importEventList, &importEventListEnd, ehdr );
				ehdr = NULL;
		
				// bump counter
				importNumEvents++;
				break;
			case ACMIRecStationarySfx:
				// Read the data
				if ( !fread( &sfx, sizeof( ACMIStationarySfxData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(ehdr == NULL);
				ehdr = new ACMIEventHeader;
				//F4Assert(ehdr != NULL);

				// fill in data
				ehdr->eventType = hdr.type;
				ehdr->index = importNumEvents;
				ehdr->time = hdr.time;
				ehdr->timeEnd = hdr.time + sfx.timeToLive;
				ehdr->x = sfx.x;
				ehdr->y = sfx.y;
				ehdr->z = sfx.z;
				ehdr->type = sfx.type;
				ehdr->scale = sfx.scale;

				importEventVec.push_back(*ehdr);
				// Append our new data.
				importEventList = AppendToEndOfList(importEventList, &importEventListEnd, ehdr );
				ehdr = NULL;
		
				// bump counter
				importNumEvents++;
				break;

			case ACMIRecFeatureStatus:
				// Read the data
				if ( !fread( &fs, sizeof( ACMIFeatureStatusData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(fedata == NULL);
				fedata = new ACMIFeatEventImportData;
				//F4Assert(fedata != NULL);

				// fill in data
				fedata->uniqueID = fs.uniqueID;
				fedata->data.index = -1;	// will be filled in later
				fedata->data.time = hdr.time;
				fedata->data.newStatus = fs.newStatus;
				fedata->data.prevStatus = fs.prevStatus;

				importFeatEventVec.push_back(*fedata);
				// Append our new data.
				importFeatEventList = AppendToEndOfList(importFeatEventList, &importFeatEventListEnd, fedata );
				fedata = NULL;
		
				// bump counter
				importNumFeatEvents++;
				break;

			// not ready for these yet
			case ACMIRecMovingSfx:
				// Read the data
				if ( !fread( &msfx, sizeof( ACMIMovingSfxData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(ehdr == NULL);
				ehdr = new ACMIEventHeader;
				//F4Assert(ehdr != NULL);

				// fill in data
				ehdr->eventType = hdr.type;
				ehdr->index = importNumEvents;
				ehdr->time = hdr.time;
				ehdr->timeEnd = hdr.time + msfx.timeToLive;
				ehdr->x = msfx.x;
				ehdr->y = msfx.y;
				ehdr->z = msfx.z;
				ehdr->dx = msfx.dx;
				ehdr->dy = msfx.dy;
				ehdr->dz = msfx.dz;
				ehdr->flags = msfx.flags;
				ehdr->user = msfx.user;
				ehdr->type = msfx.type;
				ehdr->scale = msfx.scale;

				importEventVec.push_back(*ehdr);
				// Append our new data.
				importEventList = AppendToEndOfList(importEventList, &importEventListEnd, ehdr );
				ehdr = NULL;
		
				// bump counter
				importNumEvents++;
				break;

			case ACMIRecSwitch:
		
				// Read the data
				if ( !fread( &sd, sizeof( ACMISwitchData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(rawPositionData == NULL);
				rawPositionData = new ACMIRawPositionData;
				//F4Assert(rawPositionData != NULL);
		
				// fill in raw position data
				rawPositionData->uniqueID = sd.uniqueID;
				rawPositionData->type = sd.type;
				rawPositionData->flags = 0;


				rawPositionData->entityPosData.time = hdr.time;
				rawPositionData->entityPosData.type = PosTypeSwitch;
				rawPositionData->entityPosData.switchData.switchNum = sd.switchNum;
				rawPositionData->entityPosData.switchData.switchVal = sd.switchVal;
				rawPositionData->entityPosData.switchData.prevSwitchVal = sd.prevSwitchVal;
				
				
				importEntEventVec.push_back(*rawPositionData);
				// Append our new position data.
				importEntEventList = AppendToEndOfList(importEntEventList, &importEntEventListEnd, rawPositionData);
				rawPositionData = NULL;
		
				// bump counter
				importNumEntEvents++;

				break;

			case ACMIRecDOF:
		
				// Read the data
				if ( !fread( &dd, sizeof( ACMIDOFData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(rawPositionData == NULL);
				rawPositionData = new ACMIRawPositionData;
				//F4Assert(rawPositionData != NULL);
		
				// fill in raw position data
				rawPositionData->uniqueID = dd.uniqueID;
				rawPositionData->type = dd.type;
				rawPositionData->flags = 0;


				rawPositionData->entityPosData.time = hdr.time;
				rawPositionData->entityPosData.type = PosTypeDOF;
				rawPositionData->entityPosData.dofData.DOFNum = dd.DOFNum;
				rawPositionData->entityPosData.dofData.DOFVal = dd.DOFVal;
				rawPositionData->entityPosData.dofData.prevDOFVal = dd.prevDOFVal;
				
				importEntEventVec.push_back(*rawPositionData);
				// Append our new position data.
				importEntEventList = AppendToEndOfList(importEntEventList, &importEntEventListEnd, rawPositionData);
				rawPositionData = NULL;
		
				// bump counter
				importNumEntEvents++;

				break;

			case ACMIRecFeaturePosition:
		
				// Read the data
				if ( !fread( &featpos, sizeof( ACMIFeaturePositionData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				//F4Assert(rawPositionData == NULL);
				rawPositionData = new ACMIRawPositionData;
				//F4Assert(rawPositionData != NULL);
		
				// fill in raw position data
				rawPositionData->uniqueID = featpos.uniqueID;
				rawPositionData->leadIndex = featpos.leadUniqueID;
				rawPositionData->specialFlags = featpos.specialFlags;
				rawPositionData->slot = featpos.slot;
				rawPositionData->type = featpos.type;
				rawPositionData->flags = ENTITY_FLAG_FEATURE;

				rawPositionData->entityPosData.time = hdr.time;
				rawPositionData->entityPosData.type = PosTypePos;
				rawPositionData->entityPosData.posData.x = featpos.x;
				rawPositionData->entityPosData.posData.y = featpos.y;
				rawPositionData->entityPosData.posData.z = featpos.z;
				rawPositionData->entityPosData.posData.roll = featpos.roll;
				rawPositionData->entityPosData.posData.pitch = featpos.pitch;
				rawPositionData->entityPosData.posData.yaw = featpos.yaw;
				
				importPosVec.push_back(*rawPositionData);
				// Append our new position data.
				importPosList = AppendToEndOfList(importPosList, &importPosListEnd, rawPositionData);
				rawPositionData = NULL;
		
				// bump counter
				importNumPos++;

				break;
			case ACMICallsignList:
		
				// Read the data
				if ( !fread( &import_count, sizeof( long ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				//F4Assert(Import_Callsigns == NULL);
				Import_Callsigns=new ACMI_CallRec[import_count];
				//F4Assert(Import_Callsigns != NULL);

				if(!fread(Import_Callsigns,import_count * sizeof(ACMI_CallRec),1,flightFile))
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}
				break;

			default:
				// KCK: I was hitting this repeatidly.. So I'm making it a ShiAssert (and therefore ignorable)
//				ShiAssert(0);
				break;
		}

		// save begin and end times
		if ( hdr.type != ACMIRecTodOffset )
		{
			if ( begTime < 0.0 )
				begTime = hdr.time;
			if ( hdr.time > endTime )
				endTime = hdr.time;
		}
	}
	clock_t t;
	// build the importEntityList
	MonoPrint("ACMITape Import: Parsing Entities ....\n");
	t = clock();
	ParseEntities();
	t = clock() - t;
	MonoPrint("ARRAY : thread entity It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);


	MonoPrint("ACMITape Import: Parsing Entities2 ....\n");
	t = clock();
	ParseEntities2();
	t = clock() - t;
	MonoPrint("VECTOR : thread entity It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);




	// setup the tape header
	tapeHdr.fileID = 'TAPE';
	tapeHdr.numEntities = importNumEnt;
	tapeHdr.numFeat = importNumFeat;
	tapeHdr.entityBlockOffset = sizeof( ACMITapeHeader );
	tapeHdr.featBlockOffset = tapeHdr.entityBlockOffset +
								  sizeof( ACMIEntityData ) * importNumEnt;
	tapeHdr.timelineBlockOffset = tapeHdr.featBlockOffset +
								  sizeof( ACMIEntityData ) * importNumFeat;
	tapeHdr.firstEntEventOffset = tapeHdr.timelineBlockOffset +
								  sizeof( ACMIEntityPositionData ) * importNumPos;
	tapeHdr.firstGeneralEventOffset = tapeHdr.firstEntEventOffset +
								  sizeof( ACMIEntityPositionData ) * importNumEntEvents;
	tapeHdr.firstEventTrailerOffset = tapeHdr.firstGeneralEventOffset +
								  sizeof( ACMIEventHeader ) * importNumEvents;
	tapeHdr.firstFeatEventOffset = tapeHdr.firstEventTrailerOffset +
								  sizeof( ACMIEventTrailer ) * importNumEvents;
	tapeHdr.firstTextEventOffset = tapeHdr.firstFeatEventOffset +
								  sizeof( ACMIFeatEvent ) * importNumFeatEvents;
	tapeHdr.numEntityPositions = importNumPos;
	tapeHdr.numEvents = importNumEvents;
	tapeHdr.numFeatEvents = importNumFeatEvents;
	tapeHdr.numEntEvents = importNumEntEvents;
	tapeHdr.totPlayTime = endTime - begTime;
	tapeHdr.startTime =  begTime;

	

	// set up the chain offsets of entity positions
	MonoPrint("ACMITape Import: Threading Positions ....\n");
	
	t = clock();
	ThreadEntityPositions2(&tapeHdr);
	t = clock() - t;
	MonoPrint("VECTOR : thread entity It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	
	
	t = clock();
	ThreadEntityPositions(&tapeHdr);
	t = clock() - t;
	MonoPrint("ARRAY : It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);



	// set up the chain offsets of entity events
	MonoPrint("ACMITape Import: Threading Entity Events ....\n");
	
	
	

	t = clock();
	ThreadEntityEvents2(&tapeHdr);
	t = clock() - t;
	MonoPrint("VECTOR : It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);

	t = clock();
	ThreadEntityEvents(&tapeHdr);
	t = clock() - t;
	MonoPrint("ARRAY : It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);

	// Calculate size of .vhs file.
	tapeHdr.fileSize = tapeHdr.timelineBlockOffset +
					   sizeof( ACMIEntityPositionData ) * importNumPos +
					   sizeof( ACMIEntityPositionData ) * importNumEntEvents +
					   sizeof( ACMIEventHeader ) * importNumEvents +
					   sizeof( ACMIFeatEvent ) * importNumFeatEvents +
					   sizeof( ACMIEventTrailer ) * importNumEvents;

	// Open a writecopy file mapping.
	// Write out file in .vhs format.
	MonoPrint("ACMITape Import: Writing Tape File ....\n");
	//WriteTapeFile( outTapeFileName, &tapeHdr );
	MonoPrint("ACMITape Import: Writing Tape File ....\n");
	WriteTapeFile2(outTapeFileName, &tapeHdr);

	// Cleanup import data.
	CleanupACMIImportPositionData ( flightFile, rawPositionData );

	// now delete the acmi.flt file
	//remove("campaign\\save\\fltfiles\\acmi.flt");
	//remove(inFltFile);
				
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMITape::ParseEntities ( void )
{
	int
		i = 0,
		count = 0;
	
	LIST			
		*entityPtr,
		*rawList;

	ACMIRawPositionData
		*entityType;

	ACMIEntityData
		*importEntityInfo;

	importEntityList = NULL;

	rawList = importPosList;
	for (count = 0; count < importNumPos; count++)
	{
		// rawList = LIST_NTH(importPosList, count);
		entityType = (ACMIRawPositionData *)rawList->node;

		if ( entityType->flags & ENTITY_FLAG_FEATURE )
		{
			// look for existing entity
			entityPtr = importFeatList;
			if (entityPtr != NULL)
				importEntityInfo = (ACMIEntityData *)entityPtr->node;
			else
				importEntityInfo = NULL;

			for (i = 0; (
				i < importNumFeat && 	
				importEntityInfo != NULL && 
				entityType->uniqueID != importEntityInfo->uniqueID); i++)
			{
				// entityPtr = LIST_NTH(importEntityList, i);
				//importEntityInfo = ( ACMIEntityData * )entityPtr->node;
			/*	if(entityType->uniqueID == importEntityInfo->uniqueID)
				{
					break;
				}*/
	
				entityPtr = entityPtr->next;
				if (entityPtr != NULL)
					importEntityInfo = (ACMIEntityData *)entityPtr->node;
				else
					importEntityInfo = NULL;
			}
	
			// create new import entity record
			if(i == importNumFeat)
			{
				//std::cout << "NOOP" << std::endl;
				importEntityInfo = new ACMIEntityData;
				importEntityInfo->count =0;

				//F4Assert( importEntityInfo );
				importEntityInfo->uniqueID = entityType->uniqueID;
				importEntityInfo->type = entityType->type;
				importEntityInfo->flags = entityType->flags;
				importEntityInfo->leadIndex = entityType->leadIndex;
				importEntityInfo->specialFlags = entityType->specialFlags;
				importEntityInfo->slot = entityType->slot;

				importFeatVec.push_back(*importEntityInfo);

				importFeatList = AppendToEndOfList(importFeatList, &importFeatListEnd, importEntityInfo);
				importNumFeat++;
			}
		}
		else
		{
			// not a feature

			// look for existing entity
			entityPtr = importEntityList;

			for (i = 0; i < importNumEnt; i++)
			{
				// entityPtr = LIST_NTH(importEntityList, i);
				importEntityInfo = ( ACMIEntityData * )entityPtr->node;
				if(entityType->uniqueID == importEntityInfo->uniqueID)
				{
					break;
				}
	
				entityPtr = entityPtr->next;
			}
	
			// create new import entity record
			if(i == importNumEnt)
			{
				importEntityInfo = new ACMIEntityData;
				importEntityInfo->count =0;

				//F4Assert( importEntityInfo );
				importEntityInfo->uniqueID = entityType->uniqueID;
				importEntityInfo->type = entityType->type;
				importEntityInfo->flags = entityType->flags;
// remove				importEntityInfo->teamColor = entityType->entityPosData.teamColor;
// remove				strcpy((importEntityInfo->label), (char*) entityType->entityPosData.label);

				importEntityVec.push_back(*importEntityInfo);
				importEntityList = AppendToEndOfList(importEntityList, &importEntityListEnd, importEntityInfo);
				importNumEnt++;
			}
		}

		rawList = rawList->next;
	}

	MonoPrint("ACMITape Import: Counting ....\n");
	// Count instances of each unique type
	LIST* list1 = importEntityList;
	LIST* list2;
	ACMIEntityData* thing1;
	ACMIEntityData* thing2;
	int objCount;

	while (list1)
	{
		thing1 = (ACMIEntityData*)list1->node;
		if (thing1->count == 0)
		{
			thing1->count = 1;
			objCount = 2;
			list2 = list1->next;
			while (list2)
			{
				thing2 = (ACMIEntityData*)list2->node;
				if (thing2->type == thing1->type && thing2->count == 0)
				{
					thing2->count = objCount;
					objCount ++;
				}
				list2 = list2->next;
			}
		}
		list1 = list1->next;
	}
	MonoPrint("ACMITape Import: Counting ended ....\n");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMITape::ParseEntities2(void)
{
	int
		i = 0,
		count = 0;

	int importPosVecSize = importPosVec.size();

	for (count = 0; count < importPosVecSize; count++)
	{

		if (importPosVec[count].flags & ENTITY_FLAG_FEATURE)
		{
			// look for existing entity
			for (i = 0; i < importFeatVec.size(); i++)
			{
				if (importPosVec[count].uniqueID == importFeatVec[i].uniqueID)
				{
					break;
				}
			}
			
			// create new import entity record
			if (i == importFeatVec.size())
			{
				ACMIEntityData* importEntityInfo = new ACMIEntityData;
				importEntityInfo->count = 0;

				importEntityInfo->uniqueID = importPosVec[count].uniqueID;
				importEntityInfo->type = importPosVec[count].type;
				importEntityInfo->flags = importPosVec[count].flags;
				importEntityInfo->leadIndex = importPosVec[count].leadIndex;
				importEntityInfo->specialFlags = importPosVec[count].specialFlags;
				importEntityInfo->slot = importPosVec[count].slot;

				importFeatVec.push_back(*importEntityInfo);

				//importNumFeat++;
			}
		}
		else
		{
			// not a feature

			// look for existing entity
			for (i = 0; i < importEntityVec.size(); i++)
			{
				if (importPosVec[count].uniqueID == importEntityVec[i].uniqueID)
				{
					break;
				}
			}

			// create new import entity record
			if (i == importEntityVec.size())
			{
				ACMIEntityData* importEntityInfo = new ACMIEntityData;
				importEntityInfo->count = 0;

				importEntityInfo->uniqueID = importPosVec[count].uniqueID;
				importEntityInfo->type = importPosVec[count].type;
				importEntityInfo->flags = importPosVec[count].flags;
	
				importEntityVec.push_back(*importEntityInfo);
				//importNumEnt++;
			}
		}
	}

	MonoPrint("ACMITape Import: Counting ....\n");

	int objCount;

	i = 0;
	int j = 0;
	int entitynum = importEntityVec.size();
	while (i < entitynum)
	{

		if (importEntityVec[i].count == 0)
		{
			importEntityVec[i].count = 1;
			objCount = 2;

			j = i + 1;
			while (j < entitynum)
			{

				if (importEntityVec[j].type == importEntityVec[i].type && importEntityVec[j].count == 0)
				{
					importEntityVec[j].count = objCount;
					objCount++;
				}
				j++;

			}
		}
		i++;
	}
	MonoPrint("ACMITape Import: Counting ended ....\n");
}







/*
** Description:
**		At this point importEntList and importPosList should be populated.
**		Now, we're going to have to setup the offset pointers to do the
**		file mapping.  Each entity chains back and forth thru its position
**		list.
**      Entity and Position Lists
*/
void ACMITape::ThreadEntityPositions ( ACMITapeHeader *tapeHdr )
{
	int i, j;
	long prevOffset;
	LIST *entityListPtr, *posListPtr, *featListPtr;
	ACMIEntityData *entityPtr, *featPtr;
	//ACMIRawPositionData *posPtr;
	ACMIRawPositionData *prevPosPtr;
	ACMIFeatEventImportData *fePtr;
	BOOL foundFirst;
	long currOffset;

	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together

	entityListPtr = importEntityList;

	for ( i = 0; i < importNumEnt; i++ )
	{
		// entityListPtr = LIST_NTH(importEntityList, i);
		entityPtr = (ACMIEntityData *)entityListPtr->node;
		foundFirst = FALSE;
		prevOffset = 0;
		prevPosPtr = NULL;
		entityPtr->firstPositionDataOffset = 0;

		posListPtr = importPosList;




		for ( j = 0; j < importNumPos; j++ )
		{
			//printf("posListPtr pos: %p\n", posListPtr);

			ACMIRawPositionData *posPtr;
			posPtr = (ACMIRawPositionData *)posListPtr->node;

			// check the id to see if this position belongs to the entity
			if (posPtr->uniqueID != entityPtr->uniqueID)
			{
				// nope
				//std::cout << "inif" << std::endl;
				posListPtr = posListPtr->next;
				continue;
			}
				//std::cout << "outif" << std::endl;


				// calculate the offset of this positional record
				currOffset = tapeHdr->timelineBlockOffset +
					sizeof(ACMIEntityPositionData) * j;

				// if it's the 1st in the chain, set the offset to it in
				// the entity's record
				// Set everytime and check and use which offset is the lowest
				if (foundFirst == FALSE)
				{
					entityPtr->firstPositionDataOffset = currOffset;
					foundFirst = TRUE;
				}

				// thread current to previous
				posPtr->entityPosData.prevPositionUpdateOffset = prevOffset;
				posPtr->entityPosData.nextPositionUpdateOffset = 0;

				// thread previous to current
				if (prevPosPtr)
				{
					prevPosPtr->entityPosData.nextPositionUpdateOffset = currOffset;
				}

				// set vals for next time thru loop
				prevOffset = currOffset;
				prevPosPtr = posPtr;
			

			// next in list
			posListPtr = posListPtr->next;

		} // end for position loop

		entityListPtr = entityListPtr->next;
	} // end for entity loop

	// ------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------

	printf("addr importEntityList: %p\n", importEntityList);

	entityListPtr = importFeatList;
	for ( i = 0; i < importNumFeat; i++ )
	{
		entityPtr = (ACMIEntityData *)entityListPtr->node;
		foundFirst = FALSE;
		prevOffset = 0;
		prevPosPtr = NULL;
		entityPtr->firstPositionDataOffset = 0;

		posListPtr = importPosList;
		for ( j = 0; j < importNumPos; j++ )
		{
			// posListPtr = LIST_NTH(importPosList, j);
			ACMIRawPositionData *posPtr;
			posPtr = (ACMIRawPositionData *)posListPtr->node;

			// check the id to see if this position belongs to the entity
			if ( posPtr->uniqueID == entityPtr->uniqueID )
			{
				// nope
				//posListPtr = posListPtr->next;
				//continue;


				// calculate the offset of this positional record
				currOffset = tapeHdr->timelineBlockOffset +
					sizeof(ACMIEntityPositionData) * j;

				// if it's the 1st in the chain, set the offset to it in
				// the entity's record
				if (foundFirst == FALSE)
				{
					entityPtr->firstPositionDataOffset = currOffset;
					foundFirst = TRUE;
				}

				// thread current to previous
				posPtr->entityPosData.prevPositionUpdateOffset = prevOffset;
				posPtr->entityPosData.nextPositionUpdateOffset = 0;

				// thread previous to current
				if (prevPosPtr)
				{
					prevPosPtr->entityPosData.nextPositionUpdateOffset = currOffset;
				}

				// set vals for next time thru loop
				prevOffset = currOffset;
				prevPosPtr = posPtr;
			} // End of if 

			// next in list
			posListPtr = posListPtr->next;

		} // end for position loop

		// while we're doing the features, for each one, go thru the
		// feature event list looking for our unique ID in the events
		// and setting the index value of our feature in the event
		posListPtr = importFeatEventList;
		for ( j = 0; j < importNumFeatEvents; j++ )
		{
			// posListPtr = LIST_NTH(importPosList, j);
			fePtr = (ACMIFeatEventImportData *)posListPtr->node;

			// check the id to see if this event belongs to the entity
			if ( fePtr->uniqueID == entityPtr->uniqueID )
			{
				fePtr->data.index = i;
			}

			// next in list
			posListPtr = posListPtr->next;

		} // end for feature event loop

		// now go thru the feature list again and find lead unique ID's and
		// change them to indices into the list

		// actually NOW, go through and just make sure they exist... otherwise, clear
		if ( entityPtr->leadIndex != -1)
		{
			featListPtr = importFeatList;
			for ( j = 0; j < importNumFeat; j++ )
			{
				// we don't compare ourselves
				if ( j != i )
				{
					featPtr = (ACMIEntityData *)featListPtr->node;
					if ( entityPtr->leadIndex == featPtr->uniqueID )
					{
						entityPtr->leadIndex = j;
						break;
					}
	
				}
				// next in list
				featListPtr = featListPtr->next;
			}

			// if we're gone thru the whole list and haven't found
			// a lead index, we're in trouble.  To protect, set the
			// lead index to -1
			if ( j == importNumFeat )
			{
				entityPtr->leadIndex = -1;
			}
		}

		entityListPtr = entityListPtr->next;
	} // end for feature entity loop


}




void ACMITape::ThreadEntityPositions2(ACMITapeHeader *tapeHdr)
{
	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together



	int importEntityVecSize = importEntityVec.size();
	int importPosVecSize = importPosVec.size();
	int importFeatVecSize = importFeatVec.size();

	for (int i = 0; i < importEntityVecSize; i++)
	{
		long currOffset;
		BOOL foundFirst = FALSE;
		long prevOffset = 0;
		ACMIRawPositionData *prevPosPtr = NULL;
		importEntityVec[i].firstPositionDataOffset = 0;
		int prevPosVec = -1;

		for (int j = 0; j < importPosVecSize; j++)
		{

			// check the id to see if this position belongs to the entity
			if (importPosVec[j].uniqueID == importEntityVec[i].uniqueID)
			{

				// calculate the offset of this positional record
				currOffset = tapeHdr->timelineBlockOffset +
					sizeof(ACMIEntityPositionData) * j;

				// if it's the 1st in the chain, set the offset to it in
				// the entity's record
				if (foundFirst == FALSE)
				{
					importEntityVec[i].firstPositionDataOffset = currOffset;
					foundFirst = TRUE;
				}

				// thread current to previous
				importPosVec[j].entityPosData.prevPositionUpdateOffset = prevOffset;
				importPosVec[j].entityPosData.nextPositionUpdateOffset = 0;

				// thread previous to current
				if (prevPosVec != -1)
				{
					importPosVec[prevPosVec].entityPosData.nextPositionUpdateOffset = currOffset;
				}

				// set vals for next time thru loop
				prevOffset = currOffset;
				prevPosVec = j;

			} //end of if

		} // end for position loop

	} // end for entity loop

	  // ------------------------------------------------------------------------------------
	  // ------------------------------------------------------------------------------------
	  // ------------------------------------------------------------------------------------

	// we run an outer and inner loop here.
	// the outer loops steps thru each Feature
	// the inner loop searches each position update for one owned by the
	// Feature and chains them together




	for (int i = 0; i < importFeatVecSize; i++)
	{
		long currOffset;
		BOOL foundFirst = FALSE;
		long prevOffset = 0;
		ACMIRawPositionData *prevPosPtr = NULL;
		importFeatVec[i].firstPositionDataOffset = 0;
		int prevPosVec = -1;


		for (int j = 0; j < importPosVecSize; j++)
		{	
			// check the id to see if this position belongs to the entity
			if (importPosVec[j].uniqueID == importFeatVec[i].uniqueID)
			{
				// calculate the offset of this positional record
				currOffset = tapeHdr->timelineBlockOffset +
					sizeof(ACMIEntityPositionData) * j;

				// if it's the 1st in the chain, set the offset to it in
				// the entity's record
				if (foundFirst == FALSE)
				{
					importFeatVec[i].firstPositionDataOffset = currOffset;
					foundFirst = TRUE;
				}

				// thread current to previous
				importPosVec[j].entityPosData.prevPositionUpdateOffset = prevOffset;
				importPosVec[j].entityPosData.nextPositionUpdateOffset = 0;

				// thread previous to current
				if (prevPosVec != -1)
				{
					importPosVec[prevPosVec].entityPosData.nextPositionUpdateOffset = currOffset;
				}

				// set vals for next time thru loop
				prevOffset = currOffset;
				prevPosVec = j;
			} // End of if 

		} // end for position loop

		  // while we're doing the features, for each one, go thru the
		  // feature event list looking for our unique ID in the events
		  // and setting the index value of our feature in the event

		for (int j = 0; j < importNumFeatEvents; j++)
		{

			// check the id to see if this event belongs to the entity
			if (importFeatEventVec[j].uniqueID == importFeatVec[i].uniqueID)
			{
				importFeatEventVec[j].data.index = i;
			}
		} // end for feature event loop


		// now go thru the feature list again and find lead unique ID's and
		// change them to indices into the list
		// actually NOW, go through and just make sure they exist... otherwise, clear


		if (importFeatVec[i].leadIndex != -1)
		{

			int j;
			for (j = 0; j < importFeatVecSize; j++)
			{
				// we don't compare ourselves
				if (j != i)
				{
					if (importFeatVec[i].leadIndex == importFeatVec[j].uniqueID)
					{
						importFeatVec[i].leadIndex = j;
						break;
					}
				}

			}

			// if we're gone thru the whole list and haven't found
			// a lead index, we're in trouble.  To protect, set the
			// lead index to -1
			if (j == importNumFeat)
			{
				importFeatVec[i].leadIndex = -1;
			}
		}
	} // end for feature entity loop


}


/*
** Description:
**		At this point importEntList and importPosList should be populated.
**		Now, we're going to have to setup the offset pointers to do the
**		file mapping.  Each entity chains back and forth thru its position
**		list.
*/
void ACMITape::ThreadEntityEvents(ACMITapeHeader *tapeHdr)
{
	int i, j;
	long prevOffset;
	LIST *entityListPtr, *posListPtr;
	ACMIEntityData *entityPtr;
	ACMIRawPositionData *posPtr;
	ACMIRawPositionData *prevPosPtr;
	BOOL foundFirst;
	long currOffset;

	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together

	entityListPtr = importEntityList;
	for (i = 0; i < importNumEnt; i++)
	{
		// entityListPtr = LIST_NTH(importEntityList, i);
		entityPtr = (ACMIEntityData *)entityListPtr->node;
		foundFirst = FALSE;
		prevOffset = 0;
		prevPosPtr = NULL;
		entityPtr->firstEventDataOffset = 0;

		posListPtr = importEntEventList;
		for (j = 0; j < importNumEntEvents; j++)
		{
			// posListPtr = LIST_NTH(importPosList, j);
			posPtr = (ACMIRawPositionData *)posListPtr->node;

			// check the id to see if this position belongs to the entity
			if (posPtr->uniqueID != entityPtr->uniqueID)
			{
				// nope
				posListPtr = posListPtr->next;
				continue;
			}

			// calculate the offset of this positional record
			currOffset = tapeHdr->firstEntEventOffset +
				sizeof(ACMIEntityPositionData) * j;

			// if it's the 1st in the chain, set the offset to it in
			// the entity's record
			if (foundFirst == FALSE)
			{
				entityPtr->firstEventDataOffset = currOffset;
				foundFirst = TRUE;
			}

			// thread current to previous
			posPtr->entityPosData.prevPositionUpdateOffset = prevOffset;
			posPtr->entityPosData.nextPositionUpdateOffset = 0;

			// thread previous to current
			if (prevPosPtr)
			{
				prevPosPtr->entityPosData.nextPositionUpdateOffset = currOffset;
			}

			// set vals for next time thru loop
			prevOffset = currOffset;
			prevPosPtr = posPtr;

			// next in list
			posListPtr = posListPtr->next;

		} // end for position loop

		entityListPtr = entityListPtr->next;
	} // end for entity loop
}

void ACMITape::ThreadEntityEvents2(ACMITapeHeader *tapeHdr)
{
	//int calc = 0;

	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together


	int importEntityVecSize = importEntityVec.size(); // importNumEnt
	int importPosVecSize = importPosVec.size(); // importNumPos
	int importFeatVecSize = importFeatVec.size(); // importNumFeat
	int importEntEventVecSize = importEntEventVec.size(); // importNumEntEvents

	// Now threadded 
	par_for(0, importEntityVecSize, [&](int i, int cpu)
	{
		long currOffset;
		BOOL foundFirst = FALSE;
		long prevOffset = 0;
		ACMIRawPositionData *prevPosPtr = NULL;
		importEntityVec[i].firstEventDataOffset = 0;

		int prevPosVec = -1;

		for (int j = 0; j < importEntEventVecSize; j++)
		{

			// check the id to see if this position belongs to the entity
			if (importEntEventVec[j].uniqueID == importEntityVec[i].uniqueID)
			{

			// calculate the offset of this positional record
			currOffset = tapeHdr->firstEntEventOffset +
				sizeof(ACMIEntityPositionData) * j;

			// if it's the 1st in the chain, set the offset to it in
			// the entity's record
			if (foundFirst == FALSE)
			{
				importEntityVec[i].firstEventDataOffset = currOffset;
				foundFirst = TRUE;
			}

			// thread current to previous
			importEntEventVec[j].entityPosData.prevPositionUpdateOffset = prevOffset;
			importEntEventVec[j].entityPosData.nextPositionUpdateOffset = 0;

			// thread previous to current
			if (prevPosVec != -1)
			{
				importEntEventVec[prevPosVec].entityPosData.nextPositionUpdateOffset = currOffset;
			}

			// set vals for next time thru loop
			prevOffset = currOffset;
			prevPosVec = j;
		
			} //end of if
		}
	});// end for threadded entity loop
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
** Description:
**		At this point importEntList and importPosList should be populated.
**		Also the entities and positions are now threaded
**		write out the file
*/
void ACMITape::WriteTapeFile ( char *fname, ACMITapeHeader *tapeHdr )
{
	int i,j;
	LIST *entityListPtr, *posListPtr, *eventListPtr;
	ACMIEntityData *entityPtr;
	ACMIEventHeader *eventPtr;
	ACMIRawPositionData *posPtr;
	ACMIFeatEventImportData *fePtr;
	FILE *tapeFile;
	long ret;

	tapeFile = fopen(fname, "wb");
	if (tapeFile == NULL)
	{
		MonoPrint("Error opening new tape file\n");
		return;
	}

	// write the header
	ret = fwrite( tapeHdr, sizeof( ACMITapeHeader ), 1, tapeFile );
	if ( !ret )
	 	goto error_exit;


	// write out the entities
	entityListPtr = importEntityList;
	for ( i = 0; i < importNumEnt; i++ )
	{
		// entityListPtr = LIST_NTH(importEntityList, i);
		entityPtr = (ACMIEntityData *)entityListPtr->node;

		ret = fwrite( entityPtr, sizeof( ACMIEntityData ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;
		entityListPtr = entityListPtr->next;
	} // end for entity loop

	// write out the features
	entityListPtr = importFeatList;
	for ( i = 0; i < importNumFeat; i++ )
	{
		// entityListPtr = LIST_NTH(importEntityList, i);
		entityPtr = (ACMIEntityData *)entityListPtr->node;

		ret = fwrite( entityPtr, sizeof( ACMIEntityData ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;
		entityListPtr = entityListPtr->next;
	} // end for entity loop

	// write out the entitiy positions
	posListPtr = importPosList;
	for ( i = 0; i < importNumPos; i++ )
	{
		// posListPtr = LIST_NTH(importPosList, i);
		posPtr = (ACMIRawPositionData *)posListPtr->node;

		// we now want to do a "fixup" of the radar targets.  These are
		// currently in "uniqueIDs" and we want to convert them into
		// an index into the entity list
		if ( posPtr->entityPosData.posData.radarTarget != -1 )
		{
			entityListPtr = importEntityList;
			for ( j = 0; j < importNumEnt; j++ )
			{
				entityPtr = (ACMIEntityData *)entityListPtr->node;

				if ( posPtr->entityPosData.posData.radarTarget == entityPtr->uniqueID )
				{
					posPtr->entityPosData.posData.radarTarget = j;
					break;
				}

				entityListPtr = entityListPtr->next;
			} // end for entity loop

			// did we find it?
			if ( j == importNumEnt )
			{
				// nope
				posPtr->entityPosData.posData.radarTarget = -1;
			}
		} // end if there's a radar target

		ret = fwrite( &posPtr->entityPosData, sizeof( ACMIEntityPositionData ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;

		posListPtr = posListPtr->next;
	}

	// write out the entitiy events
	posListPtr = importEntEventList;
	for ( i = 0; i < importNumEntEvents; i++ )
	{
		// posListPtr = LIST_NTH(importPosList, i);
		posPtr = (ACMIRawPositionData *)posListPtr->node;

		ret = fwrite( &posPtr->entityPosData, sizeof( ACMIEntityPositionData ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;

		posListPtr = posListPtr->next;
	}

	// allocate the trailer list
	importEventTrailerList = new ACMIEventTrailer[importNumEvents];
	//F4Assert( importEventTrailerList );

	// write out the events
	eventListPtr = importEventList;
	for ( i = 0; i < importNumEvents; i++ )
	{
		// eventListPtr = LIST_NTH(importEventList, i);
		eventPtr = (ACMIEventHeader *)eventListPtr->node;

		//eventPtr->

		// set the trailer data
		importEventTrailerList[i].index = i;
		importEventTrailerList[i].timeEnd = eventPtr->timeEnd;

		ret = fwrite( eventPtr, sizeof( ACMIEventHeader ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;

		eventListPtr = eventListPtr->next;

	} // end for events loop

	// now sort the trailers in ascending order by endTime and
	// write them out
	qsort( importEventTrailerList,
		   importNumEvents,
		   sizeof( ACMIEventTrailer ),
		   CompareEventTrailer );
	
	for ( i = 0; i < importNumEvents; i++ )
	{
		ret = fwrite( &importEventTrailerList[i], sizeof( ACMIEventTrailer ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;

	} // end for events loop

	// write out the feature events
	posListPtr = importFeatEventList;
	for ( i = 0; i < importNumFeatEvents; i++ )
	{
		// posListPtr = LIST_NTH(importPosList, i);
		fePtr = (ACMIFeatEventImportData *)posListPtr->node;

		ret = fwrite( &fePtr->data, sizeof( ACMIFeatEvent ), 1, tapeFile );
		if ( !ret )
	 		goto error_exit;

		posListPtr = posListPtr->next;
	}

	// finally import and write out the text events
	ImportTextEventList( tapeFile, tapeHdr );

	// normal exit
	fclose( tapeFile );
	return;

error_exit:
	MonoPrint("Error writing new tape file\n");
	if ( tapeFile )
		fclose( tapeFile );
	return;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int CompareEventTrailer(const ACMIEventTrailer& i, const ACMIEventTrailer& j)
{
	if (i.timeEnd < j.timeEnd)
		return -1;
	else if (i.timeEnd > j.timeEnd)
		return 1;
	else
		return 0;
}


void ACMITape::WriteTapeFile2(char *fname, ACMITapeHeader *tapeHdr)
{
	FILE *tapeFile;


	int importEntityVecSize = importEntityVec.size(); // importNumEnt
	int importPosVecSize = importPosVec.size(); // importNumPos
	int importFeatVecSize = importFeatVec.size(); // importNumFeat
	int importEntEventVecSize = importEntEventVec.size(); // importNumEntEvents


	try {

		int i, j;
	
		long ret;

		tapeFile = fopen(fname, "wb");
		if (tapeFile == NULL)
		{
			MonoPrint("Error opening new tape file\n");
			return;
		}

		// write the header
		ret = fwrite(tapeHdr, sizeof(ACMITapeHeader), 1, tapeFile);
		if (!ret)
			throw "error_exit";


		// write out the entities 
		for (i = 0; i < importEntityVecSize; i++)
		{
			ret = fwrite(&importEntityVec[i], sizeof(ACMIEntityData), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		} // end for entity loop



		 // write out the features
		for (i = 0; i < importFeatVecSize; i++)
		{
			ret = fwrite(&importFeatVec[i], sizeof(ACMIEntityData), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		} // end for entity loop

		// write out the entitiy positions
		for (i = 0; i < importPosVecSize; i++)
		{
			// we now want to do a "fixup" of the radar targets.  These are
			// currently in "uniqueIDs" and we want to convert them into
			// an index into the entity list
			if (importPosVec[i].entityPosData.posData.radarTarget != -1)
			{
				for (j = 0; j < importEntityVecSize; j++)
				{
					if (importPosVec[i].entityPosData.posData.radarTarget == importEntityVec[j].uniqueID)
					{
						importPosVec[i].entityPosData.posData.radarTarget = j;
						break;
					}
				} // end for entity loop

				  // did we find it?
				if (j == importEntityVecSize)
				{
					// nope
					importPosVec[i].entityPosData.posData.radarTarget = -1;
				}

			} // end if there's a radar target

			ret = fwrite(&importPosVec[i].entityPosData, sizeof(ACMIEntityPositionData), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		}

		// write out the entitiy events
		for (i = 0; i < importNumEntEvents; i++)
		{
			ret = fwrite(&(importEntEventVec[i].entityPosData), sizeof(ACMIEntityPositionData), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		}

		// allocate the trailer list
		std::vector<ACMIEventTrailer> importEventTrailerVec(importNumEvents);

		// write out the events 
		for (i = 0; i < importNumEvents; i++)
		{
			// set the trailer data
			importEventTrailerVec[i].index = i;
			importEventTrailerVec[i].timeEnd = importEventVec[i].timeEnd;

			ret = fwrite(&importEventVec[i], sizeof(ACMIEventHeader), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		} // end for events loop
		
		/*
		Using qsort because sort and sort_stable don't output the same exact result
		Don't know if it's a problem I use that for now.
		*/
		qsort(&importEventTrailerVec[0],
			importEventTrailerVec.size(),
			sizeof(ACMIEventTrailer),
			CompareEventTrailer);

		for (i = 0; i < importNumEvents; i++)
		{
			ret = fwrite(&importEventTrailerVec[i], sizeof(ACMIEventTrailer), 1, tapeFile);
			if (!ret)
				throw "error_exit";

		} // end for events loop

		// write out the feature events
		for (i = 0; i < importNumFeatEvents; i++)
		{
			ret = fwrite(&importFeatEventVec[i].data, sizeof(ACMIFeatEvent), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		}

		// finally import and write out the text events
		ImportTextEventList(tapeFile, tapeHdr);

		// normal exit
		fclose(tapeFile);
		return;

	} catch (const std::exception& e) {


		MonoPrint("Error writing new tape file\n");
		if (tapeFile)
			fclose(tapeFile);
		return;
	}
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//BOOL ACMITape::GetEntityPosition
//(
//	int index,
//	float &x,
//	float &y,
//	float &z,
//	float &yaw,
//	float &pitch,
//	float &roll,
//	float &speed,
//	float &turnrate,
//	float &turnradius
//)
//{
//	float
//		deltaTime;
//
//	float dx, dy, dz;
//	float dx1, dy1, dz1;
//
//	ACMIEntityPositionData
//		*pos1,
//		*pos2,
//		*pos3;
//
//	// init speed to 0.0
//	speed = 0.0f;
//	turnrate = 0.0f;
//	turnradius = 0.0f;
//
//	F4Assert(index >= 0 && index < NumEntities());
//
//	pos1 = CurrentEntityPositionHead(index);
//
//	// If there is not at least 1 positional update, the entity doesn't exist.
//	F4Assert(pos1 != NULL);
//
//	if(pos1->time > _simTime)
//	{
//		x = pos1->posData.x;
//		y = pos1->posData.y;
//		z = pos1->posData.z;
//		yaw = pos1->posData.yaw;
//		pitch = pos1->posData.pitch;
//		roll = pos1->posData.roll;
//		return FALSE;
//	}
//
//	pos2 = HeadNext(pos1);
//	if(pos2 == NULL)
//	{
//		x = pos1->posData.x;
//		y = pos1->posData.y;
//		z = pos1->posData.z;
//		yaw = pos1->posData.yaw;
//		pitch = pos1->posData.pitch;
//		roll = pos1->posData.roll;
//		return FALSE;		
//	}
//	else
//	{
//   	pos3 = HeadPrev(pos1);
//		F4Assert(pos1->time <= _simTime);
//		F4Assert(pos2->time > _simTime);
//
//		dx = pos2->posData.x - pos1->posData.x;
//		dy = pos2->posData.y - pos1->posData.y;
//		dz = pos2->posData.z - pos1->posData.z;
//
//		// Interpolate.
//		deltaTime = 
//		(
//			(_simTime - pos1->time) /
//			(pos2->time - pos1->time)
//		);
//
//		x = 
//		(
//			pos1->posData.x + dx * deltaTime
//		);
//
//		y = 
//		(
//			pos1->posData.y + dy * deltaTime
//		);
//
//		z = 
//		(
//			pos1->posData.z + dz * deltaTime
//		);
//
//		yaw = AngleInterp( pos1->posData.yaw, pos2->posData.yaw, deltaTime );
//		pitch = AngleInterp( pos1->posData.pitch, pos2->posData.pitch, deltaTime );
//		roll = AngleInterp( pos1->posData.roll, pos2->posData.roll, deltaTime );
//
//		// get the average speed
//		speed = (float)sqrt( dx * dx + dy * dy + dz * dz ) / ( pos2->time - pos1->time );
//		float dAng = pos2->posData.yaw - pos1->posData.yaw;
//		if ( fabs( dAng ) > 180.0f * DTR )
//		{
//			if ( dAng >= 0.0f )
//				dAng -= 360.0f * DTR;
//			else
//				dAng += 360.0f * DTR;
//
//		}
//
//      if (pos3)
//      {
//		   dx1 = pos1->posData.x - pos3->posData.x;
//		   dy1 = pos1->posData.y - pos3->posData.y;
//		   dz1 = pos1->posData.z - pos3->posData.z;
//
//         // Turn rate = solid angle delta between velocity vectors
//         turnrate = (float)acos ((dx*dx1 + dy*dy1 + dz*dz1)/
//            (float)sqrt((dx*dx + dy*dy + dz*dz) * (dx1*dx1 + dy1*dy1 + dz1*dz1)));
//         turnrate *= RTD / ( pos2->time - pos1->time );
////		   turnrate = RTD * fabs( dAng ) / ( pos2->time - pos1->time );
//
//		   if ( turnrate != 0.0f )
//		   {
//			   // sec to turn 360 deg
//			   float secs = 360.0f/turnrate;
//
//			   // get circumference
//			   float circum = speed * secs;
//
//			   // now we get turn radius ( circum = 2 * PI * R )
//			   turnradius = circum/( 2.0f * PI );
//		   }
//      }
//      else
//      {
//         turnrate = 0.0F;
//         turnradius = 0.0F;
//      }
//	}
//
//	return TRUE;
//}



/*
 * append new node to end of list
 * caller should cast returned value to appropriate type
 */
LIST *
AppendToEndOfList( LIST * list, LIST **end, void * node )
{
   LIST * newnode;

   newnode = new LIST;

   newnode -> node = node;
   newnode -> next = NULL;

   /* list was null */
   if ( !list ) 
   {
     list = newnode;
	 *end = list;
   }
   else 
   {
      /* chain in at end */
      (*end) -> next = newnode;
	  *end = newnode;
   }

   return( list );
}


/*
 * destroy a list
 * optionally free the data pointed to by node, using supplied destructor fn
 * If destructor is NULL, node data not affected, only list nodes get freed
 */
void 
DestroyTheList( LIST * list )
{
   LIST * prev,
        * curr;

   if ( !list )
      return;

   prev = list;
   curr = list -> next;

   while ( curr )
   {
      // if ( destructor )
      //    (*destructor)(prev -> node);

	  delete prev->node;

      prev -> next = NULL;

      delete prev;

      prev = curr;
      curr = curr -> next;
   }

   // if( destructor )
   //    (*destructor)( prev -> node );

   delete prev->node;

   prev -> next = NULL;

   delete prev;

   //ListGlobalPack();
}


/*
** Description:
**		Reads the event file and writes out associated text events with
**		the tape.
*/
void
ACMITape::ImportTextEventList( FILE *fd, ACMITapeHeader *tapeHdr )
{
	// fd is tapefile vhs and tapehdr is the tape header
	long ret;

	tapeHdr->numTextEvents = 0;

	/*
	** Don't care about the folowing stuff, it's only used by Falcon ACMI viewer
	*/

	// PJW Totally rewrote event debriefing stuff... thus the new code
	//while ( cur )
	//{
	//	te.intTime = cur->eventTime;
	//	GetTimeString(cur->eventTime, timestr);

	//	_tcscpy(te.timeStr,timestr + 3);
	//	_tcscpy(te.msgStr,cur->eventString);

	//	// KCK: Edit out some script info which is used in debreiefings
	//	_TCHAR	*strptr = _tcschr(te.msgStr,'@');
	//	if (strptr)
	//	{
	//		strptr[0] = ' ';
	//		strptr[1] = '-';
	//		strptr[2] = ' ';
	//	}

	//	ret = fwrite( &te, sizeof( ACMITextEvent ), 1, fd );
	//	if ( !ret )
	//	{
	//		MonoPrint( "Error writing TAPE event element\n" );
	//		break;
	//	}
	//	tapeHdr->numTextEvents++;

	//	// next one
	//	cur = cur->next;

	//} // end for events loop


	// write callsign list
	if(Import_Callsigns)
	{
		ret = fwrite(&import_count, sizeof(long),1, fd );
		if ( !ret )
	 		goto error_exit;

		ret = fwrite(Import_Callsigns, import_count * sizeof(ACMI_CallRec),1, fd );
		if ( !ret )
	 		goto error_exit;
	}

	// write the header again (bleck)
	ret = fseek( fd, 0, SEEK_SET );
	if ( ret )
	{
		MonoPrint( "Error seeking TAPE start\n" );
		goto error_exit;
	}
	ret = fwrite( tapeHdr, sizeof( ACMITapeHeader ), 1, fd );
	if ( !ret )
	{
		MonoPrint( "Error writing TAPE header again\n" );
	}

error_exit:
	return;
}


/*
** Description:
**		returns pointer to 1st text event element and the count of
**		elements
*/
void *
ACMITape::GetTextEvents( int *count )
{
	if (_tapeHdr.numTextEvents > 1048576) // Sanity check
	{
		count = 0;
		return NULL;
	}

	*count = _tapeHdr.numTextEvents;
	return (void *)((char *)_tape + _tapeHdr.firstTextEventOffset);
}
