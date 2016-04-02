#include "wheredetailswindow.h"


#define MP_MAX_STRLEN         64

#define WIN_FONT	AEE_FONT_LARGE

// WhereDetails window: Displays main menu.
struct CWhereDetailsWin
{
	INHERIT_CWindow(IWindow);

};

typedef struct CWhereDetailsWin CWhereDetailsWin;


static void       CWhereDetailsWin_Delete(IWindow * po);
static void       CWhereDetailsWin_Enable(IWindow * po, boolean bEnable);
static void       CWhereDetailsWin_Redraw(IWindow * po);
static boolean    CWhereDetailsWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

/*===============================================================================

                     CWhereDetailsWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CWhereDetailsWin_New(CTopSoupApp * pOwner)
{
   CWhereDetailsWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CWhereDetailsWin_Enable, CWhereDetailsWin_Redraw, CWhereDetailsWin_HandleEvent, CWhereDetailsWin_Delete);
   pme = (CWhereDetailsWin *)CWindow_New(sizeof(CWhereDetailsWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	  TS_SetSoftButtonText(pme->m_pOwner, 0, IDS_STRING_BACK, 0);
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CWhereDetailsWin_Delete(IWindow * po)
{
   CWhereDetailsWin *  pme = (CWhereDetailsWin *)po;

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CWhereDetailsWin_Enable(IWindow * po, boolean bEnable)
{
   CWhereDetailsWin *  pme = (CWhereDetailsWin *)po;
 
   if (!CWindow_ProcessEnable(po, bEnable))
	   return;
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CWhereDetailsWin_Redraw(IWindow * po)
{
   CWhereDetailsWin *  pme = (CWhereDetailsWin *)po;

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
		int xMargin = 4;

		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_WHERE_DETAILS_0, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy = 64;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, bufRes, &rect);

		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_WHERE_DETAILS_1, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, bufRes, &rect);


		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_WHERE_DETAILS_2, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, bufRes, &rect);


		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_WHERE_DETAILS_3, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, bufRes, &rect);


		ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_WHERE_DETAILS_4, bufRes, sizeof(bufRes));
		h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + 12;
		xx = xMargin;
		yy += h;
		dxx = pme->m_pOwner->m_cxWidth - 2;
		dyy = h;
		SETAEERECT(&rect, xx, yy, dxx, dyy);
		TS_DrawText(pme->m_pIDisplay, bufRes, &rect);
   }

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CWhereDetailsWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CWhereDetailsWin *  pme = (CWhereDetailsWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){

	   //·µ»ØÎÒÔÚÄÄ
	   if( AVK_SOFT2 == wParam )
	   {
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_WHERE, 0);
			return TRUE;
	   }
   }
   
   return bRet;
}
