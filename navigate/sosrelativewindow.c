#include "sosrelativewindow.h"

#define MP_MAX_STRLEN 64
#define MP_NEW_DEST_CY 32

#define MAX_CONTENT_SIZE 30

#define NET_DEST_LIST_ID 3

#define WIN_FONT AEE_FONT_NORMAL

//����״̬����
typedef enum
{
  VIEW_MAIN,
  VIEW_EDIT,
  VIEW_IDLE
} EViewType;

//�༭��
typedef enum
{
  EDIT_A,
  EDIT_B,
  EDIT_C
} EEditType;


// NewDestination window: Displays main menu.
struct CSOSRelativeWin
{
	INHERIT_CWindow(IWindow);

	IMenuCtl *			m_pMainMenu;
	ITextCtl			*m_pTextCtl;

	AECHAR				m_szTextA[MP_MAX_STRLEN];
	AECHAR				m_szTextB[MP_MAX_STRLEN];
	AECHAR				m_szTextC[MP_MAX_STRLEN];

	EEditType			m_eEditType;
	EViewType			m_eViewType;



};

typedef struct CSOSRelativeWin CSOSRelativeWin;

static void       CSOSRelativeWin_Delete(IWindow * po);
static void       CSOSRelativeWin_Enable(IWindow * po, boolean bEnable);
static void       CSOSRelativeWin_Redraw(IWindow * po);
static boolean    CSOSRelativeWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

/*===============================================================================

                     CSOSRelativeWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CSOSRelativeWin_New(CTopSoupApp * pOwner)
{
   CSOSRelativeWin *        pme;
   VTBL(IWindow)     vtbl;

   IWINDOW_SETVTBL(&vtbl, CSOSRelativeWin_Enable, CSOSRelativeWin_Redraw, CSOSRelativeWin_HandleEvent, CSOSRelativeWin_Delete);
   pme = (CSOSRelativeWin *)CWindow_New(sizeof(CSOSRelativeWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	   AEERect   rRect;

		int      cx = pme->m_pOwner->m_cxWidth;
		int      cy = pme->m_pOwner->m_cyHeight;
		int		 dy = MP_NEW_DEST_CY;

		if ((ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu)) ||
			(ISHELL_CreateInstance( pme->m_pIShell, AEECLSID_TEXTCTL, (void **)&pme->m_pTextCtl)) )
			TS_WINERR_RETURN(pme);

		rRect = ((CTopSoupApp*)pme->m_pOwner)->m_rectWin;
		rRect.dy = rRect.dy/3 + 10;

		//MENU
		TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth,&rRect , 0);

		/*
		SETAEERECT( &rRect, 0, pme->m_pOwner->m_nFontHeight, cx, cy - ( 3 * pme->m_pOwner->m_nFontHeight ) );
		IMENUCTL_SetRect( pme->m_pMainMenu, &rRect );

		// Set the style of the menu control so that all the icons are displayed as
		// transparent.  This is set based on the default menu styles.
		ISHELL_GetItemStyle( pme->m_pIShell, AEE_IT_MENU, &rNormalStyle, &rSelStyle );
		rNormalStyle.roImage = AEE_RO_TRANSPARENT;
		rSelStyle.roImage = AEE_RO_TRANSPARENT;
		IMENUCTL_SetStyle( pme->m_pMainMenu, &rNormalStyle, &rSelStyle );
		*/

		//����ʹ�ø��������ݳ�ʼ���������ʼΪ0
		ITEXTCTL_SetRect( pme->m_pTextCtl, &pme->m_pOwner->m_rectWin);

       ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextA,sizeof(pme->m_szTextA));
       ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextB,sizeof(pme->m_szTextB));
       ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextC,sizeof(pme->m_szTextC));
       /*
		if (WSTRLEN(pme->m_pOwner->m_szTextLon) > 0)
		{
			WSTRCPY(pme->m_szTextA, pme->m_pOwner->m_szTextLon);
		}
		else
		{
			WSTRCPY(pme->m_szTextA, L"0.0");
		}

		if (WSTRLEN(pme->m_pOwner->m_szTextLat) > 0)
		{
			WSTRCPY(pme->m_szTextB, pme->m_pOwner->m_szTextLat);
		}
		else
		{
			WSTRCPY(pme->m_szTextB, L"0.0");
		}

		if (WSTRLEN(pme->m_pOwner->m_szTextDesc) > 0)
		{
			WSTRCPY(pme->m_szTextC, pme->m_pOwner->m_szTextDesc);
		}
		else
		{
			ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextC,sizeof(pme->m_szTextC));
		}
        */

		pme->m_eViewType = VIEW_MAIN;

		ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SET_RELATIVE,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
		TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_EDIT);


		//�¼��Ӳ˵�����Ϊ0
		pme->m_pOwner->m_wMenuLastSel[TSW_DEST_NEW_FUCTION] = 0;
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CSOSRelativeWin_Delete(IWindow * po)
{
   CSOSRelativeWin *  pme = (CSOSRelativeWin *)po;

   //XXX __begin
   if(pme->m_pMainMenu)
	   pme->m_pOwner->m_wMenuLastSel[TSW_DEST_NEW] = IMENUCTL_GetSel(pme->m_pMainMenu);
   TS_RELEASEIF(pme->m_pTextCtl);
   TS_RELEASEIF(pme->m_pMainMenu);
   //XXX _end

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CSOSRelativeWin_Enable(IWindow * po, boolean bEnable)
{
   CSOSRelativeWin *  pme = (CSOSRelativeWin *)po;

   if (!CWindow_ProcessEnable(po, bEnable))
	   return;

   //XXX __begin
   if (!pme->m_bActive)
   {
      IMENUCTL_SetActive(pme->m_pMainMenu, FALSE);
      return;
   }

   IMENUCTL_SetActive(pme->m_pMainMenu, TRUE);
   IMENUCTL_SetSel(pme->m_pMainMenu, ((CTopSoupApp*)pme->m_pOwner)->m_wMenuLastSel[TSW_DEST_NEW]);
   //XXX __end

}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CSOSRelativeWin_Redraw(IWindow * po)
{
	CSOSRelativeWin *  pme = (CSOSRelativeWin *)po;

	if (!pme->m_bActive)
		return;

	//XXX __begin
	IDISPLAY_ClearScreen(pme->m_pIDisplay);

	if (pme->m_eViewType == VIEW_MAIN)
	{
		CtlAddItem  ai;
		AECHAR szText[MP_MAX_STRLEN];
		AECHAR szBuf[MP_MAX_STRLEN];

		//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SET_RELATIVE,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
        TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_EDIT);

		TS_DrawBackgroud(po);

		//ITEXTCTL_Reset(pme->m_pTextCtl);

		IMENUCTL_Reset(pme->m_pMainMenu);

		// 1 Fill in the CtlAddItem structure values
		//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LON,szBuf,sizeof(szBuf));
		//WSPRINTF(szText, MP_MAX_STRLEN, L"%s:\t%s", szBuf, pme->m_szTextB);
		WSPRINTF(szText, MP_MAX_STRLEN, L"1.  \t%s", pme->m_szTextB);
		ai.pText = szText;
		ai.pImage = NULL;
		ai.pszResImage = NULL;//KITIMG_RES_FILE;
		ai.pszResText = NULL;//NAVIGATE_RES_FILE;
		ai.wText = 0;//wTextID;
		ai.wFont = AEE_FONT_LARGE;
		ai.wImage = 0;//wImageID;
		ai.wItemID = 0;//wItemID;
		ai.dwData = 0;//dwData;

		// Add the item to the menu control
		IMENUCTL_AddItemEx( pme->m_pMainMenu, &ai );

		// 2 Fill in the CtlAddItem structure values
		//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LAT,szBuf,sizeof(szBuf));
		//WSPRINTF(szText, MP_MAX_STRLEN, L"%s:\t%s", szBuf, pme->m_szTextA);
        WSPRINTF(szText, MP_MAX_STRLEN, L"2.  \t%s", pme->m_szTextA);
		ai.pText = szText;
		ai.pImage = NULL;
		ai.pszResImage = NULL;//KITIMG_RES_FILE;
		ai.pszResText = NULL;//NAVIGATE_RES_FILE;
		ai.wText = 0;//wTextID;
		ai.wFont = AEE_FONT_LARGE;
		ai.wImage = 0;//wImageID;
		ai.wItemID = 1;//wItemID;
		ai.dwData = 0;//dwData;

		// Add the item to the menu control
		IMENUCTL_AddItemEx( pme->m_pMainMenu, &ai );

		// 3 Fill in the CtlAddItem structure values
		//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_DESC,szBuf,sizeof(szBuf));
		//WSPRINTF(szText, MP_MAX_STRLEN, L"%s:\t%s", szBuf, pme->m_szTextC);
        WSPRINTF(szText, MP_MAX_STRLEN, L"3.  \t%s", pme->m_szTextA);
		ai.pText = szText;
		ai.pImage = NULL;
		ai.pszResImage = NULL;//KITIMG_RES_FILE;
		ai.pszResText = NULL;//NAVIGATE_RES_FILE;
		ai.wText = 0;//wTextID;
		ai.wFont = AEE_FONT_LARGE;
		ai.wImage = 0;//wImageID;
		ai.wItemID = 2;//wItemID;
		ai.dwData = 0;//dwData;

		// Add the item to the menu control
		IMENUCTL_AddItemEx( pme->m_pMainMenu, &ai );

		// Active Menu
		ITEXTCTL_SetActive( pme->m_pTextCtl, FALSE);
		IMENUCTL_SetActive( pme->m_pMainMenu, TRUE);
		IMENUCTL_SetSel(pme->m_pMainMenu, ((CTopSoupApp*)pme->m_pOwner)->m_wMenuLastSel[TSW_DEST_NEW]);
		IMENUCTL_Redraw(pme->m_pMainMenu);

        {
            AECHAR bufRes[MP_MAX_STRLEN];
            int a = 0, b = 0;
            int h = 0, xx = 0, yy = 0, dxx = 0, dyy = 0;
            AEERect rect;
            int xMargin = 0, yMargin = 2;

            ISHELL_LoadResString(pme->m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_RELATIVE_INFO_0, bufRes, sizeof(bufRes));
            h = IDISPLAY_GetFontMetrics(pme->m_pIDisplay, WIN_FONT, &a, &b) + yMargin;
            xx = xMargin;
            yy = pme->m_pOwner->m_rectWin.dy/2+36;
            dxx = pme->m_pOwner->m_cxWidth - 2;
            dyy = h;
            SETAEERECT(&rect, xx, yy, dxx, dyy);

            {
                int nFits = 0;
                int len = WSTRLEN(bufRes);
                AECHAR extRes[MP_MAX_STRLEN];

                IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx, &nFits);
                if (nFits < len) {
                    WSTRCPY(extRes, &bufRes[nFits]);
                    bufRes[nFits] = 0;
                    TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

                    WSTRCPY(bufRes, extRes);
                }

                IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx, &nFits);
                if (nFits < len) {
                    xx = xMargin;
                    yy += h;
                    dxx = pme->m_pOwner->m_cxWidth - 2;
                    dyy = h;
                    SETAEERECT(&rect, xx, yy, dxx, dyy);

                    WSTRCPY(extRes, &bufRes[nFits]);
                    bufRes[nFits] = 0;
                    TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);

                    WSTRCPY(bufRes, extRes);
                }

                //IDISPLAY_MeasureTextEx(pme->m_pIDisplay, WIN_FONT, bufRes, -1, dxx,  &nFits);
                //if (nFits < len)
                {
                    xx = xMargin;
                    yy += h;
                    dxx = pme->m_pOwner->m_cxWidth - 2;
                    dyy = h;
                    SETAEERECT(&rect, xx, yy, dxx, dyy);
                    TS_DrawText(pme->m_pIDisplay, WIN_FONT, bufRes, &rect);
                }
            }
        }
	}
	else if (pme->m_eViewType == VIEW_EDIT)
	{
		AEERect rRect;
		int      cx = pme->m_pOwner->m_cxWidth;
		int      cy = pme->m_pOwner->m_cyHeight;


		if (pme->m_pMainMenu)
			pme->m_pOwner->m_wMenuLastSel[TSW_DEST_NEW] = IMENUCTL_GetSel(pme->m_pMainMenu);

		SETAEERECT( &rRect, 0, TS_TITLE_Y, cx, pme->m_pOwner->m_rectWin.dy );

		// DeActive Menu
		IMENUCTL_SetActive( pme->m_pMainMenu, FALSE);
		ITEXTCTL_SetActive( pme->m_pTextCtl, TRUE);

		switch (pme->m_eEditType)
		{
        case EDIT_A:
            //ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LON,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
            TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_OK);
            TS_DrawBackgroud(po);
            ITEXTCTL_SetInputMode( pme->m_pTextCtl, AEE_TM_NUMBERS );
            ITEXTCTL_SetText( pme->m_pTextCtl, pme->m_szTextB, -1);
            break;
                
		case EDIT_B:
			//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LAT,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_OK);
			TS_DrawBackgroud(po);
			ITEXTCTL_SetInputMode( pme->m_pTextCtl, AEE_TM_NUMBERS );
			ITEXTCTL_SetText( pme->m_pTextCtl, pme->m_szTextA, -1);
			break;

		case EDIT_C:
			//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_DESC,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_OK);
			TS_DrawBackgroud(po);
			ITEXTCTL_SetInputMode( pme->m_pTextCtl, AEE_TM_NUMBERS );
			ITEXTCTL_SetText( pme->m_pTextCtl, pme->m_szTextC, -1);
			break;

		default:
			break;
		}
	}

	IDISPLAY_Update(pme->m_pIDisplay);
	//XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CSOSRelativeWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
	CSOSRelativeWin *  pme = (CSOSRelativeWin *)po;
	boolean     bRet = TRUE;

	DBGPRINTF("eCode:%x key:%x", eCode, wParam);

	//��ҳ��
	if (pme->m_eViewType == VIEW_MAIN)
	{
		//XXX __begin
		if ( TS_ISSOFT(eCode)){
			if( AVK_SOFT1 == wParam )
			{
				CTopSoupApp_SetWindow(pme->m_pOwner, TSW_DEST_NEW_FUCTION, 0);
				return TRUE;
			}

			//{

			/*
			//���浽���ݿ��ļ���¼��
			AECHAR * pTextLat=NULL;
			AECHAR * pTextLon=NULL;
			AECHAR * pTextDesc=NULL;
			char	szBuf[64];

			  pTextLat= ITEXTCTL_GetTextPtr( pme->m_pTextLat );
			  pTextLon = ITEXTCTL_GetTextPtr( pme->m_pTextLon );
			  pTextDesc = ITEXTCTL_GetTextPtr( pme->m_pTextDesc );

				WSTRTOSTR(pTextLat, szBuf, WSTRLEN(pTextLat) + 1);
				DBGPRINTF("LAT:%s", szBuf);
				WSTRTOSTR(pTextLon, szBuf, WSTRLEN(pTextLon) + 1);
				DBGPRINTF("LON:%s", szBuf);
				WSTRTOSTR(pTextDesc, szBuf, WSTRLEN(pTextDesc) + 1);
				DBGPRINTF("DESC:%s", szBuf);

				  if (WSTRLEN(pTextLat) == 0 || WSTRLEN(pTextLon) == 0 || WSTRLEN(pTextDesc) == 0)
				  {
				  DBGPRINTF("Re Input!");//TODO
				  return TRUE;
				  }

					if (TS_AddExpenseItem(pme->m_pOwner, pTextDesc, pTextLat, pTextLon))
					{
					//TODO Ӧ����ת���ĸ�ҳ��,����ͣ���ڵ�ǰ?
					CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
					}
					else
					{
					DBGPRINTF("SAVE ERROR!");//TODO
					return TRUE;
					}
				*/
				//return TRUE;
			//}

			if( AVK_SOFT2 == wParam )
			{
				CTopSoupApp_SetWindow(pme->m_pOwner, TSW_SOS, 0);
				return TRUE;
			}
		}


		//ʹ��KEY_SELECT�򿪱༭ҳ��
		if (TS_ISSEL(eCode, wParam))
		{
			uint16 nID = IMENUCTL_GetSel(pme->m_pMainMenu);
			DBGPRINTF("SEL:%d", nID);

			pme->m_eEditType = nID;
			pme->m_eViewType = VIEW_EDIT;

			CSOSRelativeWin_Redraw((IWindow*)pme);

			return TRUE;
		}

		if (TS_ISEVTKEY(eCode))
		{
			return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);
		}
	}
	//�༭ҳ��
	else
	{
		if (ITEXTCTL_HandleEvent( pme->m_pTextCtl, eCode, wParam, dwParam ))
			return TRUE;

		//��Ӧ���ؼ�
		if ( TS_ISSOFT(eCode)){
			if( AVK_SOFT2 == wParam )
			{
				//�л���ҳ��
				pme->m_eEditType = 0;
				pme->m_eViewType = VIEW_MAIN;

				CSOSRelativeWin_Redraw((IWindow*)pme);

				return TRUE;
			}
		}


		//ʹ��KEY_SELECT�򿪱༭ҳ��
		if (TS_ISSEL(eCode, wParam))
		{
			//ȡ�õ�ǰ�༭����,У��ɹ����л���ҳ��
			AECHAR * pText=NULL;
			char	szBuf[64];
			pText = ITEXTCTL_GetTextPtr( pme->m_pTextCtl );
			WSTRTOSTR(pText, szBuf, sizeof(szBuf));

			if (pme->m_eEditType == EDIT_B)
			{
				//У�龭γ��
				if (TS_CheckLat(pText) == FALSE
					|| (WSTRLEN(pText) > 12))
				{
					AECHAR prompt[TS_MAX_STRLEN];
					DBGPRINTF("LOCATION DATA ERROR!");//TODO ������ʾ

					ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_LAT,prompt,sizeof(prompt));

					//��ʾ����
					MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));
					WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);
					//TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
					TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);

					return TRUE;
				}

				WSTRCPY(pme->m_szTextA, pText);
				WSTRCPY(pme->m_pOwner->m_szTextLon, pText);
				DBGPRINTF("GetLat pText:%s", szBuf);
			}
			else if (pme->m_eEditType == EDIT_A)
			{
				//У�龭γ��
				if (TS_CheckLon(pText) == FALSE
					|| (WSTRLEN(pText) > 12))
				{
					AECHAR prompt[TS_MAX_STRLEN];
					DBGPRINTF("LOCATION DATA ERROR!");//TODO ������ʾ

					ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_LON,prompt,sizeof(prompt));

					//��ʾ����
					MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));
					WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);
					//TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
					TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);

					return TRUE;
				}

				WSTRCPY(pme->m_szTextB, pText);
				WSTRCPY(pme->m_pOwner->m_szTextLat, pText);
				DBGPRINTF("GetLon pText:%s", szBuf);
			}
			else if (pme->m_eEditType == EDIT_C)
			{
				//У��Ŀ�ĵ����Ƴ���
				if (WSTRLEN(pText) > 30)
				{
					AECHAR prompt[TS_MAX_STRLEN];
					DBGPRINTF("LOCATION DATA ERROR!");//TODO ������ʾ
					
					ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_DESC,prompt,sizeof(prompt));
					
					//��ʾ����
					MEMSET(pme->m_pOwner->m_pTextctlText,0,sizeof(pme->m_pOwner->m_pTextctlText));	  
					WSTRCPY(pme->m_pOwner->m_pTextctlText, pme->m_pOwner->m_szTextDesc);	   
					//TS_DrawSplash(pme->m_pOwner,prompt,1500,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
					TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
					
					return TRUE;
				}

				WSTRCPY(pme->m_szTextC, pText);
				WSTRCPY(pme->m_pOwner->m_szTextDesc, pText);
				DBGPRINTF("GetDesc pText:%s", szBuf);
			}
			else
			{
				return TRUE;
			}
			
			
			//�л���ҳ��
			pme->m_eEditType = 0;
			pme->m_eViewType = VIEW_MAIN;
			
			CSOSRelativeWin_Redraw((IWindow*)pme);
			
			return TRUE;
		}

	}
	
	return bRet;
}