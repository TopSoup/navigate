#include "wherewindow.h"

#define MP_MAX_STRLEN         32
#define MP_WHERE_CY			  32

#define WIN_FONT	AEE_FONT_LARGE

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
	//IStatic *		m_pTextInfo;	//信息[debug]

	boolean			m_bGetGpsInfo;

	AEEGPSMode		m_gpsMode;
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


	pme->m_bGetGpsInfo = FALSE;
	
	ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MY_LOCATION,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	//TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_FUCTION,IDS_STRING_BACK,0);
	TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);

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

//格式化浮点数
static AECHAR* FLT2RAD(AECHAR* szBuf, double val)
{
	AEEApplet* pApp = (AEEApplet*)GETAPPINSTANCE();
	double tmp = 0, tt = 0, min = 0;
	int d = 0, m = 0, s = 0;
	AECHAR szD[3], szM[3];
	
	if (szBuf == NULL)
		return NULL;

	ISHELL_LoadResString(pApp->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEGREE, szD, sizeof(szD));
	ISHELL_LoadResString(pApp->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MINUTE, szM, sizeof(szM));

	tmp = FABS(val);
	tt = FFLOOR(tmp);
	d = FLTTOINT(tt);
	m = FLTTOINT(FMUL(FSUB(tmp, tt), 600000.0));
	m = (m % 10 >= 5) ? (m + 10) / 10 : m / 10;
	s = m % 1000;
	m /= 1000;
	
	WSPRINTF(szBuf, 32, L"%d%s %02d.%03d%s", d, szD, m, s, szM);
	return szBuf;
}

//格式化浮点数
static int FORMATFLT(AECHAR* szLon, AECHAR* szLat, double lon, double lat)
{
	AECHAR eFlag[3], nFlag[3];
	AECHAR szBuf[32];

	if (szBuf == NULL)
		return -1;

	//东经/西经
	if (FCMP_GE(lon, 0))
	{
		WSTRCPY(eFlag, L"E");
	}
	else
	{
		WSTRCPY(eFlag, L"W");
	}

	//南纬/北纬
	if (FCMP_GE(lat, 0))
	{
		WSTRCPY(nFlag, L"N");
	}
	else
	{
		WSTRCPY(nFlag, L"S");
	}

	WSPRINTF(szLon, 32, L"%s, %s", eFlag, FLT2RAD(szBuf, lon));
	WSPRINTF(szLat, 32, L"%s, %s", nFlag, FLT2RAD(szBuf, lat));

	return 0;
}


/*===========================================================================
   This function redraws the Where window.
===========================================================================*/
static void CWhereWin_Redraw(IWindow * po)
{
	CWhereWin *  pme = (CWhereWin *)po;
	struct _GetGPSInfo *pGetGpsInfo = &pme->m_pOwner->m_gpsInfo;
	if (!pme->m_bActive)
		return;
	
	DBGPRINTF("@CWhereWin_Redraw in");

	IDISPLAY_ClearScreen(pme->m_pIDisplay);

	//更新底部文字
	if (pme->m_bGetGpsInfo)
	{
		TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_FUCTION,IDS_STRING_BACK,0);
	}
	else
	{
		TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);
	}

	//绘制背景及框架
	TS_DrawBackgroud(po);


	//当取得定位结果时更新显示
	if (pGetGpsInfo->pPosDet 
		&& pme->m_bGetGpsInfo)
	{
		//if (pGetGpsInfo->theInfo.nErr == SUCCESS)
		{
			AECHAR wBuf[MP_MAX_STRLEN], wBuf2[MP_MAX_STRLEN];
			AECHAR bufRes[MP_MAX_STRLEN], bufRes2[MP_MAX_STRLEN], bufRes3[MP_MAX_STRLEN];
			AECHAR bufLat[MP_MAX_STRLEN], bufLon[MP_MAX_STRLEN], bufVel[MP_MAX_STRLEN], bufHeading[MP_MAX_STRLEN];
			ts_time_t now;
			int a = 0, b = 0;
			int h = 0, xx = 0, yy = 0, dxx = 0, dyy = 0;
			AEERect rect;
			int xMargin = 4;
			
			//定位类型
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_STATUS, bufRes, sizeof(bufRes));
			h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
			xx = xMargin;
			yy = 64;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  bufRes, &rect);
			
			//时间
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_TIME, bufRes, sizeof(bufRes));
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MONTH, bufRes2, sizeof(bufRes));
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DAY, bufRes3, sizeof(bufRes));
			
			get_time_now(&now);
			
			WSPRINTF(wBuf, sizeof(wBuf), L"%02d%s%02d%s ", now.month, bufRes2, now.day, bufRes3);
			WSPRINTF(wBuf2, sizeof(wBuf2), L"%02d:%02d:%02d", now.hour, now.minute, now.second);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s%s", bufRes, wBuf, wBuf2);
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);

			//格式化经纬度
			//For Test Hack
#ifdef AEE_SIMULATOR
			pGetGpsInfo->theInfo.lat = 38.0422378880;
			pGetGpsInfo->theInfo.lon = 114.4925141047;
#endif
			FORMATFLT(bufLon, bufLat, pGetGpsInfo->theInfo.lon, pGetGpsInfo->theInfo.lat);

			//经度
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LON, bufRes, sizeof(bufRes));
			//FLOATTOWSTR(pGetGpsInfo->theInfo.lon, bufLon, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufLon);
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);

			//纬度		
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LAT, bufRes, sizeof(bufRes));
			//FLOATTOWSTR(pGetGpsInfo->theInfo.lat, bufLat, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufLat);
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);

			
			//速度
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_VEL, bufRes, sizeof(bufRes));
			//FLOATTOWSTR(pGetGpsInfo->theInfo.velocityHor, bufVel, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, TS_FLT2SZ(bufVel, pGetGpsInfo->theInfo.velocityHor));
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);
		
			//方向
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_HEADING, bufRes, sizeof(bufRes));
			//FLOATTOWSTR(pGetGpsInfo->theInfo.heading, bufHeading, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, TS_FLT2SZ(bufHeading, pGetGpsInfo->theInfo.heading));
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);
			
			//海拔
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_ALT, bufRes, sizeof(bufRes));
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %d.0", bufRes, pGetGpsInfo->theInfo.height);
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);
		}
	}
	else
	{
		{
			AECHAR prompt[TS_MAX_STRLEN];
			
			ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATING,prompt,sizeof(prompt));
			TS_DrawSplash(pme->m_pOwner,prompt,2000,NULL,NULL);
		}
	}

	//WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"Pro: %d nErr:%u", pGetGpsInfo->wProgress,  pGetGpsInfo->theInfo.nErr);
	//TS_FitStaticText(pme->m_pIDisplay, pme->m_pTextInfo, AEE_FONT_NORMAL, pme->m_szText);	


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
		case AVK_SELECT:
			pme->m_bGetGpsInfo = TRUE;
			bRet = TRUE;
			CWhereWin_Redraw((IWindow*)pme);
			break;
		case AVK_SOFT1:
			if (pme->m_bGetGpsInfo)
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
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;
	ZEROAT( pGetGPSInfo );

	pGetGPSInfo->theInfo.gpsConfig.server.svrType = AEEGPS_SERVER_DEFAULT;
	pGetGPSInfo->theInfo.gpsConfig.qos = 16;
	pGetGPSInfo->theInfo.gpsConfig.optim = 1;
	pGetGPSInfo->theInfo.gpsConfig.mode = pme->m_gpsMode;
	pGetGPSInfo->theInfo.gpsConfig.nFixes = 0;
	pGetGPSInfo->theInfo.gpsConfig.nInterval = 10;
	
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
			ISHELL_SetTimerEx( pme->m_pIShell, 10000, &pGetGPSInfo->cbProgressTimer );
		}
	}
}

static void CWhereWin_LocStop( IWindow *po )
{
	CWhereWin *pme = (CWhereWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

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
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

	DBGPRINTF("CWhereWin_GetGPSInfo_Callback in nErr:%d", pGetGPSInfo->theInfo.nErr);

	if( pGetGPSInfo->theInfo.nErr == SUCCESS ) {
		/* Process new data from IPosDet */
		pGetGPSInfo->dwFixNumber++;
		pGetGPSInfo->dwFixDuration += pGetGPSInfo->wProgress;
		pGetGPSInfo->wProgress = 0;
		DBGPRINTF("@GetGPSInfo fix:%d", pGetGPSInfo->dwFixNumber);

		pme->m_bGetGpsInfo = TRUE;
		CWhereWin_Redraw(po);
	}
	else if( pGetGPSInfo->theInfo.nErr == EIDLE ) {
		/* End of tracking */
		DBGPRINTF("@End of tracking");
		pGetGPSInfo->dwFixNumber = 0;
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

		pGetGPSInfo->dwFixNumber = 0;
		
		CWhereWin_Redraw(po);
	}
}

/*===========================================================================
   This function called by location modoule.
===========================================================================*/
static void CWhereWin_GetGPSInfo_SecondTicker( IWindow *po )
{
	CWhereWin *pme = (CWhereWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

	if( pGetGPSInfo->bPaused == FALSE ) {
	  pGetGPSInfo->wProgress++;
	  DBGPRINTF("@Where GetGPS progress:%d", pGetGPSInfo->wProgress);
	  CWhereWin_Redraw(po);
	}

	if( pGetGPSInfo->bAbort == FALSE ) {
	  ISHELL_SetTimerEx( pme->m_pIShell, 10000, &pGetGPSInfo->cbProgressTimer );
	}
}

