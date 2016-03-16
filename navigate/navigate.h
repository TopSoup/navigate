#ifndef  TS_NAVIGATE_H
#define  TS_NAVIGATE_H

#include "navigatepch.h"

typedef struct _IWindow  IWindow;

//TODO how to add annotate
typedef enum TSWindow
{
	TSW_NONE,
	TSW_MAIN,
	TSW_LAST
} TSWindow;


// navigate app global structure.
struct CTopSoupApp
{
	AEEApplet		a; 
	int               m_cxWidth; 
	int               m_cyHeight; 
	uint16            m_nColorDepth; 
	int               m_nNChSize; 
	int               m_nLChSize; 
	AEECallback       m_cbRedraw; 				
	TSWindow          m_eActiveWin; 
	TSWindow          m_eSuspendWin; 
	IWindow *         m_pWin; 
	flg               m_bRedraw:1;

	//XXX to add code here
	IImage *          m_pHdrImage;
	AEERect           m_rectHdr;

	uint16            m_wMainWin;    // CurSel of CMainWin
};

typedef struct CTopSoupApp  CTopSoupApp;



#define CTopSoupApp_CancelRedraw(p)   { CALLBACK_Cancel(&(p)->m_cbRedraw); (p)->m_bRedraw = FALSE; }
#define CTopSoupApp_DisableWin(p)     { IWINDOW_Disable((p)->m_pWin); CTopSoupApp_CancelRedraw(p); }

void CTopSoupApp_Redraw(CTopSoupApp * pme, boolean bDefer);

#endif