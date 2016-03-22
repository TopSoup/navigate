#ifndef __LOCATION_H__
#define __LOCATION_H__

/*======================================================
FILE:  SP_Track.h

SERVICES: Tracking

GENERAL DESCRIPTION:
	Sample code to demonstrate services of IPosDet.

        Copyright © 2003 QUALCOMM Incorporated.
               All Rights Reserved.
            QUALCOMM Proprietary/GTDR
=====================================================*/
#include "AEEComdef.h"
#include "AEEPosDet.h"
#include "transform.h"

typedef struct _TrackState TrackState;
typedef enum {
   TRACK_LOCAL,      /* Uses AEEGPS_TRACK_LOCAL */
   TRACK_NETWORK,     /* Uses AEEGPS_TRACK_NETOWORK */
   TRACK_AUTO        /* Attempts using AEEGPS_TRACK_LOCAL if it fails uses AEEGPS_TRACK_NETWORK */
} TrackType;

typedef struct {
	double lat;       /* latitude on WGS-84 Geoid */
	double lon;       /* longitude on WGS-84 Geoid */
}Coordinate;

typedef struct {
   int    nErr;      /* SUCCESS or AEEGPS_ERR_* */
   uint32 dwFixNum;  /* fix number in this tracking session. */
   double lat;       /* latitude on WGS-84 Geoid */
   double lon;       /* longitude on WGS-84 Geoid */
   short  height;    /* Height from WGS-84 Geoid */
   double velocityHor; /* Horizontal velocity, meters/second*/
   double heading;	 /*  Current Heading degrees  */
   double distance;  /*  Distance between current position with destPos */
   double destHeading; /* Heading degrees between current position with destPos */

   //Config Param
   AEEGPSServer server; /* Specifies the server type and configuration */
   AEEGPSQos qos;		/* Corresponds to the number of seconds that the application wants the position determination engine to search for satellites */
   AEEGPSOpt optim;		/* Specifies whether to optimize the position determination request for speed, accuracy, or data exchange */
   AEEGPSMode mode;		/* Specifies gps work mode */

   Coordinate destPos;	/* Specifies destination */
   boolean bSetDestPos;

}PositionData;

struct _GetGPSInfo {
   PositionData theInfo;
   IPosDet      *pPosDet;
   AEECallback  cbPosDet;
   AEECallback  cbProgressTimer;
   uint32       dwFixNumber;
   uint32       dwFixDuration;
   uint32       dwFail;
   uint32       dwTimeout;
   uint16       wProgress;
   boolean      bPaused;
   uint16       wMainMenuEntry;
   boolean      bAbort;
   TrackState   *pts;
};

#ifdef __cplusplus
extern "C" {
#endif

   /* Return Values : 
             SUCCESS
             EBADPARM - One or more of the Invalid arguments
             EUNSUPPORTED - Unimplemented
             ENOMEMORY - When system is out of memory.
             EFAILED - General failure.
             EALREADY - When tracking is already in progress.


      Values in PositionData::nErr :
             SUCCESS
             AEEGPS_ERR_*
             EIDLE - When a session is halted or done.
   */


   /* Creates and initializes a handle for tracking. 
   ** Invoke the CALLBACK_Cancel( pcb ) to destroy this object. */
   int Track_Init( IShell *pIShell, IPosDet *pIPos, AEECallback *pcb, TrackState **po );

   /* Starts the tracking using the object created in Track_Init */
   int Track_Start( TrackState *pts, TrackType t, int nFixes, int nInterval, PositionData *pData );

   /* Stops the tracking, does not clean up the object, it can be
   ** further used with Track_Start. Only CALLBACK_Cancel(pcb) releases
   ** the object. */
   int Track_Stop( TrackState *pts );

   /* Calculate the distance between A and B */
   double Track_Calc_Distance( double latA, double lngA, double latB, double lngB );

   /* Calculate the Azimuth between A and B */
   double Track_Calc_Azimuth( double latA, double lngA, double latB, double lngB );

   /* For test */
   void Track_Test_All();

#ifdef __cplusplus
}
#endif

#endif