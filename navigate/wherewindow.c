#include "wherewindow.h"

#define MP_MAX_STRLEN         64
#define MP_WHERE_CY			  32

// Where window: Displays main menu.
struct CWhereWin
{
	INHERIT_CWindow(IWindow);
	
	AECHAR          m_szText[MP_MAX_STRLEN];

	AECHAR          m_szMode[MP_MAX_STRLEN];

	IStatic *		m_pTextMethod;	//定位方式
	IStatic *		m_pTextTime;	//定位时间
	IStatic *		m_pTextLon;		//纬度
	IStatic *		m_pTextLat;		//经度
	IStatic *		m_pTextVel;		//速度
	IStatic *		m_pTextHeading;	//方向角度
	IStatic *		m_pTextHeight;	//海拔
	//IStatic *		m_pTextInfo;	//信息

	IImageCtl *		m_pImageCtl;	//过渡图片

	AEEGPSMode		m_gpsMode;
	struct _GetGPSInfo		m_gpsInfo;	//
};

typedef struct CWhereWin CWhereWin;


static void       CWhereWin_Delete(IWindow * po);
static void       CWhereWin_Enable(IWindow * po, boolean bEnable);
static void       CWhereWin_Redraw(IWindow * po);
static boolean    CWhereWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);
static void		  CWhereWin_LocStart ( IWindow *po );
static void		  CWhereWin_LocStop ( IWindow *po );
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
 * @brief 获得当前时间
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
		int      cx = pme->m_pOwner->m_rectWin.dx;
		int		 y = pme->m_pOwner->m_rectWin.y;
		int      dy = MP_WHERE_CY;
		AEERect  rect;

		//定位结果提示信息
		if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextMethod) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextTime) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextLon) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextLat) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextVel) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextHeading) != SUCCESS ||
			ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextHeight) != SUCCESS ){
		//	ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextInfo) != SUCCESS) {
		   return FALSE;
		}
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextMethod, &rect);
		y += MP_WHERE_CY;
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextTime, &rect);
		y += MP_WHERE_CY;
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextLon, &rect);
		y += MP_WHERE_CY;
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextLat, &rect);
		y += MP_WHERE_CY;
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextVel, &rect);
		y += MP_WHERE_CY;
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextHeading, &rect);
		y += MP_WHERE_CY;
		SETAEERECT(&rect, 0, y, cx, dy);
		ISTATIC_SetRect(pme->m_pTextHeight, &rect);
		//y += MP_WHERE_CY*2;
		//SETAEERECT(&rect, 0, y, cx, dy);
		//ISTATIC_SetRect(pme->m_pTextInfo, &rect);

		//默认为网络测试模式
		STRTOWSTR("Mode: NETWORK", pme->m_szMode, sizeof(pme->m_szMode));
	}

	//初始化定位信息
	pme->m_gpsMode = AEEGPS_MODE_TRACK_NETWORK;	
	CWhereWin_LocStart((IWindow *)pme);


	ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MY_LOCATION,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_FUCTION,IDS_STRING_BACK,0);


   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the Where window.
===========================================================================*/
static void CWhereWin_Delete(IWindow * po)
{
	CWhereWin *  pme = (CWhereWin *)po;	

	//释放定位模块
	CWhereWin_LocStop(po);
	
	//定位结果相关控件
	TS_RELEASEIF(pme->m_pTextMethod);
	TS_RELEASEIF(pme->m_pTextTime);
	TS_RELEASEIF(pme->m_pTextLon);
	TS_RELEASEIF(pme->m_pTextLat);
	TS_RELEASEIF(pme->m_pTextVel);
	TS_RELEASEIF(pme->m_pTextHeading);
	TS_RELEASEIF(pme->m_pTextHeight);
	//TS_RELEASEIF(pme->m_pTextInfo);
	TS_RELEASEIF(pme->m_pImageCtl);
	
	FREE(pme);
}

/*===========================================================================
   This function enables/disables the Where window.
===========================================================================*/
static void CWhereWin_Enable(IWindow * po, boolean bEnable)
{
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

	TS_DrawBackgroud(po);
	
	//当取得定位结果时更新显示
	//if (pme->m_gpsInfo.pPosDet)
	{
		//if (pme->m_gpsInfo.theInfo.nErr == SUCCESS)
		{
			char szBuf[MP_MAX_STRLEN];
			AECHAR bufLat[32], bufLon[32], bufVel[32], bufHeading[32];
			ts_time_t now;

			//STRTOWSTR("Method: NetWork", pme->m_szText, sizeof(pme->m_szText));
			//TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextMethod, AEE_FONT_NORMAL, pme->m_szText);
			TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextMethod, AEE_FONT_NORMAL, pme->m_szMode);

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
	
	//WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Pro: %d nErr:%u", pme->m_gpsInfo.wProgress,  pme->m_gpsInfo.theInfo.nErr);
	//TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextInfo, AEE_FONT_NORMAL, pme->m_szText);	


	IDISPLAY_Update(pme->m_pIDisplay);

	{
		static i = 0;
		if ( 0 == i ){
			TS_DrawSplash(pme->m_pOwner,2000);
			i++;
		}
	}

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
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_WHERE_FUCTION, 0);
			bRet = TRUE;
			break;

		case AVK_SOFT2:
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_MAIN, 0);
			bRet = TRUE;
			break;

		case AVK_1:
			DBGPRINTF("Mode: NETWORK");
			pme->m_gpsMode = AEEGPS_MODE_TRACK_NETWORK;
			CWhereWin_LocStop((IWindow*)pme);
			CWhereWin_LocStart((IWindow*)pme);
			STRTOWSTR("NETWORK", pme->m_szMode, sizeof(pme->m_szMode));
			CWhereWin_Redraw((IWindow*)pme);
			bRet = TRUE;
			break;

		case AVK_2:
			DBGPRINTF("STANDALONE MODE");
			pme->m_gpsMode = AEEGPS_MODE_TRACK_STANDALONE;
			CWhereWin_LocStop((IWindow*)pme);
			CWhereWin_LocStart((IWindow*)pme);
			STRTOWSTR("Mode: STANDALONE", pme->m_szMode, sizeof(pme->m_szMode));
			CWhereWin_Redraw((IWindow*)pme);
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
static void CWhereWin_LocStart( IWindow *po )
{
	CWhereWin *pme = (CWhereWin*)po;
	int nErr = SUCCESS;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_gpsInfo;
	ZEROAT( pGetGPSInfo );

	pGetGPSInfo->theInfo.gpsConfig.server.svrType = AEEGPS_SERVER_DEFAULT;
	pGetGPSInfo->theInfo.gpsConfig.qos = 16;
	pGetGPSInfo->theInfo.gpsConfig.optim = 1;
	pGetGPSInfo->theInfo.gpsConfig.mode = pme->m_gpsMode;
	pGetGPSInfo->theInfo.gpsConfig.nFixes = 0;
	pGetGPSInfo->theInfo.gpsConfig.nInterval = 0;
	
	if( ISHELL_CreateInstance( pme->m_pIShell, AEECLSID_POSDET,(void **)&pGetGPSInfo->pPosDet ) == SUCCESS ) {
		
		CALLBACK_Init( &pGetGPSInfo->cbPosDet, CWhereWin_GetGPSInfo_Callback, pme );
		CALLBACK_Init( &pGetGPSInfo->cbProgressTimer, CWhereWin_GetGPSInfo_SecondTicker, pme );
		
		nErr = Loc_Init( pme->m_pIShell, pGetGPSInfo->pPosDet, &pGetGPSInfo->cbPosDet, &pGetGPSInfo->pts );
		nErr = Loc_Start( pGetGPSInfo->pts, &pGetGPSInfo->theInfo );
		if( nErr != SUCCESS ) {
			pGetGPSInfo->theInfo.nErr = nErr;
			DBGPRINTF("Loc_Start Failed! Err:%d", nErr);
			CWhereWin_Redraw((IWindow*)pme);
		}
		else {
			ISHELL_SetTimerEx( pme->m_pIShell, 1000, &pGetGPSInfo->cbProgressTimer );
		}
	}
}

static void CWhereWin_LocStop( IWindow *po )
{
	CWhereWin *pme = (CWhereWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_gpsInfo;

	if (pGetGPSInfo->pPosDet)
	{
		Loc_Stop(pGetGPSInfo->pts);
		
		CALLBACK_Cancel( &pGetGPSInfo->cbProgressTimer );
		CALLBACK_Cancel( &pGetGPSInfo->cbPosDet );
		TS_RELEASEIF( pGetGPSInfo->pPosDet );
	}
}

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
		
		CWhereWin_LocStop((IWindow*)pme);
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

