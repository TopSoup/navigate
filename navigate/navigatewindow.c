#include "navigatewindow.h"

#define MP_MAX_STRLEN         64
#define MP_NAVIGATE_CY			  32

#define WIN_FONT AEE_FONT_LARGE

/************************************************************************/
/* TIMER CONTROL                                                        */
/************************************************************************/
#ifdef AEE_SIMULATOR
#define WATCHER_TIMER	15
#else
#define WATCHER_TIMER	60
#endif

// Navigate window: Displays main menu.
struct CNavigateWin
{
	INHERIT_CWindow(IWindow);
	
	AECHAR          m_szText[MP_MAX_STRLEN];
	IStatic *		m_pTitle;
	
	IStatic *		m_pTextDestination;	//目标名称
	IStatic *		m_pTextDistance;	//距离
	IStatic *		m_pTextInfo;		//信息

	IGraphics*      m_pGraphics;              // Pointer to the application IGraphics object
	IBitmap*        m_pBitmap;                // Pointer to the IBitmap drawing canvas
	AEERect         m_rViewportRect;          // The rectangle of the drawing viewport
	int16		    m_nWindowXOrigin;         // The starting X coordinate of the canvas displayed in left most pixel of the viewport
	uint16          m_nBitmapWidth;           // The width of the drawing canvas

	IImageCtl *		m_pImageCtl;		//罗盘图片

	AEECallback		m_cbWatcherTimer;
	AEEGPSMode		m_gpsMode;			//GPS模式
	boolean			m_bGetGpsInfo;
};

typedef struct CNavigateWin CNavigateWin;

static void       CNavigateWin_Delete(IWindow * po);
static void       CNavigateWin_Enable(IWindow * po, boolean bEnable);
static void       CNavigateWin_Redraw(IWindow * po);
static boolean    CNavigateWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void		  CNavigateWin_LocStart(CNavigateWin *pme);
static void		  CNavigateWin_LocStop(CNavigateWin *pme);
static void		  CNavigateWin_GetGPSInfo_SecondTicker( IWindow *po );
static void		  CNavigateWin_GetGPSInfo_Callback( IWindow *po );
static void		  TS_PRINTD(const char* p, double param);

/*===============================================================================

                     CNavigateWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the Navigate window.
===========================================================================*/
IWindow * CNavigateWin_New(CTopSoupApp * pOwner)
{
   CNavigateWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CNavigateWin_Enable, CNavigateWin_Redraw, CNavigateWin_HandleEvent, CNavigateWin_Delete);
   pme = (CNavigateWin *)CWindow_New(sizeof(CNavigateWin), pOwner, &vtbl);
   if (!pme)
      return NULL;
   
	{
		int      cx = pme->m_pOwner->m_cxWidth;
		int      cy = pme->m_pOwner->m_cyHeight;
		int		 dy = 0;
		AEERect  rectTitle, rect;
		IImage	 *pi = NULL;
		IBitmap* pDeviceBitmap;

		// Create IStatic for file name
		SETAEERECT(&rectTitle, 0, 0, cx, 16);
		
		if( ( ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTitle) != SUCCESS ) ||
			( ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_GRAPHICS, (void**)&pme->m_pGraphics) != SUCCESS) ||
			( ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextDestination) != SUCCESS) ||
			(ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextDistance) != SUCCESS) ||
			( ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTextInfo) != SUCCESS)
		    )
			TS_WINERR_RETURN(pme);
		
		//标题+16
		ISTATIC_SetRect(pme->m_pTitle, &rectTitle);
		ISTATIC_SetProperties(pme->m_pTitle, ST_CENTERTEXT | ST_NOSCROLL);

		SETAEERECT( &pme->m_rViewportRect, 0, 0, cx, cy);
		// Create the bitmap used to double buffer the drawing surface.  To do this,
		// we get the device bitmap from the IDiplay object, and then create a new
		// one using the display bitmap as a template.
		// Set the size of the Bitmap to double buffer the graphics drawn to the screen.
		// For now we will make the Bitmap dimensions the same height as the viewport
		// and twice the width.

		if( IDISPLAY_GetDeviceBitmap( pme->m_pIDisplay, &pDeviceBitmap ) != SUCCESS )
		return FALSE;

		pme->m_nBitmapWidth = pme->m_rViewportRect.dx * 2;

		if( IBITMAP_CreateCompatibleBitmap( pDeviceBitmap, &pme->m_pBitmap, pme->m_nBitmapWidth, pme->m_rViewportRect.dy ) != SUCCESS )
		{
			IBITMAP_Release( pDeviceBitmap );
			return FALSE;
		}

		IBITMAP_Release( pDeviceBitmap );
		SETAEERECT( &rect, 0, 0, pme->m_nBitmapWidth, pme->m_rViewportRect.dy );
		IBITMAP_FillRect( pme->m_pBitmap, &rect, IBITMAP_RGBToNative( pme->m_pBitmap, MAKE_RGB(0x43,0xCD,0x80) ), AEE_RO_COPY );

		//定位罗盘+232
		if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_IMAGECTL, (void **)(&pme->m_pImageCtl)))
			TS_WINERR_RETURN(pme);

		SETAEERECT(&rect, 0, 0, 240, 232);
		IIMAGECTL_SetRect(pme->m_pImageCtl, &rect);
		//IIMAGECTL_SetProperties(pme->m_pImageCtl, CP_BORDER);

		pi = ISHELL_LoadResImage(pme->m_pIShell, NAVIGATE_RES_FILE, IDP_NAVIGATOR);
		if (pi)
		{
			IIMAGECTL_SetImage(pme->m_pImageCtl, pi);
			IIMAGE_Release(pi);
		}

		//目的地,距离和提示信息
		dy = 232+16+4;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextDestination, &rect);
		dy += MP_NAVIGATE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextDistance, &rect);
		dy += MP_NAVIGATE_CY;
		SETAEERECT(&rect, 0, dy, cx, dy);
		ISTATIC_SetRect(pme->m_pTextInfo, &rect);
	}

	//初始化定位信息
	{
		int nErr = SUCCESS;
		struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;
		ZEROAT(pGetGPSInfo);

		pme->m_gpsMode = AEEGPS_MODE_TRACK_STANDALONE;//AEEGPS_MODE_TRACK_NETWORK;

		//启动定位
		CNavigateWin_LocStart(pme);

		//Callback
		CALLBACK_Init(&pme->m_cbWatcherTimer, CNavigateWin_GetGPSInfo_SecondTicker, pme);
		ISHELL_SetTimerEx(pme->m_pIShell, 1000, &pme->m_cbWatcherTimer);

		pme->m_bGetGpsInfo = FALSE;
	}


	ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_NAVIGATE,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	//TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_FUCTION,IDS_STRING_BACK,0);
	TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);


   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the Navigate window.
===========================================================================*/
static void CNavigateWin_Delete(IWindow * po)
{
	CNavigateWin *  pme = (CNavigateWin *)po;
	
	//释放定位模块
	CNavigateWin_LocStop(pme);
	
	CALLBACK_Cancel(&pme->m_cbWatcherTimer);

	TS_RELEASEIF(pme->m_pImageCtl);
	TS_RELEASEIF(pme->m_pBitmap);
	TS_RELEASEIF(pme->m_pGraphics);

	TS_RELEASEIF(pme->m_pTextDestination);
	TS_RELEASEIF(pme->m_pTextDistance);
	TS_RELEASEIF(pme->m_pTextInfo);
	TS_RELEASEIF(pme->m_pTitle);

	
	FREE(pme);
}

/*===========================================================================
   This function enables/disables the Navigate window.
===========================================================================*/
static void CNavigateWin_Enable(IWindow * po, boolean bEnable)
{
	CNavigateWin *  pme = (CNavigateWin *)po;
	
	if (!CWindow_ProcessEnable(po, bEnable))
		return;

	IIMAGECTL_SetActive(pme->m_pImageCtl, bEnable);
}

/*===========================================================================
   This function redraws the Navigate window.
===========================================================================*/
static void CNavigateWin_Redraw(IWindow * po)
{
	CNavigateWin *  pme = (CNavigateWin *)po;
	AEECircle rCircle;
	AEELine rLine;
	AEETriangle rTriangle;
	double headingArrow = FDIV(M_PI, 8);
	double heading = 0, destHeading = 0, distance = 0;
	double r = 100;
	
	if (!pme->m_bActive)
		return;
	
	IDISPLAY_BitBlt( pme->m_pIDisplay, pme->m_rViewportRect.x, pme->m_rViewportRect.y, pme->m_rViewportRect.dx, pme->m_rViewportRect.dy, pme->m_pBitmap, pme->m_nWindowXOrigin, 0, AEE_RO_COPY );
	
		//更新底部文字
	if (pme->m_bGetGpsInfo)
	{
		//TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_INFO,IDS_STRING_BACK,0);	//TODO
		TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);
	}
	else
	{
		TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);
	}

		//绘制背景及框架
	TS_DrawBackgroud(po);

//	if (pme->m_pTitle)
//	{
//		// File name (title) text
//		STRTOWSTR("Navigate", pme->m_szText, sizeof(pme->m_szText));
//		TS_FitStaticText(pme->m_pIDisplay, pme->m_pTitle, AEE_FONT_NORMAL, pme->m_szText);
//	}

	//当取得定位结果时更新显示
	if (pme->m_pOwner->m_gpsInfo.pPosDet
		&& pme->m_bGetGpsInfo)
	{
		if (pme->m_pOwner->m_gpsInfo.theInfo.nErr == SUCCESS)
		{
			DBGPRINTF("@GetGpsInfo!");

			distance = pme->m_pOwner->m_gpsInfo.theInfo.distance;
			heading = FMUL(pme->m_pOwner->m_gpsInfo.theInfo.heading, M_D2R);
			destHeading = FMUL(pme->m_pOwner->m_gpsInfo.theInfo.destHeading, M_D2R);
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

		IDISPLAY_Update(pme->m_pIDisplay);
		return ;
	}
	
	//显示罗盘
	IIMAGECTL_Redraw(pme->m_pImageCtl);

	// 绘制当前方向线和箭头
	IGRAPHICS_SetFillMode( pme->m_pGraphics, FALSE);
	IGRAPHICS_SetPaintMode( pme->m_pGraphics, AEE_PAINT_COPY );
	
	IGRAPHICS_SetColor( pme->m_pGraphics, 255, 0, 0, 0 );
    IGRAPHICS_SetFillColor( pme->m_pGraphics, 255, 0, 0, 0 );

	//罗盘刻度圆
	rCircle.cx = 120;
	rCircle.cy = 114;
	rCircle.r = FLTTOINT(r);
	IGRAPHICS_DrawCircle( pme->m_pGraphics, &rCircle );

	//当前方向角直线
	rLine.sx = 120;
	rLine.sy = 114;
	rLine.ex = 120+FLTTOINT(FMUL(r, FSIN(heading)));
	rLine.ey = 114-FLTTOINT(FMUL(r, FCOS(heading)));
	DBGPRINTF("@GetGpsInfo Line:(%d,%d)-(%d,%d)-r:%d", rLine.sx, rLine.sy, rLine.ex, rLine.ey, rCircle.r);
	IGRAPHICS_DrawLine( pme->m_pGraphics, &rLine );
	
	//方向角直线箭头
	IGRAPHICS_SetFillMode( pme->m_pGraphics, TRUE );
	rTriangle.x0 = rLine.ex+FLTTOINT(FMUL(20, FSIN(FADD(FADD(M_PI, headingArrow), heading))));
	rTriangle.y0 = rLine.ey-FLTTOINT(FMUL(20, FCOS(FADD(FADD(M_PI, headingArrow), heading))));
	rTriangle.x1 = rLine.ex;
	rTriangle.y1 = rLine.ey;
	rTriangle.x2 = rLine.ex+FLTTOINT(FMUL(20, FSIN(FSUB(FADD(M_PI, heading), headingArrow))));
	rTriangle.y2 = rLine.ey-FLTTOINT(FMUL(20, FCOS(FSUB(FADD(M_PI, heading), headingArrow))));
	IGRAPHICS_DrawTriangle( pme->m_pGraphics, &rTriangle );


	 {
		AECHAR bufRes[MP_MAX_STRLEN];
		AECHAR szBuf[MP_MAX_STRLEN];
		int a = 0, b = 0;
		int h = 0, xx = 0, yy = 0, dxx = 0, dyy = 0;
		AEERect rect;
		int xMargin = 0;

		//STRTOWSTR("Destination: BeiJing", pme->m_szText, sizeof(pme->m_szText));
		ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_NAME,bufRes,sizeof(bufRes));
		WSPRINTF(szBuf, sizeof(szBuf), L"%s: %s", bufRes, pme->m_pOwner->m_szTextDesc);
		//ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_WHERE_DETAILS_0, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 2;
		xx = xMargin;
		yy = 232+2;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT, szBuf, &rect);

		//目的地
		if (FCMP_G(distance, 0))
		{
			AECHAR bufDis[32];
			
			TS_PRINTD("FSIN", FSIN(destHeading));
			TS_PRINTD("FSIN", FCOS(destHeading));
			
			rCircle.cx = 120+FLTTOINT(FMUL(r, FSIN(destHeading)));
			rCircle.cy = 114-FLTTOINT(FMUL(r, FCOS(destHeading)));
			rCircle.r = 5;
			DBGPRINTF("@GetGpsInfo Dest:(%d,%d,r-%d)", rCircle.cx, rCircle.cy, rCircle.r);
			IGRAPHICS_DrawCircle( pme->m_pGraphics, &rCircle );
			
			//FLOATTOWSTR(distance, bufDis, 32);
			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DISTANCE, bufRes, sizeof(bufRes));
			WSPRINTF(szBuf, sizeof(szBuf), L"%s: %sm", bufRes, TS_FLT2SZ(bufDis, distance));
			//WSPRINTF(szBuf, sizeof(szBuf), L"%s: %sm", bufRes, bufDis);
			h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 2;
			xx = xMargin;
#ifdef AEE_SIMULATOR
			yy += h-5;
#else
			yy += h;
#endif
			dxx = pme->m_pOwner->m_cxWidth - 2;
			dyy = h;
			SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, szBuf, &rect);
		}

	 }
	IDISPLAY_Update(pme->m_pIDisplay);
}

/*===========================================================================
   This function processes events routed to Navigate window.
===========================================================================*/
static boolean CNavigateWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
	CNavigateWin *  pme = (CNavigateWin *)po;
	boolean     bRet = TRUE;
		
	//if (MP_ISEVTKEY(eCode))
	//	return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);
	
	//if (!MP_ISEVTCMD(eCode))
	//	return FALSE;

	switch (eCode)
	{
	case EVT_KEY:
		switch (wParam)
		{
		case AVK_SELECT:
			pme->m_bGetGpsInfo = TRUE;	//FOR TEST
			break;
		case AVK_SOFT1:
			bRet = FALSE;
			break;

		case AVK_SOFT2:
			CTopSoupApp_SetWindow(pme->m_pOwner, pme->m_pOwner->m_pPreNaviWin, 0);
			bRet = TRUE;
			break;

		case AVK_1://FOR TEST
			//DBGPRINTF("Mode: NETWORK");
			//pme->m_gpsMode = AEEGPS_MODE_TRACK_NETWORK;
			//CNavigateWin_LocStop(pme);
			//CNavigateWin_LocStart(pme);
			////STRTOWSTR("NETWORK", pme->m_szMode, sizeof(pme->m_szMode));
			////CWhereWin_Redraw((IWindow*)pme);
			//bRet = TRUE;
			break;

		case AVK_2://FOR TEST
			//DBGPRINTF("STANDALONE MODE");
			//pme->m_gpsMode = AEEGPS_MODE_TRACK_STANDALONE;
			//CNavigateWin_LocStop(pme);
			//CNavigateWin_LocStart(pme);
			////STRTOWSTR("Mode: STANDALONE", pme->m_szMode, sizeof(pme->m_szMode));
			////CWhereWin_Redraw((IWindow*)pme);
			//bRet = TRUE;
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
static void CNavigateWin_LocStart(CNavigateWin *pme)
{
	int nErr = SUCCESS;
	GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

	ZEROAT(pGetGPSInfo);

	pGetGPSInfo->theInfo.gpsConfig.server.svrType = AEEGPS_SERVER_DEFAULT;
	pGetGPSInfo->theInfo.gpsConfig.qos = 16;
	pGetGPSInfo->theInfo.gpsConfig.optim = 1;
	pGetGPSInfo->theInfo.gpsConfig.mode = pme->m_gpsMode;
	pGetGPSInfo->theInfo.gpsConfig.nFixes = 0;
	pGetGPSInfo->theInfo.gpsConfig.nInterval = 5;

	//if (pDest != NULL)
	{
		Coordinate dest;
		dest.lat = WSTRTOFLOAT(pme->m_pOwner->m_szTextLat);
		dest.lon = WSTRTOFLOAT(pme->m_pOwner->m_szTextLon);
		pGetGPSInfo->theInfo.destPos = dest;
		pGetGPSInfo->theInfo.bSetDestPos = TRUE;
	}

	if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_POSDET, (void **)&pGetGPSInfo->pPosDet) == SUCCESS) {

		CALLBACK_Init(&pGetGPSInfo->cbPosDet, CNavigateWin_GetGPSInfo_Callback, pme);

		nErr = Loc_Init(pme->m_pIShell, pGetGPSInfo->pPosDet, &pGetGPSInfo->cbPosDet, &pGetGPSInfo->pts);
		nErr = Loc_Start(pGetGPSInfo->pts, &pGetGPSInfo->theInfo);
		if (nErr != SUCCESS) {
			pGetGPSInfo->theInfo.nErr = nErr;
			DBGPRINTF("Loc_Start Failed! Err:%d", nErr);
		}
		else
		{
			pGetGPSInfo->bAbort = FALSE;
		}
	}
}


static void CNavigateWin_LocStop(CNavigateWin *pme)
{
	GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

	pme->m_bGetGpsInfo = FALSE;
	if (pGetGPSInfo->pPosDet)
	{
		Loc_Stop(pGetGPSInfo->pts);

		CALLBACK_Cancel(&pGetGPSInfo->cbProgressTimer);
		CALLBACK_Cancel(&pGetGPSInfo->cbPosDet);
		TS_RELEASEIF(pGetGPSInfo->pPosDet);
	}
}

/*===========================================================================
   This function called by location modoule.
===========================================================================*/
static void CNavigateWin_GetGPSInfo_Callback( IWindow *po )
{
	CNavigateWin *pme = (CNavigateWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

	DBGPRINTF("CNavigateWin_GetGPSInfo_Callback in nErr:%d", pGetGPSInfo->theInfo.nErr);

	if( pGetGPSInfo->theInfo.nErr == SUCCESS ) {
		/* Process new data from IPosDet */
		pGetGPSInfo->dwFixNumber++;
		pGetGPSInfo->dwFixDuration += pGetGPSInfo->wProgress;
		pGetGPSInfo->wProgress = 0;
		DBGPRINTF("@GetGPSInfo fix:%d", pGetGPSInfo->dwFixNumber);

		//经纬度有效时才算定位成功
		if (FCMP_G(pGetGPSInfo->theInfo.lat,0) && FCMP_G(pGetGPSInfo->theInfo.lon,0))
		{
			pme->m_bGetGpsInfo = TRUE;
			pGetGPSInfo->wIdleCount = 0;
		}
		//else
		//{
		//	pme->m_bGetGpsInfo = FALSE;
		//}

		CNavigateWin_Redraw(po);
	}
	else if( pGetGPSInfo->theInfo.nErr == EIDLE ) {
		/* End of tracking */
		DBGPRINTF("@End of tracking");
	}
	else if( pGetGPSInfo->theInfo.nErr == AEEGPS_ERR_TIMEOUT ) {
		/* Record the timeout and perhaps re-try. */
		pGetGPSInfo->dwTimeout++;
		//rgn = GETGPSINFO_PAINT_FIXCOUNT;
	}
	else {

		CNavigateWin_LocStop(pme);
		DBGPRINTF("@Something is not right here. Requires corrective action. Bailout");

		/* Something is not right here. Requires corrective action. Bailout */
		pGetGPSInfo->bAbort = TRUE;

		pGetGPSInfo->dwFixNumber = 0;

		CNavigateWin_Redraw(po);
	}
}

/*===========================================================================
   This function called by location modoule.
===========================================================================*/
static void CNavigateWin_GetGPSInfo_SecondTicker( IWindow *po )
{
	CNavigateWin *pme = (CNavigateWin*)po;
	struct _GetGPSInfo *pGetGPSInfo = &pme->m_pOwner->m_gpsInfo;

	if( pGetGPSInfo->bPaused == FALSE ) {
	  pGetGPSInfo->wProgress++;
	  DBGPRINTF("@Where GetGPS progress:%d", pGetGPSInfo->wProgress);
	  CNavigateWin_Redraw(po);
	}

	if( pGetGPSInfo->bAbort == FALSE ) {
		pGetGPSInfo->wIdleCount++;
		DBGPRINTF("@Where GetGPS wIdleCount:%d", pGetGPSInfo->wIdleCount);
	}

	//重新启动
	//1 空闲30秒
	//2 尝试2分钟未定位成功
	if (pGetGPSInfo->wIdleCount > WATCHER_TIMER || pGetGPSInfo->wProgress > 60 * 2)
	{
		//play_tts(pme, L"restart location");

		DBGPRINTF("@Where GetGPS CNavigateWin_LocStart");
		CNavigateWin_LocStop(pme);
		CNavigateWin_LocStart(pme);
	}

	ISHELL_SetTimerEx(pme->m_pIShell, 1000, &pme->m_cbWatcherTimer);
}

static void TS_PRINTD(const char* p, double param)
{
   char szBuf[64];
   AECHAR bufW[32];

   MEMSET(szBuf,0,sizeof(szBuf));
   FLOATTOWSTR(param, bufW, 32);
   WSTRTOSTR(bufW,szBuf, 64);

   DBGPRINTF("%s %s", p, szBuf);
}