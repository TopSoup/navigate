#include "navigatedestwindow.h"



// NavigateDest window: Displays main menu.
struct CNavigateDestWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;
};

typedef struct CNavigateDestWin CNavigateDestWin;


static void       CNavigateDestWin_Delete(IWindow * po);
static void       CNavigateDestWin_Enable(IWindow * po, boolean bEnable);
static void       CNavigateDestWin_Redraw(IWindow * po);
static boolean    CNavigateDestWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CNavigateDestWin_About(IWindow * po);

/*===============================================================================

                     CNavigateDestWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CNavigateDestWin_New(CTopSoupApp * pOwner)
{
   CNavigateDestWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CNavigateDestWin_Enable, CNavigateDestWin_Redraw, CNavigateDestWin_HandleEvent, CNavigateDestWin_Delete);
   pme = (CNavigateDestWin *)CWindow_New(sizeof(CNavigateDestWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	  //XXX __begin
      //Initialize logo below the header
      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
         TS_WINERR_RETURN(pme);

      TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth,&((CTopSoupApp*)pme->m_pOwner)->m_rectWin , 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DEST_LIST, NULL, IDI_OBJECT_15201, IDS_STRING_DEST_LIST, 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DEST_NEW,   NULL, IDI_OBJECT_15202, IDS_STRING_DEST_NEW,   0);
	  TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DEST_INFO,   NULL, IDI_OBJECT_15203, IDS_STRING_DEST_INFO,   0);

	  ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NAVIGATE,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	  TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_SELECT,IDS_STRING_BACK,0);
	  //XXX __end


   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CNavigateDestWin_Delete(IWindow * po)
{
   CNavigateDestWin *  pme = (CNavigateDestWin *)po;

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
static void CNavigateDestWin_Enable(IWindow * po, boolean bEnable)
{
   CNavigateDestWin *  pme = (CNavigateDestWin *)po;
 

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
static void CNavigateDestWin_Redraw(IWindow * po)
{
   CNavigateDestWin *  pme = (CNavigateDestWin *)po;

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
static boolean CNavigateDestWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CNavigateDestWin *  pme = (CNavigateDestWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT1 == wParam )
	   {
		   return IMENUCTL_HandleEvent(pme->m_pMainMenu, EVT_KEY, AVK_SELECT, 0);
	   }

	   //ÍË³ö³ÌÐò
	   if( AVK_SOFT2 == wParam )
	   {
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_MAIN, 0);
			return TRUE;
	   }
   }
   

   if (TS_ISEVTKEY(eCode)) 
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDS_STRING_DEST_LIST:
		  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_LIST, 0);
         break;

      case IDS_STRING_DEST_NEW:
		  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_NEW, 0);
		  break;

	  case IDS_STRING_DEST_INFO:
		  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_INFO, 0);
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
static void CNavigateDestWin_About(IWindow * po)
{
   CNavigateDestWin *  pme = (CNavigateDestWin *)po;

	CTopSoupApp_DisableWin(pme->m_pOwner);

   IDISPLAY_ClearScreen(pme->m_pIDisplay);
   TS_DrawBackgroud(po);
   ISHELL_ShowCopyright(pme->m_pIShell);
}
