#ifndef  TS_WINDOW_H
#define  TS_WINDOW_H

#include "navigatepch.h"


/*===========================================================================

FILE: window.h

SERVICES: offer a base window interface to make brew app window

DESCRIPTION
  
 
   	   Copyright ?2015 TopSoup Incorporated.
	                  All Rights Reserved.
                   TopSoup Proprietary/GTDR

===========================================================================*/


#include "AEEAppGen.h"	      // AEEApplet structure and Applet services


typedef struct CTopSoupApp  CTopSoupApp;  //TODO how
 
// 通用IWindow接口
typedef struct _IWindow  IWindow;
QINTERFACE(IWindow)
{
	// Enables/Disables the window. Window controls will not process
	// events if the window is disabled.
	void     (*Enable)(IWindow * po, boolean bEnable);

	// Redraws the window if enabled
	void     (*Redraw)(IWindow * po);

	// Handles the events routed to the window
	boolean  (*HandleEvent)(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

	// Releases the window resources
	void     (*Delete)(IWindow * po);
};

#define IWINDOW_Enable(p)                 GET_PVTBL(p, IWindow)->Enable(p, TRUE)
#define IWINDOW_Disable(p)                GET_PVTBL(p, IWindow)->Enable(p, FALSE)
#define IWINDOW_Redraw(p)                 GET_PVTBL(p, IWindow)->Redraw(p)
#define IWINDOW_HandleEvent(p, e, w, dw)  GET_PVTBL(p, IWindow)->HandleEvent(p, e, w, dw)
#define IWINDOW_Delete(p)                 GET_PVTBL(p, IWindow)->Delete(p)

#define IWINDOW_SETVTBL(pVtbl, pfnEn, pfnRd, pfnHE, pfnDel) \
	(pVtbl)->Enable      = (pfnEn); \
	(pVtbl)->Redraw      = (pfnRd); \
	(pVtbl)->HandleEvent = (pfnHE); \
	(pVtbl)->Delete      = (pfnDel)


#define INHERIT_CWindow(iname) \
	DECLARE_VTBL(iname) \
	CTopSoupApp * m_pOwner; \
	IShell *       m_pIShell; \
	IDisplay *     m_pIDisplay; \
	flg            m_bActive:1

// Base class of all IWindow objects.
struct CWindow
{
	INHERIT_CWindow(IWindow);
};
typedef struct CWindow        CWindow;

IWindow *  CWindow_New(int16 nSize, CTopSoupApp * pOwner, VTBL(IWindow) * pvt);
boolean    CWindow_ProcessEnable(IWindow * po, boolean bEnable);





//example

/*
//create a window named CMainWin inherit from cwindow
struct CMainWin
{
	INHERIT_CWindow(IWindow);

	IImage *       m_pLogo;
	AEERect        m_rectLogo;
	IMenuCtl *     m_pMainMenu;
	flg            m_bAbout:1;
};

{
	CMainWin *        pme;
	VTBL(IWindow)     vtbl;

	IWINDOW_SETVTBL(&vtbl, CMainWin_Enable, CMainWin_Redraw, CMainWin_HandleEvent, CMainWin_Delete);
	pme = (CMainWin *)CWindow_New(sizeof(CMainWin), pOwner, &vtbl);
	if (!pme)
		return NULL;

}
*/
#endif