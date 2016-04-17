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
	TSW_MAIN,			//��ҳ��

	/* Second Level */
	TSW_WHERE,				//1 ������
	TSW_NAVIGATE_DEST,		//2 Ŀ���캽
	TSW_SOS,				//3 SOS

	/* Third Level */
	//2
	TSW_DEST_LIST,	//Ŀ�ĵ��б�
	TSW_DEST_NEW,	//�½�Ŀ�ĵ�
	TSW_DEST_INFO,	//����˵��

	//TSW_WHERE-ѡ��
	TSW_WHERE_FUCTION, //������ѡ���

	/* Fouth Level */
    TSW_WHERE_FUCTION_SAVE, //����λ�ô���
    TSW_DEST_LIST_FUCTION,  //Ŀ�ĵ��б�-ѡ���
    TSW_DEST_NEW_FUCTION, //�½�Ŀ�ĵ�-ѡ���

	/* Fifth Level */
	TSW_DEST_LIST_FUCTION_INFO, //Ŀ�ĵ���Ϣ
	TSW_DEST_LIST_FUCTION_EDIT, //Ŀ�ĵ�������


	//3 
	//Ŀ�ĵ��б�������
	TSW_DEST_LIST_ITEM,	//�б�������
	TSW_LOCINFO,//�б���-ѡ��-λ����Ϣ˵��

	//SOS�ͷ��Ͷ��Ÿ��ý���
	TSW_SEND_NUM,		//���յ绰����
	

	//λ����Ϣ˵��
	TSW_WHERE_DETAILS,			//������ѡ��->��Ϣ˵��
	TSW_LOCATION_RANGE_INFO,	//Ŀ�ĵؾ�γ�ȷ�Χ˵��

	/* Interval */
	//1
	TSW_LOCATING,		//��λ��...
	TSW_NAVIGATE,		//�캽
	TSW_NAVIGATE_EX,	//�캽2
	
	//3 
	TSW_ADDRBOOK,		//��ϵ�˽���
	TSW_SMSBOX,			//�����ռ���

	
	//������Ϣ[λ������, ������ϵ��, ]
	TSW_TEXTCTL,

	TSW_SOS_RELATIVE,		//1 SOS���������趨
	TSW_SOS_INFO,			//2 SOS����˵��

	TSW_LAST
} TSWindow;

//SOS ����
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
    int               m_id;                       //��ǰ֪ͨ�ĺ�������
    char			  m_szNum[MAX_SOS_NUM][32];   //֪ͨ�ĺ����б�

	//Destination DataBse
	IDatabase*		m_pDatabase;		// Database


	//�½�Ŀ�ĵ�ʹ�õı���
	AECHAR				m_szTextLat[32];
	AECHAR				m_szTextLon[32];
	AECHAR				m_szTextDesc[32];

	//textctl
	AECHAR          m_pTextctlText[TS_MAX_STRLEN];
	int             m_pTextctlMode;     //input mode
	TSWindow        m_pTextctlWin; 
	int				m_op;				//TEXTCTL OP: 0-���浽Ŀ�ĵ��б� 1-���ŷ���
	boolean			m_opStatus;

	//�캽ǰһ������
	TSWindow        m_pPreNaviWin; 

	boolean			m_bBackground;		//�Ƿ�����Ϊ��̨״̬
};

typedef struct CTopSoupApp  CTopSoupApp;



#define CTopSoupApp_CancelRedraw(p)   { CALLBACK_Cancel(&(p)->m_cbRedraw); (p)->m_bRedraw = FALSE; }
#define CTopSoupApp_DisableWin(p)     { IWINDOW_Disable((p)->m_pWin); CTopSoupApp_CancelRedraw(p); }

void CTopSoupApp_Redraw(CTopSoupApp * pme, boolean bDefer);
boolean CTopSoupApp_SetWindow(CTopSoupApp * pme, TSWindow eWin, uint32 dwParam);
void CTopSoupApp_onSplashDrawOver(void * po);

//����λ�ö���
//��ʽ: Ŀ��λ��:1111#γ��:E,20.012345#����:N,120.012345
void CTopSoupApp_SendSMSMessage(CTopSoupApp * pMe, uint16 wParam, AECHAR *szDesc,AECHAR* szLat,AECHAR* szLon,char* phoneNumber);

//��ʽ������������
void CTopSoupApp_SendSOSSMSMessage (CTopSoupApp * pme, uint16 wParam, AECHAR *szDesc, char* phoneNumber);


#endif