// AcmiTape.cpp
// File created : 2017-9-23
// Originally written by Jim DiZoglio (x257) as ACMIView class
// 
//
// Last update : 2017-10-21
// By loitho

// previous last modified: 9/25/97	Michael P. Songy



/* STFU _CRT_SECURE_NO_WARNINGS */
#pragma warning(disable:4996)

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <tchar.h>
#include <iostream>
#include <vector>

#include "AcmiTape.h"
#include "acmirec.h"
#include "threading.h"

#if _DEBUG
	#define MonoPrint  printf
#else
	#define MonoPrint  NULL
#endif 

#define MonoPrint  printf

long tempTarget; // for missile lock.
				


/*Converted list to vector*/
std::vector<ACMIEntityData> importEntityVec;
std::vector<ACMIEntityData> importFeatVec;
std::vector<ACMIRawPositionData> importPosVec;
std::vector<ACMIEventHeader> importEventVec;
std::vector<ACMIRawPositionData> importEntEventVec;
std::vector<ACMIFeatEventImportData> importFeatEventVec;



ACMIEventTrailer *importEventTrailerList;


ACMI_CallRec *ACMI_Callsigns=NULL;
ACMI_CallRec *Import_Callsigns=NULL;
long import_count=0;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
	

	OutputDebugString("TEST-DEBUG");
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
		
	
	// this value comes from tod type record
	tapeHdr.todOffset =  0.0f;


	// Load flight file for positional data.
	flightFile = fopen(inFltFile, "rb");
						
	if (flightFile == NULL)
	{
		MonoPrint("Error opening acmi flight file");
		return FALSE;
	}

	begTime = -1.0;
	endTime = 0.0;
	//OutputDebugString("TEST-DEBUG\n");
	clock_t t;

	MonoPrint("ACMITape Import: Reading Raw Data ....\n");
	t = clock();

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
				rawPositionData = new ACMIRawPositionData;

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

				rawPositionData->entityPosData.posData.x = genpos.x;
				rawPositionData->entityPosData.posData.y = genpos.y;
				rawPositionData->entityPosData.posData.z = genpos.z;
				rawPositionData->entityPosData.posData.roll = genpos.roll;
				rawPositionData->entityPosData.posData.pitch = genpos.pitch;
				rawPositionData->entityPosData.posData.yaw = genpos.yaw;
				rawPositionData->entityPosData.posData.radarTarget= tempTarget;

				// Append our new position data.								
				importPosVec.push_back(*rawPositionData);

				rawPositionData = NULL;
		
				// bump counter
				//importNumPos++;

				break;
			case ACMIRecTracerStart:

				// Read the data
				if ( !fread( &tracer, sizeof( ACMITracerStartData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				ehdr = new ACMIEventHeader;

				/*Maybe change and stop querying the vec size*/

				// fill in data
				ehdr->eventType = hdr.type;
				ehdr->time = hdr.time;
				ehdr->timeEnd = hdr.time + 2.5F;
				ehdr->index = importEventVec.size(); //  importNumEvents;
				ehdr->x = tracer.x;
				ehdr->y = tracer.y;
				ehdr->z = tracer.z;
				ehdr->dx = tracer.dx;
				ehdr->dy = tracer.dy;
				ehdr->dz = tracer.dz;

				// Append our new data.
				importEventVec.push_back(*ehdr);
			
				ehdr = NULL;
		
				// bump counter
				//importNumEvents++;
				break;
			case ACMIRecStationarySfx:
				// Read the data
				if ( !fread( &sfx, sizeof( ACMIStationarySfxData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				ehdr = new ACMIEventHeader;

				// fill in data
				ehdr->eventType = hdr.type;
				ehdr->index = importEventVec.size();
				ehdr->time = hdr.time;
				ehdr->timeEnd = hdr.time + sfx.timeToLive;
				ehdr->x = sfx.x;
				ehdr->y = sfx.y;
				ehdr->z = sfx.z;
				ehdr->type = sfx.type;
				ehdr->scale = sfx.scale;

				// Append our new data.
				importEventVec.push_back(*ehdr);

				ehdr = NULL;
		
				// bump counter
				//importNumEvents++;
				break;

			case ACMIRecFeatureStatus:
				// Read the data
				if ( !fread( &fs, sizeof( ACMIFeatureStatusData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				fedata = new ACMIFeatEventImportData;

				// fill in data
				fedata->uniqueID = fs.uniqueID;
				fedata->data.index = -1;	// will be filled in later
				fedata->data.time = hdr.time;
				fedata->data.newStatus = fs.newStatus;
				fedata->data.prevStatus = fs.prevStatus;

				// Append our new data.
				importFeatEventVec.push_back(*fedata);
				fedata = NULL;
		
				// bump counter
				//importNumFeatEvents++;
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
				ehdr = new ACMIEventHeader;

				// fill in data
				ehdr->eventType = hdr.type;
				ehdr->index = importEventVec.size();
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

				// Append our new data.
				importEventVec.push_back(*ehdr);

				ehdr = NULL;
		
				// bump counter
				//importNumEvents++;
				break;

			case ACMIRecSwitch:
		
				// Read the data
				if ( !fread( &sd, sizeof( ACMISwitchData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				rawPositionData = new ACMIRawPositionData;
		
				// fill in raw position data
				rawPositionData->uniqueID = sd.uniqueID;
				rawPositionData->type = sd.type;
				rawPositionData->flags = 0;

				rawPositionData->entityPosData.time = hdr.time;
				rawPositionData->entityPosData.type = PosTypeSwitch;
				rawPositionData->entityPosData.switchData.switchNum = sd.switchNum;
				rawPositionData->entityPosData.switchData.switchVal = sd.switchVal;
				rawPositionData->entityPosData.switchData.prevSwitchVal = sd.prevSwitchVal;

				// Append our new position data.
				importEntEventVec.push_back(*rawPositionData);
				rawPositionData = NULL;
		
				// bump counter
				//importNumEntEvents++;

				break;

			case ACMIRecDOF:
		
				// Read the data
				if ( !fread( &dd, sizeof( ACMIDOFData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				rawPositionData = new ACMIRawPositionData;
		
				// fill in raw position data
				rawPositionData->uniqueID = dd.uniqueID;
				rawPositionData->type = dd.type;
				rawPositionData->flags = 0;


				rawPositionData->entityPosData.time = hdr.time;
				rawPositionData->entityPosData.type = PosTypeDOF;
				rawPositionData->entityPosData.dofData.DOFNum = dd.DOFNum;
				rawPositionData->entityPosData.dofData.DOFVal = dd.DOFVal;
				rawPositionData->entityPosData.dofData.prevDOFVal = dd.prevDOFVal;
				
				
				// Append our new position data.
				importEntEventVec.push_back(*rawPositionData);

				rawPositionData = NULL;
		
				// bump counter
				//importNumEntEvents++;

				break;

			case ACMIRecFeaturePosition:
		
				// Read the data
				if ( !fread( &featpos, sizeof( ACMIFeaturePositionData ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				// Allocate a new data node.
				rawPositionData = new ACMIRawPositionData;

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
				
				// Append our new position data.
				importPosVec.push_back(*rawPositionData);

				rawPositionData = NULL;
		
				// bump counter
				//importNumPos++;

				break;
			case ACMICallsignList:
		
				// Read the data
				if ( !fread( &import_count, sizeof( long ), 1, flightFile ) )
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}

				Import_Callsigns=new ACMI_CallRec[import_count];

				if(!fread(Import_Callsigns,import_count * sizeof(ACMI_CallRec),1,flightFile))
				{
					CleanupACMIImportPositionData ( flightFile, rawPositionData );
					return FALSE;
				}
				break;

			default:
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

	t = clock() - t;
	MonoPrint("import entity It took me %d clicks (%f seconds).\n\n", t, ((float)t) / CLOCKS_PER_SEC);

	
	// build the importEntityList
	MonoPrint("ACMITape Import: Parsing Entities ....\n");
	t = clock();
	ParseEntities();
	t = clock() - t;
	MonoPrint("VECTOR : thread entity It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);


	int importEntityVecSize = importEntityVec.size();		// importNumEnt
	int importPosVecSize = importPosVec.size();				// importNumPos
	int importFeatVecSize = importFeatVec.size();			// importNumFeat
	int importEntEventVecSize = importEntEventVec.size();	// importNumEntEvents
	int importFeatEventVecSize = importFeatEventVec.size();	// importNumFeatEvents
	int importEventVecSize = importEventVec.size();			// importNumEvents

	// setup the tape header
	tapeHdr.fileID = 'TAPE';
	tapeHdr.numEntities = importEntityVecSize;
	tapeHdr.numFeat = importFeatVecSize;
	tapeHdr.entityBlockOffset = sizeof( ACMITapeHeader );
	tapeHdr.featBlockOffset = tapeHdr.entityBlockOffset +
								  sizeof( ACMIEntityData ) * importEntityVecSize;
	tapeHdr.timelineBlockOffset = tapeHdr.featBlockOffset +
								  sizeof( ACMIEntityData ) * importFeatVecSize;
	tapeHdr.firstEntEventOffset = tapeHdr.timelineBlockOffset +
								  sizeof( ACMIEntityPositionData ) * importPosVecSize;
	tapeHdr.firstGeneralEventOffset = tapeHdr.firstEntEventOffset +
								  sizeof( ACMIEntityPositionData ) * importEntEventVecSize;
	tapeHdr.firstEventTrailerOffset = tapeHdr.firstGeneralEventOffset +
								  sizeof( ACMIEventHeader ) * importEventVecSize;
	tapeHdr.firstFeatEventOffset = tapeHdr.firstEventTrailerOffset +
								  sizeof( ACMIEventTrailer ) * importEventVecSize;
	tapeHdr.firstTextEventOffset = tapeHdr.firstFeatEventOffset +
								  sizeof( ACMIFeatEvent ) * importFeatEventVecSize;
	tapeHdr.numEntityPositions = importPosVecSize;
	tapeHdr.numEvents = importEventVecSize;
	tapeHdr.numFeatEvents = importFeatEventVecSize;
	tapeHdr.numEntEvents = importEntEventVecSize;
	tapeHdr.totPlayTime = endTime - begTime;
	tapeHdr.startTime =  begTime;

	

	// set up the chain offsets of entity positions
	MonoPrint("ACMITape Import: Threading Positions ....\n");
	
	t = clock();
	ThreadEntityPositions(&tapeHdr);
	t = clock() - t;
	MonoPrint("VECTOR : thread entity It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	

	// set up the chain offsets of entity events
	MonoPrint("ACMITape Import: Threading Entity Events ....\n");

	t = clock();
	ThreadEntityEvents(&tapeHdr);
	t = clock() - t;
	MonoPrint("VECTOR : It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);


	// Calculate size of .vhs file.
	tapeHdr.fileSize = tapeHdr.timelineBlockOffset +
					   sizeof( ACMIEntityPositionData ) * importPosVecSize +
					   sizeof( ACMIEntityPositionData ) * importEntEventVecSize +
					   sizeof( ACMIEventHeader ) * importEventVecSize +
					   sizeof( ACMIFeatEvent ) * importFeatEventVecSize +
					   sizeof( ACMIEventTrailer ) * importEventVecSize;

	// Open a writecopy file mapping.
	// Write out file in .vhs format.
	
	MonoPrint("ACMITape Import: Writing Tape File ....\n");
	
	t = clock();
	WriteTapeFile(outTapeFileName, &tapeHdr);
	t = clock() - t;
	MonoPrint("VECTOR : It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
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

void ACMITape::ParseEntities(void)
{
	int	i = 0;

	int importPosVecSize = importPosVec.size();

	for (int count = 0; count < importPosVecSize; count++)
	{

		if (importPosVec[count].flags & ENTITY_FLAG_FEATURE)
		{
			// look for existing entity
			for (i = 0; i < importFeatVec.size() && importPosVec[count].uniqueID != importFeatVec[i].uniqueID; i++)
			{			
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
			for (i = 0; i < importEntityVec.size() && importPosVec[count].uniqueID != importEntityVec[i].uniqueID; i++)
			{
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

	/*
	** I have no Idea if it's really usefull as Tacview doesn't seem to be using those values.
	** But they were doing that in the original code so *shrug*
	*/
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
void ACMITape::ThreadEntityPositions(ACMITapeHeader *tapeHdr)
{
	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together
	int importEntityVecSize = importEntityVec.size();	// importNumEnt
	int importPosVecSize = importPosVec.size();			// importNumPos
	int importFeatVecSize = importFeatVec.size();		// importNumFeat
	int importEntEventVecSize = importEntEventVec.size(); // importNumEntEvents
	int importFeatEventVecSize = importFeatEventVec.size();

	//for (int i = 0; i < importEntityVecSize; i++)
	//{
	par_for(0, importEntityVecSize, [&](int i, int cpu)
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

	}); // end for threaded entity loop

	  // ------------------------------------------------------------------------------------
	  // ------------------------------------------------------------------------------------
	  // ------------------------------------------------------------------------------------

	// we run an outer and inner loop here.
	// the outer loops steps thru each Feature
	// the inner loop searches each position update for one owned by the
	// Feature and chains them together
	
	clock_t t;
	// build the importEntityList
	//MonoPrint("ACMITape Import: testloop....\n");
	//t = clock();

	



	//for (int i = 0; i < importFeatVecSize; i++)
	par_for(0, importFeatVecSize, [&](int i, int cpu)
	{
		long currOffset;
		BOOL foundFirst = FALSE;
		long prevOffset = 0;
		ACMIRawPositionData *prevPosPtr = NULL;
		importFeatVec[i].firstPositionDataOffset = 0;
		int prevPosVec = -1;

		/* can't thread that because we need to parse the vector in order*/


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
		for (int j = 0; j < importFeatEventVecSize; j++)
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
				if (j != i && importFeatVec[i].leadIndex == importFeatVec[j].uniqueID)
				{
				
						importFeatVec[i].leadIndex = j;
						break;
				}
			}

			// if we're gone thru the whole list and haven't found
			// a lead index, we're in trouble.  To protect, set the
			// lead index to -1
			if (j == importFeatVecSize)
			{
				importFeatVec[i].leadIndex = -1;
			}
		}


		//std::vector<ACMIEntityData>::iterator it = std::find_if(importFeatVec.begin(), importFeatVec.end(), 
		//	[&](const ACMIEntityData& CurrentimportFeatVec) { return CurrentimportFeatVec.leadIndex == importFeatVec[i].leadIndex; });



	}); // end for feature entity loop

	//t = clock() - t;
	//MonoPrint("LOOP TEST : It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);

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
	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together


	int importEntityVecSize = importEntityVec.size(); // importNumEnt
	int importPosVecSize = importPosVec.size(); // importNumPos
	int importFeatVecSize = importFeatVec.size(); // importNumFeat
	int importEntEventVecSize = importEntEventVec.size(); // importNumEntEvents

	/*  Now threadded 
	** Reason we do not need mutex is the outer loop get a different entity each time
	** and the inner loop is only going to link together the position that are owned by the entity
	** So each thread will never try to link data that belongs to another thread
	*/
	par_for(0, importEntityVecSize, [&](int i, int cpu)
	{
		long currOffset;
		BOOL foundFirst = FALSE;
		long prevOffset = 0;
		ACMIRawPositionData *prevPosPtr = NULL;
		importEntityVec[i].firstEventDataOffset = 0;

		int prevPosVec = -1;

		//for (int j = 0; j < importEntEventVecSize; j++)

		/* 
		** https://stackoverflow.com/questions/3752019/how-to-get-the-index-of-a-value-in-a-vector-using-for-each
		** j is a loop index for for_each
		*/
		std::for_each(importEntEventVec.begin(), importEntEventVec.end(), [&, j = 0](ACMIRawPositionData &CurrentimportEntEventVec) mutable 
		{

			if (CurrentimportEntEventVec.uniqueID == importEntityVec[i].uniqueID)
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
				CurrentimportEntEventVec.entityPosData.prevPositionUpdateOffset = prevOffset;
				CurrentimportEntEventVec.entityPosData.nextPositionUpdateOffset = 0;

				// thread previous to current
				if (prevPosVec != -1)
				{
					importEntEventVec[prevPosVec].entityPosData.nextPositionUpdateOffset = currOffset;
				}

				// set vals for next time thru loop
				prevOffset = currOffset;
				prevPosVec = j;

			} //end of if
			++j;
		});

		//for (int j = 0; j < importEntEventVecSize; ++j)
		//{

		//	// check the id to see if this position belongs to the entity
		//	if (importEntEventVec[j].uniqueID == importEntityVec[i].uniqueID)
		//	{

		//		// calculate the offset of this positional record
		//		currOffset = tapeHdr->firstEntEventOffset +
		//			sizeof(ACMIEntityPositionData) * j;

		//		// if it's the 1st in the chain, set the offset to it in
		//		// the entity's record
		//		if (foundFirst == FALSE)
		//		{
		//			importEntityVec[i].firstEventDataOffset = currOffset;
		//			foundFirst = TRUE;
		//		}

		//		// thread current to previous
		//		importEntEventVec[j].entityPosData.prevPositionUpdateOffset = prevOffset;
		//		importEntEventVec[j].entityPosData.nextPositionUpdateOffset = 0;

		//		// thread previous to current
		//		if (prevPosVec != -1)
		//		{
		//			importEntEventVec[prevPosVec].entityPosData.nextPositionUpdateOffset = currOffset;
		//		}

		//		// set vals for next time thru loop
		//		prevOffset = currOffset;
		//		prevPosVec = j;

		//	} //end of if
		//}


	});// end for threadded entity loop
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int CompareEventTrailer(const void *p1, const void *p2)
{
	ACMIEventTrailer *t1 = (ACMIEventTrailer *)p1;
	ACMIEventTrailer *t2 = (ACMIEventTrailer *)p2;

	if (t1->timeEnd < t2->timeEnd)
		return -1;
	else if (t1->timeEnd > t2->timeEnd)
		return 1;
	else
		return 0;
}


//int CompareEventTrailer(const ACMIEventTrailer& i, const ACMIEventTrailer& j)
//{
//	if (i.timeEnd < j.timeEnd)
//		return -1;
//	else if (i.timeEnd > j.timeEnd)
//		return 1;
//	else
//		return 0;
//}

/*
** Description:
**		At this point importEntList and importPosList should be populated.
**		Also the entities and positions are now threaded
**		write out the file
*/

void ACMITape::WriteTapeFile(char *fname, ACMITapeHeader *tapeHdr)
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
		//MonoPrint("ACMITapeHeader %d - size : %ld  \n", sizeof(ACMITapeHeader), tapeHdr->fileSize);
		ret = fwrite(tapeHdr, sizeof(ACMITapeHeader), 1, tapeFile);
		if (!ret)
			throw "error_exit";


		// write out the entities 
//		for (i = 0; i < importEntityVecSize; i++)
//		{
		ret = fwrite(importEntityVec.data(), sizeof(ACMIEntityData) * importEntityVecSize, 1, tapeFile);
		if (!ret)
			throw "error_exit";
		//		} // end for entity loop



				 // write out the features
		ret = fwrite(importFeatVec.data(), sizeof(ACMIEntityData) * importFeatVecSize, 1, tapeFile);
		if (!ret)
			throw "error_exit";
		// end for entity loop

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
		for (i = 0; i < importEntEventVecSize; i++)
		{
			ret = fwrite(&(importEntEventVec[i].entityPosData), sizeof(ACMIEntityPositionData), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		}

		// allocate the trailer list


		int importEventVecSize = importEventVec.size();
		std::vector<ACMIEventTrailer> importEventTrailerVec(importEventVecSize);

		// write out the events 
		for (i = 0; i < importEventVecSize; i++)
		{
			// set the trailer data
			importEventTrailerVec[i].index = i;
			importEventTrailerVec[i].timeEnd = importEventVec[i].timeEnd;

			ret = fwrite(&importEventVec[i], sizeof(ACMIEventHeader), 1, tapeFile);
			if (!ret)
				throw "error_exit";
		} // end for events loop

		int importEventTrailerVecSize = importEventTrailerVec.size();

		/*
		Using qsort because sort and sort_stable don't output the same exact result
		Don't know if it's a problem I use that for now.
		*/
		if (importEventTrailerVecSize > 0)
		{
			qsort(&importEventTrailerVec[0], importEventTrailerVec.size(), sizeof(ACMIEventTrailer), CompareEventTrailer);

			ret = fwrite(importEventTrailerVec.data(), sizeof(ACMIEventTrailer) * importEventTrailerVecSize, 1, tapeFile);
			if (!ret)
				throw "error_exit";
		}



		// write out the feature events
		int importFeatEventVecSize = importFeatEventVec.size();

		for (i = 0; i < importFeatEventVecSize; i++)
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

	}
	catch (const std::exception& e)
	{


		MonoPrint("Error writing new tape file: %s\n", e.what());
		if (tapeFile)
			fclose(tapeFile);
		return;
	}
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
//void *
//ACMITape::GetTextEvents( int *count )
//{
//	if (_tapeHdr.numTextEvents > 1048576) // Sanity check
//	{
//		count = 0;
//		return NULL;
//	}
//
//	*count = _tapeHdr.numTextEvents;
//	return (void *)((char *)_tape + _tapeHdr.firstTextEventOffset);
//}
