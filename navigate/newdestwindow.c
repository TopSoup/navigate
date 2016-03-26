#include "newdestwindow.h"

#define MP_MAX_STRLEN 64
#define MP_NEW_DEST_CY 16

#define MAX_CONTENT_SIZE 30


// NewDestination window: Displays main menu.
struct CNewDestWin
{
	INHERIT_CWindow(IWindow);

	AECHAR				m_szText[MP_MAX_STRLEN];

	IStatic *			m_pStaticLat;	  // 经度
	IStatic *			m_pStaticLon;	  // 纬度
	IStatic *			m_pStaticDesc;	  // 描述

	ITextCtl*			m_pTextLat;       // 经度
    ITextCtl*			m_pTextLon;       // 纬度
	ITextCtl*			m_pTextDesc;      // 描述
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
		AEERect  rect;
	   
		//XXX __begin
		if ((ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void**)(&pme->m_pStaticLat)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void**)(&pme->m_pStaticLon)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void**)(&pme->m_pStaticDesc)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextLat)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextLon)) != SUCCESS) ||
		   (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_TEXTCTL, (void**)(&pme->m_pTextDesc)) != SUCCESS ))
		   TS_WINERR_RETURN(pme);

		SETAEERECT(&rect, 0, dy, cx/4, dy);
		ISTATIC_SetRect(pme->m_pStaticLat, &rect);

		dy += MP_NEW_DEST_CY;
		SETAEERECT(&rect, 0, dy, cx/4, dy);
		ISTATIC_SetRect(pme->m_pStaticLon, &rect);

		dy += MP_NEW_DEST_CY;
		SETAEERECT(&rect, 0, dy, cx/4, dy);
		ISTATIC_SetRect(pme->m_pStaticDesc, &rect);

		// Let's reset text control size so on subsequent this won't misbehave by erasing screen
		SETAEERECT(&rect, 0, 0, 0, 0);
		ITEXTCTL_SetRect(pme->m_pTextLat, &rect);
		ITEXTCTL_SetRect(pme->m_pTextLon, &rect);
		ITEXTCTL_SetRect(pme->m_pTextDesc, &rect);
		
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
   TS_RELEASEIF(pme->m_pStaticLat);
   TS_RELEASEIF(pme->m_pStaticLon);
   TS_RELEASEIF(pme->m_pStaticDesc);
   TS_RELEASEIF(pme->m_pTextLat);
   TS_RELEASEIF(pme->m_pTextLon);
   TS_RELEASEIF(pme->m_pTextDesc);
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
   ISTATIC_SetActive(pme->m_pStaticLat, bEnable);
   ISTATIC_SetActive(pme->m_pStaticLon, bEnable);
   ISTATIC_SetActive(pme->m_pStaticDesc, bEnable);
   //ITEXTCTL_SetActive(pme->m_pTextLat, bEnable);
   //ITEXTCTL_SetActive(pme->m_pTextLon, bEnable);
   //ITEXTCTL_SetActive(pme->m_pTextDesc, bEnable);
   //XXX __end
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CNewDestWin_Redraw(IWindow * po)
{
	CNewDestWin *  pme = (CNewDestWin *)po;
	AEERect  rRect;

	if (!pme->m_bActive)
		return;
	
	//XXX __begin
	IDISPLAY_ClearScreen(pme->m_pIDisplay);
	//TS_DrawBackgroud(po);
	
	STRTOWSTR("Lat: ", pme->m_szText, sizeof(pme->m_szText));
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pStaticLat, AEE_FONT_NORMAL, pme->m_szText);
	
	STRTOWSTR("Lon: ", pme->m_szText, sizeof(pme->m_szText));
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pStaticLon, AEE_FONT_NORMAL, pme->m_szText);

	STRTOWSTR("Name: ", pme->m_szText, sizeof(pme->m_szText));
	TS_FitStaticText(pme->m_pIDisplay, pme->m_pStaticDesc, AEE_FONT_NORMAL, pme->m_szText);


	// 1 Display lat input text
	ISTATIC_GetRect(pme->m_pStaticLat, &rRect );
    ITEXTCTL_Reset( pme->m_pTextLat );
    ITEXTCTL_SetProperties( pme->m_pTextLat, TP_FRAME );

    // Set the starting X coordinate position of the control and its width (screen width
    // minus the width of the label string).
    rRect.x = 2 + rRect.dx;
    rRect.dx = pme->m_pOwner->m_cxWidth - rRect.x -2;
	ITEXTCTL_SetText(pme->m_pTextLat, L"0.0",  MAX_CONTENT_SIZE);
    ITEXTCTL_SetRect(pme->m_pTextLat, &rRect );
    ITEXTCTL_SetInputMode( pme->m_pTextLat, AEE_TM_NUMBERS );
    //pMe->m_nInputMode = IMT_LITTLE_CASE_LETTER;
    ITEXTCTL_SetMaxSize( pme->m_pTextLat, MAX_CONTENT_SIZE+1 );
    ITEXTCTL_SetActive( pme->m_pTextLat, TRUE );
    ITEXTCTL_Redraw( pme->m_pTextLat);


	// 2 Display lat input text
	ISTATIC_GetRect(pme->m_pStaticLon, &rRect );
    ITEXTCTL_Reset( pme->m_pTextLon );
    ITEXTCTL_SetProperties( pme->m_pTextLon, TP_FRAME );

    // Set the starting X coordinate position of the control and its width (screen width
    // minus the width of the label string).
    rRect.x = 2 + rRect.dx;
    rRect.dx = pme->m_pOwner->m_cxWidth - rRect.x -2;
	ITEXTCTL_SetText(pme->m_pTextLon, L"0.0",  MAX_CONTENT_SIZE);
    ITEXTCTL_SetRect(pme->m_pTextLon, &rRect );
    ITEXTCTL_SetInputMode( pme->m_pTextLon, AEE_TM_NUMBERS );
    //pMe->m_nInputMode = IMT_LITTLE_CASE_LETTER;
    ITEXTCTL_SetMaxSize( pme->m_pTextLon, MAX_CONTENT_SIZE+1 );
    ITEXTCTL_SetActive( pme->m_pTextLon, FALSE );
    ITEXTCTL_Redraw( pme->m_pTextLon);
	

	// 2 Display lat input text
	ISTATIC_GetRect(pme->m_pStaticDesc, &rRect );
    ITEXTCTL_Reset( pme->m_pTextDesc );
    ITEXTCTL_SetProperties( pme->m_pTextDesc, TP_FRAME );

    // Set the starting X coordinate position of the control and its width (screen width
    // minus the width of the label string).
    rRect.x = 2 + rRect.dx;
    rRect.dx = pme->m_pOwner->m_cxWidth - rRect.x -2;
	ITEXTCTL_SetText(pme->m_pTextDesc, L"0.0",  MAX_CONTENT_SIZE);
    ITEXTCTL_SetRect(pme->m_pTextDesc, &rRect );
    ITEXTCTL_SetInputMode( pme->m_pTextDesc, AEE_TM_NUMBERS );
    //pMe->m_nInputMode = IMT_LITTLE_CASE_LETTER;
    ITEXTCTL_SetMaxSize( pme->m_pTextDesc, MAX_CONTENT_SIZE+1 );
    ITEXTCTL_SetActive( pme->m_pTextDesc, FALSE );
    ITEXTCTL_Redraw( pme->m_pTextDesc);



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
	
	//*
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
				break;
			}
	   }
	return bRet;
}