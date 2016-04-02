#include "wherefuctionwindow.h"



// Main window: Displays main menu.
struct CTextCtlWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	ITextCtl			*m_pTextCtl;	
};

typedef struct CTextCtlWin CTextCtlWin;


static void       CTextCtlWin_Delete(IWindow * po);
static void       CTextCtlWin_Enable(IWindow * po, boolean bEnable);
static void       CTextCtlWin_Redraw(IWindow * po);
static boolean    CTextCtlWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);


/*===============================================================================

                     CTextCtlWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CTextCtlWin_New(CTopSoupApp * pOwner)
{
   CTextCtlWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CTextCtlWin_Enable, CTextCtlWin_Redraw, CTextCtlWin_HandleEvent, CTextCtlWin_Delete);
   pme = (CTextCtlWin *)CWindow_New(sizeof(CTextCtlWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	  //XXX __begin
      //Initialize logo below the header
	   if (ISHELL_CreateInstance( pme->m_pIShell, AEECLSID_TEXTCTL, (void **)&pme->m_pTextCtl))
		   TS_WINERR_RETURN(pme);

	   ITEXTCTL_SetRect( pme->m_pTextCtl, &pme->m_pOwner->m_rectWin);

	   if (pme->m_pOwner->m_op == 0)
	   {
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_EREASE,IDS_STRING_SAVE);	   
	   }
	   else
	   {
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_EREASE,IDS_STRING_SEND);
	   }
	   
	   ITEXTCTL_SetInputMode( pme->m_pTextCtl, pme->m_pOwner->m_pTextctlMode );
	   //XXX __end


   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CTextCtlWin_Delete(IWindow * po)
{
   CTextCtlWin *  pme = (CTextCtlWin *)po;

   //XXX __begin
    TS_RELEASEIF(pme->m_pTextCtl);
   //XXX _end

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CTextCtlWin_Enable(IWindow * po, boolean bEnable)
{
   CTextCtlWin *  pme = (CTextCtlWin *)po;
 

   if (!CWindow_ProcessEnable(po, bEnable))
	   return;


   //XXX __begin
   if (!pme->m_bActive)
   {
      ITEXTCTL_SetActive(pme->m_pTextCtl, FALSE);
      return;
   }

   ITEXTCTL_SetActive(pme->m_pTextCtl, TRUE);
   //XXX __end
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CTextCtlWin_Redraw(IWindow * po)
{
   CTextCtlWin *  pme = (CTextCtlWin *)po;

   if (!pme->m_bActive)
      return;

   
   //XXX __begin
   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   TS_DrawBackgroud(po);
   ITEXTCTL_Redraw(pme->m_pTextCtl);
   

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}


static void CTextCtlWin_onSplashDrawOver(void * po) 
{
	CTopSoupApp* pme = (CTopSoupApp*)po;

    CTopSoupApp_SetWindow(pme, pme->m_pTextctlWin, 0);

}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CTextCtlWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CTextCtlWin *  pme = (CTextCtlWin *)po;
   boolean     bRet = TRUE;

   DBGPRINTF("eCode:%x wParam:%x", eCode, wParam);
   
    //XXX __begin
   if (ITEXTCTL_HandleEvent( pme->m_pTextCtl, eCode, wParam, dwParam ))
	   return TRUE;

   //响应返回键
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT2 == wParam )
	   {
		   CTopSoupApp_SetWindow(pme->m_pOwner, pme->m_pOwner->m_pTextctlWin, 0);
		   return TRUE;
	   }
   }


   //使用KEY_SELECT打开编辑页面
   if (TS_ISSEL(eCode, wParam))
   {
	   AECHAR prompt[TS_MAX_STRLEN];

	   //取得当前编辑内容,校验成功后切回主页面
	   AECHAR * pTextDesc=NULL;
	   AECHAR textLat[32], textLon[32];
	   char	szBuf[32];

	   pTextDesc = ITEXTCTL_GetTextPtr( pme->m_pTextCtl );  
	   WSTRTOSTR(pTextDesc, szBuf, WSTRLEN(pTextDesc) + 1);
	   DBGPRINTF("Desc: %s", szBuf);

	   if (pme->m_pOwner->m_op == 0)	//保存位置
	   {
			TS_FLT2SZ(textLat, pme->m_pOwner->m_gpsInfo.theInfo.lat);
			WSTRTOSTR(textLat, szBuf, WSTRLEN(textLat) + 1);
			DBGPRINTF("Lat: %s", szBuf);

			TS_FLT2SZ(textLon, pme->m_pOwner->m_gpsInfo.theInfo.lon);
			WSTRTOSTR(textLon, szBuf, WSTRLEN(textLon) + 1);
			DBGPRINTF("Lon: %s", szBuf);

			//保存到数据库
			if (WSTRLEN(textLat) == 0 || WSTRLEN(textLon) == 0 || WSTRLEN(pTextDesc) == 0)
			{
			DBGPRINTF("LOCATION DATA ERROR!");//TODO 界面提示
			return TRUE;
			}

			if (!TS_AddExpenseItem(pme->m_pOwner, pTextDesc, textLat, textLon))
			{
			DBGPRINTF("SAVE DATA ERROR!");//TODO 界面提示
			return TRUE;
			}  

			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SAVE,prompt,sizeof(prompt));
	   }
	   else if (pme->m_pOwner->m_op == 1)	//短信发送
	   {

		   CTopSoupApp_SendSMSMessage(pme->m_pOwner, 0, pTextDesc);

		   ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SEND,prompt,sizeof(prompt));
	   }

		//XXX __begin
		IDISPLAY_ClearScreen(pme->m_pIDisplay);

		TS_DrawBackgroud(po);
		ITEXTCTL_SetActive(pme->m_pTextCtl, FALSE);
		//ITEXTCTL_Redraw(pme->m_pTextCtl);


		IDISPLAY_Update(pme->m_pIDisplay);
		//XXX _end

	   //提示窗口
	   MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
	   WSTRCPY(pme->m_pOwner->m_pTextctlText, pTextDesc);	   
	   TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CTextCtlWin_onSplashDrawOver);
   }
   //XXX __end

   return bRet;
}

