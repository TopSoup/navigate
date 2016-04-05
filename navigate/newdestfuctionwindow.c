#include "wherefuctionwindow.h"

#include "navigatepch.h"


// Main window: Displays main menu.
struct CNewdestFuctionWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;

	int				m_nStatus;
	
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
	   pme->m_pOwner->m_wMenuLastSel[TSW_DEST_NEW_FUCTION] = IMENUCTL_GetSel(pme->m_pMainMenu);
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
   IMENUCTL_SetSel(pme->m_pMainMenu, ((CTopSoupApp*)pme->m_pOwner)->m_wMenuLastSel[TSW_DEST_NEW_FUCTION]);
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

	//if (pme->m_nStatus == 0)
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
			CTopSoupApp* pOwner = pme->m_pOwner;
		   
		    CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_NEW, 0);
			pOwner->m_wMenuLastSel[TSW_DEST_NEW_FUCTION] = 0;
			return TRUE;
	   }
   }
   

   if (TS_ISEVTKEY(eCode)) 
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   pme->m_nStatus = 0;
   switch (wParam)
   {
   case IDS_STRING_DEST_NAVIGATE:
	   
	   //校验经纬度
	   if (WSTRLEN(pme->m_pOwner->m_szTextLat) == 0 
		   || WSTRLEN(pme->m_pOwner->m_szTextLon) == 0 )
	   {
		   AECHAR prompt[TS_MAX_STRLEN];
		   DBGPRINTF("LOCATION DATA ERROR!");//TODO 界面提示
		   
		   ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_COORD,prompt,sizeof(prompt));
		   
		   //提示窗口
		   MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
		   WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
		   //TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
		   TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
		   pme->m_nStatus = 1;

		   return TRUE;
	   }
	   
	   //校验经纬度
	   if (TS_CheckLat(pme->m_pOwner->m_szTextLat) == FALSE 
		   || TS_CheckLon(pme->m_pOwner->m_szTextLon) == FALSE )
	   {
		   AECHAR prompt[TS_MAX_STRLEN];
		   DBGPRINTF("LOCATION DATA ERROR!");//TODO 界面提示
		   
		   if (TS_CheckLat(pme->m_pOwner->m_szTextLat) == FALSE)
			   ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_LAT,prompt,sizeof(prompt));
		   else
			   ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_LON,prompt,sizeof(prompt));
		   
		   //提示窗口
		   MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
		   WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
		   //TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
		   TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
		   pme->m_nStatus = 1;

		   return TRUE;
	   }
	   
	   //如果位置名称为空, 则使用默认日志编号格式名称
	   if (WSTRLEN(pme->m_pOwner->m_szTextDesc) == 0)
	   {
		   ts_time_t tw;
		   AECHAR szTmp[32];
		   
		   TS_GetTimeNow(&tw);
		   
		   WSPRINTF(szTmp, sizeof(szTmp), 
			   L"%04d%02d%02d%02d%02d", tw.year, tw.month, tw.day, tw.hour, tw.minute);
		   
		   WSPRINTF(pme->m_pOwner->m_szTextDesc, sizeof(pme->m_pOwner->m_szTextDesc), 
			   L"%s%02d", szTmp, tw.second);
	   }

	   pme->m_pOwner->m_pPreNaviWin = TSW_WHERE_FUCTION;
	   CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_EX, 0);
	   break;
	   
	  case IDS_STRING_SAVE_LOCATION:
		  {
			  AECHAR prompt[TS_MAX_STRLEN];
			  
			  //校验是否输入经纬度
			  if (WSTRLEN(pme->m_pOwner->m_szTextLat) == 0 
				  || WSTRLEN(pme->m_pOwner->m_szTextLon) == 0 )
			  {
				  AECHAR prompt[TS_MAX_STRLEN];
				  DBGPRINTF("LOCATION DATA ERROR!");//TODO 界面提示
				  
				  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_COORD,prompt,sizeof(prompt));
				  
				  MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
				  WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
				  //TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
				  TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
				  pme->m_nStatus = 1;

				  return TRUE;
			  }
			  
			  //校验经纬度范围
			  if (TS_CheckLat(pme->m_pOwner->m_szTextLat) == FALSE 
				  || TS_CheckLon(pme->m_pOwner->m_szTextLon) == FALSE )
			  {
				  AECHAR prompt[TS_MAX_STRLEN];
				  DBGPRINTF("LOCATION DATA ERROR!");//TODO 界面提示
				  
				  if (TS_CheckLat(pme->m_pOwner->m_szTextLat) == FALSE)
					  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_LAT,prompt,sizeof(prompt));
				  else
					  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_LON,prompt,sizeof(prompt));
				  
				  //提示窗口
				  MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
				  WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
				  //TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
				  TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
				  pme->m_nStatus = 1;

				  return TRUE;
			  }
			  
			  //如果位置名称为空, 则使用默认日志编号格式名称
			  if (WSTRLEN(pme->m_pOwner->m_szTextDesc) == 0)
			  {
				  ts_time_t tw;
				  AECHAR szTmp[32];
				  
				  TS_GetTimeNow(&tw);
				  
				  WSPRINTF(szTmp, sizeof(szTmp), 
					  L"%04d%02d%02d%02d%02d", tw.year, tw.month, tw.day, tw.hour, tw.minute);
				  
				  WSPRINTF(pme->m_pOwner->m_szTextDesc, sizeof(pme->m_pOwner->m_szTextDesc), 
					  L"%s%02d", szTmp, tw.second);
			  }
			  
			  if (!TS_AddExpenseItem(pme->m_pOwner,pme->m_pOwner->m_szTextDesc,pme->m_pOwner->m_szTextLat, pme->m_pOwner->m_szTextLon))
			  {
				  DBGPRINTF("SAVE DATA ERROR!");//TODO 界面提示
				  
				  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_SAVE,prompt,sizeof(prompt));
				  
				  //提示窗口
				  MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
				  WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
				  //TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
				  TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
				  pme->m_nStatus = 1;

				  return TRUE;
			  }  
			  
			  ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SAVE,prompt,sizeof(prompt));
			  
			  //提示窗口
			  MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
			  WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
			  TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver, (void*)pme->m_pOwner);
			  
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

