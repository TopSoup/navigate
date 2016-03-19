#include "mainwindow.h"



// Main window: Displays main menu.
struct CMainWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IImage *       m_pLogo;
	AEERect        m_rectLogo;
	IMenuCtl *     m_pMainMenu;
	flg            m_bAbout:1;
};

typedef struct CMainWin CMainWin;


static void       CMainWin_Delete(IWindow * po);
static void       CMainWin_Enable(IWindow * po, boolean bEnable);
static void       CMainWin_Redraw(IWindow * po);
static boolean    CMainWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CMainWin_About(IWindow * po);

/*===============================================================================

                     CMainWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CMainWin_New(CTopSoupApp * pOwner)
{
   CMainWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CMainWin_Enable, CMainWin_Redraw, CMainWin_HandleEvent, CMainWin_Delete);
   pme = (CMainWin *)CWindow_New(sizeof(CMainWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
      int      cx = ((CTopSoupApp*)pme->m_pOwner)->m_cxWidth;
      int      cy = ((CTopSoupApp*)pme->m_pOwner)->m_cyHeight;
      int      y, dy;
      AEERect  rect;


	  //XXX __begin
      //Initialize logo below the header
      pme->m_pLogo = ISHELL_LoadResImage(pme->m_pIShell, NAVIGATE_RES_FILE, IDB_LOGO);
      if (!pme->m_pLogo)
         TS_WINERR_RETURN(pme);

      y = ((CTopSoupApp*)pme->m_pOwner)->m_rectHdr.dy + 1;
      dy = cy/2 - y;
      SETAEERECT(&pme->m_rectLogo, 0, y, cx, dy);
      IIMAGE_SetFrameCount(pme->m_pLogo, 2);
      IIMAGE_SetAnimationRate(pme->m_pLogo, 500);

      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
         TS_WINERR_RETURN(pme);

      SETAEERECT(&rect, 0, cy/2 + 1, cx, cy/2 - 1);
      TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth, &rect, 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_PLAYFILE, NULL, IDB_PLAY,     IDM_MAIN_PLAYFILE, 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_RECORD,   NULL, IDB_RECORD,   IDM_MAIN_RECORD,   0);
      TS_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_ABOUT,    NULL, IDB_ABOUT,    IDM_MAIN_ABOUT,    0);
	  //XXX __end


   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CMainWin_Delete(IWindow * po)
{
   CMainWin *  pme = (CMainWin *)po;

   //XXX __begin
   TS_RELEASEIF(pme->m_pLogo);

   if (pme->m_pMainMenu)
	   pme->m_pOwner->m_wMainWin = IMENUCTL_GetSel(pme->m_pMainMenu);
   TS_RELEASEIF(pme->m_pMainMenu);
   //XXX _end

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CMainWin_Enable(IWindow * po, boolean bEnable)
{
   CMainWin *  pme = (CMainWin *)po;
 

   if (!CWindow_ProcessEnable(po, bEnable))
	   return;


   //XXX __begin
   if (!pme->m_bActive)
   {
      IMENUCTL_SetActive(pme->m_pMainMenu, FALSE);
      IIMAGE_Stop(pme->m_pLogo);
      return;
   }

   IMENUCTL_SetActive(pme->m_pMainMenu, TRUE);
   IMENUCTL_SetSel(pme->m_pMainMenu, ((CTopSoupApp*)pme->m_pOwner)->m_wMainWin);
   TS_DrawImage(pme->m_pLogo, &pme->m_rectLogo, TRUE);
   //XXX __end
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CMainWin_Redraw(IWindow * po)
{
   CMainWin *  pme = (CMainWin *)po;

   if (!pme->m_bActive)
      return;

   
   //XXX __begin
   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   TS_DRAWBACK(pme);
   TS_DRAWHEADER(pme);
   TS_DrawImage(pme->m_pLogo, &pme->m_rectLogo, TRUE);
   IMENUCTL_Redraw(pme->m_pMainMenu);

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
#define MP_QCP_REC_FILE       "sample.qcp"
static boolean CMainWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CMainWin *  pme = (CMainWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if (eCode == EVT_COPYRIGHT_END && pme->m_bAbout)
   {
      pme->m_bAbout = FALSE;
      CTopSoupApp_Redraw(pme->m_pOwner, TRUE);
      return TRUE;
   }
            
   if (TS_ISEVTKEY(eCode))
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDM_MAIN_PLAYFILE:
         break;

      case IDM_MAIN_RECORD:
         break;

      case IDM_MAIN_ABOUT:
         CMainWin_About(pme);
         break;

      default:
         bRet = FALSE;
         break;
   }
   //XXX __end

   return bRet;
}

/*===========================================================================
   This function displays the About dialog of the app.
===========================================================================*/
static void CMainWin_About(IWindow * po)
{
   CMainWin *  pme = (CMainWin *)po;

	CTopSoupApp_DisableWin(pme->m_pOwner);

   pme->m_bAbout = TRUE;

   IDISPLAY_ClearScreen(pme->m_pIDisplay);
   TS_DRAWBACK(pme);
   TS_DRAWHEADER(pme);
   ISHELL_ShowCopyright(pme->m_pIShell);
}
