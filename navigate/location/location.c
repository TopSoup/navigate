/*======================================================
FILE:  SP_Track.c

SERVICES: Tracking using IPosDet.
  Track_Init
  Track_Start
  Track_Stop

GENERAL DESCRIPTION:
	Sample code to demonstrate services of IPosDet. See SP_Track.h for the
   description of exported functions.

        Copyright ?2003 QUALCOMM Incorporated.
               All Rights Reserved.
            QUALCOMM Proprietary/GTDR
=====================================================*/
#include "AEEComdef.h"
#include "BREWVersion.h"
#include "AEEStdLib.h"
#include "location.h"

struct _TrackState{
   boolean     bInNotification;     /* When the state machine is notifying the client */
   boolean     bSetForCancellation; /* Track is meant to be cancelled. do so when it is safe. */
   boolean     bInProgress;         /* when tracking is in progress. */
/* For Dest Position */
   boolean	   bSetDestPos;

/**************************/

/* For TRACK_AUTO */
   boolean     bModeAuto;
   boolean     bModeLocal;

/* Private members to work with IPosDet */
   AEECallback cbIntervalTimer;
   AEECallback cbInfo;
   AEEGPSInfo theInfo;
   Coordinate lastCoordinate;
   
/**************************/

/* Clients response members. */
   AEECallback *pcbResp;
   PositionData *pResp;
/**************************/

/* Client passed members. */
   int nPendingFixes;
   int nTrackInterval;
   IPosDet    *pPos;
   IShell     *pShell;
/**************************/
};


static void Track_Notify( TrackState *pts )
{
   pts->bInNotification = TRUE;
   pts->pcbResp->pfnNotify( pts->pcbResp->pNotifyData );
   pts->bInNotification = FALSE;
}

static void Track_Cancel( AEECallback *pcb )
{
   TrackState *pts = (TrackState *)pcb->pCancelData;

   if( TRUE == pts->bInNotification ) {
      /* It is not safe to cleanup from a notification. Defer it. */
      pts->bSetForCancellation = TRUE;
      return;
   }

   /* Kill any ongoing process */
   CALLBACK_Cancel( &pts->cbInfo );
   CALLBACK_Cancel( &pts->cbIntervalTimer );
   
   pts->pcbResp->pfnCancel = 0;
   pts->pcbResp->pCancelData = 0;

   IPOSDET_Release( pts->pPos );
   ISHELL_Release( pts->pShell );

   FREE( pts );
}

static void Track_cbInterval( TrackState *pts )
{
   /* Cancel if it was deferred. */
   if( TRUE == pts->bSetForCancellation ) {

      Track_Cancel( pts->pcbResp );
      return;
   }

   DBGPRINTF( "TRACK : bAuto:%d bLocal:%d", pts->bModeAuto, pts->bModeLocal );

   // Request GPSInfo
   if( TRUE == pts->bInProgress && SUCCESS != IPOSDET_GetGPSInfo( pts->pPos, 
      AEEGPS_GETINFO_LOCATION|AEEGPS_GETINFO_ALTITUDE, AEEGPS_ACCURACY_LEVEL1, 
      &pts->theInfo, &pts->cbInfo ) ) {

	  DBGPRINTF( "IPOSDET_GetGPSInfo Failed!");
	  
      /* Report a failure and bailout */
      pts->pResp->nErr = AEEGPS_ERR_GENERAL_FAILURE;

      Track_Notify( pts );

      Track_Stop( pts );

   }
}

static void Track_Network( TrackState *pts )
{
   AEEGPSConfig config;

   DBGPRINTF( "TRACK NETWORK" );
   
   (void) IPOSDET_GetGPSConfig( pts->pPos, &config );

   config.mode = AEEGPS_MODE_TRACK_NETWORK;

   (void) IPOSDET_SetGPSConfig( pts->pPos, &config );

   pts->bModeLocal = FALSE;
}

static void Track_Local( TrackState *pts )
{
   AEEGPSConfig config;

   DBGPRINTF( "TRACK LOCAL" );

   (void) IPOSDET_GetGPSConfig( pts->pPos, &config );

   config.mode = AEEGPS_MODE_TRACK_LOCAL;

   (void) IPOSDET_SetGPSConfig( pts->pPos, &config );

   pts->bModeLocal = TRUE;
}

static void Track_cbInfo( TrackState *pts )
{
   if( pts->theInfo.status == AEEGPS_ERR_NO_ERR 
      || (pts->theInfo.status == AEEGPS_ERR_INFO_UNAVAIL && pts->theInfo.fValid) ) {

#if MIN_BREW_VERSION(2,1)
      pts->pResp->lat = WGS84_TO_DEGREES( pts->theInfo.dwLat );
#ifdef AEE_SIMULATOR
	  //FOR TEST
      pts->pResp->lon = -WGS84_TO_DEGREES( pts->theInfo.dwLon );
#else
	  pts->pResp->lon = WGS84_TO_DEGREES( pts->theInfo.dwLon );
#endif
#else
      double    wgsFactor;
      wgsFactor = FASSIGN_STR("186413.5111");
      pts->pResp->lat = FASSIGN_INT(pts->theInfo.dwLat);
      pts->pResp->lat = FDIV(pts->pResp->lat, wgsFactor);

      pts->pResp->lon = FASSIGN_INT(pts->theInfo.dwLon);
      pts->pResp->lon = FDIV(pts->pResp->lon, wgsFactor);
#endif /* MIN_BREW_VERSION 2.1 */

	  pts->pResp->height = pts->theInfo.wAltitude - 500;
	  pts->pResp->velocityHor = FMUL( pts->theInfo.wVelocityHor,0.25);
	
	  //当前夹角
	  if (FCMP_G(FABS(pts->lastCoordinate.lat), 0))
	  {
		  pts->pResp->heading = Track_Calc_Azimuth(pts->lastCoordinate.lat, pts->lastCoordinate.lon, pts->pResp->lat, pts->pResp->lon);
	  }
	  else
	  {
		  pts->pResp->heading = 0;
	  }

	  //For Test Hack
	  //pts->pResp->lat = 38.0422378880;
	  //pts->pResp->lon = 114.4925141047;

	  if (pts->pResp->bSetDestPos)
	  {
		  //计算距离和方位角
		  pts->pResp->distance = Track_Calc_Distance(pts->pResp->lat, pts->pResp->lon, pts->pResp->destPos.lat, pts->pResp->destPos.lon);
		  pts->pResp->destHeading = Track_Calc_Azimuth(pts->pResp->lat, pts->pResp->lon, pts->pResp->destPos.lat, pts->pResp->destPos.lon);
	  }
	  
	  pts->lastCoordinate.lat = pts->pResp->lat;
	  pts->lastCoordinate.lon = pts->pResp->lon;

      pts->pResp->dwFixNum++;

      pts->pResp->nErr = SUCCESS;

      Track_Notify( pts );

      if( (!pts->nPendingFixes || --pts->nPendingFixes > 0) 
         && FALSE == pts->bSetForCancellation ) {

         if( pts->bModeAuto && pts->bModeLocal == FALSE ) {

            /* Try with local first */
            Track_Local( pts );
         }

         ISHELL_SetTimerEx( pts->pShell, pts->nTrackInterval * 1000, &pts->cbIntervalTimer );
      }
      else {

         Track_Stop( pts );
      }
   }
   else {

      if( pts->bModeAuto && pts->bModeLocal ) {

         /* Retry with TRACK_NETWORK */
         Track_Network( pts );
         Track_cbInterval( pts );
      }

      else { 
         
         /* Inform the application of failure code. */
         pts->pResp->nErr = pts->theInfo.status;
         
         Track_Notify( pts );
         
         /* On timeout re-try. For other reasons bailout. */
         if( pts->theInfo.status == AEEGPS_ERR_TIMEOUT ) {
            
            Track_cbInterval( pts );
         }
         else {
            
            Track_Stop( pts );
         }
      }
   }
}


/*======================================================================= 
Function: Track_Init()

Description: 
   Creates and initializes a handle for tracking.

Prototype:

   int Track_Init( IShell *pIShell, IPosDet *pIPos, AEECallback *pcb, TrackState **po )

Parameters:
   pIShell: [in]. IShell instance.
   pIPos: [in]. IPosDet instance.
   pcb: [in]. A persistent callback invoked on a position response.
   po: [out]. Handle to internal object.

Return Value:

   SUCCESS
   EBADPARM - One or more of the Invalid arguments
   ENOMEMORY - When system is out of memory.
 
Comments:  
   Invoke the CALLBACK_Cancel( pcb ) to destroy TrackState object.

Side Effects: 
   None

See Also:
   None
=======================================================================*/
int Track_Init( IShell *pIShell, IPosDet *pIPos, AEECallback *pcb, TrackState **po )
{
   int nErr = SUCCESS;
   TrackState *pts = NULL;

   if( !pIShell || !pIPos || !pcb || !po ) {

      nErr = EBADPARM;

   }
   else if( NULL == (pts = MALLOC( sizeof(TrackState) )) ){

      nErr = ENOMEMORY;

   }
   else {

      ZEROAT( pts );

      pts->pShell = pIShell;
      ISHELL_AddRef( pIShell );

      pts->pPos = pIPos;
      IPOSDET_AddRef( pIPos );

      /* Install the notification cb */
      CALLBACK_Cancel( pcb );
      pts->pcbResp = pcb;
      pts->pcbResp->pfnCancel   = Track_Cancel;
      pts->pcbResp->pCancelData = pts;

      CALLBACK_Init( &pts->cbIntervalTimer, Track_cbInterval, pts );
      CALLBACK_Init( &pts->cbInfo, Track_cbInfo, pts );
   }

   *po = pts;
   return nErr;

}

/*======================================================================= 
Function: Track_Stop()

Description: 
   Stops the tracking, does not clean up the object, it can be
   further used with Track_Start. Only CALLBACK_Cancel(pcb) releases
   the object.

Prototype:

   int Track_Stop( TrackState *pts );

Parameters:
   pts: [in]. TrackState object created using Track_Init().

Return Value:

   SUCCESS
   EBADPARM - One or more of the Invalid arguments
 
Comments:  
   Invoke the CALLBACK_Cancel( pcb ) to destroy TrackState object.

Side Effects: 
   None

See Also:
   Track_Init()
=======================================================================*/
int Track_Stop( TrackState *pts )
{
   if( !pts ) {
      return EBADPARM;
   }

   pts->bInProgress = FALSE;

   /* Kill any ongoing process */
   CALLBACK_Cancel( &pts->cbInfo );
   CALLBACK_Cancel( &pts->cbIntervalTimer );

   /* Report that Tracking is halted */
   pts->pResp->nErr = EIDLE;
   
   //Track_Notify( pts );

   if( TRUE == pts->bSetForCancellation ) {

      Track_Cancel( pts->pcbResp );
   }
   
   return SUCCESS;
}

/*======================================================================= 
Function: Track_Start()

Description: 
   Starts the tracking using the object created in Track_Init().

Prototype:

   int Track_Start( TrackState *pts, TrackType t, int nFixes, int nInterval, PositionData *pData );

Parameters:
   pts: [in]. TrackState object created using Track_Init().
   t: [in]. Type of tracking.
   nFixes: [in]. Number of fixes.
   nInterval: [in]. Interval between fixes in seconds.
   pData: [in]. Memory in which the Position response is to be filled.

Return Value:

   SUCCESS
   EBADPARM - One or more of the Invalid arguments
   EUNSUPPORTED - Unimplemented
   ENOMEMORY - When system is out of memory.
   EFAILED - General failure.
   EALREADY - When tracking is already in progress.
 
Comments:  
   Invoke the CALLBACK_Cancel( pcb ) to destroy TrackState object.

Side Effects: 
   None

See Also:
   Track_Init()
=======================================================================*/
int Track_Start( TrackState *pts, TrackType t, int nFixes, 
                   int nInterval, PositionData *pData )
{
   int nErr = SUCCESS;

   if( !pts || !pData ) {

      nErr = EBADPARM;
   }
   else if( TRUE == pts->bInProgress ) {

      nErr = EALREADY;
   }
   else {

      AEEGPSConfig config;

      pData->dwFixNum     = 0;

      pts->pResp          = pData;
      pts->nPendingFixes  = nFixes;
      pts->nTrackInterval = nInterval;
      pts->bModeAuto      = FALSE;
      pts->bModeLocal     = FALSE;

      IPOSDET_GetGPSConfig( pts->pPos, &config );

      /* Configure the IPosDet Instance */
      if( t == TRACK_LOCAL ) {

         config.mode = AEEGPS_MODE_TRACK_LOCAL;
         pts->bModeLocal = TRUE;
      }
      else if( t == TRACK_NETWORK ){

         config.mode = AEEGPS_MODE_TRACK_NETWORK;
      }
      else if( t == TRACK_AUTO ) {

         DBGPRINTF( "TRACK AUTO" );

         if( nFixes == 1 ) {
           
            config.mode = AEEGPS_MODE_ONE_SHOT;
         }
         else {

			config.mode = AEEGPS_MODE_ONE_SHOT;
            //config.mode = AEEGPS_MODE_TRACK_LOCAL;
            pts->bModeAuto  = TRUE;
            pts->bModeLocal = TRUE;
         }
      }

      config.nFixes = nFixes;
      config.nInterval = nInterval;

      // ADDING SUPPORT FOR USER-DEFINED PDE IP ADDRESS AND QOS
      config.server = pData->server;
      config.optim = pData->optim;
      config.qos = pData->qos;

      nErr = IPOSDET_SetGPSConfig( pts->pPos, &config );

      if( nErr == EUNSUPPORTED && pts->bModeAuto ) {

         /* As TRACK_LOCAL is unsupported on certain devices. If this is auto mode 
         ** and we tried to track locally, change it network based tracking. */

         pts->bModeAuto = FALSE;
         pts->bModeLocal = FALSE;

         config.mode = AEEGPS_MODE_TRACK_NETWORK;

         nErr = IPOSDET_SetGPSConfig( pts->pPos, &config );
      }

      if( nErr == SUCCESS ) {

         pts->bInProgress    = TRUE;

         Track_cbInterval( pts );
      }

   }
   return nErr;
}

/* Calculate the distance between A and B */
double Track_Calc_Distance( double latA, double lngA, double latB, double lngB )
{
	return calc_distance(latA, lngA, latB, lngB);
}

/* Calculate the Azimuth between A and B */
double Track_Calc_Azimuth( double latA, double lngA, double latB, double lngB )
{
	return calc_azimuth(latA, lngA, latB, lngB);
}



void Track_Test_All()
{
   Coordinate c1, c2;
   double dis = 0, th = 0;
   char szDis[64], szAzh[64];
   AECHAR bufDis[32], bufAzh[32];

   //DESTINATION
   //beijing 39.911954, 116.377817
   c1.lat = 39.911954;
   c1.lon = 116.377817;

   //1 chengde 40.8493953666,118.0478839599
   c2.lat = 40.8493953666;
   c2.lon = 118.0478839599;
   dis = Track_Calc_Distance(c1.lat, c1.lon, c2.lat, c2.lon);
   th = Track_Calc_Azimuth(c1.lat, c1.lon, c2.lat, c2.lon);

   MEMSET(szDis,0,sizeof(szDis));
   FLOATTOWSTR(dis, bufDis, 32);
   WSTRTOSTR(bufDis,szDis, 64);

   MEMSET(szAzh,0,sizeof(szAzh));
   FLOATTOWSTR(th, bufAzh, 32);
   WSTRTOSTR(bufAzh, szAzh, 64);
   
   DBGPRINTF("location_get 1 dis:%s azh:%s", szDis, szAzh);

   //2 shanghai 31.1774276, 121.5272106	//1076679.0804040465
   c2.lat = 31.1774276;
   c2.lon = 121.5272106;
   dis = Track_Calc_Distance(c1.lat, c1.lon, c2.lat, c2.lon);
   th = Track_Calc_Azimuth(c1.lat, c1.lon, c2.lat, c2.lon);

   MEMSET(szDis,0,sizeof(szDis));
   FLOATTOWSTR(dis, bufDis, 32);
   WSTRTOSTR(bufDis,szDis, 64);

   MEMSET(szAzh,0,sizeof(szAzh));
   FLOATTOWSTR(th, bufAzh, 32);
   WSTRTOSTR(bufAzh, szAzh, 64);   
   DBGPRINTF("location_get 2 dis:%s azh:%s", szDis, szAzh);


   //3 sjz 38.0422378880,114.4925141047
   c2.lat = 38.0422378880;
   c2.lon = 114.4925141047;
   dis = Track_Calc_Distance(c1.lat, c1.lon, c2.lat, c2.lon);
   th = Track_Calc_Azimuth(c1.lat, c1.lon, c2.lat, c2.lon);

   MEMSET(szDis,0,sizeof(szDis));
   FLOATTOWSTR(dis, bufDis, 32);
   WSTRTOSTR(bufDis,szDis, 64);

   MEMSET(szAzh,0,sizeof(szAzh));
   FLOATTOWSTR(th, bufAzh, 32);
   WSTRTOSTR(bufAzh, szAzh, 64);
   DBGPRINTF("location_get 3 dis:%s azh:%s", szDis, szAzh);


   //4 zhangjiakou 40.3964667463,114.8377011418
   c2.lat = 40.3964667463;
   c2.lon = 114.8377011418;
   dis = Track_Calc_Distance(c1.lat, c1.lon, c2.lat, c2.lon);
   th = Track_Calc_Azimuth(c1.lat, c1.lon, c2.lat, c2.lon);

   MEMSET(szDis,0,sizeof(szDis));
   FLOATTOWSTR(dis, bufDis, 32);
   WSTRTOSTR(bufDis,szDis, 64);

   MEMSET(szAzh,0,sizeof(szAzh));
   FLOATTOWSTR(th, bufAzh, 32);
   WSTRTOSTR(bufAzh, szAzh, 64);
   DBGPRINTF("location_get 4 dis:%s azh:%s", szDis, szAzh);

}



