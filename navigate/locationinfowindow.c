#include "locationinfowindow.h"


#define MP_MAX_STRLEN         64

#define WIN_FONT  AEE_FONT_NORMAL

// LocationInfoWin window: Displays main menu.
struct CLocationInfoWin
{
   INHERIT_CWindow(IWindow);

};

typedef struct CLocationInfoWin CLocationInfoWin;


static void       CLocationInfoWin_Delete(IWindow * po);
static void       CLocationInfoWin_Enable(IWindow * po, boolean bEnable);
static void       CLocationInfoWin_Redraw(IWindow * po);
static boolean    CLocationInfoWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

/*===============================================================================

                     CLocationInfoWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CLocationInfoWin_New(CTopSoupApp * pOwner)
{
   CLocationInfoWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CLocationInfoWin_Enable, CLocationInfoWin_Redraw, CLocationInfoWin_HandleEvent, CLocationInfoWin_Delete);
   pme = (CLocationInfoWin *)CWindow_New(sizeof(CLocationInfoWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	 ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATION_RANGE_INFO,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
     TS_SetSoftButtonText(pme->m_pOwner, 0, IDS_STRING_BACK, 0);
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CLocationInfoWin_Delete(IWindow * po)
{
   CLocationInfoWin *  pme = (CLocationInfoWin *)po;

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CLocationInfoWin_Enable(IWindow * po, boolean bEnable)
{
   CLocationInfoWin *  pme = (CLocationInfoWin *)po;
 
   if (!CWindow_ProcessEnable(po, bEnable))
      return;
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CLocationInfoWin_Redraw(IWindow * po)
{
   CLocationInfoWin *  pme = (CLocationInfoWin *)po;

   if (!pme->m_bActive)
      return;

   
   //XXX __begin
   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   TS_DrawBackgroud(po);
   
   {
		AECHAR bufRes[MP_MAX_STRLEN];
		int a = 0, b = 0;
		int h = 0, xx = 0, yy = 0, dxx = 0, dyy = 0;
		AEERect rect;
		int xMargin = 0;

		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATION_RANGE_INFO_LON_0, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy = 64;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATION_RANGE_INFO_LON_1, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);


		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATION_RANGE_INFO_LAT_0, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);


		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_LOCATION_RANGE_INFO_LAT_1, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
   }

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CLocationInfoWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CLocationInfoWin *  pme = (CLocationInfoWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){

      //·µ»ØÎÒÔÚÄÄ
      if( AVK_SOFT2 == wParam )
      {
		 CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_NEW_FUCTION, 0);
         return TRUE;
      }
   }
   
   return bRet;
}
