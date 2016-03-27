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


	IStatic *			m_pStaticLat;	  // 经度
	IStatic *			m_pStaticLon;	  // 纬度
	IStatic *			m_pStaticDesc;	  // 描述

	ITextCtl*			m_pTextLat;       // 经度
    ITextCtl*			m_pTextLon;       // 纬度
	ITextCtl*			m_pTextDesc;      // 描述

	CAmountCtl*             m_pAmount;      // Amount Custom Control
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
		AEERect  rect, rRect;
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
	
		  // Create the CAmountCtl control.
		if(((pme->m_pAmount = CAMOUNTCTL_CreateInstance(pme->m_pIShell)) == NULL) || 
			(ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pText)) != SUCCESS ) )
			TS_WINERR_RETURN(pme);

		//XXX __begin
		/*
		if ((ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void**)(&pme->m_pStaticLat)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void**)(&pme->m_pStaticLon)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void**)(&pme->m_pStaticDesc)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextLat)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextLon)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextDesc)) != SUCCESS ))
		   TS_WINERR_RETURN(pme);

		SETAEERECT(&rect, 0, dy, cx/4, MP_NEW_DEST_CY);
		ISTATIC_SetRect(pme->m_pStaticLat, &rect);

		dy += MP_NEW_DEST_CY;
		SETAEERECT(&rect, 0, dy, cx/4, MP_NEW_DEST_CY);
		ISTATIC_SetRect(pme->m_pStaticLon, &rect);

		dy += MP_NEW_DEST_CY;
		SETAEERECT(&rect, 0, dy, cx/4, MP_NEW_DEST_CY);
		ISTATIC_SetRect(pme->m_pStaticDesc, &rect);

		// Let's reset text control size so on subsequent this won't misbehave by erasing screen
		SETAEERECT(&rect, 0, 0, 0, 0);
		ITEXTCTL_SetRect(pme->m_pTextLat, &rect);
		ITEXTCTL_SetRect(pme->m_pTextLon, &rect);
		ITEXTCTL_SetRect(pme->m_pTextDesc, &rect);

		ISTATIC_SetRect(pme->m_pStaticLat, &rect);
		ISTATIC_SetRect(pme->m_pStaticLon, &rect);
		ISTATIC_SetRect(pme->m_pStaticDesc, &rect);
		
		TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_SELECT,IDS_STRING_BACK,0);
		*/
		
		//if ((ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextLat)) != SUCCESS))
		//   TS_WINERR_RETURN(pme);


		// Let's reset text control size so on subsequent this won't misbehave by erasing screen
		SETAEERECT(&rect, 0, 0, 0, 0);
		ITEXTCTL_SetRect(pme->m_pText, &rect);

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
   TS_RELEASEIF(pme->m_pStaticLat);
   TS_RELEASEIF(pme->m_pStaticLon);
   TS_RELEASEIF(pme->m_pStaticDesc);
   TS_RELEASEIF(pme->m_pTextLat);
   TS_RELEASEIF(pme->m_pTextLon);
   TS_RELEASEIF(pme->m_pTextDesc);

   TS_RELEASEIF(pme->m_pText);

   if (pme->m_pAmount)
	CAMOUNTCTL_Release(pme->m_pAmount);

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
   /*ISTATIC_SetActive(pme->m_pStaticLat, bEnable);
   ISTATIC_SetActive(pme->m_pStaticLon, bEnable);
   ISTATIC_SetActive(pme->m_pStaticDesc, bEnable);
   */
   //ITEXTCTL_SetActive(pme->m_pTextLat, bEnable);
   //ITEXTCTL_SetActive(pme->m_pTextLon, bEnable);
   //ITEXTCTL_SetActive(pme->m_pTextDesc, bEnable);
   //XXX __end

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
	AEERect  rRect;
	int		 dy = MP_NEW_DEST_CY*3;

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

/*	TS_DrawBackgroud(po);
	
	STRTOWSTR("Lat: ", pme->m_szText, sizeof(pme->m_szText));
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pStaticLat, AEE_FONT_NORMAL, pme->m_szText);
	
	STRTOWSTR("Lon: ", pme->m_szText, sizeof(pme->m_szText));
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pStaticLon, AEE_FONT_NORMAL, pme->m_szText);

	STRTOWSTR("Name: ", pme->m_szText, sizeof(pme->m_szText));
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pStaticDesc, AEE_FONT_NORMAL, pme->m_szText);

*/


	// 1 Display lat input text
	//ISTATIC_GetRect(pme->m_pStaticLat, &rRect );
//    ITEXTCTL_Reset( pme->m_pTextLat );
//	ITEXTCTL_SetTitle(pme->m_pTextLat,NAVIGATE_RES_FILE,IDS_STRING_LAT,NULL);
//    ITEXTCTL_SetProperties( pme->m_pTextLat, TP_FRAME );
//
//    // Set the starting X coordinate position of the control and its width (screen width
//    // minus the width of the label string).
//    rRect.x = 2 + rRect.dx;
//    rRect.dx = pme->m_pOwner->m_cxWidth - rRect.x -2;
//	
//	SETAEERECT(&rRect, 0, dy, pme->m_pOwner->m_cxWidth/2, MP_NEW_DEST_CY);
//
//	DBGPRINTF("latRect:(%d %d, %d-%d)", rRect.x, rRect.y, rRect.dx, rRect.dy);
//
//    ITEXTCTL_SetRect(pme->m_pTextLat, &rRect );
//    ITEXTCTL_SetInputMode( pme->m_pTextLat, AEE_TM_NUMBERS );
//    //pMe->m_nInputMode = IMT_LITTLE_CASE_LETTER;
//    ITEXTCTL_SetMaxSize( pme->m_pTextLat, 16 );
//    ITEXTCTL_SetActive( pme->m_pTextLat, FALSE );
//    ITEXTCTL_Redraw( pme->m_pTextLat);

   /****
	SETAEERECT(&rRect, 0, MP_NEW_DEST_CY, pme->m_pOwner->m_cxWidth/2, MP_NEW_DEST_CY);

	// Display Description Control
	ITEXTCTL_Reset( pme->m_pText );
	ITEXTCTL_SetProperties( pme->m_pText, TP_FRAME );
	ITEXTCTL_SetInputMode( pme->m_pText, AEE_TM_PINYIN );
	ITEXTCTL_SetMaxSize( pme->m_pText, MAX_DESC_SIZE );
	ICONTROL_SetRect( (IControl *)pme->m_pText, &rRect );
	ICONTROL_Redraw( (IControl *)pme->m_pText );
	ICONTROL_SetActive( (IControl *)pme->m_pText, TRUE );


	SETAEERECT(&rRect, 0, MP_NEW_DEST_CY*2, pme->m_pOwner->m_cxWidth/2, MP_NEW_DEST_CY);

	CAMOUNTCTL_SetValue( pme->m_pAmount, 0 );
	CAMOUNTCTL_SetRect( pme->m_pAmount, &rRect );
    CAMOUNTCTL_Redraw( pme->m_pAmount );
	CAMOUNTCTL_SetActive( pme->m_pAmount, FALSE);
	***/

/*
	// 2 Display lon input text
	ISTATIC_GetRect(pme->m_pStaticLon, &rRect );
    ITEXTCTL_Reset( pme->m_pTextLon );
	ITEXTCTL_SetTitle(pme->m_pTextLon,NAVIGATE_RES_FILE,IDS_STRING_LON,NULL);
    ITEXTCTL_SetProperties( pme->m_pTextLon, TP_FRAME );

    // Set the starting X coordinate position of the control and its width (screen width
    // minus the width of the label string).
    rRect.x = 2 + rRect.dx;
    rRect.dx = pme->m_pOwner->m_cxWidth - rRect.x -2;
	//ITEXTCTL_SetText(pme->m_pTextLon, L"0.0",  MAX_CONTENT_SIZE);
	dy += MP_NEW_DEST_CY;
	SETAEERECT(&rRect, 0, dy, pme->m_pOwner->m_cxWidth/2, MP_NEW_DEST_CY);
	DBGPRINTF("lonRect:(%d %d, %d-%d)", rRect.x, rRect.y, rRect.dx, rRect.dy);
    ITEXTCTL_SetRect(pme->m_pTextLon, &rRect );
    ITEXTCTL_SetInputMode( pme->m_pTextLon, AEE_TM_NUMBERS );
    //pMe->m_nInputMode = IMT_LITTLE_CASE_LETTER;
    ITEXTCTL_SetMaxSize( pme->m_pTextLon, 16 );
    ITEXTCTL_SetActive( pme->m_pTextLon, FALSE );
    ITEXTCTL_Redraw( pme->m_pTextLon);
	

	// 3 Display desc input text
	ISTATIC_GetRect(pme->m_pStaticDesc, &rRect );
    ITEXTCTL_Reset( pme->m_pTextDesc );
	ITEXTCTL_SetTitle(pme->m_pTextDesc,NAVIGATE_RES_FILE,IDS_STRING_NAME,NULL);
    ITEXTCTL_SetProperties( pme->m_pTextDesc, TP_FRAME );

    // Set the starting X coordinate position of the control and its width (screen width
    // minus the width of the label string).
    rRect.x = 2 + rRect.dx;
    rRect.dx = pme->m_pOwner->m_cxWidth - rRect.x -2;
	//ITEXTCTL_SetText(pme->m_pTextDesc, L"0.0",  MAX_CONTENT_SIZE);
	dy += MP_NEW_DEST_CY;
	SETAEERECT(&rRect, 0, dy, pme->m_pOwner->m_cxWidth/2, MP_NEW_DEST_CY);
	DBGPRINTF("descRect:(%d %d, %d-%d)", rRect.x, rRect.y, rRect.dx, rRect.dy);
    ITEXTCTL_SetRect(pme->m_pTextDesc, &rRect );
    ITEXTCTL_SetInputMode( pme->m_pTextDesc, AEE_TM_NUMBERS );
    //pMe->m_nInputMode = IMT_LITTLE_CASE_LETTER;
    ITEXTCTL_SetMaxSize( pme->m_pTextDesc, 16 );
    ITEXTCTL_SetActive( pme->m_pTextDesc, FALSE );
    ITEXTCTL_Redraw( pme->m_pTextDesc);

*/

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
	int len;
	int32 curPos;

	DBGPRINTF("eCode:%x key:%x", eCode, wParam);

	//When receive sel changed event, update menu title with cur item desc.
   if (TS_ISSEL(eCode, wParam))
   {            
	   AECHAR psTitle[ MAX_DESC_SIZE + 1 ];            
	   uint16 nID = IMENUCTL_GetSel(pme->m_pMainMenu);
	   DBGPRINTF("SEL:%d", nID);
	   return TRUE;
   }

   if (TS_ISEVTKEY(eCode)) 
   {
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);
   }

	// Allow the left and right arrow keys to be used to move from Description field 
	// to other fields, if the cursor is at the start or end of the Description field
	curPos = ITEXTCTL_GetCursorPos(pme->m_pText);
	len = WSTRLEN(ITEXTCTL_GetTextPtr(pme->m_pText));
	if (ITEXTCTL_HandleEvent(pme->m_pText, eCode, wParam, dwParam))
	{
		// If there is text, then make sure the cursor is at the start or end
		if (len)
		{
			// If the cursor is anywhere between the start and end, let the control handle
			// the event
			if (curPos >= TC_CURSORSTART + 1 &&    // cursor beyond start
				curPos <= len - 1)             // cursor before end
				return TRUE;
			
			// Check the left and right arrow and CLR key events
			switch( wParam)
			{
			case AVK_RIGHT:
				if (curPos == TC_CURSORSTART || curPos == (len - 1))
					return TRUE;
				break;
			case AVK_LEFT:
				if (curPos == (TC_CURSORSTART + 1) || curPos == len)
					return TRUE;
				break;
			case AVK_CLR:
				return TRUE;
			}
		}
	}

	if (CAMOUNTCTL_HandleEvent(pme->m_pAmount, eCode, wParam, dwParam))
      return TRUE;

	/*	
	// text控件处理DOWN键后，后面程序继续处理
    if (ITEXTCTL_HandleEvent(pme->m_pTextLat, eCode, wParam, dwParam))
    {
        if ( eCode==EVT_KEY && (wParam == AVK_DOWN || wParam == AVK_UP))
        {
            // 不返回TRUE，使后面程序继续处理
        }
        else
        {
            return TRUE;
        }
		DBGPRINTF("LAT eCode:%x key:%x", eCode, wParam);
		return TRUE;
    }

    // text控件处理DOWN键后，后面程序继续处理
    if (ITEXTCTL_HandleEvent(pme->m_pTextLon, eCode, wParam, dwParam))
    {
        if ( eCode==EVT_KEY && (wParam == AVK_DOWN || wParam == AVK_UP))
        {
            // 不返回TRUE，使后面程序继续处理
        }
        else
        {
            return TRUE;
        }

		DBGPRINTF("LON eCode:%x key:%x", eCode, wParam);

		return TRUE;
    }
	
	// text控件处理DOWN键后，后面程序继续处理
    if (ITEXTCTL_HandleEvent(pme->m_pTextDesc, eCode, wParam, dwParam))
    {
        if ( eCode==EVT_KEY && (wParam == AVK_DOWN || wParam == AVK_UP))
        {
            // 不返回TRUE，使后面程序继续处理
        }
        else
        {
            return TRUE;
        }

		DBGPRINTF("DESC eCode:%x key:%x", eCode, wParam);
		return TRUE;
    }
	//*/
	
	//XXX __begin
	if ( TS_ISSOFT(eCode)){
		if( AVK_SOFT1 == wParam )
		{
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

			return TRUE;
		}
		//退出程序
		if( AVK_SOFT2 == wParam )
		{
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
			return TRUE;
		}
	}
	
	switch (eCode)
	{
	case EVT_KEY:
		// Add your code here...
		switch(wParam)
		{		
			/*
			case AVK_UP:  // Handle 'UP' button presses if the text control has focus.
				{
					// focus switch
					if (ITEXTCTL_IsActive(pme->m_pTextLat))
					{
						ITEXTCTL_SetActive(pme->m_pTextLat, FALSE);
						ITEXTCTL_Redraw(pme->m_pTextLat);
						
						ITEXTCTL_SetActive(pme->m_pTextDesc, TRUE);
						ITEXTCTL_Redraw(pme->m_pTextDesc);
						return TRUE;
					}
					
					if (ITEXTCTL_IsActive(pme->m_pTextLon))
					{
						ITEXTCTL_SetActive(pme->m_pTextLon, FALSE);
						ITEXTCTL_Redraw(pme->m_pTextLon);
						
						ITEXTCTL_SetActive(pme->m_pTextLat, TRUE);
						ITEXTCTL_Redraw(pme->m_pTextLat);
						return TRUE;
					}

					if (ITEXTCTL_IsActive(pme->m_pTextDesc))
					{
						ITEXTCTL_SetActive(pme->m_pTextDesc, FALSE);
						ITEXTCTL_Redraw(pme->m_pTextDesc);
						
						ITEXTCTL_SetActive(pme->m_pTextLon, TRUE);
						ITEXTCTL_Redraw(pme->m_pTextLon);
						return TRUE;
					}
				}
				break;
				
				
			case AVK_DOWN:  // Handle 'DOwN' button presses if the text control has focus.
			case AVK_SELECT:  // Handle 'SELECT' button presses if the text control has focus.
					DBGPRINTF("text..");
					// focus switch
					if (ITEXTCTL_IsActive(pme->m_pTextLat))
					{
						DBGPRINTF("text lat..");
						ITEXTCTL_SetActive(pme->m_pTextLat, FALSE);
						ITEXTCTL_Redraw(pme->m_pTextLat);
						
						ITEXTCTL_SetActive(pme->m_pTextLon, TRUE);
						ITEXTCTL_Redraw(pme->m_pTextLon);
						return TRUE;
					}
					
					if (ITEXTCTL_IsActive(pme->m_pTextLon))
					{
						DBGPRINTF("text lon..");
						ITEXTCTL_SetActive(pme->m_pTextLon, FALSE);
						ITEXTCTL_Redraw(pme->m_pTextLon);
						
						ITEXTCTL_SetActive(pme->m_pTextDesc, TRUE);
						ITEXTCTL_Redraw(pme->m_pTextDesc);
						return TRUE;
					}

					if (ITEXTCTL_IsActive(pme->m_pTextDesc))
					{
						DBGPRINTF("text desc..");
						ITEXTCTL_SetActive(pme->m_pTextDesc, FALSE);
						ITEXTCTL_Redraw(pme->m_pTextDesc);
						
						ITEXTCTL_SetActive(pme->m_pTextLat, TRUE);
						ITEXTCTL_Redraw(pme->m_pTextLat);
						return TRUE;
					}
				break;*/
			}
			
	   }
	return bRet;
}