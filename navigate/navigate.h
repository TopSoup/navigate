#ifndef  TS_NAVIGATE_H
#define  TS_NAVIGATE_H

#include "navigatepch.h"

#include "location.h"

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

#define MAX_SOS_NUM                  3                     // Max Num for SOS NUM

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
	TSW_LOCINFO,//列表项-选项-位置信息说明

	//SOS和发送短信复用界面
	TSW_SEND_NUM,		//接收电话号码
	

	//位置信息说明
	TSW_WHERE_DETAILS,			//我在哪选项->信息说明
	TSW_LOCATION_RANGE_INFO,	//目的地经纬度范围说明

	/* Interval */
	//1
	TSW_LOCATING,		//定位中...
	TSW_NAVIGATE,		//领航
	TSW_NAVIGATE_EX,	//领航2
	
	//3 
	TSW_ADDRBOOK,		//联系人界面
	TSW_SMSBOX,			//短信收件箱

	
	//输入信息[位置名称, 短信联系人, ]
	TSW_TEXTCTL,

	TSW_SOS_RELATIVE,		//1 SOS亲属号码设定
	TSW_SOS_INFO,			//2 SOS功能说明

	TSW_LAST
} TSWindow;

//SOS 操作
typedef enum _SOSOP{
    SOS_IDLE,
    SOS_SMS_SENDING,
    SOS_CALL_CALLING,
}SOSOP;

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

	uint16            m_wMenuLastSel[TSW_LAST];    // CurSel of CMainWin

	//GPS
	GetGPSInfo		  m_gpsInfo;	//

    boolean			  m_bGetGpsInfo; //where win gpsstatus

	//SMS
	ISMS              *m_pISMS;      
    ISMSMsg           *m_pISMSMsg;       // To encapsulate composed/sent message 
	AEECallback       m_cb;				// General callback
	uint32            m_retVal;         // General place holder for error

	//Tel
	ICallMgr *m_pCallMgr;
	ICall *m_pOutgoingCall;
    CallListener callListener;

    //SOS
	AECHAR				m_szTextA[32];
	AECHAR				m_szTextB[32];
	AECHAR				m_szTextC[32];

	boolean			  m_bEnableSOS;
    SOSOP             m_OP;
    int               m_id;                       //当前通知的号码索引
    char			  m_szNum[MAX_SOS_NUM][32];   //通知的号码列表

	//Destination DataBse
	IDatabase*		m_pDatabase;		// Database


	//新建目的地使用的变量
	AECHAR				m_szTextLat[32];
	AECHAR				m_szTextLon[32];
	AECHAR				m_szTextDesc[32];

	//textctl
	AECHAR          m_pTextctlText[TS_MAX_STRLEN];
	int             m_pTextctlMode;     //input mode
	TSWindow        m_pTextctlWin; 
	int				m_op;				//TEXTCTL OP: 0-保存到目的地列表 1-短信发送
	boolean			m_opStatus;

	//领航前一个窗口
	TSWindow        m_pPreNaviWin; 

	boolean			m_bBackground;		//是否设置为后台状态
};

typedef struct CTopSoupApp  CTopSoupApp;



#define CTopSoupApp_CancelRedraw(p)   { CALLBACK_Cancel(&(p)->m_cbRedraw); (p)->m_bRedraw = FALSE; }
#define CTopSoupApp_DisableWin(p)     { IWINDOW_Disable((p)->m_pWin); CTopSoupApp_CancelRedraw(p); }

void CTopSoupApp_Redraw(CTopSoupApp * pme, boolean bDefer);
boolean CTopSoupApp_SetWindow(CTopSoupApp * pme, TSWindow eWin, uint32 dwParam);
void CTopSoupApp_onSplashDrawOver(void * po);

//发送位置短信
//格式: 目标位置:1111#纬度:E,20.012345#经度:N,120.012345
void CTopSoupApp_SendSMSMessage(CTopSoupApp * pMe, uint16 wParam, AECHAR *szDesc,AECHAR* szLat,AECHAR* szLon,char* phoneNumber);

//格式：紧急求助！
void CTopSoupApp_SendSOSSMSMessage (CTopSoupApp * pme, uint16 wParam, AECHAR *szDesc, char* phoneNumber);


#endif