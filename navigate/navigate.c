#include "logicmacro.h"

/*-------------------------------------------------------------------
            Function Prototypes
-------------------------------------------------------------------*/
static boolean    CTopSoupApp_InitAppData(IApplet* po);
//FIXME why origin is not static
static void		  CTopSoupApp_FreeAppData(IApplet* po);
static boolean    CTopSoupApp_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CTopSoupApp_RedrawNotify(CTopSoupApp * pme);
static void		  CTopSoupApp_ReleaseRes(CTopSoupApp * pme);

/*===============================================================================
                        SMS & TELEPHONE TEST
=============================================================================== */
#define USAGE_SMS_TX_ASCII      100   
#define USAGE_SMS_TX_UNICODE    101

// PHONE NUMBER
#define DESTINATION_NUMBER "15511823090" 
 
// ascii 短信内容，对于unicode短信内容，必须由资源文件bar中获取，否则编码不对 
#define MO_TEXT_ASCII "Destination:Beijing#lat:37.123456#lon:114.121345" 

static void		  CTopSoupApp_SendSMSMessage(CTopSoupApp * pMe, uint16 wParam);
static boolean    CTopSoupApp_ReceiveSMSMessage(CTopSoupApp *pme, uint32 uMsgId);
static void		  CTopSoupApp_MakeSOSCall(CTopSoupApp * pme, char* szNumber);
static void		  CTopSoupApp_EndSOSCall(CTopSoupApp * pme);

//XXX
//#define MP_SPLASH_TIMER       750
//static void       CTopSoupApp_DrawSplash(CTopSoupApp * pme);


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
#define TS_HEADER_Y           17
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
   
   // Get screen pixel count
   pdi = MALLOC(sizeof(AEEDeviceInfo));
   if (!pdi)
      return FALSE;
   ISHELL_GetDeviceInfo(pme->a.m_pIShell, pdi);
   pme->m_cxWidth = pdi->cxScreen;
   pme->m_cyHeight = pdi->cyScreen;
   pme->m_nColorDepth = pdi->nColorDepth;
   FREEIF(pdi);

   IDISPLAY_GetFontMetrics(pme->a.m_pIDisplay, AEE_FONT_LARGE, &nAscent, &nDescent);
   pme->m_nLChSize = nAscent + nDescent;

   IDISPLAY_GetFontMetrics(pme->a.m_pIDisplay, AEE_FONT_NORMAL, &nAscent, &nDescent);
   pme->m_nNChSize = nAscent + nDescent;


   CALLBACK_Init(&pme->m_cbRedraw, (PFNNOTIFY)CTopSoupApp_RedrawNotify, pme);

   //XXX __begin
   //Header
   pme->m_pHdrImage = ISHELL_LoadResImage(pme->a.m_pIShell, BACKIMG_RES_FILE, IDI_OBJECT_5003);
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

   
   pme->m_pBackImage = ISHELL_LoadResImage(pme->a.m_pIShell,BACKIMG_RES_FILE,IDI_OBJECT_5000);
   if (!pme->m_pBackImage) {	
		CTopSoupApp_ReleaseRes(pme);  	
	   return FALSE;
   }
   IImage_GetInfo(pme->m_pBackImage,&info);
   SETAEERECT(&pme->m_rectBack,0,0,info.cx,info.cy);

   pme->m_pBottomImage = ISHELL_LoadResImage(pme->a.m_pIShell,BACKIMG_RES_FILE,IDI_OBJECT_5004);
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
	
	//SMS
	TS_RELEASEIF(pme->m_pISMS);
	TS_RELEASEIF(pme->m_pISMSMsg);
	
	//Tel
	TS_RELEASEIF(pme->m_pCallMgr);
	
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

    switch ( eCode ) 
    {   
         case EVT_APP_START:   // Process Start event
            //XXX
			pme->m_eActiveWin = TSW_MAIN;
			CTopSoupApp_SetWindow(pme, TSW_MAIN, 0);
            return TRUE;

         case EVT_APP_STOP:        // process STOP event
            return (TRUE);

         case EVT_APP_SUSPEND:
            pme->m_eSuspendWin = pme->m_eActiveWin;
            CTopSoupApp_SetWindow(pme, TSW_NONE, 0);
            return (TRUE);

         case EVT_APP_RESUME:
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
						 DBGPRINTF("Rece unkown mask sms: %x",wp->dwMask);
						 return FALSE;
					 }
				 }
			 }
			 return (TRUE);

         case EVT_KEY:	            // Process key event
			 {
				//FOR SMS & TEL TEST
				if (wParam == AVK_0)
				{
					DBGPRINTF("SEND SMS TEST ...");
					//CTopSoupApp_SendSMSMessage(pme, USAGE_SMS_TX_ASCII);
					CTopSoupApp_SendSMSMessage(pme, USAGE_SMS_TX_UNICODE);
				}

				//FOR TEL TEST
				if (wParam == AVK_1)
				{
					DBGPRINTF("CALL TEST ...");
					CTopSoupApp_MakeSOSCall(pme, "15511823090");
				}

				if (wParam == AVK_2)
				{
					DBGPRINTF("CALL TEST END ...");
					CTopSoupApp_EndSOSCall(pme);
				}
			 }

         case EVT_COMMAND:          // Process menu command event
         case EVT_COPYRIGHT_END:    // Copyright dialog ended
            if (pme->m_pWin)
               return IWINDOW_HandleEvent(pme->m_pWin, eCode, wParam, dwParam);

    }

    return FALSE;
}

/*===========================================================================
   This function draws the splash screen and brings up the main window
   after the splash timer runs out.
===========================================================================*/
/*
static void CTopSoupApp_DrawSplash(CTopSoupApp * pme)
{
   // The following 'If statement' is entered only after the splash timer runs out...
   if (pme->m_eActiveWin == TSW_MAIN)
   {
      CTopSoupApp_SetWindow(pme, TSW_MAIN, 0);
      return;
   }

   // Draw the splash screen, set the timer.
   // The timer callback calls this function and redraws the main window.
   {
      IImage * pi = ISHELL_LoadResImage(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDB_LOGO);

      if (pi)
      {
         AEERect  rect;

         IDISPLAY_ClearScreen(pme->a.m_pIDisplay);
         SETAEERECT(&rect, 0, 0, pme->m_cxWidth, pme->m_cyHeight);
         TS_DrawImage(pi, &rect, TRUE);
         IDISPLAY_Update(pme->a.m_pIDisplay);
         TS_RELEASEIF(pi);
      }

      // Set main window as active and start the timer.
      pme->m_eActiveWin = TSW_MAIN;
      ISHELL_SetTimer(pme->a.m_pIShell, MP_SPLASH_TIMER, (PFNNOTIFY)CTopSoupApp_DrawSplash, pme); }  
}
*/

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
      case TSW_MAIN:       
         pme->m_pWin = CMainWin_New(pme); 
         break;

	  case TSW_WHERE:   
		  pme->m_pWin = CWhereWin_New(pme); 
		  break;
		  
	  case TSW_NAVIGATE:     
		  pme->m_pWin = CNavigateWin_New(pme, (Coordinate*)(dwParam)); 
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
				WSTRTOUTF8((AECHAR*)TmpOpt.pVal,WSTRLEN((AECHAR*)TmpOpt.pVal), (byte*)szText, sizeof(szText));
				//WSTRTOSTR((AECHAR*)TmpOpt.pVal,WSTRLEN((AECHAR*)TmpOpt.pVal), (char*)szText)//, sizeof(szText));
				DBGPRINTF("WSZ msg from %s: %s",szPhone,szText);
			} else if( ISMSMSG_GetOpt(pSMS,MSGOPT_PAYLOAD_SZ,&TmpOpt) == AEE_SUCCESS ) {
				STRCPY(szText,(const char*)(TmpOpt.pVal));
				DBGPRINTF("SZ msg from %s: %s",szPhone,szText);
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
   int dwSecs = 0;

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
}

static void CTopSoupApp_SendSMSMessage (CTopSoupApp * pme, uint16 wParam)  
{   
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
			awo[i].pVal = (void *)DESTINATION_NUMBER;   
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
			awo[i].pVal = (void *)DESTINATION_NUMBER;   
			i++;   
			
			/* unicode text to be send */   
			awo[i].nId  = MSGOPT_PAYLOAD_WSZ ;   
			ISHELL_LoadResString(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDS_MO_TEXT_UNICODE, pszBuf, sizeof(pszBuf));
			{
				char szbuf[128];
				WSTRTOSTR(pszBuf, szbuf, WSTRLEN(pszBuf) + 1);
				DBGPRINTF(szbuf);
			}
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
	
    // Display above event.    
    //DisplayEvent (pme, wParam);   
	
    return;   
}


static void CTopSoupApp_OriginateListener(CTopSoupApp *pme, ModelEvent *pEvent)
{
	AEETCallEvent* pCallEvent = (AEETCallEvent*) pEvent;
	
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
			
			break;
		}
		
	default:
		{
			DBGPRINTF("OriginateListener Rx:  %d", pCallEvent->evCode);
			break;
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









