#include "logicmacro.h"

#define TS_VERSION  "1.0.5-20170120"

/*-------------------------------------------------------------------
            Function Prototypes
-------------------------------------------------------------------*/
static boolean    CTopSoupApp_InitAppData(IApplet* po);
//FIXME why origin is not static
static void		  CTopSoupApp_FreeAppData(IApplet* po);
static boolean    CTopSoupApp_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CTopSoupApp_RedrawNotify(CTopSoupApp * pme);
static void		  CTopSoupApp_ReleaseRes(CTopSoupApp * pme);

//构建SOS短信：
//pos == NULL： 开启求助短信`
//pos != NULL:  发送带位置信息的求助短信
static void          CTopSoupApp_MakeSOSMsg(CTopSoupApp *pme, AECHAR szMsg[256], Coordinate *pos);
/************************************************************************/
/* 从配置文件加载亲友联系方式                                           */
/************************************************************************/
static uint32 LoadSOSConfig(IShell *iShell, char szNum[3][32]);

/*===============================================================================
                        SMS & TELEPHONE TEST
=============================================================================== */
#define USAGE_SMS_TX_UNICODE    0
#define USAGE_SMS_TX_ASCII      1
 
// ascii 短信内容，对于unicode短信内容，必须由资源文件bar中获取，否则编码不对 
#define MO_TEXT_ASCII "Destination:Beijing#lat:37.123456#lon:114.121345" 

/*===============================================================================
                        SOS Call
=============================================================================== */
#define TS_EVT_SOS_CALL 0xe04d

//Event
#define EVT_SMS_END				EVT_USER + 100		// 发送SMS结束
#define EVT_CALL_END			EVT_USER + 101		// 拨打电话结束
#define EVT_START_SOS           EVT_USER + 1000     // 启动SOS报警

//解析短信内容
//格式: 目标位置:1111#纬度:E,20.012345#经度:N,120.012345
static boolean CTopSoupApp_SaveSMSMessage(CTopSoupApp* pme, char* szMsg);
static boolean    CTopSoupApp_ReceiveSMSMessage(CTopSoupApp *pme, uint32 uMsgId);
static void		  CTopSoupApp_MakeSOSCall(CTopSoupApp * pme, char* szNumber);
static void		  CTopSoupApp_EndSOSCall(CTopSoupApp * pme);

//SOS功能：如果有亲友号码，则开启SOS，并给每个号码发送短信，和拨打电话，开启定位，定位成功后将位置信息通过短信发送
//加载配置文件
static void CTopSoupApp_StartSOS(CTopSoupApp *pme);

//
// navigate app can either be statically built into BREW or dynamically linked during run-time.
// If AEE_STATIC is defined, then navigate app will be a static app.
// NOTE: Static apps can be built only by OEMs or Carriers. App developers can build dynamic apps only.
//
// FIXME do not code it at all,need to code from begin
#if defined(AEE_STATIC)

int         CTopSoupApp_CreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj);
static int  CTopSoupApp_Load(IShell *ps, void * pHelpers, IModule ** pMod);

//
// Constant Data...
//
static const AEEAppInfo    gaiMediaPlayerApp = {AEECLSID_NAVIGATE,NAVIGATE_RES_FILE,IDS_TITLE,IDB_TNAIL,IDB_TNAIL,0,0,0};

/*===========================================================================

                      PUBLIC FUNCTION DECLARATIONS

===========================================================================*/


//===========================================================================
//
// 
//
//===========================================================================
PFNMODENTRY CTopSoupApp_GetModInfo(IShell * ps, AEECLSID ** ppClasses, AEEAppInfo ** pApps, uint16 * pnApps,uint16 * pwMinPriv)
{
   *pApps = (AEEAppInfo *)&gaiMediaPlayerApp;
   *pnApps = 1;
   return((PFNMODENTRY)CTopSoupApp_Load);
}

//===========================================================================
//
//===========================================================================
static int CTopSoupApp_Load(IShell *ps, void * pHelpers, IModule ** pMod)
{
   return(AEEStaticMod_New((int16)(sizeof(AEEMod)),ps,pHelpers,pMod,CTopSoupApp_CreateInstance,NULL));
}

#endif  //AEE_STATIC

/*===========================================================================

Function:  CTopSoupApp_InitAppData()

Description:
		This function initializes the app specific data.

Prototype:
	void CTopSoupApp_InitAppData(IApplet* po)

Parameters:
   IApplet* po: Pointer to the applet structure. This is of the type defined
	to store applet data. (CTopSoupApp*, in this case).

Return Value:
	TRUE: If successful
	FALSE: IF failed

Comments:  None

Side Effects: None

==============================================================================*/
boolean CTopSoupApp_InitAppData(IApplet* po)
{
   CTopSoupApp *    pme = (CTopSoupApp*)po;
   int               nAscent, nDescent;
   AEEDeviceInfo *   pdi;
   //XXX
   AEEImageInfo      info;
   AEERect           rect;
   AEERect           rect1;
   int				 nErr;
   
   {
	   //格式: 目标位置:1111#纬度:E,20.012345#经度:N,120.012345
	   //char *str = "目标位置:1111#纬度:E,20.012345#经度:N,120.012345";
	   //CTopSoupApp_SaveSMSMessage(pme, str);
   }

   //init some data
   pme->m_bEnableSOS = FALSE;
   pme->m_OP = SOS_IDLE;
   pme->m_id = -1;

   // Get screen pixel count
   pdi = MALLOC(sizeof(AEEDeviceInfo));
   if (!pdi)
      return FALSE;
   ISHELL_GetDeviceInfo(pme->a.m_pIShell, pdi);
   pme->m_cxWidth = pdi->cxScreen;
   pme->m_cyHeight = pdi->cyScreen;
   pme->m_nColorDepth = pdi->nColorDepth;
   FREEIF(pdi);

   pme->m_nFontHeight = IDISPLAY_GetFontMetrics(pme->a.m_pIDisplay, AEE_FONT_LARGE, &nAscent, &nDescent) + 1;
   pme->m_nLChSize = nAscent + nDescent;

   IDISPLAY_GetFontMetrics(pme->a.m_pIDisplay, AEE_FONT_NORMAL, &nAscent, &nDescent);
   pme->m_nNChSize = nAscent + nDescent;


   CALLBACK_Init(&pme->m_cbRedraw, (PFNNOTIFY)CTopSoupApp_RedrawNotify, pme);

   //XXX __begin
   //Header
   pme->m_pHdrImage = ISHELL_LoadResImage(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDI_OBJECT_5003);
   if (!pme->m_pHdrImage) {
	   CTopSoupApp_ReleaseRes(pme);    
	   return FALSE;
   }
   IImage_GetInfo(pme->m_pHdrImage,&info);
   SETAEERECT(&pme->m_rectHdr, 0, TS_HEADER_Y, info.cx, info.cy);
   
   if (ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pHdrStatic)) {
		CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   ISTATIC_SetRect(pme->m_pHdrStatic, &pme->m_rectHdr);
   ISTATIC_SetProperties(pme->m_pHdrStatic,  ST_MIDDLETEXT | ST_CENTERTEXT | ST_NOSCROLL);
   
   ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_TITLE,pme->m_pHdrText,sizeof(pme->m_pHdrText));

   
   pme->m_pBackImage = ISHELL_LoadResImage(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDI_OBJECT_5000);
   if (!pme->m_pBackImage) {	
		CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   IImage_GetInfo(pme->m_pBackImage,&info);
   SETAEERECT(&pme->m_rectBack,0,0,info.cx,info.cy);

   pme->m_pBottomImage = ISHELL_LoadResImage(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDI_OBJECT_5004);
   if (!pme->m_pBottomImage){
       CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   IImage_GetInfo(pme->m_pBottomImage,&info);
   SETAEERECT(&pme->m_rectBtm,0,(pme->m_cyHeight)- info.cy,info.cx,info.cy);
  
   if (ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pLeftSoftStatic)) {
	   CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   SETAEERECT(&rect,0,pme->m_rectBtm.y,pme->m_rectBtm.dx/3,pme->m_rectBtm.dy);
   ISTATIC_SetRect(pme->m_pLeftSoftStatic, &rect);
   ISTATIC_SetProperties(pme->m_pLeftSoftStatic, ST_CENTERTEXT | ST_MIDDLETEXT | ST_NOSCROLL);

   if (ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pMidSoftStatic)) {
	   CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   SETAEERECT(&rect1,rect.dx,rect.y,rect.dx,rect.dy);
   ISTATIC_SetRect(pme->m_pMidSoftStatic, &rect1);
   ISTATIC_SetProperties(pme->m_pMidSoftStatic,  ST_MIDDLETEXT | ST_CENTERTEXT | ST_NOSCROLL);

   if (ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pRightSoftStatic)) {
	   CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   SETAEERECT(&rect,rect1.x + rect1.dx,rect1.y,pme->m_rectBtm.dx - rect1.x - rect1.dx,rect1.dy);
   ISTATIC_SetRect(pme->m_pRightSoftStatic, &rect);
   ISTATIC_SetProperties(pme->m_pRightSoftStatic, ST_MIDDLETEXT | ST_CENTERTEXT | ST_NOSCROLL);


   SETAEERECT(&pme->m_rectWin, 0, pme->m_rectHdr.y + pme->m_rectHdr.dy + 1, pme->m_cxWidth, pme->m_cyHeight - pme->m_rectHdr.y - pme->m_rectHdr.dy - 1 - pme->m_rectBtm.dy);
   
   MEMSET(pme->m_wMenuLastSel,0,sizeof(pme->m_wMenuLastSel));
   
   //XXX _end

   pme->m_pWin = CMainWin_New(pme);
   if (!pme->m_pWin){
	   CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }


   //FOR SMS & TEL TEST
   if (ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_SMS, (void **)&pme->m_pISMS) != SUCCESS)
	{
		DBGPRINTF("AEECLSID_SMS Create Failed!");
		return FALSE;
	}

   // Register for Text and EMS 
   ISHELL_RegisterNotify(pme->a.m_pIShell, AEECLSID_NAVIGATE, AEECLSID_SMSNOTIFIER, (AEESMS_TYPE_TEXT << 16) | NMASK_SMS_TYPE);   

   //Tel
   nErr =ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_CALLMGR, (void**) &pme->m_pCallMgr);
   DBGPRINTF("CreateInst AEECLSID_CALLMGR ret %d", nErr);
   if(nErr != AEE_SUCCESS)
   {
       return FALSE;
   }

   {
		IDBMgr * pDBMgr = NULL;

		// Create the IDBMgr object
		if(ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_DBMGR, (void**)(&pDBMgr)) != SUCCESS)
		return FALSE;

		// Open the ExpenseTracker database.  If it does not exist then create it.
		// Once the database is opened, the IDBMgr is not longer needed so it is released.
		if((pme->m_pDatabase = IDBMGR_OpenDatabase(pDBMgr, DESTINATION_DB_FILE, TRUE)) == NULL) {
		IDBMGR_Release(pDBMgr);
		return FALSE;
		}
		else
		{
		IDBMGR_Release(pDBMgr);
		}
	}

   return TRUE;
}

/*===========================================================================
Function:  CTopSoupApp_FreeAppData()

Description:
	This function frees the app data. This function is registered with the
	applet framework when the applet is created (inside AEEClsCreateInstance() function).
	This function is called by the app framework when the reference count of the 
	applet reaches zero. This function must free all the app data that has been
	allocated by this app. For ex: if their is data that was remembered when
	the app is suspended and resumed, those data must be freed here.

Prototype:
	void CTopSoupApp_FreeAppData(IApplet* po)

Parameters:
   IApplet* po: Pointer to the applet structure. This is of the type defined
	to store applet data. (CTopSoupApp*, in this case).

Return Value:
	None

Comments:  None

Side Effects: None
==============================================================================*/
static void CTopSoupApp_FreeAppData(IApplet* po)
{
	CTopSoupApp * pme = (CTopSoupApp *)po;
	
	//un-register sms notify
	ISHELL_RegisterNotify(pme->a.m_pIShell, AEECLSID_NAVIGATE, AEECLSID_SMSNOTIFIER,0);

	//SMS
	TS_RELEASEIF(pme->m_pISMS);
	TS_RELEASEIF(pme->m_pISMSMsg);
	
	//Tel
	TS_RELEASEIF(pme->m_pCallMgr);
	
	//DataBase
	TS_RELEASEIF( pme->m_pDatabase);

	CTopSoupApp_ReleaseRes(pme);
}

/*===========================================================================

FUNCTION: AEEClsCreateInstance

DESCRIPTION
	This function is invoked while the app is being loaded. All Modules must provide this 
	function. Ensure to retain the same name and parameters for this function.
	In here, the module must verify the ClassID and then invoke the AEEApplet_New() function
	that has been provided in AEEAppGen.c. 

   After invoking AEEApplet_New(), this function can do app specific initialization. In this
   example, a generic structure is provided so that app developers need not change app specific
   initialization section every time.

PROTOTYPE:
	int AEEClsCreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj)

PARAMETERS:
	clsID: [in]: Specifies the ClassID of the applet which is being loaded

	pIShell: [in]: Contains pointer to the IShell interface. 

	pIModule: pin]: Contains pointer to the IModule interface to the current module to which
	this app belongs

	ppObj: [out]: On return, *ppApplet must point to a valid IBase object. 
	If the classID	represnts an applet, then ppObj must point to a valid AEEApplet structure.Allocation
	of memory for this structure and initializing the base data members is done by AEEApplet_New().

DEPENDENCIES
  none

RETURN VALUE
  AEE_SUCCESS: If the class/app creation was successful. 
  EFAILED: Error occurred while creating the class/app. In this case, the app/class will
  not be loaded.

SIDE EFFECTS
  none
===========================================================================*/
#if defined(AEE_STATIC)
int CTopSoupApp_CreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj)
#else
int AEEClsCreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj)
#endif
{
	*ppObj = NULL;

   //
   // Here a check is done to see if the ClsID is that of navigate app.
   // The reason is if this module has more than one applets or classes, then this function is invoked
   // once for each applet or class. Checking here ensures that the correct IApplet or class object is
   // constructed.
   //
   if(ClsId == AEECLSID_NAVIGATE)
	{

		//Create the applet
      if(AEEApplet_New(sizeof(CTopSoupApp), ClsId, pIShell,po,(IApplet**)ppObj,(AEEHANDLER)CTopSoupApp_HandleEvent,(PFNFREEAPPDATA)CTopSoupApp_FreeAppData))
		{
			//Initialize applet data
         if(CTopSoupApp_InitAppData((IApplet*)*ppObj))
			{
				//Data initialized successfully
				return(AEE_SUCCESS);
			}
			else
			{
				//Release the applet. This will free the memory allocated for the applet when
				*ppObj = NULL;
				IAPPLET_Release((IApplet*)*ppObj);
				return EFAILED;
			}

      }//AEEApplet_New

   }// ClsId == AEECLSID_NAVIGATE

   return(EFAILED);
}

/*===========================================================================

FUNCTION CTopSoupApp_HandleEvent

DESCRIPTION
	This is the EventHandler for this app. All events to this app are handled in this
	function. All APPs must supply an Event Handler.

PROTOTYPE:
	boolean CTopSoupApp_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam)

PARAMETERS:
	pi: Pointer to the AEEApplet structure. This structure contains information specific
	to this applet. It was initialized during the AppClsCreateInstance() function.

	ecode: Specifies the Event sent to this applet

   wParam, dwParam: Event specific data.

DEPENDENCIES
  none

RETURN VALUE
  TRUE: If the app has processed the event
  FALSE: If the app did not process the event

SIDE EFFECTS
  none
===========================================================================*/

static boolean CTopSoupApp_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
    CTopSoupApp * pme = (CTopSoupApp *)pi;

    //if (eCode != EVT_APP_NO_SLEEP)
    //  DBGPRINTF("@TS Recv eCode:%x", eCode);

    switch ( eCode ) 
    {   
         case EVT_APP_START:   // Process Start event
            DBGPRINTF(TS_VERSION);

            //SOS模式开始发送短信和呼叫联系人
            if (pme->m_bEnableSOS)
            {
                CTopSoupApp_StartSOS(pme);
            }
            else
            {
                pme->m_eActiveWin = TSW_MAIN;
                CTopSoupApp_SetWindow(pme, TSW_MAIN, 0);
            }

            return TRUE;

         case EVT_APP_STOP:        // process STOP event
            pme->m_id = -1;
            pme->m_OP = SOS_IDLE;
            pme->m_bEnableSOS = FALSE;
            return (TRUE);

         case EVT_APP_SUSPEND:
            pme->m_eSuspendWin = pme->m_eActiveWin;
            CTopSoupApp_SetWindow(pme, TSW_NONE, 0);
            return (TRUE);

         case EVT_APP_RESUME:
            DBGPRINTF("@EVT_APP_RESUME");
            CTopSoupApp_SetWindow(pme, pme->m_eSuspendWin, 0);
            return (TRUE);

		 case EVT_NOTIFY:
			 {
				 AEENotify *wp = (AEENotify *)dwParam;
				 DBGPRINTF("receive notify cls=%x", wp->cls);
				 
				 if( wp->cls == AEECLSID_SMSNOTIFIER ) {
					 if( wp->dwMask == ((AEESMS_TYPE_TEXT << 16) | NMASK_SMS_TYPE) ) {
						 uint32 uMsgId = (uint32)wp->pData;
						 CTopSoupApp_ReceiveSMSMessage(pme, uMsgId);
						 DBGPRINTF("msgid :%d", uMsgId);
					 } else {
						 DBGPRINTF("Recv unkown mask sms: %x",wp->dwMask);
						 return FALSE;
					 }
				 }
             }
			 return (TRUE);

        case EVT_START_SOS:
            if (pme->m_bEnableSOS)
            {
                DBGPRINTF("Already Enabled SOS");
                return (TRUE);
            }
            DBGPRINTF("@Recv Start Navigate Use SOS Mode!");

            pme->m_bEnableSOS = TRUE;
            ISHELL_StartApplet(pme->a.m_pIShell, AEECLSID_NAVIGATE);
            return (TRUE);

        case EVT_SMS_END:
            //收到短信发送结束消息
            if (pme->m_id >= 0 && pme->m_id < MAX_SOS_NUM)
            {
                CTopSoupApp_MakeSOSCall(pme, pme->m_szNum[pme->m_id]);
                pme->m_OP = SOS_CALL_CALLING;
            }
            else
            {
                DBGPRINTF("@MakeSOSCall Failed with index:%d", pme->m_id);
                //索引异常，结束单呼流程
                pme->m_bEnableSOS = FALSE;
                pme->m_OP = SOS_IDLE;
                pme->m_id = -1;

                ISHELL_CloseApplet(pme->a.m_pIShell, FALSE);
            }
            return (TRUE);

        case EVT_CALL_END:
            //收到拨打电话结束消息
            pme->m_id ++;   //使用下一个
            if (pme->m_id >= 0 && pme->m_id < MAX_SOS_NUM && STRLEN(pme->m_szNum[pme->m_id]) > 0)
            {
                AECHAR szMsg[256];
                CTopSoupApp_MakeSOSMsg(pme, szMsg, NULL);
                DBGPRINTF("@SOS Send SMS To Num: %s Msg len:%d", pme->m_szNum[pme->m_id], WSTRLEN(szMsg));
                CTopSoupApp_SendSOSSMSMessage(pme, USAGE_SMS_TX_UNICODE, szMsg, pme->m_szNum[pme->m_id]);
                pme->m_OP = SOS_SMS_SENDING;
            }
            else
            {
                DBGPRINTF("@SOS End", pme->m_id);
                //结束单呼流程
                pme->m_bEnableSOS = FALSE;
                pme->m_OP = SOS_IDLE;
                pme->m_id = -1;

                ISHELL_CloseApplet(pme->a.m_pIShell, FALSE);
            }

            return (TRUE);

         case EVT_KEY:	            // Process key event
			 {
				//DBGPRINTF("eCode:%x Key:%x", eCode, wParam);

				//FOR SMS & TEL TEST
				if (wParam == AVK_0)
				{
					//DBGPRINTF("SEND SMS TEST ...");
					//CTopSoupApp_SendSMSMessage(pme, USAGE_SMS_TX_ASCII);
					//CTopSoupApp_SendSMSMessage(pme, USAGE_SMS_TX_UNICODE, L"TIANANMEN");
                    //CTopSoupApp_StartSOS(pme);
				}

				//FOR TEL TEST
				if (wParam == AVK_PTT)
				{
					//DBGPRINTF("SOS CALL TEST ...");
					//CTopSoupApp_MakeSOSCall(pme, "15511823090");
                    //CTopSoupApp_StartSOS(pme);    //与系统冲突，有时会死机？
				}

				if (wParam == AVK_2)
				{
					//DBGPRINTF("CALL TEST END ...");
					//CTopSoupApp_EndSOSCall(pme);
				}
			 }
		 case EVT_KEY_PRESS:
		 case EVT_KEY_RELEASE:		// For '#' Switch Input Method
         case EVT_COMMAND:          // Process menu command event
		 case EVT_CTL_SEL_CHANGED:  // Process Sel Changed event
         case EVT_COPYRIGHT_END:    // Copyright dialog ended
		 default:
            if (pme->m_pWin)
               return IWINDOW_HandleEvent(pme->m_pWin, eCode, wParam, dwParam);

    }

    return FALSE;
}



/*===========================================================================
   This function switches from one window to another:
   (1) Releases all the resources associated with the current window
   (2) Contructs the new window, if any
   (3) Enables and redraws the new window, if any
===========================================================================*/
boolean CTopSoupApp_SetWindow(CTopSoupApp * pme, TSWindow eWin, uint32 dwParam)
{
	// If same window, then redraw and return.
   if (pme->m_pWin && pme->m_eActiveWin == eWin && eWin != TSW_NONE)
   {
      CTopSoupApp_Redraw(pme, TRUE);
      return TRUE;
   }

   TS_RELEASEWIN(pme->m_pWin);

   //reset res
   MEMSET(pme->m_pLeftSoftText,0,sizeof(pme->m_pLeftSoftText));
   MEMSET(pme->m_pRightSoftText,0,sizeof(pme->m_pRightSoftText));
   MEMSET(pme->m_pMidSoftText,0,sizeof(pme->m_pMidSoftText));

   switch (eWin)
   {

	  //1
      case TSW_MAIN:       
         pme->m_pWin = CMainWin_New(pme); 
         break;

	  //2
	  case TSW_WHERE:   
		  pme->m_pWin = CWhereWin_New(pme); 
		  break;
		  
	  case TSW_NAVIGATE_DEST:     
		  pme->m_pWin = CNavigateDestWin_New(pme); 
		  break;
	
	  case TSW_SOS:
		  pme->m_pWin = CSOSWin_New(pme); 
	      break;

	  //3
	  case TSW_DEST_LIST:     
		  pme->m_pWin = CDestListWin_New(pme); 
	      break;

	  case TSW_DEST_NEW:
		  pme->m_pWin = CNewDestWin_New(pme); 
	      break;

	  case TSW_DEST_INFO:
		  pme->m_pWin = CDestInfoWin_New(pme); 
	      break;

	  case TSW_WHERE_FUCTION:
		  pme->m_pWin = (IWindow*)CWhereFuctionWin_New(pme);
		  break;

	  case TSW_WHERE_DETAILS:
		  pme->m_pWin = (IWindow*)CWhereDetailsWin_New(pme);
		  break;

	  case TSW_DEST_NEW_FUCTION:
		  pme->m_pWin = (IWindow*)CNewdestFuctionWin_New(pme);
		  break;

	  case TSW_DEST_LIST_FUCTION:
		  pme->m_pWin = (IWindow*)CDestlistFuctionWin_New(pme,(uint16)dwParam);
		  break;

	  case TSW_TEXTCTL:
		  pme->m_pWin = (IWindow*)CTextCtlWin_New(pme,(uint16)dwParam);
		  break;


	  case TSW_NAVIGATE:
		  {
			  //使用当前选择的项作为领航目标
				TS_GetExpenseItem(pme,(uint16)dwParam,pme->m_szTextDesc,pme->m_szTextLat,pme->m_szTextLon);

				pme->m_pWin = (IWindow*)CNavigateWin_New(pme);

				break;
				
		  }

	  case TSW_LOCINFO:
		  pme->m_pWin = (IWindow*)CLocinfoWin_New(pme,(uint16)dwParam);
		  break;


	  case TSW_LOCATION_RANGE_INFO:
		  pme->m_pWin = (IWindow*)CLocationInfoWin_New(pme);
		  break;
	
	  case TSW_NAVIGATE_EX:     
		  pme->m_pWin = CNavigateWin_New(pme); 
		  break;

      //SOS
      case TSW_SOS_RELATIVE:
           pme->m_pWin = (IWindow*)CSOSRelativeWin_New(pme);
           break;

      case TSW_SOS_INFO:
           pme->m_pWin = (IWindow*)CSOSInfoWin_New(pme);
           break;

      case TSW_NONE:       
         return TRUE; 
         break;

      default:             
         return FALSE; 
         break;
   }

   if (!pme->m_pWin)
   {
      eWin = TSW_NONE;
      return FALSE;
   }

   pme->m_eActiveWin = eWin;

   CTopSoupApp_Redraw(pme, TRUE);

   return TRUE;
}

/*===========================================================================
   This function schedules an aynchronous redraw if bDefer is TRUE else
   it redraw immediately.
===========================================================================*/
void CTopSoupApp_Redraw(CTopSoupApp * pme, boolean bDefer)
{
   if (pme->m_bRedraw)
      return;

   pme->m_bRedraw = TRUE;

   if (bDefer)
      ISHELL_Resume(pme->a.m_pIShell, &pme->m_cbRedraw);
   else
      CTopSoupApp_RedrawNotify(pme);
}

/*===========================================================================
   This function redraws the current window.
   This function is called either by IShell in response to the resume scheduled
   in CTopSoupApp_Redraw() or by CTopSoupApp_Redraw() directly.
===========================================================================*/
static void CTopSoupApp_RedrawNotify(CTopSoupApp * pme)
{
   if (!pme->m_bRedraw)
      return;

   pme->m_bRedraw = FALSE;

   if (pme->m_pWin)
   {
      IWINDOW_Enable(pme->m_pWin);
      IWINDOW_Redraw(pme->m_pWin);
   }
}

static void CTopSoupApp_ReleaseRes(CTopSoupApp * pme)
{
	TS_RELEASEIF(pme->m_pHdrImage);
	TS_RELEASEIF(pme->m_pBackImage);
	TS_RELEASEIF(pme->m_pBottomImage);
	TS_RELEASEIF(pme->m_pHdrStatic);
	TS_RELEASEIF(pme->m_pLeftSoftStatic);
	TS_RELEASEIF(pme->m_pMidSoftStatic);
	TS_RELEASEIF(pme->m_pRightSoftStatic);


	CTopSoupApp_CancelRedraw(pme);
	TS_RELEASEWIN(pme->m_pWin);
}


/*===============================================================================
                        SMS & TELEPHONE TEST
=============================================================================== */

//解析短信内容
//格式: 目标位置:1111#纬度:E,20.012345#经度:N,120.012345
static boolean CTopSoupApp_SaveSMSMessage(CTopSoupApp* pme, char* szMsg)
{
	char *pszTok = NULL;
	char szBuf[128];
	char *pBuf = NULL;
	char szTmp[32];
	char szDesc[32], szLon[32], szLat[32];
	AECHAR textDesc[32], textLon[32], textLat[32];
	int len = 0;
	AECHAR prompt[32];
	
	if (szMsg == NULL || STRLEN(szMsg) < 10)
		return FALSE;

	MEMSET(textDesc,0,sizeof(textDesc));
	MEMSET(textLat,0,sizeof(textLat));
	MEMSET(textLon,0,sizeof(textLon));
	
	STRCPY(szBuf, szMsg);
	pBuf = szBuf;
	
	//解析短信
	
	//#1
	pszTok = STRCHR(pBuf, '#');
	if (pszTok == NULL)
		return FALSE;
	len = pszTok-pBuf;
	MEMCPY(szTmp, pBuf, len);
	szTmp[len] = 0;
	pBuf = pszTok + 1;	//偏移过#
	
	pszTok = STRCHR(szTmp, ':');
	if (pszTok == NULL)
		return FALSE;

	STRCPY(szDesc, pszTok+1);
	STRTOWSTR(szDesc,textDesc,sizeof(textDesc));
	DBGPRINTF("@textDesc:%s", szDesc);

	//#2
	pszTok = STRCHR(pBuf, '#');
	if (pszTok == NULL)
		return FALSE;

	len = pszTok-pBuf;
	MEMCPY(szTmp, pBuf, len);
	szTmp[len] = 0;
	pBuf = pszTok + 1;	//偏移过#

	pszTok = STRCHR(szTmp, ',');
	if (pszTok == NULL)
		return FALSE;

	STRCPY(szLat, pszTok+1);
	STRTOWSTR(szLat,textLat,sizeof(textLat));
	DBGPRINTF("@szLat: %s", szLat);

	//#3
	pszTok = STRCHR(pBuf, ',');
	if (pszTok == NULL)
		return FALSE;

	STRCPY(szLon, pszTok+1);
	STRTOWSTR(szLon,textLon,sizeof(textLon));
	DBGPRINTF("@szLon:%s", szLon);

	//保存到数据库
	if ( -1 == TS_AddExpenseItemOnce(pme, textDesc, textLat, textLon))
	{
		DBGPRINTF("SAVE DATA ERROR!");//TODO 界面提示
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_SAVE,prompt,sizeof(prompt));
		
		//提示窗口
		MEMSET(pme->m_pTextctlText,0,sizeof(pme->m_pTextctlText));	  
		WSTRCPY(pme->m_pTextctlText, textDesc);	   
		//TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
		TS_DrawSplash(pme,prompt,1500,0, 0);
		return TRUE;
	} else if( 0 == TS_AddExpenseItemOnce(pme, textDesc, textLat, textLon) )
	{
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SAVE,prompt,sizeof(prompt));

		//提示窗口
		MEMSET(pme->m_pTextctlText,0,sizeof(pme->m_pTextctlText));	  
		WSTRCPY(pme->m_pTextctlText, textDesc);	   
		//TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
		TS_DrawSplash(pme,prompt,500,0, 0);
	}else if( 1 == TS_AddExpenseItemOnce(pme, textDesc, textLat, textLon) ) {
        DBGPRINTF("DATA EXIST!");
	}
	
	//提示信息
	//TS_SPLAH
	
	return TRUE;
}


static boolean CTopSoupApp_SaveSMSMessageUnicode(CTopSoupApp* pme, AECHAR* szMsg)
{
	AECHAR *pszTok = NULL;
	AECHAR szBuf[128];
	AECHAR *pBuf = NULL;
	AECHAR szTmp[32];
	AECHAR textDesc[32], textLon[32], textLat[32];
	int len = 0;
	AECHAR prompt[32];

	if (szMsg == NULL || WSTRLEN(szMsg) < 10)
		return FALSE;

	MEMSET(textDesc,0,sizeof(textDesc));
	MEMSET(textLat,0,sizeof(textLat));
	MEMSET(textLon,0,sizeof(textLon));
	

	MEMSET(szBuf,0,sizeof(szBuf));
	WSTRCPY(szBuf, szMsg);
	pBuf = szBuf;

	//解析短信

	//#1
	pszTok = WSTRCHR(pBuf, L'#');
	if (pszTok == NULL)
		return FALSE;
	len = pszTok-pBuf;
	MEMCPY(szTmp, pBuf, len*sizeof(AECHAR));
	szTmp[len] = 0;
	pBuf = pszTok + 1;	//偏移过#

	pszTok = WSTRCHR(szTmp, L':');
	if (pszTok == NULL)
		return FALSE;

	WSTRCPY(textDesc, pszTok+1);

	//#2
	pszTok = WSTRCHR(pBuf, L'#');
	if (pszTok == NULL)
		return FALSE;

	len = pszTok-pBuf;
	MEMCPY(szTmp, pBuf, len*sizeof(AECHAR));
	szTmp[len] = 0;
	pBuf = pszTok + 1;	//偏移过#

	pszTok = WSTRCHR(szTmp, L',');
	if (pszTok == NULL)
		return FALSE;

	WSTRCPY(textLat, pszTok+1);

	//#3
	pszTok = WSTRCHR(pBuf, L',');
	if (pszTok == NULL)
		return FALSE;

	WSTRCPY(textLon, pszTok+1);


	//保存到数据库
	if ( -1 == TS_AddExpenseItemOnce(pme, textDesc, textLat, textLon))
	{
		DBGPRINTF("SAVE DATA ERROR!");//TODO 界面提示
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_INVALID_SAVE,prompt,sizeof(prompt));

		//提示窗口
		MEMSET(pme->m_pTextctlText,0,sizeof(pme->m_pTextctlText));	  
		WSTRCPY(pme->m_pTextctlText, textDesc);	   
		//TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
		TS_DrawSplash(pme,prompt,1500,0, 0);
		return TRUE;
	} else if ( 0 == TS_AddExpenseItemOnce(pme, textDesc, textLat, textLon) )
	{
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_PROMPT_ALREADY_SAVE,prompt,sizeof(prompt));

		//提示窗口
		MEMSET(pme->m_pTextctlText,0,sizeof(pme->m_pTextctlText));	  
		WSTRCPY(pme->m_pTextctlText, textDesc);	   
		//TS_DrawSplash(pme->m_pOwner,prompt,1000,(PFNNOTIFY)CNewdestFuctionWin_onSplashDrawOver);
		TS_DrawSplash(pme,prompt,500,0, 0);
	} else if( 1 == TS_AddExpenseItemOnce(pme, textDesc, textLat, textLon) ) {
		DBGPRINTF("DATA EXIST!");//TODO 界面提示
	}

	//提示信息
	//TS_SPLAH

	return TRUE;
}

static boolean CTopSoupApp_ReceiveSMSMessage(CTopSoupApp* pme, uint32 uMsgId)
{
	ISMSMsg *pSMS  = NULL;
	if(ISMS_ReceiveMsg(pme->m_pISMS, uMsgId, &pSMS) ==AEE_SUCCESS) {
		SMSMsgOpt TmpOpt;
		char szPhone[32], szText[256];
		MEMSET(szPhone, 0, sizeof(szPhone));
		MEMSET(szText, 0, sizeof(szText));

		if(ISMSMSG_GetOpt(pSMS, MSGOPT_FROM_DEVICE_SZ,&TmpOpt)==AEE_SUCCESS) {
			STRCPY(szPhone, (char*)TmpOpt.pVal);
			//@yao 短信中心和亲情号码
			//if( NULL == STRSTR("10659031107260,18931880692,18903110989",szPhone) ) {
			//	DBGPRINTF("sms sender is not in center list");
			//	return FALSE;
			//}

			//短信内容:目标位置:北京天安门,经度:E,114.000纬度:N,33.2222
			///        目标位置:北京天安门#经度:E,114.000#纬度:N,33.2222
			if(ISMSMSG_GetOpt(pSMS, MSGOPT_PAYLOAD_WSZ,&TmpOpt)==AEE_SUCCESS) {
				//WSTRTOUTF8((AECHAR*)TmpOpt.pVal,WSTRLEN((AECHAR*)TmpOpt.pVal), (byte*)szText, sizeof(szText));
				//WSTRTOSTR((AECHAR*)TmpOpt.pVal, (char*)szText, WSTRLEN((AECHAR*)TmpOpt.pVal));//, sizeof(szText));
				//DBGPRINTF("WSZ msg from %s: %s",szPhone,(AECHAR*)TmpOpt.pVal);
				CTopSoupApp_SaveSMSMessageUnicode(pme, (AECHAR*)TmpOpt.pVal);
			} else if( ISMSMSG_GetOpt(pSMS,MSGOPT_PAYLOAD_SZ,&TmpOpt) == AEE_SUCCESS ) {
				STRCPY(szText,(const char*)(TmpOpt.pVal));
				DBGPRINTF("SZ msg from %s: %s",szPhone,szText);
				CTopSoupApp_SaveSMSMessage(pme, szText);
			} else {
				DBGPRINTF("Can not get sms text");
				return FALSE;
			}
		} else {
			DBGPRINTF("Can not get sms number: ");
		}
		ISMSMSG_Release(pSMS);
	}
	return TRUE;
}

static void SMSCallBack_Send(void *p)   
{   
   CTopSoupApp *pme = (CTopSoupApp *)p;
   uint16 errType = AEESMS_GETERRORTYPE(pme->m_retVal);
   uint16 err = AEESMS_GETERROR(pme->m_retVal);

   SMSMsgOpt smo[2];

   DBGPRINTF("SMSCallBack_Send called");   

   smo[1].nId = MSGOPT_END;
   smo[1].pVal = NULL;

   switch(err) {
      case AEESMS_ERROR_NONE:
         DBGPRINTF("Sent OK");
         break;

      default:
         DBGPRINTF("Error type: %d \n", errType);
         DBGPRINTF("Error = %d \n", err);
         DBGPRINTF("Error!");
         break;
   }

   // 短信发送完毕后，释放ISMSMsg，否则它将保留在其中，对下一条短信产生影响   
   if(pme->m_pISMSMsg != NULL)   
   {   
       ISMSMSG_Release(pme->m_pISMSMsg);   
       pme->m_pISMSMsg = NULL;   
   }

   //如果开启了SOS模式，则需要通知上层短信已发送成功
   if (pme->m_bEnableSOS && pme->m_OP == SOS_SMS_SENDING)
   {
       DBGPRINTF("@SMSCallBack_Send SMS");
       if (!ISHELL_SendEvent(pme->a.m_pIShell, AEECLSID_NAVIGATE, EVT_SMS_END, 0, 0)) {
           DBGPRINTF("ISHELL_SendEvent EVT_SMS_END failure");
       }
   }
}

void CTopSoupApp_SendSMSMessage (CTopSoupApp * pme, uint16 wParam, AECHAR *szDesc,AECHAR* lat,AECHAR* lon,char* phoneNumber)
{   
	AECHAR textDest[32], textLat[32], textLon[32];
	AECHAR szLat[32], szLon[32];
	
	// Make sure the pointers we'll be using are valid   
	if (pme == NULL || pme->a.m_pIShell == NULL || pme->a.m_pIDisplay == NULL)   
		return;
	
	switch (wParam)   
	{   
	case USAGE_SMS_TX_ASCII:   
		{
			int nErr;   
			WebOpt awo[6]; /* ***IMPORTANT**** grow this if you add more   
			WebOpts here, or shrink it and call AddOpt() multiple times */   
			int    i = 0;   
			uint32 nReturn=0;                                    
			
			
			nErr =ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_SMSMSG, (void **)&pme->m_pISMSMsg);   
			DBGPRINTF("CreateInstance of AEECLSID_SMSMSG ret %d", nErr);   
			if(nErr != AEE_SUCCESS)   
			{   
				break;   
			}   
			
			/* NULL terminated string providing destination device number.   
			'+' as first character signifies international number.  */   
			awo[i].nId  = MSGOPT_TO_DEVICE_SZ ;   
			awo[i].pVal = (void *)phoneNumber;   
			i++;   
			
			/* ascii text to be send */   
			awo[i].nId  = MSGOPT_PAYLOAD_SZ ;   
			awo[i].pVal = (void *)MO_TEXT_ASCII;   
			i++;   
			
			/* encoding */   
			awo[i].nId  = MSGOPT_PAYLOAD_ENCODING;   
			awo[i].pVal = (void *)AEE_ENC_ISOLATIN1;   
			i++;   
			
			awo[i].nId  = MSGOPT_MOSMS_ENCODING;   
			awo[i].pVal = (void *)AEESMS_ENC_ASCII;   
			i++;   
#if 0   
			/* user ack */   
			awo[i].nId  = MSGOPT_USER_ACK;   
			awo[i].pVal = (void *)TRUE;   
			i++;   
#endif   
			/* this is absolutely necessary, do not remove, marks the end of the   
			array of WebOpts */   
			awo[i].nId  = WEBOPT_END;   
			
			/* add 'em */   
			nErr =ISMSMSG_AddOpt(pme->m_pISMSMsg, awo);   
			DBGPRINTF("ISMSMSG_AddOpt ret %d", nErr);   
			
			CALLBACK_Init(&pme->m_cb, SMSCallBack_Send, pme);   
			ISMS_SendMsg(pme->m_pISMS, pme->m_pISMSMsg, &pme->m_cb, &pme->m_retVal);   
			
			// Higher 16 bits specify error type specified as AEESMS_ERRORTYPE_XXX    
			// lower  16 bits specify error specified as AEESMS_ERROR_XXX   
			DBGPRINTF("ISMS_SendMsg ret 0x%x", nReturn);   
		} 
		break;   
		
	case USAGE_SMS_TX_UNICODE:   
		{ 
			int nErr;   
			WebOpt awo[6]; /* ***IMPORTANT**** grow this if you add more   
			WebOpts here, or shrink it and call AddOpt() multiple times */   
			int    i = 0;   
			uint32 nReturn=0;                                    
			AECHAR pszBuf[100];   
			
			nErr =ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_SMSMSG, (void **)&pme->m_pISMSMsg);   
			DBGPRINTF("CreateInstance of AEECLSID_SMSMSG ret %d", nErr);   
			if(nErr != AEE_SUCCESS)   
			{   
				break;   
			}   
			
			/* NULL terminated string providing destination device number.   
			'+' as first character signifies international number.  */   
			awo[i].nId  = MSGOPT_TO_DEVICE_SZ ;   
			awo[i].pVal = (void *)phoneNumber;   
			i++;   
			
			/* unicode text to be send */   
			awo[i].nId  = MSGOPT_PAYLOAD_WSZ ;   
#if TEST_SMS
			ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_MO_TEXT_UNICODE, pszBuf, sizeof(pszBuf));
			{
				char szbuf[128];
				WSTRTOSTR(pszBuf, szbuf, WSTRLEN(pszBuf) + 1);
				DBGPRINTF(szbuf);
			}
#else
			ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_MO_TEXT_DEST, textDest, sizeof(textDest));
			ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_EDIT_LAT, textLat, sizeof(textLat));
			ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_EDIT_LON, textLon, sizeof(textLon));
			
			{
				if ( NULL == lat || NULL == lon ){
					char szBuf[32];
					TS_FLT2SZ(szLat, pme->m_gpsInfo.theInfo.lat);
					//FLOATTOWSTR(pme->m_gpsInfo.theInfo.lat, szLat, 32);
					WSTRTOSTR(szLat, szBuf, sizeof(szBuf));
					DBGPRINTF("Lat: %s", szBuf);

					TS_FLT2SZ(szLon, pme->m_gpsInfo.theInfo.lon);
					//FLOATTOWSTR(pme->m_gpsInfo.theInfo.lon, szLat, 32);
					WSTRTOSTR(szLon, szBuf, sizeof(szBuf));
					DBGPRINTF("Lon: %s", szBuf);
				} else {
					MEMSET(szLat,0,sizeof(szLat));
					MEMSET(szLon,0,sizeof(szLon));

					MEMCPY(szLat,lat,sizeof(szLat));
					MEMCPY(szLon,lon,sizeof(szLon));
				}
			}
			
			WSPRINTF(pszBuf, sizeof(pszBuf), L"%s:%s#%s:E,%s#%s:N,%s", textDest, szDesc, textLat, szLat, textLon, szLon);
#endif
			awo[i].pVal = (void *)pszBuf;   
			i++;   
			
			/* encoding */   
			awo[i].nId  = MSGOPT_PAYLOAD_ENCODING;   
			awo[i].pVal = (void *)AEE_ENC_UNICODE ;   
			i++;   
			
			awo[i].nId  = MSGOPT_MOSMS_ENCODING;   
			awo[i].pVal = (void *)AEESMS_ENC_UNICODE;   
			i++;   
			
#if 0   
			/* user ack */   
			awo[i].nId  = MSGOPT_READ_ACK;   
			awo[i].pVal = (void *)TRUE;   
			i++;   
#endif   
			
			/* this is absolutely necessary, do not remove, marks the end of the   
			array of WebOpts */   
			awo[i].nId  = MSGOPT_END;   
			
			/* add 'em */   
			nErr =ISMSMSG_AddOpt(pme->m_pISMSMsg, awo);   
			DBGPRINTF("ISMSMSG_AddOpt ret %d", nErr);   
			
			CALLBACK_Init(&pme->m_cb, SMSCallBack_Send, pme);   
			ISMS_SendMsg(pme->m_pISMS, pme->m_pISMSMsg, &pme->m_cb, &pme->m_retVal);   
			
			// Higher 16 bits specify error type specified as AEESMS_ERRORTYPE_XXX    
			// lower  16 bits specify error specified as AEESMS_ERROR_XXX   
			DBGPRINTF("ISMS_SendMsg ret 0x%x", nReturn);   
		}  
		break;
		
	default:   
		return;   
    }
	
    return;   
}


void CTopSoupApp_SendSOSSMSMessage (CTopSoupApp * pme, uint16 wParam, AECHAR *szDesc, char* phoneNumber)
{
    // Make sure the pointers we'll be using are valid
    if (pme == NULL || pme->a.m_pIShell == NULL || pme->a.m_pIDisplay == NULL)
        return;

    if (USAGE_SMS_TX_UNICODE == wParam)
    {
        int nErr;
        WebOpt awo[6]; /* ***IMPORTANT**** grow this if you add more
        WebOpts here, or shrink it and call AddOpt() multiple times */
        int    i = 0;
        uint32 nReturn=0;
        //AECHAR pszBuf[100];

        nErr =ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_SMSMSG, (void **)&pme->m_pISMSMsg);
        DBGPRINTF("CreateInstance of AEECLSID_SMSMSG ret %d", nErr);
        if(nErr != AEE_SUCCESS)
            return;

        /* NULL terminated string providing destination device number.
        '+' as first character signifies international number.  */
        awo[i].nId  = MSGOPT_TO_DEVICE_SZ ;
        awo[i].pVal = (void *)phoneNumber;
        i++;

        /* unicode text to be send */
        awo[i].nId  = MSGOPT_PAYLOAD_WSZ ;
        //WSTRCPY(pszBuf, szDesc);
        awo[i].pVal = (void *)szDesc;
        i++;
        DBGPRINTF("ISMSMSG_AddOpt MSGOPT_PAYLOAD_WSZ");

        /* encoding */
        awo[i].nId  = MSGOPT_PAYLOAD_ENCODING;
        awo[i].pVal = (void *)AEE_ENC_UNICODE ;
        i++;

        awo[i].nId  = MSGOPT_MOSMS_ENCODING;
        awo[i].pVal = (void *)AEESMS_ENC_UNICODE;
        i++;

#if 0
        /* user ack */
        awo[i].nId  = MSGOPT_READ_ACK;
        awo[i].pVal = (void *)TRUE;
        i++;
#endif

        /* this is absolutely necessary, do not remove, marks the end of the
        array of WebOpts */
        awo[i].nId  = MSGOPT_END;

        /* add 'em */
        nErr =ISMSMSG_AddOpt(pme->m_pISMSMsg, awo);
        DBGPRINTF("ISMSMSG_AddOpt ret %d", nErr);

        CALLBACK_Init(&pme->m_cb, SMSCallBack_Send, pme);
        ISMS_SendMsg(pme->m_pISMS, pme->m_pISMSMsg, &pme->m_cb, &pme->m_retVal);

        // Higher 16 bits specify error type specified as AEESMS_ERRORTYPE_XXX
        // lower  16 bits specify error specified as AEESMS_ERROR_XXX
        DBGPRINTF("ISMS_SendMsg ret 0x%x", nReturn);
    }
    return;
}

static void CTopSoupApp_OriginateListener(CTopSoupApp *pme, ModelEvent *pEvent)
{
	AEETCallEvent* pCallEvent = (AEETCallEvent*) pEvent;
	int ret = 0;

	switch (pCallEvent->evCode)
	{
	case AEET_EVENT_CALL_CONNECT:
		{
			DBGPRINTF("Rx:   AEET_EVENT_CALL_CONNECT cd=%d", pCallEvent->call.cd);
			break;
		}
		
	case AEET_EVENT_CALL_ERROR:
		{
			DBGPRINTF("AEET_EVENT_CALL_ERROR");
			LISTENER_Cancel(&pme->callListener);
			ret = 1;
			break;
		}
		
	case AEET_EVENT_INBAND_FWD_BURST_DTMF:
		{
			DBGPRINTF("AEET_EVENT_INBAND_FWD_BURST_DTMF");
			break;
		}
		
	case AEET_EVENT_CALL_END:
		{
			DBGPRINTF("Rx:   AEET_EVENT_CALL_END");
			LISTENER_Cancel(&pme->callListener);
			if (pme->m_pOutgoingCall != NULL)
			{
				ICALL_End(pme->m_pOutgoingCall);
				TS_RELEASEIF(pme->m_pOutgoingCall);
			}
            ret = 1;

			break;
		}
		
	default:
		{
			DBGPRINTF("OriginateListener Rx:  %d", pCallEvent->evCode);
			break;
		}
	}

    //CALL结束，如果是SOS模式，则需要发送CALL_END消息
    if (ret == 1)
    {
        DBGPRINTF("@Call End!");
        //如果开启了SOS模式，则需要通知上层短信已发送成功
        if (pme->m_bEnableSOS && pme->m_OP == SOS_CALL_CALLING)
        {
            if (!ISHELL_SendEvent(pme->a.m_pIShell, AEECLSID_NAVIGATE, EVT_CALL_END, 0, 0)) {
                DBGPRINTF("ISHELL_SendEvent EVT_CALL_END failure");
            }
        }
    }
}

//For SOS Test
static void CTopSoupApp_MakeSOSCall(CTopSoupApp * pme, char* szNumber)
{
	int Result;

	LISTENER_Init(&pme->callListener, (PFNLISTENER) CTopSoupApp_OriginateListener, pme);
	Result = ICALLMGR_Originate(pme->m_pCallMgr, AEET_CALL_TYPE_VOICE, szNumber, NULL, &pme->m_pOutgoingCall, &pme->callListener);
	DBGPRINTF("ICALLMGR_Originate return with res=%d", Result);
}

static void CTopSoupApp_EndSOSCall(CTopSoupApp * pme)
{
	DBGPRINTF("CTopSoupApp_EndSOSCall in %x", pme->m_pOutgoingCall);
	if (pme->m_pOutgoingCall != NULL)
	{
		ICALL_End(pme->m_pOutgoingCall);
		TS_RELEASEIF(pme->m_pOutgoingCall);
	}
}

/************************************************************************/
/* 从配置文件加载亲友联系方式,只记录有效的地址                          */
/************************************************************************/
static uint32 LoadSOSConfig(IShell *iShell, char szNum[3][32])
{
    IFileMgr	*pIFileMgr = NULL;
    IFile		*pIFile = NULL;
    IShell		*pIShell = NULL;

    char 	szBuf[128];
    char    *pszBuf = NULL;
    char    *pszTok = NULL;
    int32	nResult = 0;
    FileInfo	fiInfo;
    char    szA[32], szB[32], szC[32];
    uint32 len = 0;
    int i =0;

    pIShell = iShell;

    // Create the instance of IFileMgr
    nResult = ISHELL_CreateInstance(pIShell, AEECLSID_FILEMGR, (void**)&pIFileMgr);
    if (SUCCESS != nResult) {
        DBGPRINTF("Create AEECLSID_FILEMGR Failed!");
        return nResult;
    }

    nResult = IFILEMGR_Test(pIFileMgr, RELATIVE_ADDRESS_CFG);
    if (nResult != SUCCESS)
    {
        DBGPRINTF("CONFIG NOT EXIST!");
        IFILEMGR_Release(pIFileMgr);
        return SUCCESS;
    }

    pIFile = IFILEMGR_OpenFile(pIFileMgr, RELATIVE_ADDRESS_CFG, _OFM_READ);
    if (!pIFile) {
        DBGPRINTF("Open Configure File Failed! %s", RELATIVE_ADDRESS_CFG);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    if (SUCCESS != IFILE_GetInfo(pIFile, &fiInfo)) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    if (fiInfo.dwSize == 0) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    //only use 128
    len = fiInfo.dwSize;
    if (len >= 128)
        len = 127;

    MEMSET(szBuf, 0, 128);
    nResult = IFILE_Read(pIFile, szBuf, len);
    if ((uint32)nResult < len) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }

    pszBuf = szBuf;

    //查找第一个联系人号码
    MEMSET(szA,0,sizeof(szA));
    pszTok = STRCHR(pszBuf, '#');
    if (pszTok == NULL) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }
    len = pszTok-pszBuf;
    if (len > 1) {
        MEMCPY(szA, pszBuf, len * sizeof(char));
        szA[len] = 0;
    }
    pszBuf = pszTok + 1;
    DBGPRINTF("szA:%s", szA);

    //查找第二个联系人号码
    MEMSET(szB,0,sizeof(szB));
    pszTok = STRCHR(pszBuf, '#');
    if (pszTok == NULL) {
        IFILE_Release(pIFile);
        IFILEMGR_Release(pIFileMgr);
        return EFAILED;
    }
    len = pszTok-pszBuf;
    if (len > 1) {
        MEMCPY(szB, pszBuf, len * sizeof(char));
        szB[len] = 0;
    }
    pszBuf = pszTok + 1;
    DBGPRINTF("szB:%s", szB);

    //查找第三个联系人号码
    MEMSET(szC,0,sizeof(szC));
    len = fiInfo.dwSize-(pszBuf-szBuf);
    if (len > TS_MIN_RELATIVE_NUM && len < TS_MAX_RELATIVE_NUM)
    {
        MEMCPY(szC, pszBuf, len);
        szC[len] = 0;
    }
    DBGPRINTF("szC:%s", szC);

    i = 0;
	if (STRLEN(szA) > 0)
	{
		STRCPY(szNum[i++], szA);
	}

	if (STRLEN(szB) > 0)
	{
        STRCPY(szNum[i++], szB);
	}

	if (STRLEN(szC) > 0)
	{
        STRCPY(szNum[i++], szC);
	}

	IFILE_Release(pIFile);
	IFILEMGR_Release(pIFileMgr);

	return (i == 0) ? EFAILED : SUCCESS;
}

//构建SOS短信：
//pos == NULL： 开启求助短信
//pos != NULL:  发送带位置信息的求助短信
static void CTopSoupApp_MakeSOSMsg(CTopSoupApp *pme, AECHAR szMsg[256], Coordinate *pos)
{
    ts_time_t now;
    char szBuf[128];
    AECHAR szTmp[128];
    AECHAR szSOSInfo[32];
    AECHAR szMon[6];
    AECHAR szDay[6];
    AECHAR szHour[6];
    AECHAR szMinute[6];
    AECHAR szTail[32];

    TS_GetTimeNow(&now);

    ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MONTH,szMon,sizeof(szMon));  // 月
    ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_DAY,szDay,sizeof(szDay));    // 日
    ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_HOUR,szHour,sizeof(szHour)); // 时
    ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MIN,szMinute,sizeof(szMinute)); // 分

    if (pos == NULL)
    {
        ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SOS_SMS,szSOSInfo,sizeof(szSOSInfo));
        ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_MIN_OPEN,szTail,sizeof(szTail));   // 分开启求助

        //1 构建开启求助短信：求助信息! 4月18日20时18分开启求助
        WSPRINTF(szTmp, sizeof(szTmp), L"%s%d%s%d%s", szSOSInfo, now.month, szMon, now.day, szDay);
        WSPRINTF(szMsg, sizeof(AECHAR)*256, L"%s%d%s%d%s", szTmp, now.hour, szHour, now.minute, szTail);
    }
    else
    {
        AECHAR szTmp2[128];
        AECHAR textLat[32], textLon[32];
        AECHAR szLat[32], szLon[32];
        ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_SOS_SMS_LAST,szSOSInfo,sizeof(szSOSInfo));
        ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_EDIT_LAT, textLat, sizeof(textLat));
        ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_EDIT_LON, textLon, sizeof(textLon));
        ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,IDS_STRING_IS_NAVIGATE,szTail,sizeof(szTail));   // 分开启求助

        TS_FLT2SZ(szLat, pos->lat);
        //FLOATTOWSTR(pme->m_gpsInfo.theInfo.lat, szLat, 32);
        WSTRTOSTR(szLat, szBuf, sizeof(szBuf));
        DBGPRINTF("Lat: %s", szBuf);

        TS_FLT2SZ(szLon, pos->lon);
        //FLOATTOWSTR(pme->m_gpsInfo.theInfo.lon, szLat, 32);
        WSTRTOSTR(szLon, szBuf, sizeof(szBuf));
        DBGPRINTF("Lon: %s", szBuf);

        //2 构建位置信息的求助短信：求助信息: 最后位置4月18日20时21分在东经114度27.947分，北纬38度5.280分，是否想该位置领航?
        //2 构建位置信息的求助短信：求助信息! 最后位置:4月18日20时21分#纬度:E,20.012345#经度:N,120.012345, 是否想该位置领航?

        //求助信息! 最后位置:4月18日
        WSPRINTF(szTmp, sizeof(szTmp), L"%s%d%s%d%s", szSOSInfo, now.month, szMon, now.day, szDay);
        //TMP+20时21分#
        WSPRINTF(szTmp2, sizeof(szTmp2), L"%s%d%s%d%s#", szTmp, now.hour, szHour, now.minute, szMinute);
        //TMP2+纬度:E,20.012345#经度:N,120.012345, 是否想该位置领航?
        WSPRINTF(szMsg, sizeof(AECHAR)*256, L"%s%s:E,%s#%s:N,%s, %s", szTmp2, textLat, szLat, textLon, szLon, szTail);
    }

    WSTRTOSTR(szMsg, szBuf, sizeof(szBuf));

    DBGPRINTF("@MakeSOSMsg:%s", szBuf);
}

static void CTopSoupApp_onSplashCall(void * po)
{
    CTopSoupApp* pme = (CTopSoupApp*)po;

    ISHELL_CloseApplet(pme->a.m_pIShell, FALSE);
}

//SOS功能：如果有亲友号码，则开启SOS，并给每个号码发送短信，和拨打电话，开启定位，定位成功后将位置信息通过短信发送
//加载配置文件
static void CTopSoupApp_StartSOS(CTopSoupApp *pme) {
	int ret = EFAILED;
    int i = MAX_SOS_NUM;

    //显示提示启用SOS
    {
        AECHAR prompt[TS_MAX_STRLEN];
        ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_SOS_PROMPT, prompt, sizeof(prompt));
        TS_DrawSplash(pme, prompt, 10000, 0, 0);
    }

    if (SUCCESS == LoadSOSConfig((IShell *) pme->a.m_pIShell, pme->m_szNum)) {
        for (i = 0; i < MAX_SOS_NUM; i++) {
            if (STRLEN(pme->m_szNum[i]) > 0) {
                AECHAR szMsg[256];
                CTopSoupApp_MakeSOSMsg(pme, szMsg, NULL);
				DBGPRINTF("@SOS Send SMS To Num: %s Msg len:%d", pme->m_szNum[i], WSTRLEN(szMsg));
                CTopSoupApp_SendSOSSMSMessage(pme, USAGE_SMS_TX_UNICODE, szMsg, pme->m_szNum[i]);
                break;
            }
            else {
                DBGPRINTF("@SOS Num Is Empty:%d", i);
            }
        }
    }

    if (i < MAX_SOS_NUM) {
        ret = SUCCESS;
    }
    else {
        ret = EFAILED;
    }

    if (ret ==  EFAILED)
    {
        DBGPRINTF("@No SOS Num ");
        //TODO alert message!
        pme->m_id = -1;
        pme->m_OP = SOS_IDLE;
        pme->m_bEnableSOS = FALSE;

        ISHELL_CloseApplet(pme->a.m_pIShell, FALSE);

        //显示配置SOS联系人号码提示
        //{
        //    AECHAR prompt[TS_MAX_STRLEN];
        //  TS_DrawSplash_Stop(pme);
        //    ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_STRING_SOS_CFG_ADDRESS, prompt, sizeof(prompt));
        //    TS_DrawSplash(pme, prompt, 5000, CTopSoupApp_onSplashCall, 0);
        //}

        //配置联系人
        //pme->m_eActiveWin = TSW_SOS;
        //CTopSoupApp_SetWindow(pme, TSW_SOS, 0);
    }
    else
    {
        //记录当前亲友索引和更改SOS状态
        pme->m_id = i;
        pme->m_OP = SOS_SMS_SENDING;
    }
}




