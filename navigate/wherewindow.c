#include "wherewindow.h"

#define MP_MAX_STRLEN         32
#define MP_WHERE_CY			  32

#define WIN_FONT	AEE_FONT_LARGE

/************************************************************************/
/* TIMER CONTROL                                                        */
/************************************************************************/
#ifdef AEE_SIMULATOR
#define WATCHER_TIMER	15
#else
#define WATCHER_TIMER	60
#endif

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

	AEECallback		m_cbWatcherTimer;
	AEEGPSMode		m_gpsMode;			//GPS模式
	ts_time_t		m_getGpsTime;
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

   //初始化定位信息
    //if(FALSE == pme->m_pOwner->m_bGetGpsInfo)
	{
		int nErr = SUCCESS;
		struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;
		ZEROAT(pGetGPSInfo);

		pme->m_gpsMode = AEEGPS_MODE_TRACK_STANDALONE;//AEEGPS_MODE_TRACK_NETWORK;

		//启动定位
		CWhereWin_LocStart((IWindow*)pme);

		//Callback
		CALLBACK_Init(&pme->m_cbWatcherTimer, CWhereWin_GetGPSInfo_SecondTicker, pme);
		ISHELL_SetTimerEx(pme->m_pIShell, 1000, &pme->m_cbWatcherTimer);

		pme->m_pOwner->m_bGetGpsInfo = FALSE;

		TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);
	} 
	// else {
	// 	TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_FUCTION,IDS_STRING_BACK,0);
	// }
	
	ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MY_LOCATION,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	
	pme->m_pOwner->m_wMenuLastSel[TSW_WHERE_FUCTION] = 0;

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
	
	CALLBACK_Cancel(&pme->m_cbWatcherTimer);

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
int FORMATFLT(AECHAR* szLon, AECHAR* szLat, double lon, double lat)
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
	if (pme->m_pOwner->m_bGetGpsInfo)
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
		&& pme->m_pOwner->m_bGetGpsInfo)
	{
		//if (pGetGpsInfo->theInfo.nErr == SUCCESS)
		{
			AECHAR wBuf[MP_MAX_STRLEN], wBuf2[MP_MAX_STRLEN];
			AECHAR bufRes[MP_MAX_STRLEN], bufRes2[MP_MAX_STRLEN], bufRes3[MP_MAX_STRLEN];
			AECHAR bufLat[MP_MAX_STRLEN], bufLon[MP_MAX_STRLEN], bufVel[MP_MAX_STRLEN], bufHeading[MP_MAX_STRLEN];
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
			
			WSPRINTF(wBuf, sizeof(wBuf), L"%02d%s%02d%s ", pme->m_getGpsTime.month, bufRes2, pme->m_getGpsTime.day, bufRes3);
			WSPRINTF(wBuf2, sizeof(wBuf2), L"%02d:%02d:%02d", pme->m_getGpsTime.hour, pme->m_getGpsTime.minute, pme->m_getGpsTime.second);
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

			
			//速度[使用节和公里/小时表示]
			{
			AECHAR szKn[32];
			AECHAR knRes[10];
			AECHAR kmRes[10];
			double kn = 0, km = 0;

			kn = FMUL(FDIV(pGetGpsInfo->theInfo.velocityHor, 1852.0), 3600.0);	//1节=1.852公里/小时 velocityHor为m/s --> 1节 = V*3600/1852
            km = FMUL(pGetGpsInfo->theInfo.velocityHor, 3.6);  //m/s --> km/h
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_VEL, bufRes, sizeof(bufRes));
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SPEED_KN, knRes, sizeof(knRes));
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SPEED_KM, kmRes, sizeof(kmRes));
			//FLOATTOWSTR(pGetGpsInfo->theInfo.velocityHor, bufVel, 32);
			//FLOATTOWSTR(kn, szKn, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s%s(%s%s)", 
				bufRes, TS_FLT2SZ_3(szKn, kn), knRes, TS_FLT2SZ_3(bufVel, km), kmRes);
			//WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s%s(%s%s)", 
			//	bufRes, szKn, knRes, bufVel, kmRes);
			xx = xMargin;
			yy += h;
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);
			}

			//方向
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_HEADING, bufRes, sizeof(bufRes));
			//FLOATTOWSTR(pGetGpsInfo->theInfo.heading, bufHeading, 32);
			WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, TS_FLT2SZ_3(bufHeading, pGetGpsInfo->theInfo.heading));
			//WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufHeading);
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
			IImage * pi = ISHELL_LoadResImage(pme->m_pIShell, NAVIGATE_RES_FILE, IDP_OBJECT_PROMPT);
			IStatic * pInfoStatic = NULL;
			
			if (pi)
			{
				AEERect  rect;
				AEEImageInfo      info;
				int16 x,y;
				AECHAR prompt[TS_MAX_STRLEN];

				IIMAGE_GetInfo(pi,&info);
				x = ( pme->m_pOwner->m_rectWin.dx - info.cx ) / 2;
				y =  pme->m_pOwner->m_rectWin.y + (  pme->m_pOwner->m_rectWin.dy - info.cy ) / 2;
				SETAEERECT(&rect,x,y,info.cx,info.cy);
				TS_DrawImage(pi, &rect, TRUE);
				
					
				ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATING,prompt,sizeof(prompt));
				
				ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pInfoStatic);
				ISTATIC_SetRect(pInfoStatic, &rect);
				ISTATIC_SetProperties(pInfoStatic,  ST_MIDDLETEXT | ST_CENTERTEXT | ST_NOSCROLL);
				TS_FitStaticText(pme->m_pIDisplay, pInfoStatic, AEE_FONT_LARGE, prompt);
				
				TS_RELEASEIF(pi);
				TS_RELEASEIF(pInfoStatic);
			}
		}
		{
		//  AECHAR prompt[TS_MAX_STRLEN];	
		//	ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATING,prompt,sizeof(prompt));
		//	TS_DrawSplash(pme->m_pOwner,prompt,2000,NULL);
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
		case AVK_SOFT1:
			if (pme->m_pOwner->m_bGetGpsInfo)
			{			  
			 
				//记录经纬度
			    TS_FLT2SZ(pme->m_pOwner->m_szTextLat, pme->m_pOwner->m_gpsInfo.theInfo.lat);
			    TS_FLT2SZ(pme->m_pOwner->m_szTextLon, pme->m_pOwner->m_gpsInfo.theInfo.lon);
				//FLOATTOWSTR(pme->m_pOwner->m_gpsInfo.theInfo.lat, pme->m_pOwner->m_szTextLat, 32);
				//FLOATTOWSTR(pme->m_pOwner->m_gpsInfo.theInfo.lon, pme->m_pOwner->m_szTextLon, 32);

			  //如果位置名称为空, 则使用默认日志编号格式名称
			  if (WSTRLEN(pme->m_pOwner->m_szTextDesc) == 0)
			  {
				  ts_time_t tw;
				  AECHAR szTmp[32];
				  
				  TS_GetTimeNow(&tw);
				  
				  WSPRINTF(szTmp, sizeof(szTmp), 
					  L"%04d%02d%02d%02d", tw.year, tw.month, tw.day, tw.hour);
				  
				  WSPRINTF(pme->m_pOwner->m_szTextDesc, sizeof(pme->m_pOwner->m_szTextDesc), 
					  L"%s%02d%02d", szTmp, tw.minute, tw.second);
			  }

				CTopSoupApp_SetWindow(pme->m_pOwner, TSW_WHERE_FUCTION, 0);
			}

			bRet = TRUE;
			break;

		case AVK_SOFT2:
			{
				CTopSoupApp_SetWindow(pme->m_pOwner, TSW_MAIN, 0);
				bRet = TRUE;
				break;

			}


		case AVK_1://FOR TEST
			/*DBGPRINTF("Mode: NETWORK");
			pme->m_gpsMode = AEEGPS_MODE_TRACK_NETWORK;
			CWhereWin_LocStop((IWindow*)pme);
			CWhereWin_LocStart((IWindow*)pme);
			STRTOWSTR("NETWORK", pme->m_szMode, sizeof(pme->m_szMode));
			CWhereWin_Redraw((IWindow*)pme);
			bRet = TRUE;*/
			break;

		case AVK_2://FOR TEST
		/*	DBGPRINTF("STANDALONE MODE");
			pme->m_gpsMode = AEEGPS_MODE_TRACK_STANDALONE;
			CWhereWin_LocStop((IWindow*)pme);
			CWhereWin_LocStart((IWindow*)pme);
			STRTOWSTR("Mode: STANDALONE", pme->m_szMode, sizeof(pme->m_szMode));
			CWhereWin_Redraw((IWindow*)pme);
			bRet = TRUE;*/
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
	pGetGPSInfo->theInfo.gpsConfig.nInterval = 5;
	
	if( ISHELL_CreateInstance( pme->m_pIShell, AEECLSID_POSDET,(void **)&pGetGPSInfo->pPosDet ) == SUCCESS ) {
		
		CALLBACK_Init( &pGetGPSInfo->cbPosDet, CWhereWin_GetGPSInfo_Callback, pme );
		
		nErr = Loc_Init( pme->m_pIShell, pGetGPSInfo->pPosDet, &pGetGPSInfo->cbPosDet, &pGetGPSInfo->pts );
		nErr = Loc_Start( pGetGPSInfo->pts, &pGetGPSInfo->theInfo );
		if( nErr != SUCCESS ) {
			pGetGPSInfo->theInfo.nErr = nErr;
			DBGPRINTF("Loc_Start Failed! Err:%d", nErr);
			CWhereWin_Redraw((IWindow*)pme);
		}
		else {
			pGetGPSInfo->bAbort = FALSE;
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

		//经纬度有效时才算定位成功
	    if (FCMP_G(pGetGPSInfo->theInfo.lat,0) && FCMP_G(pGetGPSInfo->theInfo.lon,0))
		{
			pme->m_pOwner->m_bGetGpsInfo = TRUE;
			pGetGPSInfo->wIdleCount = 0;
			TS_GetTimeNow(&pme->m_getGpsTime);
		}
		//else
		//{
		//	pme->m_pOwner->m_bGetGpsInfo = FALSE;
		//}

		if (pme->m_pOwner->m_bGetGpsInfo) {
			//记录当前定位信息, 备用
			{
				ts_time_t now;
				char szTmp[128];
				char szGpsTime[32];
				char szLat[16];
				char szLon[16];
				char szHeading[16];
				char szVel[16];
				AECHAR szwLat[16], szwLon[16];
				AECHAR szwKn[16], szwKm[16];
				double kn = 0, km = 0;
				int heading = 0;

				TS_GetTimeNow(&now);
				SNPRINTF(szTmp, sizeof(szTmp), "%d-%02d-%02d", now.year, now.month, now.day);
				SNPRINTF(szGpsTime, sizeof(szGpsTime), "%s,%02d:%02d:%02d", szTmp, now.hour, now.minute, now.second);

				TS_FLT2SZ_7(szwLat, pGetGPSInfo->theInfo.lat);
				TS_FLT2SZ_7(szwLon, pGetGPSInfo->theInfo.lon);
				WSTRTOSTR(szwLat, szLat, sizeof(szLat));
				WSTRTOSTR(szwLon, szLon, sizeof(szLon));

				//For Test
				//pGetGPSInfo->velocityHor = 12.250;

				kn = FMUL(FDIV(pGetGPSInfo->theInfo.velocityHor, 1852.0), 3600.0);	//1节=1.852公里/小时 velocityHor为m/s --> 1节 = V*3600/1852
				km = FMUL(pGetGPSInfo->theInfo.velocityHor, 3.6);  //m/s --> km/h
				
				TS_FLT2SZ_1(szwKm, km);
				TS_FLT2SZ_1(szwKn, kn);
				
				//WSTRTOSTR(szwKm, szVel, sizeof(szVel));
				WSTRTOSTR(szwKn, szVel, sizeof(szVel));

				heading = FLTTOINT(pGetGPSInfo->theInfo.heading);
				SPRINTF(szHeading, "%d", heading);
				
				confmgr_puts(pme->m_pOwner->iConf, "gps", "lat", szLat);
				confmgr_puts(pme->m_pOwner->iConf, "gps", "lon", szLon);
				confmgr_puts(pme->m_pOwner->iConf, "gps", "vel", szVel);
				confmgr_puts(pme->m_pOwner->iConf, "gps", "heading", szHeading);
				confmgr_puts(pme->m_pOwner->iConf, "gps", "time", szGpsTime);
			}
		}
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

	if (pGetGPSInfo->bPaused == FALSE) {
		pGetGPSInfo->wProgress++;
		DBGPRINTF("@Where GetGPS progress:%d", pGetGPSInfo->wProgress);
		CWhereWin_Redraw(po);
	}

	if (pGetGPSInfo->bAbort == FALSE) {
		pGetGPSInfo->wIdleCount++;
		DBGPRINTF("@Where GetGPS wIdleCount:%d", pGetGPSInfo->wIdleCount);
	}

	//重新启动
	//1 空闲30秒
	//2 尝试2分钟未定位成功
	if (pGetGPSInfo->wIdleCount > WATCHER_TIMER || pGetGPSInfo->wProgress > 60 * 2)
	{
		//play_tts(pme, L"restart location");

		DBGPRINTF("@Where GetGPS CWhereWin_LocStart");
		CWhereWin_LocStop((IWindow*)pme);
		CWhereWin_LocStart((IWindow*)pme);
	}

	ISHELL_SetTimerEx(pme->m_pIShell, 1000, &pme->m_cbWatcherTimer);
}

