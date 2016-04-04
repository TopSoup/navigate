#include "locinfowindow.h"

#define MP_WHERE_CY			  32

#define WIN_FONT	AEE_FONT_LARGE

// Where window: Displays main menu.
struct CLocinfoWin
{
	INHERIT_CWindow(IWindow);
	
	AECHAR          m_szText[TS_MAX_STRLEN];

	uint16          m_wRectID;		//界面需要展示的目的地ID
};

typedef struct CLocinfoWin CLocinfoWin;

static void       CLocinfoWin_Delete(IWindow * po);
static void       CLocinfoWin_Enable(IWindow * po, boolean bEnable);
static void       CLocinfoWin_Redraw(IWindow * po);
static boolean    CLocinfoWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

/*===============================================================================

                     CLocinfoWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the Locinfo window.
===========================================================================*/
IWindow * CLocinfoWin_New(CTopSoupApp * pOwner,uint16 wRecID)
{
   CLocinfoWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CLocinfoWin_Enable, CLocinfoWin_Redraw, CLocinfoWin_HandleEvent, CLocinfoWin_Delete);
   pme = (CLocinfoWin *)CWindow_New(sizeof(CLocinfoWin), pOwner, &vtbl);
   if (!pme)
      return NULL;


   pme->m_wRectID = wRecID;

	ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_LOCATION_INFO,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,0);

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the Locinfo window.
===========================================================================*/
static void CLocinfoWin_Delete(IWindow * po)
{
	CLocinfoWin *  pme = (CLocinfoWin *)po;	
	
	FREE(pme);
}

/*===========================================================================
   This function enables/disables the Locinfo window.
===========================================================================*/
static void CLocinfoWin_Enable(IWindow * po, boolean bEnable)
{
	if (!CWindow_ProcessEnable(po, bEnable))
		return;
}

/*===========================================================================
   This function redraws the Locinfo window.
===========================================================================*/
static void CLocinfoWin_Redraw(IWindow * po)
{
	CLocinfoWin *  pme = (CLocinfoWin *)po;
    AECHAR bufLat[TS_MAX_STRLEN], bufLon[TS_MAX_STRLEN], bufDestName[TS_MAX_STRLEN], bufDestSaveTime[TS_MAX_STRLEN];


	if (!pme->m_bActive)
		return;
	DBGPRINTF("@CLocinfoWin_Redraw in");
	IDISPLAY_ClearScreen(pme->m_pIDisplay);
	TS_DrawBackgroud(po);

	
	//数据库中获取数据
    TS_GetExpenseItem(pme->m_pOwner,pme->m_wRectID,bufDestName,bufLat,bufLon);
	MEMSET(bufDestSaveTime,0,sizeof(bufDestSaveTime));
	

	{
		AECHAR bufRes[TS_MAX_STRLEN];
		int h = 0, xx = 0, yy = 0, dxx = 0, dyy = 0;
		int a = 0, b = 0;
		AEERect rect;
		int xMargin = 4;
		
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		yy = 64;

		//目的地名称
		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST, bufRes, sizeof(bufRes));
		WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufDestName);
		xx = xMargin;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);
		
		//经度
		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LON, bufRes, sizeof(bufRes));
		WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufLon);
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);

		//纬度		
		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LAT, bufRes, sizeof(bufRes));
		WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufLat);
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);

        //存储时间
	/*	ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SAVETIME, bufRes, sizeof(bufRes));
		WSPRINTF(pme->m_szText, sizeof(pme->m_szText), L"%s: %s", bufRes, bufDestSaveTime);
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT,  pme->m_szText, &rect);*/
		
	}

	IDISPLAY_Update(pme->m_pIDisplay);

	DBGPRINTF("@CLocinfoWin_Redraw in");
}


/*===========================================================================
   This function processes events routed to Locinfo window.
===========================================================================*/
static boolean CLocinfoWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
	CLocinfoWin *  pme = (CLocinfoWin *)po;
	boolean     bRet = FALSE;

	if(TS_ISSOFT(eCode)){
		if( wParam == AVK_SOFT2 ) {
			CTopSoupApp_SetWindow(pme->m_pOwner,TSW_DEST_LIST,pme->m_wRectID);
			return TRUE;
		}

	}

	return bRet;
}






