#include "sosrelativewindow.h"

#define MP_MAX_STRLEN 64
#define MP_NEW_DEST_CY 32

#define MAX_CONTENT_SIZE 30

#define NET_DEST_LIST_ID 3

#define WIN_FONT AEE_FONT_NORMAL



//界面状态控制
typedef enum
{
  VIEW_MAIN,
  VIEW_EDIT,
  VIEW_IDLE
} EViewType;

//编辑项
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

/************************************************************************/
/* 从配置文件加载亲友联系方式                                           */
/************************************************************************/
static uint32 LoadConfig(CSOSRelativeWin *pme);

/************************************************************************/
/* 保存亲友联系方式到配置文件                                           */
/************************************************************************/
static uint32 SaveConfig(CSOSRelativeWin *pme);

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

		//尝试使用父窗口数据初始化，否则初始为0
		ITEXTCTL_SetRect( pme->m_pTextCtl, &pme->m_pOwner->m_rectWin);

       MEMSET(pme->m_pOwner->m_szTextA, 0, sizeof(pme->m_pOwner->m_szTextA));
       MEMSET(pme->m_pOwner->m_szTextB, 0, sizeof(pme->m_pOwner->m_szTextB));
       MEMSET(pme->m_pOwner->m_szTextC, 0, sizeof(pme->m_pOwner->m_szTextC));

       LoadConfig(pme);
       DBGPRINTF("LoadConfig Over");
		pme->m_eViewType = VIEW_MAIN;

		ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SET_RELATIVE,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
		TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_EDIT);
		//下级子菜单重置为0
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

        if (WSTRLEN(pme->m_szTextA) == 0)
        {
            ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextA,sizeof(pme->m_szTextA));
        }

        if (WSTRLEN(pme->m_szTextB) == 0)
        {
            ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextB,sizeof(pme->m_szTextB));
        }

        if (WSTRLEN(pme->m_szTextC) == 0)
        {
            ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_NOT_SET,pme->m_szTextC,sizeof(pme->m_szTextC));
        }

		//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SET_RELATIVE,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
        TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_EDIT);

		TS_DrawBackgroud(po);

		//ITEXTCTL_Reset(pme->m_pTextCtl);

		IMENUCTL_Reset(pme->m_pMainMenu);

		// 1 Fill in the CtlAddItem structure values
		//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LON,szBuf,sizeof(szBuf));
		//WSPRINTF(szText, MP_MAX_STRLEN, L"%s:\t%s", szBuf, pme->m_szTextA);
		WSPRINTF(szText, MP_MAX_STRLEN, L"1.  \t%s", pme->m_szTextA);
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
		//WSPRINTF(szText, MP_MAX_STRLEN, L"%s:\t%s", szBuf, pme->m_szTextB);
        WSPRINTF(szText, MP_MAX_STRLEN, L"2.  \t%s", pme->m_szTextB);
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
        WSPRINTF(szText, MP_MAX_STRLEN, L"3.  \t%s", pme->m_szTextC);
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

        //绘制提示信息
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
            if (WSTRLEN(pme->m_pOwner->m_szTextA) > 0) {
                ITEXTCTL_SetText(pme->m_pTextCtl, pme->m_pOwner->m_szTextA, -1);
            }
            else
            {
                ITEXTCTL_SetText(pme->m_pTextCtl, L"", -1);
            }
            break;
                
		case EDIT_B:
			//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_LAT,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_OK);
			TS_DrawBackgroud(po);
			ITEXTCTL_SetInputMode( pme->m_pTextCtl, AEE_TM_NUMBERS );
            if (WSTRLEN(pme->m_pOwner->m_szTextB) > 0) {
                ITEXTCTL_SetText(pme->m_pTextCtl, pme->m_pOwner->m_szTextB, -1);
            }
            else
            {
                ITEXTCTL_SetText(pme->m_pTextCtl, L"", -1);
            }
			break;

		case EDIT_C:
			//ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_EDIT_DESC,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
			TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_OK);
			TS_DrawBackgroud(po);
			ITEXTCTL_SetInputMode( pme->m_pTextCtl, AEE_TM_NUMBERS );
            if (WSTRLEN(pme->m_pOwner->m_szTextC) > 0) {
                ITEXTCTL_SetText(pme->m_pTextCtl, pme->m_pOwner->m_szTextC, -1);
            }
            else
            {
                ITEXTCTL_SetText(pme->m_pTextCtl, L"", -1);
            }
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

	//主页面
	if (pme->m_eViewType == VIEW_MAIN)
	{
		//XXX __begin
		if ( TS_ISSOFT(eCode)){
			if( AVK_SOFT2 == wParam )
			{
				CTopSoupApp_SetWindow(pme->m_pOwner, TSW_SOS, 0);
				return TRUE;
			}
		}


		//使用KEY_SELECT打开编辑页面
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
	//编辑页面
	else
	{
		if (ITEXTCTL_HandleEvent( pme->m_pTextCtl, eCode, wParam, dwParam ))
			return TRUE;

		//响应返回键
		if ( TS_ISSOFT(eCode)){
			if( AVK_SOFT2 == wParam )
			{
				//切回主页面
				pme->m_eEditType = 0;
				pme->m_eViewType = VIEW_MAIN;

				CSOSRelativeWin_Redraw((IWindow*)pme);

				return TRUE;
			}
		}


		//使用KEY_SELECT打开编辑页面
		if (TS_ISSEL(eCode, wParam))
		{
			//取得当前编辑内容,校验成功后切回主页面
			AECHAR * pText=NULL;
			char	szBuf[64];
			pText = ITEXTCTL_GetTextPtr( pme->m_pTextCtl );
			WSTRTOSTR(pText, szBuf, sizeof(szBuf));

            //校验联系方式长度
            if ((WSTRLEN(pText) > TS_MAX_RELATIVE_NUM))
            {
                AECHAR prompt[TS_MAX_STRLEN];
                DBGPRINTF("Address Length ERROR!");

                ISHELL_LoadResString(pme->m_pOwner->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_PHONE_NUM,prompt,sizeof(prompt));

                //提示窗口
                TS_DrawSplash(pme->m_pOwner,prompt,1500,0, 0);
                return TRUE;
            }

            if (pme->m_eEditType == EDIT_A)
            {
                WSTRCPY(pme->m_szTextA, pText);
                WSTRCPY(pme->m_pOwner->m_szTextA, pText);
                DBGPRINTF("GetB pText:%s", szBuf);
            }
			else if (pme->m_eEditType == EDIT_B)
            {
                WSTRCPY(pme->m_szTextB, pText);
                WSTRCPY(pme->m_pOwner->m_szTextB, pText);
                DBGPRINTF("GetA pText:%s", szBuf);
            }
			else if (pme->m_eEditType == EDIT_C)
			{
				WSTRCPY(pme->m_szTextC, pText);
                WSTRCPY(pme->m_pOwner->m_szTextC, pText);
				DBGPRINTF("GetC pText:%s", szBuf);
			}
			else
			{
				return TRUE;
			}

			//切回主页面
			pme->m_eEditType = 0;
			pme->m_eViewType = VIEW_MAIN;
			SaveConfig(pme);
			CSOSRelativeWin_Redraw((IWindow*)pme);
			
			return TRUE;
		}

	}
	
	return bRet;
}


/************************************************************************/
/* 从配置文件加载亲友联系方式                                           */
/************************************************************************/
static uint32 LoadConfig(CSOSRelativeWin *pme)
{
    IFileMgr	*pIFileMgr = NULL;
    IFile		*pIFile = NULL;
    IShell		*pIShell = NULL;

    char 	szBuf[128];
    char    *pszBuf = NULL;
    char    *pszTok = NULL;
    int32	nResult = 0;
    FileInfo	fiInfo;
    char    szA[32], szB[32], szC[32];
    uint32 len = 0;

    pIShell = pme->m_pIShell;

    // Create the instance of IFileMgr
    nResult = ISHELL_CreateInstance(pIShell, AEECLSID_FILEMGR, (void**)&pIFileMgr);
    if (SUCCESS != nResult) {
        DBGPRINTF("Create AEECLSID_FILEMGR Failed!");
        return nResult;
    }

    nResult = IFILEMGR_Test(pIFileMgr, RELATIVE_ADDRESS_CFG);
    if (nResult != SUCCESS)
    {
        DBGPRINTF("CONFIG NOT EXIST!");
        IFILEMGR_Release(pIFileMgr);

        return SUCCESS;
    }

    pIFile = IFILEMGR_OpenFile(pIFileMgr, RELATIVE_ADDRESS_CFG, _OFM_READ);
    if (!pIFile) {
        DBGPRINTF("Open Configure File Failed! %s", RELATIVE_ADDRESS_CFG);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    if (SUCCESS != IFILE_GetInfo(pIFile, &fiInfo)) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    if (fiInfo.dwSize == 0) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    //only use 128
    len = fiInfo.dwSize;
    if (len >= 128)
        len = 127;

    MEMSET(szBuf, 0, 128);
    nResult = IFILE_Read(pIFile, szBuf, len);
    if ((uint32)nResult < len) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    pszBuf = szBuf;

    //查找第一个联系人号码
    MEMSET(szA,0,sizeof(szA));
    pszTok = STRCHR(pszBuf, '#');
    if (pszTok == NULL) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }
    len = pszTok-pszBuf;
    if (len > 1) {
        MEMCPY(szA, pszBuf, len * sizeof(char));
        szA[len] = 0;
    }
    pszBuf = pszTok + 1;
    DBGPRINTF("szA:%s", szA);

    //查找第二个联系人号码
    MEMSET(szB,0,sizeof(szB));
    pszTok = STRCHR(pszBuf, '#');
    if (pszTok == NULL) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }
    len = pszTok-pszBuf;
    if (len > 1) {
        MEMCPY(szB, pszBuf, len * sizeof(char));
        szB[len] = 0;
    }
    pszBuf = pszTok + 1;
    DBGPRINTF("szB:%s", szB);

    //查找第三个联系人号码
    MEMSET(szC,0,sizeof(szC));
    len = fiInfo.dwSize-(pszBuf-szBuf);
    if (len > TS_MIN_RELATIVE_NUM && len < TS_MAX_RELATIVE_NUM)
    {
        MEMCPY(szC, pszBuf, len);
        szC[len] = 0;
    }
    DBGPRINTF("szC:%s", szC);

    if (STRLEN(szA) > 0)
    {
        STRTOWSTR(szA, pme->m_szTextA, sizeof(pme->m_szTextA));
        WSTRCPY(pme->m_pOwner->m_szTextA, pme->m_szTextA);
    }

    if (STRLEN(szB) > 0)
    {
        STRTOWSTR(szB, pme->m_szTextB, sizeof(pme->m_szTextB));
        WSTRCPY(pme->m_pOwner->m_szTextB, pme->m_szTextB);
    }

    if (STRLEN(szC) > 0)
    {
        STRTOWSTR(szC, pme->m_szTextC, sizeof(pme->m_szTextC));
        WSTRCPY(pme->m_pOwner->m_szTextC, pme->m_szTextC);
    }

	IFILE_Release(pIFile);
	IFILEMGR_Release(pIFileMgr);
	return SUCCESS;
}

/************************************************************************/
/* 保存亲友联系方式到配置文件                                           */
/************************************************************************/
static uint32 SaveConfig(CSOSRelativeWin *pme)
{

    IFileMgr	*pIFileMgr = NULL;
    IFile		*pIFile = NULL;
    IShell		*pIShell = NULL;

    int32	nResult = 0;
    char    szBuf[64];
    char    szA[32], szB[32], szC[32];
    uint32     len = 0;

    pIShell = pme->m_pIShell;

    // Create the instance of IFileMgr
    nResult = ISHELL_CreateInstance(pIShell, AEECLSID_FILEMGR, (void**)&pIFileMgr);
    if (SUCCESS != nResult) {
        DBGPRINTF("Create AEECLSID_FILEMGR Failed!");
        return nResult;
    }

    nResult = IFILEMGR_Test(pIFileMgr, RELATIVE_ADDRESS_CFG);
    if (nResult == SUCCESS)
    {
        DBGPRINTF("REMOVE CONFIG!");
        IFILEMGR_Remove(pIFileMgr, RELATIVE_ADDRESS_CFG);
    }

    pIFile = IFILEMGR_OpenFile(pIFileMgr, RELATIVE_ADDRESS_CFG, _OFM_CREATE);
    if (!pIFile) {
        DBGPRINTF("Create Configure File Failed! %s", RELATIVE_ADDRESS_CFG);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    MEMSET(szA,0,sizeof(szA));
    MEMSET(szB,0,sizeof(szB));
    MEMSET(szC,0,sizeof(szC));
    if (WSTRLEN(pme->m_pOwner->m_szTextA) > 0)
    {
        WSTRTOSTR(pme->m_pOwner->m_szTextA, szA, sizeof(szA));
    }

    if (WSTRLEN(pme->m_pOwner->m_szTextB) > 0)
    {
        WSTRTOSTR(pme->m_pOwner->m_szTextB, szB, sizeof(szB));
    }

    if (WSTRLEN(pme->m_pOwner->m_szTextC) > 0)
    {
        WSTRTOSTR(pme->m_pOwner->m_szTextC, szC, sizeof(szC));
    }

    //构建配置内容: A#B#C
    SPRINTF(szBuf, "%s#%s#%s", szA, szB, szC);
    len = STRLEN(szBuf);
    DBGPRINTF("Save Relative address:%s len:%d", szBuf, len);

    nResult = IFILE_Write(pIFile, szBuf, len);
    if ((uint32)nResult < len) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    IFILE_Release(pIFile);
    IFILEMGR_Release(pIFileMgr);

    return SUCCESS;
}