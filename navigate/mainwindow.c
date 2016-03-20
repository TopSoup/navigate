#include "mainwindow.h"



// Main window: Displays main menu.
struct CMainWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;
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
	  //XXX __begin
      //Initialize logo below the header
      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
         TS_WINERR_RETURN(pme);

      TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth,&((CTopSoupApp*)pme->m_pOwner)->m_rectWin , 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_MY_LOCATION, NULL, IDI_OBJECT_15201, IDS_STRING_MY_LOCATION, 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_NAVIGATE,   NULL, IDI_OBJECT_15202, IDS_STRING_NAVIGATE,   0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_APPINFO,    NULL, IDI_OBJECT_15203, IDS_STRING_APPINFO,    0);

	  TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_SELECT,IDS_STRING_BACK,0);
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
      return;
   }

   IMENUCTL_SetActive(pme->m_pMainMenu, TRUE);
   IMENUCTL_SetSel(pme->m_pMainMenu, ((CTopSoupApp*)pme->m_pOwner)->m_wMainWin);
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

   TS_DrawBackgroud(po);
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
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT1 == wParam )
		   return TRUE;
	   if( AVK_SOFT2 == wParam )
		   return TRUE;
   }
   
   if (TS_ISEVTKEY(eCode)) 
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDS_STRING_MY_LOCATION:
         break;

      case IDS_STRING_NAVIGATE:
         break;

      case IDS_STRING_APPINFO:
         CMainWin_About((IWindow*)pme);
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

   IDISPLAY_ClearScreen(pme->m_pIDisplay);
   TS_DrawBackgroud(po);
   ISHELL_ShowCopyright(pme->m_pIShell);
}
