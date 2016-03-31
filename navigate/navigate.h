#ifndef  TS_NAVIGATE_H
#define  TS_NAVIGATE_H

#include "navigatepch.h"

typedef struct _IWindow  IWindow;

//string TODO solove include file problem, put it in logimacro mode
#define TS_MAX_STRLEN         64

/*  DataBase */
#define EXPENSE_LIST_ID             1000                  // Base ID of expense items displayed in the menu control
#define REPORT_LIST_ID              2000                  // Base ID of report file names displayed in the menu control

#define DESTINATION_DB_FILE       "destination.db"         // Base database file name
#define MAX_RES_STRING_BUF_SIZE      256                  // Max buffer size for string buffers
#define MAX_AMOUNT_DIGITS            8                    // Max digits for amount strings                
#define AMOUNT_BUFFER_SIZE           10                   // Max string buffer size for amount strings
#define MAX_DESC_SIZE                32

//TODO how to add annotate
typedef enum TSWindow
{
	TSW_NONE,

	/* First Level */
	TSW_MAIN,			//主页面

	/* Second Level */
	TSW_WHERE,				//1 我在哪
	TSW_NAVIGATE_DEST,		//2 目标领航
	TSW_SOS,				//3 SOS

	/* Third Level */
	//2
	TSW_DEST_LIST,	//目的地列表
	TSW_DEST_NEW,	//新建目的地
	TSW_DEST_INFO,	//功能说明

	//TSW_WHERE-选项
	TSW_WHERE_FUCTION, //我在哪选项窗口

	/* Fouth Level */
    TSW_WHERE_FUCTION_SAVE, //保存位置窗口
    TSW_DEST_LIST_FUCTION,  //目的地列表-选项窗口
    TSW_DEST_NEW_FUCTION, //新建目的地-选项窗口

	/* Fifth Level */
	TSW_DEST_LIST_FUCTION_INFO, //目的地信息
	TSW_DEST_LIST_FUCTION_EDIT, //目的地重命名


	//3 
	//目的地列表项内容
	TSW_DEST_LIST_ITEM,	//列表项内容

	//SOS和发送短信复用界面
	TSW_SEND_NUM,		//接收电话号码
	

	/* Interval */
	//1
	TSW_LOCATING,		//定位中...
	TSW_NAVIGATE,		//领航
	
	//3 
	TSW_ADDRBOOK,		//联系人界面
	TSW_SMSBOX,			//短信收件箱

	//ctl
	TSW_TEXTCTL,
	
	TSW_LAST
} TSWindow;

// navigate app global structure.
struct CTopSoupApp
{
	AEEApplet		  a; 
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

	int               m_nFontHeight;  // Stores the height of the AEE_FONT_BOLD font.  This will
	//XXX to add code here
	IImage *          m_pHdrImage;
	AEERect           m_rectHdr;

	IImage *		  m_pBackImage;
	AEERect           m_rectBack;

	IImage *          m_pBottomImage;
	AEERect           m_rectBtm;

	IStatic *         m_pHdrStatic;
	AECHAR            m_pHdrText[TS_MAX_STRLEN];

	AEERect           m_rectWin;

	IStatic *         m_pLeftSoftStatic;
	IStatic *         m_pRightSoftStatic;
	IStatic *         m_pMidSoftStatic;
	AECHAR            m_pLeftSoftText[TS_MAX_STRLEN];
	AECHAR            m_pMidSoftText[TS_MAX_STRLEN];
	AECHAR            m_pRightSoftText[TS_MAX_STRLEN];

	uint16            m_wMainWin;    // CurSel of CMainWin


	//SMS
	ISMS              *m_pISMS;      
    ISMSMsg           *m_pISMSMsg;       // To encapsulate composed/sent message 
	AEECallback       m_cb;				// General callback
	uint32            m_retVal;         // General place holder for error

	//Tel
	ICallMgr *m_pCallMgr;
	ICall *m_pOutgoingCall;
    CallListener callListener;

	//Destination DataBse
	IDatabase*		m_pDatabase;		// Database


	//textctl
	AECHAR          m_pTextctlText[TS_MAX_STRLEN];
	int             m_pTextctlMode;     //input mode
	TSWindow        m_pTextctlWin;      

};

typedef struct CTopSoupApp  CTopSoupApp;



#define CTopSoupApp_CancelRedraw(p)   { CALLBACK_Cancel(&(p)->m_cbRedraw); (p)->m_bRedraw = FALSE; }
#define CTopSoupApp_DisableWin(p)     { IWINDOW_Disable((p)->m_pWin); CTopSoupApp_CancelRedraw(p); }

void CTopSoupApp_Redraw(CTopSoupApp * pme, boolean bDefer);
boolean CTopSoupApp_SetWindow(CTopSoupApp * pme, TSWindow eWin, uint32 dwParam);
void CTopSoupApp_onSplashDrawOver(void * po);

#endif