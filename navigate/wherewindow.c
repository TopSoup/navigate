#include "wherewindow.h"

#define MP_MAX_STRLEN         64
#define MP_WHERE_CY			  32

// Where window: Displays main menu.
struct CWhereWin
{
	INHERIT_CWindow(IWindow);
	
	AECHAR          m_szText[MP_MAX_STRLEN];
	IStatic *		m_pTitle;

	AECHAR          m_szMode[MP_MAX_STRLEN];
	
	IStatic *		m_pTextMethod;	//��λ��ʽ
	IStatic *		m_pTextTime;	//��λʱ��
	IStatic *		m_pTextLon;		//γ��
	IStatic *		m_pTextLat;		//����
	IStatic *		m_pTextVel;		//�ٶ�
	IStatic *		m_pTextHeading;	//����Ƕ�
	IStatic *		m_pTextHeight;	//����
	IStatic *		m_pTextInfo;	//��Ϣ

	IImageCtl *		m_pImageCtl;	//����ͼƬ

	struct _GetGPSInfo		m_gpsInfo;	//
};

typedef struct CWhereWin CWhereWin;


static void       CWhereWin_Delete(IWindow * po);
static void       CWhereWin_Enable(IWindow * po, boolean bEnable);
static void       CWhereWin_Redraw(IWindow * po);
static boolean    CWhereWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);
static void		  CWhereWin_GetGPSInfo_SecondTicker( IWindow *po );
static void		  CWhereWin_GetGPSInfo_Callback( IWindow *po );

/**
 * @brief
 */
typedef struct ts_time_t {
	uint16			year;
	uint8			month;
	uint8			day;
	uint8			hour;
	uint8			minute;
	uint8			second;
	uint16			millisecond;
} ts_time_t;

/**
 * @brief ��õ�ǰʱ��
 */
int get_time_now(ts_time_t* tw) {
	uint32 secs;
	JulianType julian;
	
	if( NULL == tw )
		return -1;
	
	secs = GETTIMESECONDS();
	GETJULIANDATE(secs,&julian);
	
	tw->year = julian.wYear;
	tw->month = (uint8)julian.wMonth;
	tw->day = (uint8)julian.wDay;
	tw->hour = (uint8)julian.wHour;
	tw->minute = (uint8)julian.wMinute;
	tw->second = (uint8)julian.wSecond;
	tw->millisecond = 0;
	
	return 0;
}

/*===============================================================================

                     CWhereWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the Where window.
===========================================================================*/
IWindow * CWhereWin_New(CTopSoupApp * pOwner)
{
   CWhereWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CWhereWin_Enable, CWhereWin_Redraw, CWhereWin_HandleEvent, CWhereWin_Delete);
   pme = (CWhereWin *)CWindow_New(sizeof(CWhereWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

{
		int      cx = pme->m_pOwner->m_cxWidth;
		int      cy = pme->m_pOwner->m_cyHeight;
		int		 dy = MP_WHERE_CY;
		AEERect  rectTitle, rect;
		
		// Create IStatic for file name
		SETAEERECT(&rectTitle, 0, 0, cx, 16);
		
		if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTitle))
			TS_WINERR_RETURN(pme);
		
		ISTATIC_SetRect(pme->m_pTitle, &rectTitle);
		ISTATIC_SetProperties(pme->m_pTitle, ST_CENTERTEXT | ST_NOSCROLL);

		//��λ������ʾ��Ϣ��ʾ

		//��λ�����ʾ��Ϣ
		if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextMethod) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextTime) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextLon) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextLat) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextVel) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextHeading) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextHeight) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextInfo) != SUCCESS) {
		   return FALSE;
		}
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextMethod, &rect);
		dy += MP_WHERE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextTime, &rect);
		dy += MP_WHERE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextLon, &rect);
		dy += MP_WHERE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextLat, &rect);
		dy += MP_WHERE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextVel, &rect);
		dy += MP_WHERE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextHeading, &rect);
		dy += MP_WHERE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextHeight, &rect);
		dy += MP_WHERE_CY*2;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextInfo, &rect);

		//Ĭ��Ϊ�������ģʽ
		STRTOWSTR("NETWORK", pme->m_szMode, sizeof(pme->m_szMode));
	}

	//��ʼ����λ��Ϣ
	{
		int nErr = SUCCESS;
		struct _GetGPSInfo *pGetGPSInfo = &pme->m_gpsInfo;
		ZEROAT( pGetGPSInfo );
		
		pGetGPSInfo->theInfo.server.svrType = AEEGPS_SERVER_DEFAULT;//pMe->gpsSettings.server;
		pGetGPSInfo->theInfo.qos = 16;//pMe->gpsSettings.qos;
		pGetGPSInfo->theInfo.optim = 1;//pMe->gpsSettings.optim;
		
		if( ISHELL_CreateInstance( pme->m_pIShell, AEECLSID_POSDET,(void **)&pGetGPSInfo->pPosDet ) == SUCCESS ) {
			
			CALLBACK_Init( &pGetGPSInfo->cbPosDet, CWhereWin_GetGPSInfo_Callback, pme );
			CALLBACK_Init( &pGetGPSInfo->cbProgressTimer, CWhereWin_GetGPSInfo_SecondTicker, pme );
			
			nErr = Track_Init( pme->m_pIShell, pGetGPSInfo->pPosDet, &pGetGPSInfo->cbPosDet, &pGetGPSInfo->pts );
			nErr = Track_Start( pGetGPSInfo->pts, TRACK_NETWORK, 0, 0, &pGetGPSInfo->theInfo );
			if( nErr != SUCCESS ) {
				pGetGPSInfo->theInfo.nErr = nErr;
				DBGPRINTF("Track_Start Failed! Err:%d", nErr);
			}
			else {
				ISHELL_SetTimerEx( pme->m_pIShell, 1000, &pGetGPSInfo->cbProgressTimer );
			}
		}
	}


   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the Where window.
===========================================================================*/
static void CWhereWin_Delete(IWindow * po)
{
CWhereWin *  pme = (CWhereWin *)po;
	
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_gpsInfo;
	
	//�ͷŶ�λģ��
	if (pGetGPSInfo->pPosDet)
	{
		Track_Stop(pGetGPSInfo->pts);
		
		CALLBACK_Cancel( &pGetGPSInfo->cbProgressTimer );
		CALLBACK_Cancel( &pGetGPSInfo->cbPosDet );
		TS_RELEASEIF( pGetGPSInfo->pPosDet );
	}
	
	//��λ�����ؿؼ�
	TS_RELEASEIF(pme->m_pTextMethod);
	TS_RELEASEIF(pme->m_pTextTime);
	TS_RELEASEIF(pme->m_pTextLon);
	TS_RELEASEIF(pme->m_pTextLat);
	TS_RELEASEIF(pme->m_pTextVel);
	TS_RELEASEIF(pme->m_pTextHeading);
	TS_RELEASEIF(pme->m_pTextHeight);
	TS_RELEASEIF(pme->m_pTextInfo);
	TS_RELEASEIF(pme->m_pImageCtl);
	TS_RELEASEIF(pme->m_pTitle);
	
	FREE(pme);
}

/*===========================================================================
   This function enables/disables the Where window.
===========================================================================*/
static void CWhereWin_Enable(IWindow * po, boolean bEnable)
{
	CWhereWin *  pme = (CWhereWin *)po;
	
	if (!CWindow_ProcessEnable(po, bEnable))
		return;
}

/*===========================================================================
   This function redraws the Where window.
===========================================================================*/
static void CWhereWin_Redraw(IWindow * po)
{
	CWhereWin *  pme = (CWhereWin *)po;
	
	if (!pme->m_bActive)
		return;
	
	DBGPRINTF("@CWhereWin_Redraw in");

	IDISPLAY_ClearScreen(pme->m_pIDisplay);
	
	if (pme->m_pTitle)
	{
		// File name (title) text
		STRTOWSTR("Where", pme->m_szText, sizeof(pme->m_szText));
		TS_FitStaticText(pme->m_pIDisplay, pme->m_pTitle, AEE_FONT_NORMAL, pme->m_szText);
	}

	//��ȡ�ö�λ���ʱ������ʾ
	//if (pme->m_gpsInfo.pPosDet)
	{
		//if (pme->m_gpsInfo.theInfo.nErr == SUCCESS)
		{
			char szBuf[MP_MAX_STRLEN];
			AECHAR bufLat[32], bufLon[32], bufVel[32], bufHeading[32];
			ts_time_t now;

			STRTOWSTR("Method: NetWork", pme->m_szText, sizeof(pme->m_szText));
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextMethod, AEE_FONT_NORMAL, pme->m_szText);

			get_time_now(&now);
			
			SNPRINTF(szBuf, sizeof(szBuf), "Time: %d-%d %d:%d:%d", now.month, now.day, now.hour, now.minute, now.second);
			STRTOWSTR(szBuf, pme->m_szText, sizeof(pme->m_szText));
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextTime, AEE_FONT_NORMAL, pme->m_szText);

			FLOATTOWSTR(pme->m_gpsInfo.theInfo.lon, bufLon, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Lon: %s", bufLon);
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextLon, AEE_FONT_NORMAL, pme->m_szText);

			FLOATTOWSTR(pme->m_gpsInfo.theInfo.lat, bufLat, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Lat: %s", bufLat);
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextLat, AEE_FONT_NORMAL, pme->m_szText);

			FLOATTOWSTR(pme->m_gpsInfo.theInfo.velocityHor, bufVel, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Vel: %s", bufVel);
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextVel, AEE_FONT_NORMAL, pme->m_szText);

			FLOATTOWSTR(pme->m_gpsInfo.theInfo.heading, bufHeading, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Heading: %s", bufHeading);
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextHeading, AEE_FONT_NORMAL, pme->m_szText);
			
			SNPRINTF(szBuf, sizeof(szBuf), "Height: %d", pme->m_gpsInfo.theInfo.height);
			STRTOWSTR(szBuf, pme->m_szText, sizeof(pme->m_szText));
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextHeight, AEE_FONT_NORMAL, pme->m_szText);
		}
	}
	
	WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Pro: %d nErr:%u", pme->m_gpsInfo.wProgress,  pme->m_gpsInfo.theInfo.nErr);
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextInfo, AEE_FONT_NORMAL, pme->m_szText);	


	IDISPLAY_Update(pme->m_pIDisplay);

	DBGPRINTF("@CWhereWin_Redraw in");
}

/*===========================================================================
   This function processes events routed to Where window.
===========================================================================*/
static boolean CWhereWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
	CWhereWin *  pme = (CWhereWin *)po;
	boolean     bRet = TRUE;

	switch (eCode)
	{
	case EVT_KEY:
		switch (wParam)
		{
		case AVK_SOFT1:
			bRet = FALSE;
			break;

		case AVK_SOFT2:
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_MAIN, 0);
			bRet = TRUE;
			break;

		case AVK_1:
			DBGPRINTF("NETWORK MODE");
			STRTOWSTR("NETWORK", pme->m_szMode, sizeof(pme->m_szMode));
			bRet = TRUE;
			break;

		case AVK_2:
			DBGPRINTF("STANDALONE MODE");
			STRTOWSTR("STANDALONE", pme->m_szMode, sizeof(pme->m_szMode));
			bRet = TRUE;
			break;


		default:
			bRet = FALSE;
			break;
		}
	
		break;
	}

	return bRet;
}

/*===========================================================================
   This function called by location modoule.
===========================================================================*/
static void CWhereWin_GetGPSInfo_Callback( IWindow *po )
{
	CWhereWin *pme = (CWhereWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_gpsInfo;

	DBGPRINTF("CWhereWin_GetGPSInfo_Callback in nErr:%d", pGetGPSInfo->theInfo.nErr);

	if( pGetGPSInfo->theInfo.nErr == SUCCESS ) {
		/* Process new data from IPosDet */
		pGetGPSInfo->dwFixNumber++;
		pGetGPSInfo->dwFixDuration += pGetGPSInfo->wProgress;
		pGetGPSInfo->wProgress = 0;
		DBGPRINTF("@GetGPSInfo fix:%d", pGetGPSInfo->dwFixNumber);

		CWhereWin_Redraw(po);
	}
	else if( pGetGPSInfo->theInfo.nErr == EIDLE ) {
		/* End of tracking */
		DBGPRINTF("@End of tracking");
	}
	else if( pGetGPSInfo->theInfo.nErr == AEEGPS_ERR_TIMEOUT ) {
		/* Record the timeout and perhaps re-try. */
		pGetGPSInfo->dwTimeout++;
	}
	else {
		//�ͷŶ�λģ��
		if (pGetGPSInfo->pPosDet)
		{
			Track_Stop(pGetGPSInfo->pts);
			
			CALLBACK_Cancel( &pGetGPSInfo->cbProgressTimer );
			CALLBACK_Cancel( &pGetGPSInfo->cbPosDet );
			TS_RELEASEIF( pGetGPSInfo->pPosDet );
		}

		DBGPRINTF("@Something is not right here. Requires corrective action. Bailout");
		
		/* Something is not right here. Requires corrective action. Bailout */
		pGetGPSInfo->bAbort = TRUE;

		CWhereWin_Redraw(po);
	}
}

/*===========================================================================
   This function called by location modoule.
===========================================================================*/
static void CWhereWin_GetGPSInfo_SecondTicker( IWindow *po )
{
	CWhereWin *pme = (CWhereWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_gpsInfo;

	if( pGetGPSInfo->bPaused == FALSE ) {
	  pGetGPSInfo->wProgress++;
	  DBGPRINTF("@Where GetGPS progress:%d", pGetGPSInfo->wProgress);
	  CWhereWin_Redraw(po);
	}

	if( pGetGPSInfo->bAbort == FALSE ) {
	  ISHELL_SetTimerEx( pme->m_pIShell, 1000, &pGetGPSInfo->cbProgressTimer );
	}
}
