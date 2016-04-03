#include "wherefuctionwindow.h"



// Main window: Displays main menu.
struct CNewdestFuctionWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;
};

typedef struct CNewdestFuctionWin CNewdestFuctionWin;


static void       CNewdestFuctionWin_Delete(IWindow * po);
static void       CNewdestFuctionWin_Enable(IWindow * po, boolean bEnable);
static void       CNewdestFuctionWin_Redraw(IWindow * po);
static boolean    CNewdestFuctionWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);


/*===============================================================================

                     CNewdestFuctionWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CNewdestFuctionWin_New(CTopSoupApp * pOwner)
{
   CNewdestFuctionWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CNewdestFuctionWin_Enable, CNewdestFuctionWin_Redraw, CNewdestFuctionWin_HandleEvent, CNewdestFuctionWin_Delete);
   pme = (CNewdestFuctionWin *)CWindow_New(sizeof(CNewdestFuctionWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	  //XXX __begin
      //Initialize logo below the header
      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
         TS_WINERR_RETURN(pme);

      TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth,&((CTopSoupApp*)pme->m_pOwner)->m_rectWin , 0);
      TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DEST_NAVIGATE, NULL, IDI_OBJECT_15201, IDS_STRING_DEST_NAVIGATE, 0);
	  TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_SAVE_LOCATION,   NULL, IDI_OBJECT_15202, IDS_STRING_SAVE_LOCATION, 0);
	  TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_LOCATION_RANGE_INFO,   NULL, IDI_OBJECT_15203, IDS_STRING_LOCATION_RANGE_INFO,   0);

	  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_FUCTION,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
	  TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_SELECT,IDS_STRING_BACK,0);
	  //XXX __end


   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CNewdestFuctionWin_Delete(IWindow * po)
{
   CNewdestFuctionWin *  pme = (CNewdestFuctionWin *)po;

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
static void CNewdestFuctionWin_Enable(IWindow * po, boolean bEnable)
{
   CNewdestFuctionWin *  pme = (CNewdestFuctionWin *)po;
 

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
static void CNewdestFuctionWin_Redraw(IWindow * po)
{
   CNewdestFuctionWin *  pme = (CNewdestFuctionWin *)po;

   if (!pme->m_bActive)
      return;

   
   //XXX __begin
   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   TS_DrawBackgroud(po);
   IMENUCTL_Redraw(pme->m_pMainMenu);
   

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}

static void CNewdestFuctionWin_onSplashDrawOver(void * po) 
{
	CTopSoupApp* pme = (CTopSoupApp*)po;

	CTopSoupApp_SetWindow(pme, TSW_DEST_NEW_FUCTION, 0);
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CNewdestFuctionWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CNewdestFuctionWin *  pme = (CNewdestFuctionWin *)po;
   boolean     bRet = TRUE;

   //XXX __begin
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT1 == wParam ) {
		   return IMENUCTL_HandleEvent(pme->m_pMainMenu, EVT_KEY, AVK_SELECT, 0);
	   }

	   if( AVK_SOFT2 == wParam )
	   {
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_NEW, 0);
			return TRUE;
	   }
   }
   

   if (TS_ISEVTKEY(eCode)) 
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDS_STRING_DEST_NAVIGATE:
         break;

	  case IDS_STRING_SAVE_LOCATION:
		  {
			AECHAR prompt[TS_MAX_STRLEN];

			//保存位置信息到数据库
			if (WSTRLEN(pme->m_pOwner->m_szTextLat) == 0 
				|| WSTRLEN(pme->m_pOwner->m_szTextLon) == 0 
				|| WSTRLEN(pme->m_pOwner->m_szTextDesc) == 0)
			{
				DBGPRINTF("LOCATION DATA ERROR!");//TODO 界面提示
				return TRUE;
			}

			if (!TS_AddExpenseItem(pme->m_pOwner, pme->m_pOwner->m_szTextLat, pme->m_pOwner->m_szTextLon, pme->m_pOwner->m_szTextDesc))
			{
				DBGPRINTF("SAVE DATA ERROR!");//TODO 界面提示
				return TRUE;
			}  

			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SAVE,prompt,sizeof(prompt));

			//提示窗口
		   MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
		   WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
		   TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);

		  }
		  break;
	 
	  case IDS_STRING_LOCATION_RANGE_INFO:
		  CTopSoupApp_SetWindow(pme->m_pOwner, TSW_LOCATION_RANGE_INFO, 0);
		  break;

      default:
         bRet = FALSE;
         break;
   }
   //XXX __end

   return bRet;
}

