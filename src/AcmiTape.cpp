// AcmiTape.cpp
// File created : 2017-9-23
// 
//
// Last update : 2017-11-4
// By loitho

// Originally written by Jim DiZoglio (x257) as ACMIView class
// previous last modified: 9/25/97	Michael P. Songy
// Yes, that was 20 years ago


/* STFU _CRT_SECURE_NO_WARNINGS */
#pragma warning(disable:4996)

#include <vector>

#include "AcmiTape.h"
#include "threading.h"

#if _DEBUG
	#define MonoPrint  printf
#else
	#define MonoPrint  NULL
#endif 

#define MonoPrint  printf





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ACMITape::ACMITape()
{ 
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ACMITape::~ACMITape()
{
	// Delete Callsigns
	delete Import_Callsigns;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool ACMITape::Import(const char *inFltFile, const char *outTapeFileName)
{
	FILE
		*flightFile;

	/*
	** We'll use the const to reset the structure value
	** This keeps the compiled file consistent between
	** 2 compilations
	*/

	const ACMIRawPositionData	EmptyrawPositionData;
	ACMIRawPositionData			rawPositionData;

	const ACMIEventHeader	EmptyHeader;
	ACMIEventHeader			ehdr;

	const ACMIFeatEventImportData	Emptyfedata;
	ACMIFeatEventImportData			fedata;

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
		MonoPrint("Error opening acmi flight file\n");
		return false;
	}

	begTime = -1.0;
	endTime = 0.0;
	//OutputDebugString("TEST-DEBUG\n");
	clock_t t;

	MonoPrint("(1/5) ACMITape Import: Reading Raw Data ....\n");
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
					return false;
				}
				if (hdr.type == ACMIRecAircraftPosition)
					fread(&tempTarget, sizeof(tempTarget),1,flightFile);
				else
					tempTarget = -1;

				// Clear positiondata structure.
				rawPositionData = EmptyrawPositionData;

				// fill in raw position data
				rawPositionData.uniqueID = genpos.uniqueID;
				rawPositionData.type = genpos.type;
				if ( hdr.type == ACMIRecMissilePosition )
					rawPositionData.flags = ENTITY_FLAG_MISSILE;
				else if ( hdr.type == ACMIRecAircraftPosition )
					rawPositionData.flags = ENTITY_FLAG_AIRCRAFT;
				else if ( hdr.type == ACMIRecChaffPosition )
					rawPositionData.flags = ENTITY_FLAG_CHAFF;
				else if ( hdr.type == ACMIRecFlarePosition )
					rawPositionData.flags = ENTITY_FLAG_FLARE;
				else
					rawPositionData.flags = 0;

				rawPositionData.entityPosData.time = hdr.time;
				rawPositionData.entityPosData.type = PosTypePos;

				rawPositionData.entityPosData.posData.x = genpos.x;
				rawPositionData.entityPosData.posData.y = genpos.y;
				rawPositionData.entityPosData.posData.z = genpos.z;
				rawPositionData.entityPosData.posData.roll = genpos.roll;
				rawPositionData.entityPosData.posData.pitch = genpos.pitch;
				rawPositionData.entityPosData.posData.yaw = genpos.yaw;
				rawPositionData.entityPosData.posData.radarTarget= tempTarget;

				// Append our new position data.								
				importPosVec.push_back(rawPositionData);

				break;

			case ACMIRecTracerStart:

				// Read the data
				if ( !fread( &tracer, sizeof( ACMITracerStartData ), 1, flightFile ) )
				{
					return false;
				}

				// Allocate a new data node.
				ehdr = EmptyHeader;
				
				/*Maybe change and stop querying the vec size*/

				// fill in data
				ehdr.eventType = hdr.type;
				ehdr.time = hdr.time;
				ehdr.timeEnd = hdr.time + 2.5F;
				ehdr.index = importEventVec.size(); //  importNumEvents;
				ehdr.x = tracer.x;
				ehdr.y = tracer.y;
				ehdr.z = tracer.z;
				ehdr.dx = tracer.dx;
				ehdr.dy = tracer.dy;
				ehdr.dz = tracer.dz;

				// Append our new data.
				importEventVec.push_back(ehdr);
				
				break;

			case ACMIRecStationarySfx:
				// Read the data
				if ( !fread( &sfx, sizeof( ACMIStationarySfxData ), 1, flightFile ) )
				{
					return false;
				}

				// clear header
				ehdr = EmptyHeader;

				// fill in data
				ehdr.eventType = hdr.type;
				ehdr.index = importEventVec.size();
				ehdr.time = hdr.time;
				ehdr.timeEnd = hdr.time + sfx.timeToLive;
				ehdr.x = sfx.x;
				ehdr.y = sfx.y;
				ehdr.z = sfx.z;
				ehdr.type = sfx.type;
				ehdr.scale = sfx.scale;

				// Append our new data.
				importEventVec.push_back(ehdr);

				break;

			case ACMIRecFeatureStatus:
				// Read the data
				if ( !fread( &fs, sizeof( ACMIFeatureStatusData ), 1, flightFile ) )
				{
					return false;
				}

				// Clear feature structure.
				fedata = Emptyfedata;

				// fill in data
				fedata.uniqueID = fs.uniqueID;
				fedata.data.index = -1;	// will be filled in later
				fedata.data.time = hdr.time;
				fedata.data.newStatus = fs.newStatus;
				fedata.data.prevStatus = fs.prevStatus;

				// Append our new data.
				importFeatEventVec.push_back(fedata);

				break;

			// not ready for these yet
			case ACMIRecMovingSfx:
				// Read the data
				if ( !fread( &msfx, sizeof( ACMIMovingSfxData ), 1, flightFile ) )
				{
					return false;
				}

				// Clear header structure.
				ehdr = EmptyHeader;

				// fill in data
				ehdr.eventType = hdr.type;
				ehdr.index = importEventVec.size();
				ehdr.time = hdr.time;
				ehdr.timeEnd = hdr.time + msfx.timeToLive;
				ehdr.x = msfx.x;
				ehdr.y = msfx.y;
				ehdr.z = msfx.z;
				ehdr.dx = msfx.dx;
				ehdr.dy = msfx.dy;
				ehdr.dz = msfx.dz;
				ehdr.flags = msfx.flags;
				ehdr.user = msfx.user;
				ehdr.type = msfx.type;
				ehdr.scale = msfx.scale;

				// Append our new data.
				importEventVec.push_back(ehdr);

				break;

			case ACMIRecSwitch:
		
				// Read the data
				if ( !fread( &sd, sizeof( ACMISwitchData ), 1, flightFile ) )
				{
					return false;
				}

				// Clear positiondata structure.
				rawPositionData = EmptyrawPositionData;
		
				// fill in raw position data
				rawPositionData.uniqueID = sd.uniqueID;
				rawPositionData.type = sd.type;
				rawPositionData.flags = 0;

				rawPositionData.entityPosData.time = hdr.time;
				rawPositionData.entityPosData.type = PosTypeSwitch;
				rawPositionData.entityPosData.switchData.switchNum = sd.switchNum;
				rawPositionData.entityPosData.switchData.switchVal = sd.switchVal;
				rawPositionData.entityPosData.switchData.prevSwitchVal = sd.prevSwitchVal;

				// Append our new position data.
				importEntEventVec.push_back(rawPositionData);

				break;

			case ACMIRecDOF:
		
				// Read the data
				if ( !fread( &dd, sizeof( ACMIDOFData ), 1, flightFile ) )
				{
					return false;
				}

				// Clear positiondata structure.
				rawPositionData = EmptyrawPositionData;
		
				// fill in raw position data
				rawPositionData.uniqueID = dd.uniqueID;
				rawPositionData.type = dd.type;
				rawPositionData.flags = 0;

				rawPositionData.entityPosData.time = hdr.time;
				rawPositionData.entityPosData.type = PosTypeDOF;
				rawPositionData.entityPosData.dofData.DOFNum = dd.DOFNum;
				rawPositionData.entityPosData.dofData.DOFVal = dd.DOFVal;
				rawPositionData.entityPosData.dofData.prevDOFVal = dd.prevDOFVal;
						
				// Append our new position data.
				importEntEventVec.push_back(rawPositionData);

				break;

			case ACMIRecFeaturePosition:
		
				// Read the data
				if ( !fread( &featpos, sizeof( ACMIFeaturePositionData ), 1, flightFile ) )
				{
					return false;
				}

				// Clear positiondata structure.
				rawPositionData = EmptyrawPositionData;

				// fill in raw position data
				rawPositionData.uniqueID = featpos.uniqueID;
				rawPositionData.leadIndex = featpos.leadUniqueID;
				rawPositionData.specialFlags = featpos.specialFlags;
				rawPositionData.slot = featpos.slot;
				rawPositionData.type = featpos.type;
				rawPositionData.flags = ENTITY_FLAG_FEATURE;

				rawPositionData.entityPosData.time = hdr.time;
				rawPositionData.entityPosData.type = PosTypePos;
				rawPositionData.entityPosData.posData.x = featpos.x;
				rawPositionData.entityPosData.posData.y = featpos.y;
				rawPositionData.entityPosData.posData.z = featpos.z;
				rawPositionData.entityPosData.posData.roll = featpos.roll;
				rawPositionData.entityPosData.posData.pitch = featpos.pitch;
				rawPositionData.entityPosData.posData.yaw = featpos.yaw;
				
				// Append our new position data.
				importPosVec.push_back(rawPositionData);

				break;

			case ACMICallsignList:
		
				// Read the data
				if ( !fread( &import_count, sizeof( long ), 1, flightFile ) )
				{
					return false;
				}
				//Import_Callsigns.
				//Import_Callsigns.reserve(import_count * sizeof(ACMI_CallRec));
				Import_Callsigns=new ACMI_CallRec[import_count];

				if (!fread(Import_Callsigns, import_count * sizeof(ACMI_CallRec), 1, flightFile))
				{
						return false;
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
	MonoPrint("(1/5) ACMITape Import: Reading Raw Data \t took me %d clicks (%f seconds).\n\n", t, ((float)t) / CLOCKS_PER_SEC);

	
	// build the importEntityList
	MonoPrint("(2/5) ACMITape Import: Parsing Entities ....\n");
	t = clock();
	ParseEntities();
	t = clock() - t;
	MonoPrint("(2/5) ACMITape Import: Parsing Entities \t took me %d clicks (%f seconds).\n\n", t, ((float)t) / CLOCKS_PER_SEC);


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
	MonoPrint("(3/5) ACMITape Import: Threading Positions ....\n");
	
	t = clock();
	ThreadEntityPositions(&tapeHdr);
	t = clock() - t;
	MonoPrint("(3/5) ACMITape Import: Threading Positions took me %d clicks (%f seconds).\n\n", t, ((float)t) / CLOCKS_PER_SEC);
	

	// set up the chain offsets of entity events
	MonoPrint("(4/5) ACMITape Import: Threading Entity Events ....\n");

	t = clock();
	ThreadEntityEvents(&tapeHdr);
	t = clock() - t;
	MonoPrint("(4/5) ACMITape Import: Threading Entity took me %d clicks (%f seconds).\n\n", t, ((float)t) / CLOCKS_PER_SEC);


	// Calculate size of .vhs file.
	tapeHdr.fileSize = tapeHdr.timelineBlockOffset +
					   sizeof( ACMIEntityPositionData ) * importPosVecSize +
					   sizeof( ACMIEntityPositionData ) * importEntEventVecSize +
					   sizeof( ACMIEventHeader ) * importEventVecSize +
					   sizeof( ACMIFeatEvent ) * importFeatEventVecSize +
					   sizeof( ACMIEventTrailer ) * importEventVecSize;

	// Open a writecopy file mapping.
	// Write out file in .vhs format.
	
	MonoPrint("(5/5) ACMITape Import: Writing Tape File ....\n");
	
	fclose(flightFile);

	t = clock();
	WriteTapeFile(outTapeFileName, &tapeHdr);
	t = clock() - t;
	MonoPrint("(5/5) ACMITape Import: Writing Tape File took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	
	return true;
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
		// if feature
		if (importPosVec[count].flags & ENTITY_FLAG_FEATURE)
		{
			// look for existing entity
			for (i = 0; i < importFeatVec.size() && importPosVec[count].uniqueID != importFeatVec[i].uniqueID; i++)
			{			
			}
			
			// create new import entity record if none exist
			if (i == importFeatVec.size())
			{
				ACMIEntityData importEntityInfo;
				importEntityInfo.count = 0;

				importEntityInfo.uniqueID = importPosVec[count].uniqueID;
				importEntityInfo.type = importPosVec[count].type;
				importEntityInfo.flags = importPosVec[count].flags;
				importEntityInfo.leadIndex = importPosVec[count].leadIndex;
				importEntityInfo.specialFlags = importPosVec[count].specialFlags;
				importEntityInfo.slot = importPosVec[count].slot;

				importFeatVec.push_back(importEntityInfo);
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

	MonoPrint(" Parsing: Counting Entities....\n");

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
	MonoPrint(" Parsing: Counting Entities ended....\n");
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
	
	int importEntityVecSize = importEntityVec.size();	// importNumEnt
	int importPosVecSize = importPosVec.size();			// importNumPos
	int importFeatVecSize = importFeatVec.size();		// importNumFeat
	int importEntEventVecSize = importEntEventVec.size(); // importNumEntEvents
	int importFeatEventVecSize = importFeatEventVec.size();

	clock_t t = clock();

	// we run an outer and inner loop here.
	// the outer loops steps thru each entity
	// the inner loop searches each position update for one owned by the
	// entity and chains them together
	par_for(0, importEntityVecSize, [&](int i, int cpu)
	{

		long currOffset;
		bool foundFirst = false;
		long prevOffset = 0;
		importEntityVec[i].firstPositionDataOffset = 0;
		int prevPosVec = -1;


		//std::for_each(importPosVec.begin(), importPosVec.end(), [&, j = 0](ACMIRawPositionData &CurrentimportPosVec) mutable
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
				if (foundFirst == false)
				{
					importEntityVec[i].firstPositionDataOffset = currOffset;
					foundFirst = true;
				}

				// link current to previous
				importPosVec[j].entityPosData.prevPositionUpdateOffset = prevOffset;
				importPosVec[j].entityPosData.nextPositionUpdateOffset = 0;

				// link previous to current
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

	

	t = clock() - t;
	MonoPrint(" - Thread Entity par_for 1: It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	t = clock();

	// we run an outer and inner loop here.
	// the outer loops steps thru each Feature
	// the inner loop searches each position update for one owned by the
	// Feature and chains them together
	par_for(0, importFeatVecSize, [&](int i, int cpu)
	{
		long currOffset;
		bool foundFirst = false;
		long prevOffset = 0;
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
				if (foundFirst == false)
				{
					importFeatVec[i].firstPositionDataOffset = currOffset;
					foundFirst = true;
				}

				// link current to previous
				importPosVec[j].entityPosData.prevPositionUpdateOffset = prevOffset;
				importPosVec[j].entityPosData.nextPositionUpdateOffset = 0;

				// link previous to current
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

	}); // end for feature entity loop

	t = clock() - t;
	MonoPrint(" - Thread Entity par_for 2: It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
}


/*
** Description:
**		At this point importEntityVec and importPosVec should be populated.
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
		bool foundFirst = false;
		long prevOffset = 0;
		importEntityVec[i].firstEventDataOffset = 0;

		int prevPosVec = -1;

		/* 
		** for (int j = 0; j < importEntEventVecSize; j++)
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
				if (foundFirst == false)
				{
					importEntityVec[i].firstEventDataOffset = currOffset;
					foundFirst = true;
				}

				// link current to previous
				CurrentimportEntEventVec.entityPosData.prevPositionUpdateOffset = prevOffset;
				CurrentimportEntEventVec.entityPosData.nextPositionUpdateOffset = 0;

				// link previous to current
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


/*
** Description:
**		At this point importEntityVec and importPosVec should be populated.
**		Also the entities and positions are now threaded
**		write out the file
*/

void ACMITape::WriteTapeFile(const char *fname, ACMITapeHeader *tapeHdr)
{
	FILE *tapeFile;


	int importEntityVecSize = importEntityVec.size(); // importNumEnt
	int importPosVecSize = importPosVec.size(); // importNumPos
	int importFeatVecSize = importFeatVec.size(); // importNumFeat
	int importEntEventVecSize = importEntEventVec.size(); // importNumEntEvents


	try {

		int i, j;

		size_t ret;

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
			throw std::runtime_error("Error writing to file");


		// write out the entities 
		ret = fwrite(importEntityVec.data(), sizeof(ACMIEntityData) * importEntityVecSize, 1, tapeFile);
		if (!ret)
			throw std::runtime_error("Error writing to file");
	


		// write out the features
		ret = fwrite(importFeatVec.data(), sizeof(ACMIEntityData) * importFeatVecSize, 1, tapeFile);
		if (!ret)
			throw std::runtime_error("Error writing to file");
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
				throw std::runtime_error("Error writing to file");
		}

		// write out the entitiy events
		for (i = 0; i < importEntEventVecSize; i++)
		{
			ret = fwrite(&(importEntEventVec[i].entityPosData), sizeof(ACMIEntityPositionData), 1, tapeFile);
			if (!ret)
				throw std::runtime_error("Error writing to file");
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
				throw std::runtime_error("Error writing to file");
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
				throw std::runtime_error("Error writing to file");
		}



		// write out the feature events
		int importFeatEventVecSize = importFeatEventVec.size();

		for (i = 0; i < importFeatEventVecSize; i++)
		{
			ret = fwrite(&importFeatEventVec[i].data, sizeof(ACMIFeatEvent), 1, tapeFile);
			if (!ret)
				throw std::runtime_error("Error writing to file");
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
**		Only the callsings now
*/
void ACMITape::ImportTextEventList(FILE *fd, ACMITapeHeader *tapeHdr)
{
	// fd is tapefile vhs and tapehdr is the tape header
	size_t ret;

	tapeHdr->numTextEvents = 0;

	
	// write callsign list
	if (Import_Callsigns)
	{
		ret = fwrite(&import_count, sizeof(long), 1, fd);
		if (!ret)
			goto error_exit;

		ret = fwrite(Import_Callsigns, import_count * sizeof(ACMI_CallRec), 1, fd);
		if (!ret)
			goto error_exit;
	}

	// write the header again (bleck)
	ret = fseek(fd, 0, SEEK_SET);
	if (ret)
	{
		MonoPrint("Error seeking TAPE start\n");
		goto error_exit;
	}
	ret = fwrite(tapeHdr, sizeof(ACMITapeHeader), 1, fd);
	if (!ret)
	{
		MonoPrint("Error writing TAPE header again\n");
	}

error_exit:
	return;
}