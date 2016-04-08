#include "wherefuctionwindow.h"



// Main window: Displays main menu.
struct CTextCtlWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	ITextCtl			*m_pTextCtl;

	uint16              m_wRecID;
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
IWindow * CTextCtlWin_New(CTopSoupApp * pOwner,uint16 wRecID)
{
   CTextCtlWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CTextCtlWin_Enable, CTextCtlWin_Redraw, CTextCtlWin_HandleEvent, CTextCtlWin_Delete);
   pme = (CTextCtlWin *)CWindow_New(sizeof(CTextCtlWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   pme->m_wRecID = wRecID;
   

   {
	  //XXX __begin
      //Initialize logo below the header
	  AECHAR pTextInit[TS_MAX_STRLEN];

	  MEMSET(pTextInit,0,sizeof(pTextInit));
	  if ( pme->m_pOwner->m_op == 2 )
		  TS_GetExpenseItem(pme->m_pOwner,pme->m_wRecID,pTextInit,NULL,NULL); 


	   if (ISHELL_CreateInstance( pme->m_pIShell, AEECLSID_TEXTCTL, (void **)&pme->m_pTextCtl))
		   TS_WINERR_RETURN(pme);

	   ITEXTCTL_SetRect( pme->m_pTextCtl, &pme->m_pOwner->m_rectWin);

	   if (pme->m_pOwner->m_op == 0 || pme->m_pOwner->m_op == 2)
	   {
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_EREASE,IDS_STRING_SAVE);	   
	   }
	   else
	   {
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_EREASE,IDS_STRING_SEND);
	   }
	   
	   ITEXTCTL_SetInputMode( pme->m_pTextCtl, pme->m_pOwner->m_pTextctlMode );
	   ITEXTCTL_SetText(pme->m_pTextCtl,pTextInit,WSTRLEN(pTextInit));
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
   AECHAR pTextInit[TS_MAX_STRLEN];

   if (!pme->m_bActive)
      return;

   
   //XXX __begin
   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   TS_DrawBackgroud(po);
   ITEXTCTL_SetInputMode( pme->m_pTextCtl, pme->m_pOwner->m_pTextctlMode );

   MEMSET(pTextInit,0,sizeof(pTextInit));
   if ( pme->m_pOwner->m_op == 2 )
	   TS_GetExpenseItem(pme->m_pOwner,pme->m_wRecID,pTextInit,NULL,NULL); 
    ITEXTCTL_SetText(pme->m_pTextCtl,pTextInit,WSTRLEN(pTextInit));

   ITEXTCTL_Redraw(pme->m_pTextCtl);
   

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}


static void CTextCtlWin_onSplashDrawOver(void * po) 
{
	CTopSoupApp* pme = (CTopSoupApp*)po;

	if (pme->m_opStatus)
	{	
		CTopSoupApp_SetWindow(pme, pme->m_pTextctlWin, 0);
	}
	else
	{
		CTopSoupApp_SetWindow(pme, TSW_WHERE_FUCTION, 0);
	}

}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CTextCtlWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CTextCtlWin *  pme = (CTextCtlWin *)po;
   boolean     bRet = TRUE;
   int32 pos = 0;
   
   DBGPRINTF("eCode:%x wParam:%x", eCode, wParam);
   
    //XXX __begin
   if (ITEXTCTL_HandleEvent( pme->m_pTextCtl, eCode, wParam, dwParam ))
	   return TRUE;

//   {
//	   AECHAR * pTextDesc=NULL;
//	   pos = ITEXTCTL_GetCursorPos( pme->m_pTextCtl);
//	   pTextDesc = ITEXTCTL_GetTextPtr( pme->m_pTextCtl );  
//
//	   if (WSTRLEN(pTextDesc) == 0 && pos == 0)
//	   {
//		   if (pme->m_pOwner->m_op == 0 || pme->m_pOwner->m_op == 2)
//		   {
//				TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_SAVE);	   
//		   }
//		   else
//		   {
//				TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_SEND);
//		   }
//	   }
//	   else
//	   {
//		   if (pme->m_pOwner->m_op == 0 || pme->m_pOwner->m_op == 2)
//		   {
//				TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_SAVE);	   
//		   }
//		   else
//		   {
//				TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_SEND);
//		   }
//	   }
//   }   
	
   //��Ӧ���ؼ�
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT2 == wParam )
	   {
		   CTopSoupApp_SetWindow(pme->m_pOwner, pme->m_pOwner->m_pTextctlWin, 0);
		   return TRUE;
	   }
   }

   //ʹ��KEY_SELECT�򿪱༭ҳ��
   if (TS_ISSEL(eCode, wParam))
   {
	   AECHAR prompt[TS_MAX_STRLEN];

	   //ȡ�õ�ǰ�༭����,У��ɹ����л���ҳ��
	   AECHAR * pTextDesc=NULL;
	   AECHAR textLat[32], textLon[32];
	   char	szBuf[32];

	   pTextDesc = ITEXTCTL_GetTextPtr( pme->m_pTextCtl );  
	   WSTRTOSTR(pTextDesc, szBuf, sizeof(szBuf));
	   DBGPRINTF("Desc: %s", szBuf);

	   //XXX __begin
	   //IDISPLAY_ClearScreen(pme->m_pIDisplay);
	   //TS_DrawBackgroud(po);
	   //IDISPLAY_Update(pme->m_pIDisplay);
	   //XXX _end
   
	   if (WSTRLEN(pTextDesc) == 0)
	   {
			//��ʾ����
		    pme->m_pOwner->m_opStatus = FALSE;
			MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
			WSTRCPY(pme->m_pOwner->m_pTextctlText, pTextDesc);	   

			if (pme->m_pOwner->m_op == 0 || pme->m_pOwner->m_op == 2)
			{
				ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_DESC,prompt,sizeof(prompt));				
			}
			else
			{
				ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_RECIPIENT,prompt,sizeof(prompt));	
			}
			TS_DrawSplash(pme->m_pOwner,prompt,1000,0,NULL);

		   	return bRet;	   
	   }

	   pme->m_pOwner->m_opStatus = TRUE;

	   if (pme->m_pOwner->m_op == 0 || pme->m_pOwner->m_op == 2)	//0=����λ�� 2=������
  	   {
			TS_FLT2SZ(textLat, pme->m_pOwner->m_gpsInfo.theInfo.lat);
		    //FLOATTOWSTR(pme->m_pOwner->m_gpsInfo.theInfo.lat, textLat, 32);
			WSTRTOSTR(textLat, szBuf, sizeof(szBuf));
			DBGPRINTF("Lat: %s", szBuf);

			TS_FLT2SZ(textLon, pme->m_pOwner->m_gpsInfo.theInfo.lon);
			//FLOATTOWSTR(pme->m_pOwner->m_gpsInfo.theInfo.lon, textLon, 32);
			WSTRTOSTR(textLon, szBuf, sizeof(szBuf));
			DBGPRINTF("Lon: %s", szBuf);

			//�����ж�
			if (WSTRLEN(textLat) == 0 || WSTRLEN(textLon) == 0 || WSTRLEN(pTextDesc) == 0)
			{
				ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_COORD,prompt,sizeof(prompt));
				
				//��ʾ����
				MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
				WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
				//TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
				TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
				return TRUE;

			}

			if ( pme->m_pOwner->m_op == 0 )
			{
				if (!TS_AddExpenseItem(pme->m_pOwner, pTextDesc, textLat, textLon))
				{
					DBGPRINTF("SAVE DATA ERROR!");//TODO ������ʾ
					ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_SAVE,prompt,sizeof(prompt));
					
					//��ʾ����
					MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
					WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
					//TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
					TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
					return TRUE;
				}  

			} else {
				MEMSET(textLat,0,sizeof(textLat));
				MEMSET(textLon,0,sizeof(textLon));
				TS_GetExpenseItem(pme->m_pOwner,pme->m_wRecID,NULL,textLat,textLon);
				if (!TS_UpdateExpenseItem(pme->m_pOwner, pme->m_wRecID,pTextDesc, textLat, textLon))
				{
					DBGPRINTF("UPDATE DATA ERROR!");//TODO ������ʾ
					ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_UPDATE,prompt,sizeof(prompt));
					
					//��ʾ����
					MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
					WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
					//TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
					TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
					return TRUE;
				}  
			}

			ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SAVE,prompt,sizeof(prompt));
	   }
	   else if (pme->m_pOwner->m_op == 1 || pme->m_pOwner->m_op == 3)	//���ŷ���
	   {
		   char szNum[TS_MAX_STRLEN];
		   WSTRTOSTR(pTextDesc, szNum, TS_MAX_STRLEN);

		   if( pme->m_pOwner->m_op == 3 ) {
			   AECHAR lat[TS_MAX_STRLEN];
			   AECHAR lon[TS_MAX_STRLEN];
			   AECHAR desc[TS_MAX_STRLEN];
			   
			   MEMSET(lat,0,sizeof(lat));
			   MEMSET(lon,0,sizeof(lon));
			   MEMSET(desc,0,sizeof(desc));
			   
			   TS_GetExpenseItem(pme->m_pOwner,pme->m_wRecID,desc,lat,lon);
			   CTopSoupApp_SendSMSMessage(pme->m_pOwner, 0, desc,lat,lon,szNum);  //XXX
		   }
		   else
		   {
			   //�����ж�
			   if (WSTRLEN(pme->m_pOwner->m_szTextLat) == 0 || WSTRLEN(pme->m_pOwner->m_szTextLon) == 0 || WSTRLEN(pme->m_pOwner->m_szTextDesc) == 0)
			   {
				   ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_COORD,prompt,sizeof(prompt));
				   
				   //��ʾ����
				   MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
				   WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
				   //TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
				   TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
				   return TRUE;
				   
			   }
			   
			   CTopSoupApp_SendSMSMessage(pme->m_pOwner, 0, pme->m_pOwner->m_szTextDesc, pme->m_pOwner->m_szTextLat, pme->m_pOwner->m_szTextLon,szNum);  //XXX
		   }
				
		  
		   ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SEND,prompt,sizeof(prompt));
	   }

	   //��ʾ����
	   MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
	   WSTRCPY(pme->m_pOwner->m_pTextctlText, pTextDesc);	   
	   TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CTextCtlWin_onSplashDrawOver,(void*)pme->m_pOwner );
   }
   //XXX __end

   return bRet;
}

