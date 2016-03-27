#include "destlistwindow.h"


// Destination window: Displays main menu.
struct CDestListWin
{
	INHERIT_CWindow(IWindow);

	//XXX
	IMenuCtl *     m_pMainMenu;		//Record List
};

typedef struct CDestListWin CDestListWin;


static void       CDestListWin_Delete(IWindow * po);
static void       CDestListWin_Enable(IWindow * po, boolean bEnable);
static void       CDestListWin_Redraw(IWindow * po);
static boolean    CDestListWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CDestListWin_About(IWindow * po);

/*===============================================================================

                     CDestListWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CDestListWin_New(CTopSoupApp * pOwner)
{
   CDestListWin *        pme;
   VTBL(IWindow)     vtbl;
   
   IWINDOW_SETVTBL(&vtbl, CDestListWin_Enable, CDestListWin_Redraw, CDestListWin_HandleEvent, CDestListWin_Delete);
   pme = (CDestListWin *)CWindow_New(sizeof(CDestListWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
	AEEItemStyle rNormalStyle;
	AEEItemStyle rSelStyle;
	AEERect rRect;
	int      cx = pme->m_pOwner->m_cxWidth;
	int      cy = pme->m_pOwner->m_cyHeight;

	//XXX __begin
	//Initialize logo below the header
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

	TS_SetSoftButtonText(pme->m_pOwner,IDS_STRING_SELECT,IDS_STRING_BACK,0);
	//XXX __end

	//For Test
	//TS_EmptyExpenseDatabase(pme->m_pOwner);

   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CDestListWin_Delete(IWindow * po)
{
   CDestListWin *  pme = (CDestListWin *)po;
  
   //XXX __begin
   TS_RELEASEIF(pme->m_pMainMenu);
   //XXX _end

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CDestListWin_Enable(IWindow * po, boolean bEnable)
{
   CDestListWin *  pme = (CDestListWin *)po;
 

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
static void CDestListWin_Redraw(IWindow * po)
{
   CDestListWin *  pme = (CDestListWin *)po;

   if (!pme->m_bActive)
      return;

   
   //XXX __begin
   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   TS_DrawBackgroud(po);

   {
   uint32 nTotal = 0;
#ifdef HAVE_TITLE
   AECHAR psTitle[ MAX_DESC_SIZE + 1 ];
#endif
   
   //For Test
   if( IDATABASE_GetRecordCount( pme->m_pOwner->m_pDatabase ) == 0 )
   {
		TS_AddExpenseItem(pme->m_pOwner, L"Aeijing", L"37.012345", L"114.123456");
		TS_AddExpenseItem(pme->m_pOwner, L"Beijing", L"37.112345", L"114.223456");
		TS_AddExpenseItem(pme->m_pOwner, L"Ceijing", L"37.212345", L"114.323456");
		TS_AddExpenseItem(pme->m_pOwner, L"Deijing", L"37.312345", L"114.423456");
		TS_AddExpenseItem(pme->m_pOwner, L"Eeijing", L"37.012345", L"114.123456");
		TS_AddExpenseItem(pme->m_pOwner, L"Feijing", L"37.112345", L"114.223456");
		TS_AddExpenseItem(pme->m_pOwner, L"Geijing", L"37.212345", L"114.323456");
		TS_AddExpenseItem(pme->m_pOwner, L"Heijing", L"37.312345", L"114.423456");
		TS_AddExpenseItem(pme->m_pOwner, L"Ieijing", L"37.012345", L"114.123456");
		TS_AddExpenseItem(pme->m_pOwner, L"Jeijing", L"37.112345", L"114.223456");
		TS_AddExpenseItem(pme->m_pOwner, L"Keijing", L"37.212345", L"114.323456");
		TS_AddExpenseItem(pme->m_pOwner, L"Leijing", L"37.312345", L"114.423456");
   }

   // Get Expense List
   TS_GetExpenseList( pme->m_pOwner, pme->m_pMainMenu, &nTotal);

#ifdef HAVE_TITLE
   // Set Appropriate Title
   if( IDATABASE_GetRecordCount( pme->m_pOwner->m_pDatabase ) == 0 )
   {
      IMENUCTL_SetTitle(pme->m_pMainMenu, NAVIGATE_RES_FILE, IDS_STRING_EMPTY, NULL);
   }
   else
   {
      if( IMENUCTL_GetItemCount( pme->m_pMainMenu ) == 0 )
	  {
         IMENUCTL_SetTitle(pme->m_pMainMenu, NAVIGATE_RES_FILE, IDS_STRING_NO_MATHED, NULL);
	  }
      else
      {
         uint16 nRecordID = IMENUCTL_GetSel( pme->m_pMainMenu ) - EXPENSE_LIST_ID;
         TS_GetExpenseItem( pme->m_pOwner, nRecordID, psTitle, NULL, NULL );         
         IMENUCTL_SetTitle(pme->m_pMainMenu, NULL, NULL, psTitle);
      }
   }
#endif

   // Active Menu
   IMENUCTL_SetActive( pme->m_pMainMenu, TRUE);
   }

   IMENUCTL_Redraw(pme->m_pMainMenu);

   IDISPLAY_Update(pme->m_pIDisplay);
   //XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CDestListWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CDestListWin *  pme = (CDestListWin *)po;
   boolean     bRet = TRUE;

   DBGPRINTF("dest ecode:%x", eCode);
   //XXX __begin
   if ( TS_ISSOFT(eCode)){
	   if( AVK_SOFT1 == wParam )
		   return TRUE;

	   //ÍË³ö³ÌÐò
	   if( AVK_SOFT2 == wParam )
	   {
			CTopSoupApp_SetWindow(pme->m_pOwner, TSW_NAVIGATE_DEST, 0);
			return TRUE;
	   }
   }

#ifdef HAVE_TITLE
   //When receive sel changed event, update menu title with cur item desc.
   if (TS_ISSELCHG(eCode))
   {            
	   AECHAR psTitle[ MAX_DESC_SIZE + 1 ];            
	   IMenuCtl * pMenu = (IMenuCtl*)dwParam;
	   uint16 nRecordID = IMENUCTL_GetSel(pMenu) - EXPENSE_LIST_ID;
	   TS_GetExpenseItem( pme->m_pOwner, nRecordID, psTitle, NULL, NULL );         
	   IMENUCTL_SetTitle(pMenu, NULL, NULL, psTitle);
	   IMENUCTL_Redraw(pMenu);
	   return TRUE;
   }
#endif
   
   if (TS_ISEVTKEY(eCode)) 
   {
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);
   }

   if (!TS_ISEVTCMD(eCode))
      return FALSE;

   //XXX __end

   return bRet;
}

/*===========================================================================
   This function displays the About dialog of the app.
===========================================================================*/
static void CDestListWin_About(IWindow * po)
{
   CDestListWin *  pme = (CDestListWin *)po;

	CTopSoupApp_DisableWin(pme->m_pOwner);

   IDISPLAY_ClearScreen(pme->m_pIDisplay);
   TS_DrawBackgroud(po);
   ISHELL_ShowCopyright(pme->m_pIShell);
}
