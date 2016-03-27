#include "newdestwindow.h"

#define MP_MAX_STRLEN 64
#define MP_NEW_DEST_CY 32

#define MAX_CONTENT_SIZE 30

#define NET_DEST_LIST_ID 3


// NewDestination window: Displays main menu.
struct CNewDestWin
{
	INHERIT_CWindow(IWindow);

	AECHAR				m_szText[MP_MAX_STRLEN];
	IMenuCtl *			m_pMainMenu;

	ITextCtl			*m_pText;
};

typedef struct CNewDestWin CNewDestWin;

static void       CNewDestWin_Delete(IWindow * po);
static void       CNewDestWin_Enable(IWindow * po, boolean bEnable);
static void       CNewDestWin_Redraw(IWindow * po);
static boolean    CNewDestWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

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
		int      cx = pme->m_pOwner->m_cxWidth;
		int      cy = pme->m_pOwner->m_cyHeight;
		int		 dy = MP_NEW_DEST_CY;
		AEERect  rRect;
		AEEItemStyle rNormalStyle;
		AEEItemStyle rSelStyle;
	   
		if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
			TS_WINERR_RETURN(pme);
		
		SETAEERECT( &rRect, 0, pme->m_pOwner->m_nFontHeight, cx, cy - ( 3 * pme->m_pOwner->m_nFontHeight ) );
		IMENUCTL_SetRect( pme->m_pMainMenu, &rRect );
		
		// Set the style of the menu control so that all the icons are displayed as
		// transparent.  This is set based on the default menu styles.
		ISHELL_GetItemStyle( pme->m_pIShell, AEE_IT_MENU, &rNormalStyle, &rSelStyle );
		rNormalStyle.roImage = AEE_RO_TRANSPARENT;
		rSelStyle.roImage = AEE_RO_TRANSPARENT;
		IMENUCTL_SetStyle( pme->m_pMainMenu, &rNormalStyle, &rSelStyle );   
	   
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
   TS_RELEASEIF(pme->m_pText);

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

	{
		CtlAddItem  ai;

		// Fill in the CtlAddItem structure values
		ai.pText = L"Lat: 0.0";
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


				// Fill in the CtlAddItem structure values
		ai.pText = L"Lon: 0.0";
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



				// Fill in the CtlAddItem structure values
		ai.pText = L"Dest: 0.0";
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
	}
    // Active Menu
	IMENUCTL_SetActive( pme->m_pMainMenu, TRUE);
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

	DBGPRINTF("eCode:%x key:%x", eCode, wParam);

	//When receive sel changed event, update menu title with cur item desc.
   if (TS_ISSEL(eCode, wParam))
   {          
	   uint16 nID = IMENUCTL_GetSel(pme->m_pMainMenu);
	   DBGPRINTF("SEL:%d", nID);
	   return TRUE;
   }

   if (TS_ISEVTKEY(eCode)) 
   {
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);
   }
	
	//XXX __begin
	if ( TS_ISSOFT(eCode)){
		if( AVK_SOFT1 == wParam )
		{
			/*
			//保存到数据库文件记录中
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
				//TODO 应该跳转到哪个页面,还是停留在当前?
				CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
			}
			else
			{
				DBGPRINTF("SAVE ERROR!");//TODO
				return TRUE;
			}
			*/
			return TRUE;
		}

		//退出程序
		if( AVK_SOFT2 == wParam )
		{
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
			return TRUE;
		}
	}
	

	return bRet;
}