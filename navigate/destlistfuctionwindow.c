#include "wherefuctionwindow.h"



// Main window: Displays main menu.
struct CDestlistFuctionWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;

	uint16			m_wRecID;
};

typedef struct CDestlistFuctionWin CDestlistFuctionWin;


static void       CDestlistFuctionWin_Delete(IWindow * po);
static void       CDestlistFuctionWin_Enable(IWindow * po, boolean bEnable);
static void       CDestlistFuctionWin_Redraw(IWindow * po);
static boolean    CDestlistFuctionWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);


/*===============================================================================

                     CDestlistFuctionWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CDestlistFuctionWin_New(CTopSoupApp * pOwner,uint16 wRectID)
{
   CDestlistFuctionWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CDestlistFuctionWin_Enable, CDestlistFuctionWin_Redraw, CDestlistFuctionWin_HandleEvent, CDestlistFuctionWin_Delete);
   pme = (CDestlistFuctionWin *)CWindow_New(sizeof(CDestlistFuctionWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   pme->m_wRecID = wRectID;

   {
	  //XXX __begin
      //Initialize logo below the header
      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
         TS_WINERR_RETURN(pme);

      TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth,&((CTopSoupApp*)pme->m_pOwner)->m_rectWin , 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_SMS, NULL, IDI_OBJECT_15201, IDS_STRING_SMS, 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DEST_LOCATION_INFO,   NULL, IDI_OBJECT_15202, IDS_STRING_DEST_LOCATION_INFO,   0);
	  TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_EDIT,   NULL, IDI_OBJECT_15203, IDS_STRING_EDIT,   0);
	  TS_AddMenuItem(pme->m_pMainMenu,IDS_STRING_DELETE,   NULL, IDI_OBJECT_15204, IDS_STRING_DELETE,   0);
      TS_AddMenuItem(pme->m_pMainMenu,IDS_STRING_DELETE_ALL,   NULL, IDI_OBJECT_15205, IDS_STRING_DELETE_ALL,   0);

	  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_FUCTION,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	  TS_SetSoftButtonText(pme->m_pOwner,NULL,IDS_STRING_BACK,IDS_STRING_SELECT);
	  //XXX __end


   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CDestlistFuctionWin_Delete(IWindow * po)
{
   CDestlistFuctionWin *  pme = (CDestlistFuctionWin *)po;

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
static void CDestlistFuctionWin_Enable(IWindow * po, boolean bEnable)
{
   CDestlistFuctionWin *  pme = (CDestlistFuctionWin *)po;
 

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
static void CDestlistFuctionWin_Redraw(IWindow * po)
{
   CDestlistFuctionWin *  pme = (CDestlistFuctionWin *)po;

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
static boolean CDestlistFuctionWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CDestlistFuctionWin *  pme = (CDestlistFuctionWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT1 == wParam ) {
		   return FALSE;
	   }

	   if( AVK_SOFT2 == wParam )
	   {
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_LIST, 0);
			return TRUE;
	   }
   }
   

   if (TS_ISEVTKEY(eCode)) 
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDS_STRING_SMS:
		  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_RECIPIENT,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
		  pme->m_pOwner->m_pTextctlMode = AEE_TM_NUMBERS;
		  pme->m_pOwner->m_pTextctlWin = TSW_DEST_LIST;
		  pme->m_pOwner->m_op = 1;
		  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_TEXTCTL, 0);
         break;

      case IDS_STRING_DEST_LOCATION_INFO:
		  CTopSoupApp_SetWindow(pme->m_pOwner,TSW_LOCINFO,pme->m_wRecID);
		  break;

	  case IDS_STRING_EDIT:
		  break;

	  case IDS_STRING_DELETE:
		  break;

	  case IDS_STRING_DELETE_ALL:
		  break;
	 
      default:
         bRet = FALSE;
         break;
   }
   //XXX __end

   return bRet;
}

