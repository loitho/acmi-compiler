#ifndef _FALCLIB_H
#define _FALCLIB_H

//#include "F4Vu.h"

#pragma pack (1)

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;


typedef struct
{
   ushort size;
   ushort type;
} EventIdData;

#pragma pack ()

extern FILE* F4EventFile;

// ==================================
// Falcon 4 Event stuff
// ==================================
//
//class FalconEvent : public VuMessage
//	{
//	public:
//		enum HandlingThread {
//			NoThread                     = 0x0,				// This would be rather pointless
//			SimThread                    = 0x1,
//			CampaignThread               = 0x2,
//			UIThread					 = 0x4,
//			VuThread					 = 0x8,			// Realtime thread! carefull with what you send here
//			AllThreads                   = 0xff};
//		HandlingThread handlingThread;
//
//		virtual int Size();
//		virtual int Decode(VU_BYTE **buf, int length);
//		virtual int Encode(VU_BYTE **buf);
//
//	protected:
//		FalconEvent (VU_MSG_TYPE type, HandlingThread threadID, VU_ID entityId, VuTargetEntity *target, VU_BOOL loopback=TRUE);
//		FalconEvent (VU_MSG_TYPE type, HandlingThread threadID, VU_ID senderid, VU_ID target);
//		virtual ~FalconEvent (void);
//		virtual int Activate(VuEntity *ent);
//		virtual int Process(uchar autodisp) = 0;
//
//	private:
//		virtual int LocalSize();
//	};
//
//// ==================================
//// Falcon 4 Message filter
//// ==================================
//
//class FalconMessageFilter : public VuMessageFilter
//	{
//	private:
//		FalconEvent::HandlingThread filterThread;
//		ulong						vuFilterBits;
//
//	public:
//		FalconMessageFilter(FalconEvent::HandlingThread theThread, ulong vuMessageBits);
//		virtual ~FalconMessageFilter();
//		virtual VU_BOOL Test(VuMessage *event);
//		virtual VuMessageFilter *Copy();
//	};
//
//// ==================================
//// Functions
//// ==================================
//
//void FalconSendMessage (VuMessage* theEvent, BOOL reliableTransmit = FALSE);

#endif
