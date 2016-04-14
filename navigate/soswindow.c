#include "soswindow.h"



// SOS window: Displays main menu.
struct CSOSWin
{
    INHERIT_CWindow(IWindow);

    //XXX
    IMenuCtl *     m_pMainMenu;
};

typedef struct CSOSWin CSOSWin;


static void       CSOSWin_Delete(IWindow * po);
static void       CSOSWin_Enable(IWindow * po, boolean bEnable);
static void       CSOSWin_Redraw(IWindow * po);
static boolean    CSOSWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

/*===============================================================================

                     CSOSWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
IWindow * CSOSWin_New(CTopSoupApp * pOwner)
{
    CSOSWin *        pme;
    VTBL(IWindow)     vtbl;

    IWINDOW_SETVTBL(&vtbl, CSOSWin_Enable, CSOSWin_Redraw, CSOSWin_HandleEvent, CSOSWin_Delete);
    pme = (CSOSWin *)CWindow_New(sizeof(CSOSWin), pOwner, &vtbl);
    if (!pme)
        return NULL;

    {
        //XXX __begin
        //Initialize logo below the header
        if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
        TS_WINERR_RETURN(pme);

        TS_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL,pme->m_pOwner->m_nColorDepth,&((CTopSoupApp*)pme->m_pOwner)->m_rectWin , 0);
        TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_SET_RELATIVE, NULL, IDI_OBJECT_15201, IDS_STRING_SET_RELATIVE, 0);
        TS_AddMenuItem(pme->m_pMainMenu, IDS_STRING_DEST_INFO,   NULL, IDI_OBJECT_15202, IDS_STRING_DEST_INFO,   0);

        ISHELL_LoadResString(pme->m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SOS,pme->m_pOwner->m_pHdrText,sizeof(pme->m_pOwner->m_pHdrText));
        TS_SetSoftButtonText(pme->m_pOwner,0,IDS_STRING_BACK,IDS_STRING_SELECT);

        //TODO
        pme->m_pOwner->m_wMenuLastSel[TSW_SOS_RELATIVE] = 0;

        //XXX __end
    }

    return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CSOSWin_Delete(IWindow * po)
{
    CSOSWin *  pme = (CSOSWin *)po;

    //XXX __begin
    if (pme->m_pMainMenu)
        pme->m_pOwner->m_wMenuLastSel[TSW_SOS] = IMENUCTL_GetSel(pme->m_pMainMenu);
    TS_RELEASEIF(pme->m_pMainMenu);
    //XXX _end

    FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CSOSWin_Enable(IWindow * po, boolean bEnable)
{
    CSOSWin *  pme = (CSOSWin *)po;


    if (!CWindow_ProcessEnable(po, bEnable))
        return;


    //XXX __begin
    if (!pme->m_bActive)
    {
        IMENUCTL_SetActive(pme->m_pMainMenu, FALSE);
        return;
    }

    IMENUCTL_SetActive(pme->m_pMainMenu, TRUE);
    IMENUCTL_SetSel(pme->m_pMainMenu, ((CTopSoupApp*)pme->m_pOwner)->m_wMenuLastSel[TSW_SOS]);
    //XXX __end
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CSOSWin_Redraw(IWindow * po)
{
    CSOSWin *  pme = (CSOSWin *)po;

    if (!pme->m_bActive)
        return;


    //XXX __begin
    IDISPLAY_ClearScreen(pme->m_pIDisplay);

    TS_DrawBackgroud(po);
    IMENUCTL_Redraw(pme->m_pMainMenu);


    IDISPLAY_Update(pme->m_pIDisplay);
    //XXX _end
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CSOSWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
    CSOSWin *  pme = (CSOSWin *)po;
    boolean     bRet = TRUE;

    //XXX __begin
    if ( TS_ISSOFT(eCode)){
        if( AVK_SOFT1 == wParam )
        {
            return IMENUCTL_HandleEvent(pme->m_pMainMenu, EVT_KEY, AVK_SELECT, 0);
        }

        //退出程序
        if( AVK_SOFT2 == wParam )
        {
            CTopSoupApp_SetWindow(pme->m_pOwner, TSW_MAIN, 0);
            return TRUE;
        }
    }


    if (TS_ISEVTKEY(eCode))
        return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

    if (!TS_ISEVTCMD(eCode))
        return FALSE;

    switch (wParam)
    {
        case IDS_STRING_SET_RELATIVE:
            CTopSoupApp_SetWindow(pme->m_pOwner, TSW_SOS_RELATIVE, 0);
            break;

        case IDS_STRING_DEST_INFO:
            CTopSoupApp_SetWindow(pme->m_pOwner, TSW_SOS_INFO, 0);
            break;

        default:
            bRet = FALSE;
            break;
    }
    //XXX __end

    return bRet;
}

