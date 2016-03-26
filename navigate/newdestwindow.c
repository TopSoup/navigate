#include "newdestwindow.h"



// NewDestination window: Displays main menu.
struct CNewDestWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;
};

typedef struct CNewDestWin CNewDestWin;


static void       CNewDestWin_Delete(IWindow * po);
static void       CNewDestWin_Enable(IWindow * po, boolean bEnable);
static void       CNewDestWin_Redraw(IWindow * po);
static boolean    CNewDestWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CNewDestWin_About(IWindow * po);

/*===============================================================================

                     CNewDestWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CNewDestWin_New(CTopSoupApp * pOwner)
{
   CNewDestWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CNewDestWin_Enable, CNewDestWin_Redraw, CNewDestWin_HandleEvent, CNewDestWin_Delete);
   pme = (CNewDestWin *)CWindow_New(sizeof(CNewDestWin), pOwner, &vtbl);
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
	  TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DESTINATION,   NULL, IDI_OBJECT_15203, IDS_STRING_DESTINATION,   0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_APPINFO,    NULL, IDI_OBJECT_15203, IDS_STRING_APPINFO,    0);

	  TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_SELECT,IDS_STRING_BACK,0);
	  //XXX __end


   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CNewDestWin_Delete(IWindow * po)
{
   CNewDestWin *  pme = (CNewDestWin *)po;

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
static void CNewDestWin_Enable(IWindow * po, boolean bEnable)
{
   CNewDestWin *  pme = (CNewDestWin *)po;
 

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
static void CNewDestWin_Redraw(IWindow * po)
{
   CNewDestWin *  pme = (CNewDestWin *)po;

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
static boolean CNewDestWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CNewDestWin *  pme = (CNewDestWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT1 == wParam )
		   return TRUE;

	   //ÍË³ö³ÌÐò
	   if( AVK_SOFT2 == wParam )
	   {
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
			return TRUE;
	   }
   }
   

   if (TS_ISEVTKEY(eCode)) 
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDS_STRING_MY_LOCATION:
		  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_WHERE, 0);
         break;

      case IDS_STRING_NAVIGATE:
		  {
			  //default beijing, for test
			  Coordinate	dest;
			  dest.lat = 39.911954;
			  dest.lon = 116.377817;
			  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE, (uint32)&dest);
			  
			  break;
		  }
	  case IDS_STRING_DESTINATION:
		  {
			  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_NEW, 0);
			  
			  break;
		  }
      case IDS_STRING_APPINFO:
         CNewDestWin_About((IWindow*)pme);
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
static void CNewDestWin_About(IWindow * po)
{
   CNewDestWin *  pme = (CNewDestWin *)po;

	CTopSoupApp_DisableWin(pme->m_pOwner);

   IDISPLAY_ClearScreen(pme->m_pIDisplay);
   TS_DrawBackgroud(po);
   ISHELL_ShowCopyright(pme->m_pIShell);
}
