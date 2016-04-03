#include "destinfowindow.h"


#define MP_MAX_STRLEN         64

#define WIN_FONT  AEE_FONT_NORMAL

// DestInfoWin window: Displays main menu.
struct CDestInfoWin
{
   INHERIT_CWindow(IWindow);

};

typedef struct CDestInfoWin CDestInfoWin;


static void       CDestInfoWin_Delete(IWindow * po);
static void       CDestInfoWin_Enable(IWindow * po, boolean bEnable);
static void       CDestInfoWin_Redraw(IWindow * po);
static boolean    CDestInfoWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

/*===============================================================================

                     CDestInfoWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CDestInfoWin_New(CTopSoupApp * pOwner)
{
   CDestInfoWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CDestInfoWin_Enable, CDestInfoWin_Redraw, CDestInfoWin_HandleEvent, CDestInfoWin_Delete);
   pme = (CDestInfoWin *)CWindow_New(sizeof(CDestInfoWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	 ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_INFO,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
     TS_SetSoftButtonText(pme->m_pOwner, 0, IDS_STRING_BACK, 0);
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CDestInfoWin_Delete(IWindow * po)
{
   CDestInfoWin *  pme = (CDestInfoWin *)po;

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CDestInfoWin_Enable(IWindow * po, boolean bEnable)
{
   CDestInfoWin *  pme = (CDestInfoWin *)po;
 
   if (!CWindow_ProcessEnable(po, bEnable))
      return;
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CDestInfoWin_Redraw(IWindow * po)
{
   CDestInfoWin *  pme = (CDestInfoWin *)po;

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
      int xMargin = 0, yMargin = 2;

      ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_INFO_0, bufRes, sizeof(bufRes));
      h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + yMargin;
      xx = xMargin;
      yy = 64;
      dxx = pme->m_pOwner->m_cxWidth - 2;
      dyy = h;
      SETAEERECT(&rect, xx, yy, dxx, dyy);

	  {
		int nFits = 0;
		int len = WSTRLEN(bufRes);
		AECHAR extRes[MP_MAX_STRLEN];

		IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		if (nFits < len)
		{
			WSTRCPY( extRes, &bufRes[nFits]);
			bufRes[nFits] = 0;
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

			WSTRCPY(bufRes, extRes);
		}
		
		IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		if (nFits < len)
		{
			xx = xMargin;
			  yy += h;
			  dxx = pme->m_pOwner->m_cxWidth - 2;
			  dyy = h;
			  SETAEERECT(&rect, xx, yy, dxx, dyy);

			WSTRCPY( extRes, &bufRes[nFits]);
			bufRes[nFits] = 0;
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

			WSTRCPY(bufRes, extRes);
		}

		//IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		//if (nFits < len)
		{
			xx = xMargin;
			  yy += h;
			  dxx = pme->m_pOwner->m_cxWidth - 2;
			  dyy = h;
			  SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
		}
	  }

	  yMargin += 2;
      ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_INFO_1, bufRes, sizeof(bufRes));
      h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + yMargin;
      xx = xMargin;
      yy += h;
      dxx = pme->m_pOwner->m_cxWidth - 2;
      dyy = h;
      SETAEERECT(&rect, xx, yy, dxx, dyy);
      //TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
	  {
		int nFits = 0;
		int len = WSTRLEN(bufRes);
		AECHAR extRes[MP_MAX_STRLEN];

		IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		if (nFits < len)
		{
			WSTRCPY( extRes, &bufRes[nFits]);
			bufRes[nFits] = 0;
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

			WSTRCPY(bufRes, extRes);
		}
		
		IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		if (nFits < len)
		{
			xx = xMargin;
			  yy += h;
			  dxx = pme->m_pOwner->m_cxWidth - 2;
			  dyy = h;
			  SETAEERECT(&rect, xx, yy, dxx, dyy);

			  	WSTRCPY( extRes, &bufRes[nFits]);
			bufRes[nFits] = 0;
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
			WSTRCPY(bufRes, extRes);
		}

		IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		if (nFits < len)
		{
			xx = xMargin;
			  yy += h;
			  dxx = pme->m_pOwner->m_cxWidth - 2;
			  dyy = h;
			  SETAEERECT(&rect, xx, yy, dxx, dyy);

			  	WSTRCPY( extRes, &bufRes[nFits]);
			bufRes[nFits] = 0;
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
			WSTRCPY(bufRes, extRes);
		}

		//IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		//if (nFits < len)
		{
			xx = xMargin;
			  yy += h;
			  dxx = pme->m_pOwner->m_cxWidth - 2;
			  dyy = h;
			  SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
		}

	  }

      ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DEST_INFO_2, bufRes, sizeof(bufRes));
      h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + yMargin;
      xx = xMargin;
      yy += h;
      dxx = pme->m_pOwner->m_cxWidth - 2;
      dyy = h;
      SETAEERECT(&rect, xx, yy, dxx, dyy);
      //TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
	  {
		int nFits = 0;
		int len = WSTRLEN(bufRes);
		AECHAR extRes[MP_MAX_STRLEN];

		IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
		if (nFits < len)
		{
			WSTRCPY( extRes, &bufRes[nFits]);
			bufRes[nFits] = 0;
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

			WSTRCPY(bufRes, extRes);
		}
		
		//if (nFits < len)
		{
			xx = xMargin;
			  yy += h;
			  dxx = pme->m_pOwner->m_cxWidth - 2;
			  dyy = h;
			  SETAEERECT(&rect, xx, yy, dxx, dyy);
			TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
		}
	  }
   }

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CDestInfoWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CDestInfoWin *  pme = (CDestInfoWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){

      //·µ»ØÎÒÔÚÄÄ
      if( AVK_SOFT2 == wParam )
      {
         CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
         return TRUE;
      }
   }
   
   return bRet;
}
