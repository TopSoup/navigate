/*===========================================================================

FILE: navigate.c

SERVICES: Sample Media Player applet showing usage of BREW IMedia interfaces.

DESCRIPTION
  This file contains the implementation of a navigate applet using the
  AEE IMedia interface services.
  navigate app
  (1) Plays MIDI, MP3, QCP and PMD media formats
  (2) Records QCP file
  (3) Displays BMP, BCI, PNG and JPEG image formats
  (4) Provides playback/record controls: stop, rewind, ff, pause, resume.
  (5) Allows image panning for images
  (6) Provides volume controls
  (7) Can display images/video in Full Screen mode
  (8) Can be started either in stand-alone mode or as a plugin

  Demonstrates usage of
  (1) IMedia API
  (2) AEEMediaUtil_CreateMedia()
  (3) IImageCtl
 
   	   Copyright ?2000-2002 QUALCOMM Incorporated.
	                  All Rights Reserved.
                   QUALCOMM Proprietary/GTDR

===========================================================================*/

/*===============================================================================
                     INCLUDES AND VARIABLE DEFINITIONS
=============================================================================== */
#include "AEEAppGen.h"	      // AEEApplet structure and Applet services
#include "navigate.bid"
#include "navigate.brh"

// AEE Services used by app
#include "AEEStdLib.h"        // AEE Stb Lib Services
#include "AEEMenu.h"          // AEE Menu Services
#include "AEEFile.h"          // AEE File Manager Services
#include "AEEMedia.h"         // AEE Multimedia Services
#include "AEEImageCtl.h"      // AEE ImageCtl Services
#include "AEEMimeTypes.h"     // AEE MIME definitions
#include "AEECamera.h "

#include "AEEMediaUtil.h"     // AEE Media Utility functions
#include "nmdef.h"




/*===========================================================================
                      INTERFACE DEFINITION
===========================================================================*/
//
// This is a generic IWindow interface.
//
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

/*===============================================================================
                     MACROS
=============================================================================== */
#define MP_IWINDOW_SETVTBL(pVtbl, pfnEn, pfnRd, pfnHE, pfnDel) \
   (pVtbl)->Enable      = (pfnEn); \
   (pVtbl)->Redraw      = (pfnRd); \
   (pVtbl)->HandleEvent = (pfnHE); \
   (pVtbl)->Delete      = (pfnDel)

#define MP_RELEASEIF(p)                MP_FreeIF((IBase **)&(p))
#define MP_RELEASEWIN(p)               MP_FreeWin((IWindow **)&(p))
#define MP_WINERR_RETURN(p)            { MP_RELEASEWIN(p); return NULL; }

#define MP_ISPLUGIN(p)                 ((p)->m_pOwner->m_bPlugin)

#define MP_DRAWHEADER(pme)             MP_DrawImage((pme)->m_pOwner->m_pHdrImage, &(pme)->m_pOwner->m_rectHdr, TRUE)

#define MP_ISEVTKEY(e)                 ((e) == EVT_KEY)
#define MP_ISCLR(e)                    (MP_ISEVTKEY(e) && wParam == AVK_CLR)
#define MP_ISEVTCMD(e)                 ((e) == EVT_COMMAND)
#define MP_ISCMD(e, c)                 (MP_ISEVTCMD(e) && (c) == wParam)
#define MP_ISFULLSCRN(p)               ((p)->m_bImage && (p)->m_bFullScreen)

#define CMediaPlayer_CancelRedraw(p)   { CALLBACK_Cancel(&(p)->m_cbRedraw); (p)->m_bRedraw = FALSE; }
#define CMediaPlayer_DisableWin(p)     { IWINDOW_Disable((p)->m_pWin); CMediaPlayer_CancelRedraw(p); }

#define CPlayerWin_IsImage(p)          ((p)->m_eWin == MPPW_IMAGE)
#define CPlayerWin_IsMedia(p)          ((p)->m_eWin == MPPW_PLAY || (p)->m_eWin == MPPW_RECORD)

/*===========================================================================
                      PUBLIC DATA DECLARATIONS
===========================================================================*/

/*-------------------------------------------------------------------
            Defines
-------------------------------------------------------------------*/
#define MP_SPLASH_TIMER       750

#define MP_MAX_STRLEN         64
#define MP_MAX_FILES          32
#define MP_MEDIA_DIR          "media"
#define MP_QCP_REC_FILE       "sample.qcp"
#define MP_EXT_SEPARATOR      ", "

#define MP_VOLUME_STEP        25
#define MP_SEEK_TIME          10000    // 10 seconds
#define MP_PROG_TIMER         2000

#define MP_HEADER_CY          16 
#define MP_ICONVIEWCTL_CY     20

#define MP_PROGBAR_DX_DIV     3     // dx is dx/3 of main rectangle
#define MP_PROGBAR_DY_DIV     2     // dy is dy/2 of main rectangle
#define MP_PROGBAR_DX_OFFSET  4     // dx is reduced by offset
#define MP_CLR_PROG_FILL      MAKE_RGB(0, 128, 192)

#define EVT_CREATEMEDIA       EVT_USER
#define EVT_CREATEMEDIA_QCP   EVT_USER + 1

// Based on Menu style sheet:
#define MENU8_FT                 AEE_FT_NONE
#define MENU8_SELECT_FT          AEE_FT_RAISED
#define MENU8_RO                 AEE_RO_TRANSPARENT
#define MENU8_SELECT_RO          AEE_RO_TRANSPARENT
#define MENU8_COLOR_MASK 	      (MC_BACK | MC_SEL_BACK | MC_SEL_TEXT)
#define MENU8_BACKGROUND	      MAKE_RGB(255,255,204)
#define MENU8_SELECT_BACKGROUND	MAKE_RGB(153, 204, 204)
#define MENU8_SELECT_TEXT	      RGB_BLACK

#define TB8_BACKGROUND	         MAKE_RGB(192,192,192)
#define TB8_SELECT_BACKGROUND	   MAKE_RGB(192, 192, 192)

/*-------------------------------------------------------------------
            Type Declarations
-------------------------------------------------------------------*/
typedef enum MPWindow
{
   MPW_NONE,
   MPW_MAIN,
   MPW_FILELIST,
   MPW_PLAYER,
#if defined(MEDIAPLAYER_SETTINGS)
   MPW_SETTINGS,
#endif // defined(MEDIAPLAYER_SETTINGS)
   MPW_LAST
} MPWindow;

typedef enum MPPlayerWin
{
   MPPW_PLAY,
   MPPW_RECORD,
   MPPW_IMAGE
} MPPlayerWin;

typedef struct CMediaPlayer   CMediaPlayer;
typedef struct CWindow        CWindow;
typedef struct CMainWin       CMainWin;
typedef struct CFileListWin   CFileListWin;
typedef struct CPlayerWin     CPlayerWin;
typedef struct CProgCtl       CProgCtl;

#define INHERIT_CWindow(iname) \
   DECLARE_VTBL(iname) \
   CMediaPlayer * m_pOwner; \
   IShell *       m_pIShell; \
   IDisplay *     m_pIDisplay; \
   flg            m_bActive:1

// Base class of all IWindow objects.
struct CWindow
{
   INHERIT_CWindow(IWindow);
};

// Main window: Displays main menu.
struct CMainWin
{
   INHERIT_CWindow(IWindow);

   IImage *       m_pLogo;
   AEERect        m_rectLogo;
   IMenuCtl *     m_pMainMenu;
   flg            m_bAbout:1;
};

// File list window: Lists files in media dir for selection.
struct CFileListWin
{
   INHERIT_CWindow(IWindow);

   IMenuCtl *     m_pFileMenu;
   char *         m_szFileArray[MP_MAX_FILES];
   int            m_NumFiles; // Number of files read into m_szFileArray
};

// Progress control: includes progress bar and the title
struct CProgCtl
{
   IShell *          m_pIShell;
   IDisplay *        m_pIDisplay;
   IStatic *         m_pTitle;
   AEERect           m_rectMain;
   AEERect           m_rectBar;
};

// Player window: Plays the media.
struct CPlayerWin
{
   INHERIT_CWindow(IWindow);

   MPPlayerWin       m_eWin;           // Window type

   IStatic *         m_pTitle;         // Media title text
   AECHAR            m_szText[MP_MAX_STRLEN];

   IImage *          m_pIdleImage;     // Image rect placeholder
   AEERect           m_rectImage;      // Image/Video rectangle
   AEERect           m_rectImageCopy;  // Saved Image/Video rectangle when full screen is displayed

   char *            m_pszFile;        // Saved file name alloc'd by CMediaPlayer

   CProgCtl          m_ProgCtl;        // ProgCtl
   uint16            m_wPct;

   IMenuCtl *        m_pPlayerMenu;

   AEEMediaData      m_md;             // Media data
   IMedia *          m_pMedia;         // IMedia-based object

   IImageCtl *       m_pImageCtl;      // ImageCtl displaying the selected image
   IImage *          m_pImage;         // Image contained in ImageCtl

   uint32            m_dwTotalTime;    // Total playback time
   uint32            m_dwPlayPos;      // Current playback pos
   uint16            m_wVolume;        // Volume

   flg               m_bPlayRec:1;     // = TRUE, if Playback progress flag
   flg               m_bImage:1;       // = TRUE, if media contains Video/Image component.
   flg               m_bFullScreen:1;  // = TRUE, if Video/Image is displayed full screen
   flg               m_bProgTimer:1;   // = TRUE, if prog timer is running. Regular display is not updated.
};

//
// navigate app global structure.
// Note: m_bPlugin == TRUE indicates that navigate tries to 
// play the file directly in CPlayerWin. Pressing CLR will close
// the app.
//
struct CMediaPlayer
{
   AEEApplet         a;

	int               m_cxWidth;
	int               m_cyHeight;
   uint16            m_nColorDepth;
   int               m_nNChSize;    // Large char size
   int               m_nLChSize;    // Normal char size

   ICamera*			 m_pICamera;
   AEESize			 m_sizePreview;
   AEESize			 m_sizeFrame;

   IImage *          m_pHdrImage;
   AEERect           m_rectHdr;

   MPWindow          m_eActiveWin;  // Active window
   MPWindow          m_eSuspendWin; // Suspended window
   IWindow *         m_pWin;

   uint16            m_wMainWin;    // CurSel of CMainWin
   uint16            m_wFileListWin;// CurSel of CFileListWin

   char *            m_pszAudioExt; // Registered audio extension string: "mid, mp3, ..."
   char *            m_pszVideoExt; // Registered video extension string: "pmd, ..."
   char *            m_pszImageExt; // Registered image extension string: "bmp, png, ..."

   AEECallback       m_cbRedraw;
   flg               m_bRedraw:1;   // Processing redraw

   flg               m_bPlugin:1;   // = TRUE, if navigate is in Plugin mode.
};

static int StartCameraInPreviewMode(CMediaPlayer * pme);

static void CApp_CameraNotify(void * pUser, AEECameraNotify * pn)
{
	CMediaPlayer *   pme = (CMediaPlayer *)pUser;
	
	if (!pme || !pn)
		return;
	
	switch (pn->nStatus)
	{
	case CAM_STATUS_START:
		// Preview has begun...
		break;
		
	case CAM_STATUS_FRAME:
		{
			IBitmap *      pFrame;
			AEEBitmapInfo  bi;
			
			//
			// IMPORTANT NOTE: You need to do IBITMAP_Release(pFrame) after you're done with pFrame.
			//
			if (SUCCESS != ICAMERA_GetFrame(pme->m_pICamera, &pFrame))
				return;
			
			// Get the bitmap info...this can be saved in app global structure.
			IBITMAP_GetInfo(pFrame, &bi, sizeof(bi));
			
			// Display the frame at (0, 0) location of the screen
			IDISPLAY_BitBlt(pme, 0, 0, bi.cx, bi.cy, pFrame, 0, 0, AEE_RO_COPY);
			IBITMAP_Release(pFrame);
			break;
		}
		
	case CAM_STATUS_DONE:
		// ICAMERA_Stop() was called and preview operation stopped.
		break;
		
	case CAM_STATUS_ABORT:
		// Preview got aborted.
		break;
	}
}

/*-------------------------------------------------------------------
            Function Prototypes
-------------------------------------------------------------------*/
static boolean    CMediaPlayer_InitAppData(IApplet* po);
static boolean    CMediaPlayer_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam);
static void       CMediaPlayer_DrawSplash(CMediaPlayer * pme);
static boolean    CMediaPlayer_SetWindow(CMediaPlayer * pme, MPWindow eWin, uint32 dwParam);
static void       CMediaPlayer_Redraw(CMediaPlayer * pme, boolean bDefer);
static void       CMediaPlayer_RedrawNotify(CMediaPlayer * pme);
static void       CMediaPlayer_PlayFile(CMediaPlayer * pme, const char * pszFile);
static void       CMediaPlayer_RecordQCPFile(CMediaPlayer * pme, const char * pszFile);
static boolean    CMediaPlayer_PlayImage(CMediaPlayer * pme, const char * pszFile);
static uint16     CMediaPlayer_FindHandlerType(CMediaPlayer * pme, const char * pszFile);
static uint16     CMediaPlayer_IsExtension(CMediaPlayer * pme, const char * pszFile, char * pszExt, int nExtLen);

// CWindow abstract window
static IWindow *  CWindow_New(int16 nSize, CMediaPlayer * pOwner, VTBL(IWindow) * pvt);
static boolean    CWindow_ProcessEnable(IWindow * po, boolean bEnable);

// Main window
static IWindow *  CMainWin_New(CMediaPlayer * pOwner);
static void       CMainWin_Delete(IWindow * po);
static void       CMainWin_Enable(IWindow * po, boolean bEnable);
static void       CMainWin_Redraw(IWindow * po);
static boolean    CMainWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CMainWin_About(CMainWin * pme);

// File List window
static IWindow *  CFileListWin_New(CMediaPlayer * pOwner);
static void       CFileListWin_Delete(IWindow * po);
static void       CFileListWin_Enable(IWindow * po, boolean bEnable);
static void       CFileListWin_Redraw(IWindow * po);
static boolean    CFileListWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

// Player window
static IWindow *  CPlayerWin_New(CMediaPlayer * pOwner, MPPlayerWin eWin);
static void       CPlayerWin_Delete(IWindow * po);
static void       CPlayerWin_Enable(IWindow * po, boolean bEnable);
static void       CPlayerWin_Redraw(IWindow * po);
static boolean    CPlayerWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam);

static void       CPlayerWin_MediaNotify(void * pUser, AEEMediaCmdNotify * pCmdNotify);
static void       CPlayerWin_ImageNotify(void * pUser, IImage * pImage, AEEImageInfo * pi, int nErr);
static void       CPlayerWin_UpdateProgCtl(CPlayerWin * pme, int nCmd, int nSubCmd, uint16 wResID);
static boolean    CPlayerWin_IsPause(CPlayerWin * pme);
static void       CPlayerWin_ProgTimerNotify(CPlayerWin * pme);
static boolean    CPlayerWin_SetMediaData(CPlayerWin * pme, AEEMediaData *pmd);
static boolean    CPlayerWin_FullScreen(CPlayerWin * pme, boolean bFull, boolean bDeferRedraw);

// ProgCtl
static boolean    CProgCtl_Init(CProgCtl * pme, AEERect * pRectMain);
static void       CProgCtl_SetPos(CProgCtl * pme, AECHAR * psz, uint16 wPct);
static void       CProgCtl_Release(CProgCtl * pme);
static void       CProgCtl_DrawHist(CProgCtl * pme, uint16 wPct);

// Helper Functions
static void       MP_DrawImage(IImage * pImage, AEERect * pRect, boolean bCenter);
static boolean    MP_AddMenuItem(IMenuCtl * pMenu, uint16 wTextID, AECHAR * pText, uint16 wImageID, uint16 wItemID, uint32 dwData);
static void       MP_ErrorDlg(CMediaPlayer * pme, uint16 wResErrID);
static char *     MP_GetFileName(const char * psz);
static void       MP_FrameRect(IDisplay * pd, AEERect * pRect);
static void       MP_FitStaticText(IDisplay * pd, IStatic * ps, AEEFont font, AECHAR * pszText);
static void       MP_SetMenuAttr(IMenuCtl * pMenu, AEECLSID clsMenu, uint16 nColorDepth, AEERect * pRect, uint32 dwProps);
static void       MP_FreeIF(IBase ** ppif);
static void       MP_FreeWin(IWindow ** ppif);
static boolean    MP_AddExtension(char ** ppszExtList, char * psz);

/*-------------------------------------------------------------------
            Global Constant Definitions
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
            Global Data Definitions
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
            Static variable Definitions
-------------------------------------------------------------------*/

//
// navigate app can either be statically built into BREW or dynamically linked during run-time.
// If AEE_STATIC is defined, then navigate app will be a static app.
// NOTE: Static apps can be built only by OEMs or Carriers. App developers can build dynamic apps only.
//
#if defined(AEE_STATIC)

int         CMediaPlayer_CreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj);
static int  CMediaPlayer_Load(IShell *ps, void * pHelpers, IModule ** pMod);

//
// Constant Data...
//
static const AEEAppInfo    gaiMediaPlayerApp = {AEECLSID_NAVIGATE,NAVIGATE_RES_FILE,IDS_TITLE,IDB_TNAIL,IDB_TNAIL,0,0,0};

/*===========================================================================

                      PUBLIC FUNCTION DECLARATIONS

===========================================================================*/



static int StartCameraInPreviewMode(CMediaPlayer * pme)
{
	int   nErr;
	
	// Create ICamera instance.
	nErr = ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_CAMERA, (void **)&pme->m_pICamera);
	if (nErr)
		return nErr;
	
	// Register callback notification function.
	nErr = ICAMERA_RegisterNotify(pme->m_pICamera, CApp_CameraNotify, pme);
	if (nErr)
		return nErr;
	
	pme->m_sizePreview.cx = 1024;
	pme->m_sizePreview.cy = 1024;

	pme->m_sizeFrame.cx = 1024;
	pme->m_sizeFrame.cy = 1024;

	ICAMERA_SetDisplaySize(pme->m_pICamera, &pme->m_sizePreview);
	ICAMERA_SetSize( m_pICamera, &pme->m_sizeFrame);
	nErr = ICAMERA_Preview(pme->m_pICamera);
	if (nErr)
		return nErr;
	
	return SUCCESS;
}



//===========================================================================
//
// 
//
//===========================================================================
PFNMODENTRY CMediaPlayer_GetModInfo(IShell * ps, AEECLSID ** ppClasses, AEEAppInfo ** pApps, uint16 * pnApps,uint16 * pwMinPriv)
{
   *pApps = (AEEAppInfo *)&gaiMediaPlayerApp;
   *pnApps = 1;
   return((PFNMODENTRY)CMediaPlayer_Load);
}

//===========================================================================
//
//===========================================================================
static int CMediaPlayer_Load(IShell *ps, void * pHelpers, IModule ** pMod)
{
   return(AEEStaticMod_New((int16)(sizeof(AEEMod)),ps,pHelpers,pMod,CMediaPlayer_CreateInstance,NULL));
}

#endif  //AEE_STATIC

/*===========================================================================

Function:  CMediaPlayer_InitAppData()

Description:
		This function initializes the app specific data.

Prototype:
	void CMediaPlayer_InitAppData(IApplet* po)

Parameters:
   IApplet* po: Pointer to the applet structure. This is of the type defined
	to store applet data. (CMediaPlayer*, in this case).

Return Value:
	TRUE: If successful
	FALSE: IF failed

Comments:  None

Side Effects: None

==============================================================================*/
boolean CMediaPlayer_InitAppData(IApplet* po)
{
	CMediaPlayer *    pme = (CMediaPlayer*)po;
   int               nAscent, nDescent;
   AEEDeviceInfo *   pdi;

   //////////////////////////////////////////////////////////////////////////
   int   nErr;
   
   // Create ICamera instance.
   nErr = ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_CAMERA, (void **)&pme->m_pICamera);
   if (nErr)
	   return nErr;
   
   // Register callback notification function.
   nErr = ICAMERA_RegisterNotify(pme->m_pICamera, CApp_CameraNotify, pme);
   if (nErr)
	   return nErr;
   
   pme->m_sizePreview.cx = 1024;
   pme->m_sizePreview.cy = 1024;
   
   pme->m_sizeFrame.cx = 1024;
   pme->m_sizeFrame.cy = 1024;
   
   ICAMERA_SetDisplaySize(pme->m_pICamera, &pme->m_sizePreview);
   ICAMERA_SetSize( pme->m_pICamera, &pme->m_sizeFrame);
   nErr = ICAMERA_Preview(pme->m_pICamera);
   if (nErr)
		return nErr;

   //////////////////////////////////////////////////////////////////////////
   //StartCameraInPreviewMode(pme);

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

   pme->m_pHdrImage = ISHELL_LoadResImage(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDB_HEADER);
   if (!pme->m_pHdrImage)
      return FALSE;
   SETAEERECT(&pme->m_rectHdr, 0, 0, pme->m_cxWidth, MP_HEADER_CY);

   CALLBACK_Init(&pme->m_cbRedraw, (PFNNOTIFY)CMediaPlayer_RedrawNotify, pme);

   pme->m_pWin = CMainWin_New(pme);
   if (!pme->m_pWin)
      return FALSE;

   return TRUE;
}

/*===========================================================================
Function:  CMediaPlayer_FreeAppData()

Description:
	This function frees the app data. This function is registered with the
	applet framework when the applet is created (inside AEEClsCreateInstance() function).
	This function is called by the app framework when the reference count of the 
	applet reaches zero. This function must free all the app data that has been
	allocated by this app. For ex: if their is data that was remembered when
	the app is suspended and resumed, those data must be freed here.

Prototype:
	void CMediaPlayer_FreeAppData(IApplet* po)

Parameters:
   IApplet* po: Pointer to the applet structure. This is of the type defined
	to store applet data. (CMediaPlayer*, in this case).

Return Value:
	None

Comments:  None

Side Effects: None
==============================================================================*/
void CMediaPlayer_FreeAppData(IApplet* po)
{
	CMediaPlayer * pme = (CMediaPlayer *)po;

   MP_RELEASEIF(pme->m_pHdrImage);
   
   CMediaPlayer_CancelRedraw(pme);
   MP_RELEASEWIN(pme->m_pWin);

   FREEIF(pme->m_pszAudioExt);
   FREEIF(pme->m_pszVideoExt);
   FREEIF(pme->m_pszImageExt);
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
int CMediaPlayer_CreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj)
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
	   int   nErr;
	   ICamera *m_pICamera;

	   // Create ICamera instance.
	   nErr = ISHELL_CreateInstance(pIShell, AEECLSID_CAMERA, (void **)&m_pICamera);
	   if (nErr)
	   return nErr;

		//Create the applet
      if(AEEApplet_New(sizeof(CMediaPlayer), ClsId, pIShell,po,(IApplet**)ppObj,(AEEHANDLER)CMediaPlayer_HandleEvent,(PFNFREEAPPDATA)CMediaPlayer_FreeAppData))
		{
			//Initialize applet data
         if(CMediaPlayer_InitAppData((IApplet*)*ppObj))
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

FUNCTION CMediaPlayer_HandleEvent

DESCRIPTION
	This is the EventHandler for this app. All events to this app are handled in this
	function. All APPs must supply an Event Handler.

PROTOTYPE:
	boolean CMediaPlayer_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam)

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
static boolean CMediaPlayer_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
    CMediaPlayer * pme = (CMediaPlayer *)pi;

    switch ( eCode ) 
    {   
         case EVT_APP_START:   // Process Start event
         {
            //
            // An app can be started from app manager or from another app with arguments. 
            // dwParam contains the string argument passed to the applet when it is started.
            // In case of navigate app, if the argument is present, then it means that
            // the applet is started as a plugin with argument specifying file name of the
            // media. In this case, the player window is directly invoked and the media
            // starts playing immediately.
            //
            AEEAppStart * pas = (AEEAppStart *)dwParam;
            if (pas->pszArgs)
            {
               pme->m_bPlugin = TRUE;
               CMediaPlayer_PlayFile(pme, pas->pszArgs);
            }
            else
               CMediaPlayer_DrawSplash(pme);

            return TRUE;
         }

         case EVT_APP_BROWSE_FILE:
         {
            //
            // This event is sent when ISHELL_BrowseFile() API finds
            // navigate app as the registered handler. Shell first
            // sends EVT_APP_START followed by this event. The dwParam
            // contains the file name.
            // Start the navigate app in plugin mode.
            //
            if (dwParam)
            {
               char *   psz = (char *)dwParam;
               CMediaPlayer_SetWindow(pme, MPW_NONE, NULL);
               pme->m_bPlugin = TRUE;
               CMediaPlayer_PlayFile(pme, psz);
            }
            return TRUE;
         }

         case EVT_APP_STOP:        // process STOP event
            return (TRUE);

         case EVT_APP_SUSPEND:
            //
            // If playback ia going on, then stop the playback and close the window. This ensures that
            // all the multimedia resources allocated for playback are released. 
            // On resume, open file list window.
            //
            pme->m_eSuspendWin = pme->m_eActiveWin == MPW_PLAYER ? MPW_FILELIST : pme->m_eActiveWin;
            CMediaPlayer_SetWindow(pme, MPW_NONE, 0);
            return (TRUE);

         case EVT_APP_RESUME:
            CMediaPlayer_SetWindow(pme, pme->m_eSuspendWin, 0);
            return (TRUE);

         case EVT_KEY:	            // Process key event
         case EVT_COMMAND:          // Process menu command event
         case EVT_CREATEMEDIA:      // Create media
         case EVT_CREATEMEDIA_QCP:  // Create IMediaQCP for recoring
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
static void CMediaPlayer_DrawSplash(CMediaPlayer * pme)
{
   // The following 'If statement' is entered only after the splash timer runs out...
   if (pme->m_eActiveWin == MPW_MAIN)
   {
      CMediaPlayer_SetWindow(pme, MPW_MAIN, 0);
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
         MP_DrawImage(pi, &rect, TRUE);
         IDISPLAY_Update(pme->a.m_pIDisplay);
         MP_RELEASEIF(pi);
      }

      // Set main window as active and start the timer.
      pme->m_eActiveWin = MPW_MAIN;
      ISHELL_SetTimer(pme->a.m_pIShell, MP_SPLASH_TIMER, (PFNNOTIFY)CMediaPlayer_DrawSplash, pme);
   }  
}

/*===========================================================================
   This function switches from one window to another:
   (1) Releases all the resources associated with the current window
   (2) Contructs the new window, if any
   (3) Enables and redraws the new window, if any
===========================================================================*/
static boolean CMediaPlayer_SetWindow(CMediaPlayer * pme, MPWindow eWin, uint32 dwParam)
{
   // If same window, then redraw and return.
   if (pme->m_pWin && pme->m_eActiveWin == eWin && eWin != MPW_NONE)
   {
      CMediaPlayer_Redraw(pme, TRUE);
      return TRUE;
   }

   MP_RELEASEWIN(pme->m_pWin);

   switch (eWin)
   {
      case MPW_MAIN:       
         pme->m_pWin = CMainWin_New(pme); 
         break;

      case MPW_FILELIST:   
         pme->m_pWin = CFileListWin_New(pme); 
         break;

      case MPW_PLAYER:     
         pme->m_pWin = CPlayerWin_New(pme, (MPPlayerWin)dwParam); 
         break;

      case MPW_NONE:       
         return TRUE; 
         break;

#if defined(MEDIAPLAYER_SETTINGS)
      case MPW_SETTINGS:       
         return FALSE; 
         break;
#endif // defined(MEDIAPLAYER_SETTINGS)

      default:             
         return FALSE; 
         break;
   }

   if (!pme->m_pWin)
   {
      eWin = MPW_NONE;
      return FALSE;
   }

   pme->m_eActiveWin = eWin;

   CMediaPlayer_Redraw(pme, TRUE);

   return TRUE;
}

/*===========================================================================
   This function schedules an aynchronous redraw if bDefer is TRUE else
   it redras immediately.
===========================================================================*/
static void CMediaPlayer_Redraw(CMediaPlayer * pme, boolean bDefer)
{
   if (pme->m_bRedraw)
      return;

   pme->m_bRedraw = TRUE;

   if (bDefer)
      ISHELL_Resume(pme->a.m_pIShell, &pme->m_cbRedraw);
   else
      CMediaPlayer_RedrawNotify(pme);
}

/*===========================================================================
   This function redraws the current window.
   This function is called either by IShell in response to the resume scheduled
   in CMediaPlayer_Redraw() or by CMediaPlayer_Redraw() directly.
===========================================================================*/
static void CMediaPlayer_RedrawNotify(CMediaPlayer * pme)
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

/*===========================================================================
   This function opens the player window with the specified file.
   It also tries to play the file if navigate is in plugin mode.

   Note: This function allocates memory for the file name and the memory
   is released by player window's destructor.
===========================================================================*/
static void CMediaPlayer_PlayFile(CMediaPlayer * pme, const char * pszFile)
{
   AEEMediaData   md;
   char *         pszBuf;

   // File name is a special case...
   // pszFile will be released by CFileListWin when CPlayerWin opens.
   // Allocate memory for file name here. It will be used and released 
   // by CPlayerWin.
   pszBuf = STRDUP(pszFile);
   if (!pszBuf)
      return;

   // First check if this is image...
   if (CMediaPlayer_PlayImage(pme, pszBuf))
      return;

   // This is not image...it must be media.

   md.clsData = MMD_FILE_NAME;
   md.pData = (void *)pszBuf;
   md.dwSize = 0;

   if (!CMediaPlayer_SetWindow(pme, MPW_PLAYER, MPPW_PLAY))
   {
      FREE(pszBuf);
      return;
   }

   if (CPlayerWin_SetMediaData((CPlayerWin *)pme->m_pWin, &md))
   {
      ISHELL_PostEvent(pme->a.m_pIShell, AEECLSID_NAVIGATE, EVT_CREATEMEDIA, 0, 0);
      if (pme->m_bPlugin)
         ISHELL_PostEvent(pme->a.m_pIShell, AEECLSID_NAVIGATE, EVT_COMMAND, IDM_PM_PLAY, 0);
   }
}

/*===========================================================================
   This function opens the record window with the specified file.
===========================================================================*/
static void CMediaPlayer_RecordQCPFile(CMediaPlayer * pme, const char * pszFile)
{
   AEEMediaData   md;
   char *         pszBuf;
   char sz[2] =   { DIRECTORY_CHAR, 0 };

   // File name is a special case...
   // pszFile will be released by CFileListWin when CPlayerWin opens.
   // Allocate memory for file name here. It will be used and released 
   // by CPlayerWin.
   pszBuf = MALLOC(MAX_FILE_NAME);
   if (!pszBuf)
      return;

   STRCPY(pszBuf, MP_MEDIA_DIR);
   STRCAT(pszBuf, sz);
   STRCAT(pszBuf, pszFile);
   md.clsData = MMD_FILE_NAME;
   md.pData = (void *)pszBuf;
   md.dwSize = 0;

   if (!CMediaPlayer_SetWindow(pme, MPW_PLAYER, (uint32)TRUE))
   {
      FREE(pszBuf);
      return;
   }

   if (CPlayerWin_SetMediaData((CPlayerWin *)pme->m_pWin, &md))
      ISHELL_PostEvent(pme->a.m_pIShell, AEECLSID_NAVIGATE, EVT_CREATEMEDIA_QCP, 0, 0);
}

/*===========================================================================
   This function displays the image in the player window.
===========================================================================*/
static boolean CMediaPlayer_PlayImage(CMediaPlayer * pme, const char * pszFile)
{
   IImage *       pImage = ISHELL_LoadImage(pme->a.m_pIShell, pszFile);
   CPlayerWin *   ppw;

   if (!pImage)
      return FALSE;

   if (!CMediaPlayer_SetWindow(pme, MPW_PLAYER, MPPW_IMAGE))
   {
      IIMAGE_Release(pImage);
      return FALSE;
   }

   ppw = (CPlayerWin *)pme->m_pWin;
   ppw->m_bImage = TRUE;      // This is an image
   ppw->m_bPlayRec = TRUE;    // It always plays
   ppw->m_pszFile = (char *)pszFile;  // Save the pointer

   if (ISHELL_CreateInstance(ppw->m_pIShell, AEECLSID_IMAGECTL, (void **)&ppw->m_pImageCtl))
      MP_ErrorDlg(ppw->m_pOwner, IDS_ERR_CREATEMEDIA);
   else
   {
      IIMAGECTL_SetRect(ppw->m_pImageCtl, &ppw->m_rectImage);
      IIMAGECTL_SetImage(ppw->m_pImageCtl, pImage);
      ppw->m_pImage = pImage;
      IIMAGE_Notify(pImage, CPlayerWin_ImageNotify, ppw);
      CMediaPlayer_Redraw(pme, TRUE);
   }

   return TRUE;
}

/*===========================================================================
   This function returns the image/audio/default icon resource id if
   extension is present in the internal list.
===========================================================================*/
static uint16 CMediaPlayer_IsExtension(CMediaPlayer * pme, const char * pszFile, char * pszInExt, int nExtLen)
{
   char *   psz;
   uint16   wRet = 0;
   char     szExt[10];

   psz = STRRCHR(pszFile, '.');
   if (!psz)
      return 0;

   psz++;
   if (!*psz)
      return 0;

   // Form a string like "mid, "
   STRCPY(szExt, psz);
   STRCAT(szExt, MP_EXT_SEPARATOR);

   if (pszInExt && nExtLen > (int)STRLEN(psz))
      STRCPY(pszInExt, psz);

   if (pme->m_pszAudioExt && STRSTR(pme->m_pszAudioExt, szExt))
      wRet = IDB_AUDIO_ICON;
   else if (pme->m_pszVideoExt && STRSTR(pme->m_pszVideoExt, szExt))
      wRet = IDB_VIDEO_ICON;
   else if (pme->m_pszImageExt && STRSTR(pme->m_pszImageExt, szExt))
      wRet = IDB_IMAGE_ICON;

   return wRet;
}

/*===========================================================================
   This function tries to find the handler for an extension as follows:
   (1) Search the internal lists for audio, video and image extensions
   (2) If not found, then query Shell Registry for audio/video/image based handler
   (3) If found, add the extension to internal list
===========================================================================*/
static uint16 CMediaPlayer_FindHandlerType(CMediaPlayer * pme, const char * pszFile)
{
   AEECLSID cls;
   char     szMIME[32];
   char     szExt[10] = {0,0,0,0};
   uint16   wExtType = CMediaPlayer_IsExtension(pme, pszFile, szExt, sizeof(szExt));

   if (wExtType)
      return wExtType;

   if (!STRCMP(szExt, ""))
      return 0;

   // Now query the Shell Registry...

   // Look for audio based handler
   STRCPY(szMIME, AUDIO_MIME_BASE);
   STRCAT(szMIME, szExt);
   cls = ISHELL_GetHandler(pme->a.m_pIShell, AEECLSID_MEDIA, szMIME);
   if (cls && MP_AddExtension(&pme->m_pszAudioExt, szExt))
      return IDB_AUDIO_ICON;

   // Look for video based handler
   STRCPY(szMIME, VIDEO_MIME_BASE);
   STRCAT(szMIME, szExt);
   cls = ISHELL_GetHandler(pme->a.m_pIShell, AEECLSID_MEDIA, szMIME);
   if (cls && MP_AddExtension(&pme->m_pszVideoExt, szExt))
      return IDB_VIDEO_ICON;

   // Look for image based handler
   STRCPY(szMIME, IMAGE_MIME_BASE);
   STRCAT(szMIME, szExt);
   cls = ISHELL_GetHandler(pme->a.m_pIShell, HTYPE_VIEWER, szMIME);
   if (cls && MP_AddExtension(&pme->m_pszImageExt, szExt))
      return IDB_IMAGE_ICON;

   return 0;
}

/*===============================================================================

                     CWindow Functions

=============================================================================== */
/*===========================================================================
   This function is the base class constructor of an IWindow object.
   It allocates memory for the interface and sets the vtbl.
===========================================================================*/
static IWindow * CWindow_New(int16 nSize, CMediaPlayer * pOwner, VTBL(IWindow) * pvt)
{
   CWindow *         pme;
   VTBL(IWindow)  *  pVtbl;

   pme = MALLOC(nSize + sizeof(VTBL(IWindow)));
   if (!pme)
      return NULL;
   
   pme->m_pOwner = pOwner;
   pme->m_pIShell = pme->m_pOwner->a.m_pIShell;
   pme->m_pIDisplay = pme->m_pOwner->a.m_pIDisplay;

   pVtbl = (VTBL(IWindow) *)((byte *)pme + nSize);
   MEMCPY(pVtbl, pvt, sizeof(VTBL(IWindow)));
   INIT_VTBL(pme, IWindow, *pVtbl);

   return (IWindow *)pme;
}

/*===========================================================================
   This function indicates if IWINDOW_Enable()/IWINDOW_Disable() needs to be
   processed and also sets the bActive flag appropriately.
   It is called by derived class IWINDOW_Enable() function before processing
   the command.
===========================================================================*/
static boolean CWindow_ProcessEnable(IWindow * po, boolean bEnable)
{
   CWindow *   pme = (CWindow *)po;
   boolean     bProc = TRUE;

   if (!bEnable)
   {
      if (pme->m_bActive)
      {
         pme->m_bActive = FALSE;
         CMediaPlayer_CancelRedraw(pme->m_pOwner);
      }
      else
         bProc = FALSE;
   }
   else
   {
      if (pme->m_bActive)
         bProc = FALSE;
      else
         pme->m_bActive = TRUE;
   }

   return bProc;
}

/*===============================================================================

                     CMainWin Functions

=============================================================================== */
/*===========================================================================
   This function constucts the main window.
===========================================================================*/
static IWindow * CMainWin_New(CMediaPlayer * pOwner)
{
   CMainWin *        pme;
   VTBL(IWindow)     vtbl;
   
   MP_IWINDOW_SETVTBL(&vtbl, CMainWin_Enable, CMainWin_Redraw, CMainWin_HandleEvent, CMainWin_Delete);
   pme = (CMainWin *)CWindow_New(sizeof(CMainWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
      int      cx = pme->m_pOwner->m_cxWidth;
      int      cy = pme->m_pOwner->m_cyHeight;
      int      y, dy;
      AEERect  rect;

      // Initialize logo below the header
      pme->m_pLogo = ISHELL_LoadResImage(pme->m_pIShell, NAVIGATE_RES_FILE, IDB_LOGO);
      if (!pme->m_pLogo)
         MP_WINERR_RETURN(pme);

      y = pme->m_pOwner->m_rectHdr.dy + 1;
      dy = cy/2 - y;
      SETAEERECT(&pme->m_rectLogo, 0, y, cx, dy);
      IIMAGE_SetFrameCount(pme->m_pLogo, 2);
      IIMAGE_SetAnimationRate(pme->m_pLogo, 500);

      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pMainMenu))
         MP_WINERR_RETURN(pme);

      SETAEERECT(&rect, 0, cy/2 + 1, cx, cy/2 - 1);
      MP_SetMenuAttr(pme->m_pMainMenu, AEECLSID_MENUCTL, pme->m_pOwner->m_nColorDepth, &rect, 0);
      MP_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_PLAYFILE, NULL, IDB_PLAY,     IDM_MAIN_PLAYFILE, 0);
      MP_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_RECORD,   NULL, IDB_RECORD,   IDM_MAIN_RECORD,   0);
#if defined(MEDIAPLAYER_SETTINGS)
      MP_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_SETTINGS, NULL, IDB_SETTINGS, IDM_MAIN_SETTINGS, 0);
#endif // defined(MEDIAPLAYER_SETTINGS)
      MP_AddMenuItem(pme->m_pMainMenu, IDM_MAIN_ABOUT,    NULL, IDB_ABOUT,    IDM_MAIN_ABOUT,    0);
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the main window.
===========================================================================*/
static void CMainWin_Delete(IWindow * po)
{
   CMainWin *  pme = (CMainWin *)po;

   MP_RELEASEIF(pme->m_pLogo);

   if (pme->m_pMainMenu)
      pme->m_pOwner->m_wMainWin = IMENUCTL_GetSel(pme->m_pMainMenu);
   MP_RELEASEIF(pme->m_pMainMenu);

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the main window.
===========================================================================*/
static void CMainWin_Enable(IWindow * po, boolean bEnable)
{
   CMainWin *  pme = (CMainWin *)po;

   if (!CWindow_ProcessEnable(po, bEnable))
      return;

   if (!pme->m_bActive)
   {
      IMENUCTL_SetActive(pme->m_pMainMenu, FALSE);
      IIMAGE_Stop(pme->m_pLogo);
      return;
   }

   IMENUCTL_SetActive(pme->m_pMainMenu, TRUE);
   IMENUCTL_SetSel(pme->m_pMainMenu, pme->m_pOwner->m_wMainWin);
   MP_DrawImage(pme->m_pLogo, &pme->m_rectLogo, TRUE);
}

/*===========================================================================
   This function redraws the main window.
===========================================================================*/
static void CMainWin_Redraw(IWindow * po)
{
   CMainWin *  pme = (CMainWin *)po;

   if (!pme->m_bActive)
      return;

   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   MP_DRAWHEADER(pme);
   MP_DrawImage(pme->m_pLogo, &pme->m_rectLogo, TRUE);
   IMENUCTL_Redraw(pme->m_pMainMenu);

   IDISPLAY_Update(pme->m_pIDisplay);
}

/*===========================================================================
   This function processes events routed to main window.
===========================================================================*/
static boolean CMainWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CMainWin *  pme = (CMainWin *)po;
   boolean     bRet = TRUE;

   if (eCode == EVT_COPYRIGHT_END && pme->m_bAbout)
   {
      pme->m_bAbout = FALSE;
      CMediaPlayer_Redraw(pme->m_pOwner, TRUE);
      return TRUE;
   }
            
   if (MP_ISEVTKEY(eCode))
      return IMENUCTL_HandleEvent(pme->m_pMainMenu, eCode, wParam, dwParam);

   if (!MP_ISEVTCMD(eCode))
      return FALSE;

   switch (wParam)
   {
      case IDM_MAIN_PLAYFILE:
         CMediaPlayer_SetWindow(pme->m_pOwner, MPW_FILELIST, 0);
         break;

      case IDM_MAIN_RECORD:
         CMediaPlayer_RecordQCPFile(pme->m_pOwner, MP_QCP_REC_FILE);
         break;

#if defined(MEDIAPLAYER_SETTINGS)
      case IDM_MAIN_SETTINGS:
         break;
#endif // defined(MEDIAPLAYER_SETTINGS)

      case IDM_MAIN_ABOUT:
         CMainWin_About(pme);
         break;

      default:
         bRet = FALSE;
         break;
   }

   return bRet;
}

/*===========================================================================
   This function displays the About dialog of the app.
===========================================================================*/
static void CMainWin_About(CMainWin * pme)
{
   CMediaPlayer_DisableWin(pme->m_pOwner);

   pme->m_bAbout = TRUE;

   IDISPLAY_ClearScreen(pme->m_pIDisplay);
   MP_DRAWHEADER(pme);
   ISHELL_ShowCopyright(pme->m_pIShell);
}

/*===============================================================================

                     CFileListWin Functions

=============================================================================== */

/*===========================================================================
   This function constucts the file list window.
===========================================================================*/
static IWindow * CFileListWin_New(CMediaPlayer * pOwner)
{
   CFileListWin *    pme;
   VTBL(IWindow)     vtbl;
   
   MP_IWINDOW_SETVTBL(&vtbl, CFileListWin_Enable, CFileListWin_Redraw, CFileListWin_HandleEvent, CFileListWin_Delete);
   pme = (CFileListWin *)CWindow_New(sizeof(CFileListWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   {
      int         cx = pme->m_pOwner->m_cxWidth;
      int         cy = pme->m_pOwner->m_cyHeight;
      AEERect     rect;
      FileInfo    fi;
      uint16      wItemID = 0;
      AECHAR      *szBuf;
      int         y;
      IFileMgr *  pIFileMgr;

      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MENUCTL, (void **)&pme->m_pFileMenu))
         MP_WINERR_RETURN(pme);

      y = pme->m_pOwner->m_rectHdr.dy + 1;
      SETAEERECT(&rect, 0, y, cx, cy - y);

      MP_SetMenuAttr(pme->m_pFileMenu, AEECLSID_MENUCTL, pme->m_pOwner->m_nColorDepth, &rect, 0);
      IMENUCTL_SetTitle(pme->m_pFileMenu, NAVIGATE_RES_FILE, IDS_FILELISTMENU_TITLE, NULL);

      // Enumerate all music files and add them to menu
      szBuf = MALLOC(MAX_FILE_NAME * sizeof(AECHAR));
      if(!szBuf)
         MP_WINERR_RETURN(pme);

      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_FILEMGR, (void **)&pIFileMgr))
      {
         FREE(szBuf);
         MP_WINERR_RETURN(pme);
      }

      IFILEMGR_EnumInit(pIFileMgr, MP_MEDIA_DIR, FALSE);
      while (wItemID < MP_MAX_FILES  && IFILEMGR_EnumNext(pIFileMgr, &fi))
      {
         char *   szName;

         pme->m_szFileArray[wItemID] = STRDUP(fi.szName);
         if (!pme->m_szFileArray[wItemID])
         {
            pme->m_NumFiles = wItemID;
            FREE(szBuf);
            MP_RELEASEIF(pIFileMgr);
            MP_WINERR_RETURN(pme);
         }

         // Add only the file name
         szName = MP_GetFileName(fi.szName);
         if (szName)
         {
            uint16   wIconID = CMediaPlayer_FindHandlerType(pme->m_pOwner, szName);
            if (wIconID)
            {
               STRTOWSTR(szName, szBuf, MAX_FILE_NAME);
               MP_AddMenuItem(pme->m_pFileMenu, 0, szBuf, wIconID, (uint16)(IDM_FILELIST_BASE + wItemID), 0);
            }
         }

         wItemID++;
      }

      pme->m_NumFiles = wItemID;
   
      MP_RELEASEIF(pIFileMgr);
      FREE(szBuf);
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the file list window.
===========================================================================*/
static void CFileListWin_Delete(IWindow * po)
{
   CFileListWin * pme = (CFileListWin *)po;
   int            i;

   for ( i = 0; i < pme->m_NumFiles; i++ )
   {
      if (pme->m_szFileArray[i])
         FREE((void *)pme->m_szFileArray[i]);
   }
   
   if (pme->m_pFileMenu)
      pme->m_pOwner->m_wFileListWin = IMENUCTL_GetSel(pme->m_pFileMenu);
   MP_RELEASEIF(pme->m_pFileMenu);

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the file list window.
===========================================================================*/
static void CFileListWin_Enable(IWindow * po, boolean bEnable)
{
   CFileListWin * pme = (CFileListWin *)po;

   if (!CWindow_ProcessEnable(po, bEnable))
      return;

   if (!pme->m_bActive)
   {
      IMENUCTL_SetActive(pme->m_pFileMenu, FALSE);
      return;
   }

   if (pme->m_NumFiles)
   {
      IMENUCTL_SetActive(pme->m_pFileMenu, TRUE);
      IMENUCTL_SetSel(pme->m_pFileMenu, pme->m_pOwner->m_wFileListWin);
   }
   else
   {
      MP_ErrorDlg(pme->m_pOwner, IDS_ERR_NOFILES);
   }
}

/*===========================================================================
   This function redraws the file list window.
===========================================================================*/
static void CFileListWin_Redraw(IWindow * po)
{
   CFileListWin * pme = (CFileListWin *)po;

   if (!pme->m_bActive)
      return;

   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   MP_DRAWHEADER(pme);
   IMENUCTL_Redraw(pme->m_pFileMenu);

   IDISPLAY_Update(pme->m_pIDisplay);
}

/*===========================================================================
   This function processes events routed to the file list window.
===========================================================================*/
static boolean CFileListWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CFileListWin * pme = (CFileListWin *)po;

   if (MP_ISCLR(eCode) || MP_ISCMD(eCode, IDS_OK))
   {
      CMediaPlayer_SetWindow(pme->m_pOwner, MPW_MAIN, 0);
      return TRUE;
   }

   if (MP_ISEVTKEY(eCode))
   {
      return IMENUCTL_HandleEvent(pme->m_pFileMenu, eCode, wParam, dwParam);
   }

   // Check if it is EVT_COMMAND and corresponds to files
   if (!MP_ISEVTCMD(eCode))
      return FALSE;
   if (wParam < IDM_FILELIST_BASE)
      return FALSE;

   // Play the file...
   CMediaPlayer_PlayFile(pme->m_pOwner, pme->m_szFileArray[(int)wParam - IDM_FILELIST_BASE]);

   return TRUE;
}

/*===============================================================================

                     CPlayerWin Functions

=============================================================================== */
/*===========================================================================
   This function constructs player window.
   Player window type can be
   (1) MPPW_PLAY: Plays media files
   (2) MPPW_RECORD: Records QCP file
   (3) MPPW_IMAGE: Displays image (of IImage type)
===========================================================================*/
static IWindow * CPlayerWin_New(CMediaPlayer * pOwner, MPPlayerWin eWin)
{
   CPlayerWin *      pme;
   VTBL(IWindow)     vtbl;
   int               y;
   int               dy;
   
   MP_IWINDOW_SETVTBL(&vtbl, CPlayerWin_Enable, CPlayerWin_Redraw, CPlayerWin_HandleEvent, CPlayerWin_Delete);
   pme = (CPlayerWin *)CWindow_New(sizeof(CPlayerWin), pOwner, &vtbl);
   if (!pme)
      return NULL;

   pme->m_eWin = eWin;

   // Set title and image rects. Create the static control.
   {
      int      cx = pme->m_pOwner->m_cxWidth;
      int      cy = pme->m_pOwner->m_cyHeight;
      int      nLChSize = pme->m_pOwner->m_nLChSize;
      int      cyProgMain = pme->m_pOwner->m_nNChSize + 2;
      AEERect  rectTitle;
      AEERect  rectMenu;

      // File name text rect
      y = pme->m_pOwner->m_rectHdr.dy + 1;
      dy = nLChSize;
      SETAEERECT(&rectTitle, 0, y, cx, dy);

      // Create IStatic for file name
      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTitle))
         MP_WINERR_RETURN(pme);
      ISTATIC_SetRect(pme->m_pTitle, &rectTitle);
      ISTATIC_SetProperties(pme->m_pTitle, ST_CENTERTEXT | ST_NOSCROLL);

      // Image rect
      y += rectTitle.dy;
      dy = cy - y - cyProgMain - MP_ICONVIEWCTL_CY - 2;
      SETAEERECT(&pme->m_rectImage, 0, y, cx, dy);
      MEMCPY(&pme->m_rectImageCopy, &pme->m_rectImage, sizeof(AEERect));

      // IconViewCtl rect
      y = cy - MP_ICONVIEWCTL_CY;
      dy = MP_ICONVIEWCTL_CY;
      SETAEERECT(&rectMenu, 0, y, cx, dy);
      if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_SOFTKEYCTL, (void **)&pme->m_pPlayerMenu))
         MP_WINERR_RETURN(pme);

      MP_SetMenuAttr(pme->m_pPlayerMenu, AEECLSID_ICONVIEWCTL, pme->m_pOwner->m_nColorDepth, &rectMenu, MP_MAXSOFTKEYITEMS);
   }

   if (CPlayerWin_IsImage(pme))
   {
      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_FULLSCRN, IDM_PM_FULLSCRN, 0);
   }
   else if (CPlayerWin_IsMedia(pme))
   {
      int      cx = pme->m_pOwner->m_cxWidth;
      int      cy = pme->m_pOwner->m_cyHeight;
      int      cyProgMain = pme->m_pOwner->m_nNChSize + 2;
      AEERect  rectProg;

      // ProgMain rect
      y = cy - cyProgMain - MP_ICONVIEWCTL_CY - 2;
      dy = cyProgMain;
      SETAEERECT(&rectProg, 0, y, cx, dy);


      // Initialize logo below the header
      pme->m_pIdleImage = ISHELL_LoadResImage(pme->m_pIShell, NAVIGATE_RES_FILE, IDB_LOGO);
      if (!pme->m_pIdleImage)
         MP_WINERR_RETURN(pme);

      // Initialize ProgCtl
      pme->m_ProgCtl.m_pIShell = pme->m_pIShell;
      pme->m_ProgCtl.m_pIDisplay = pme->m_pIDisplay;
      if (!CProgCtl_Init(&pme->m_ProgCtl, &rectProg))
         MP_WINERR_RETURN(pme);

      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_PLAY,     IDM_PM_PLAY,   0);
      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_STOP,     IDM_PM_STOP,   0);
      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_FULLSCRN, IDM_PM_FULLSCRN, 0);
      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_REWIND,   IDM_PM_REWIND, 0);
      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_PAUSE,    IDM_PM_PAUSE,  0);
      MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_FF,       IDM_PM_FF,     0);
      if (pme->m_eWin == MPPW_RECORD)
         MP_AddMenuItem(pme->m_pPlayerMenu, 0, NULL, IDB_RECORD, IDM_PM_RECORD, 0);
      MP_AddMenuItem(pme->m_pPlayerMenu,    0, NULL, IDB_UP,     IDM_PM_UP,     0);
      MP_AddMenuItem(pme->m_pPlayerMenu,    0, NULL, IDB_DOWN,   IDM_PM_DOWN,   0);
   }

   return (IWindow *)pme;
}

/*===========================================================================
   This function deletes the player window.
===========================================================================*/
static void CPlayerWin_Delete(IWindow * po)
{
   CPlayerWin *   pme = (CPlayerWin *)po;

   MP_RELEASEIF(pme->m_pTitle);
   MP_RELEASEIF(pme->m_pIdleImage);
   MP_RELEASEIF(pme->m_pPlayerMenu);

   // Release memory for file name allocated in CMediaPlayer
   FREEIF(pme->m_pszFile);

   if (CPlayerWin_IsMedia(pme))
   {
      MP_RELEASEIF(pme->m_pMedia);

      CProgCtl_Release(&pme->m_ProgCtl);

      if (pme->m_bProgTimer)
         ISHELL_CancelTimer(pme->m_pIShell, (PFNNOTIFY)CPlayerWin_ProgTimerNotify, pme);
   }
   else if (CPlayerWin_IsImage(pme))
   {
      MP_RELEASEIF(pme->m_pImage);
      MP_RELEASEIF(pme->m_pImageCtl);
   }

   FREE(pme);
}

/*===========================================================================
   This function enables/disables the player window.
===========================================================================*/
static void CPlayerWin_Enable(IWindow * po, boolean bEnable)
{
   CPlayerWin *   pme = (CPlayerWin *)po;
   boolean        bActive;

   if (!CWindow_ProcessEnable(po, bEnable))
      return;

   bActive = pme->m_bActive;

   ISTATIC_SetActive(pme->m_pTitle, bActive);
   IMENUCTL_SetActive(pme->m_pPlayerMenu, bActive);
   if (CPlayerWin_IsImage(pme))
      IIMAGECTL_SetActive(pme->m_pImageCtl, bActive);

   if (!pme->m_bActive)
   {
      IIMAGE_Stop(pme->m_pIdleImage);
      if (CPlayerWin_IsMedia(pme) && pme->m_pMedia)
            IMEDIA_Stop(pme->m_pMedia);
   }
   else
   {
      IMENUCTL_SetSel(pme->m_pPlayerMenu, (uint16) (pme->m_eWin == MPPW_PLAY ? IDM_PM_PLAY : IDM_PM_RECORD));
   }
}

/*===========================================================================
   This function redraws player window.
   If the full screen mode is on, then
   (1) if playback is not ongoing, then it paints the screen with black.
   (2) if image is being displayed, then redraws the image.
===========================================================================*/
static void CPlayerWin_Redraw(IWindow * po)
{
   CPlayerWin *   pme = (CPlayerWin *)po;
   char *         pszFile = pme->m_pszFile;
   AEERect        rect;

   if (!pme->m_bActive)
      return;

   if (MP_ISFULLSCRN(pme))
   {
      if (!pme->m_bPlayRec)
      {
         IDISPLAY_FillRect(pme->m_pIDisplay, NULL, RGB_BLACK);
         IDISPLAY_Update(pme->m_pIDisplay);
      }
      else if (CPlayerWin_IsImage(pme))
      {
         IDISPLAY_ClearScreen(pme->m_pIDisplay);
         IIMAGECTL_Redraw(pme->m_pImageCtl);
         IDISPLAY_Update(pme->m_pIDisplay);
      }

      return;
   }

   IDISPLAY_ClearScreen(pme->m_pIDisplay);

   // Header
   MP_DRAWHEADER(pme);

   // File name (title) text
   if (pszFile)
   {
      STRTOWSTR(MP_GetFileName(pszFile), pme->m_szText, sizeof(pme->m_szText));
      MP_FitStaticText(pme->m_pIDisplay, pme->m_pTitle, AEE_FONT_NORMAL, pme->m_szText);
      ISTATIC_Redraw(pme->m_pTitle);
   }

   // Draw the line below text.
   ISTATIC_GetRect(pme->m_pTitle, &rect);
   rect.y += rect.dy - 1; rect.dy = 1;
   MP_FrameRect(pme->m_pIDisplay, &rect);

   // SoftKey
   IMENUCTL_Redraw(pme->m_pPlayerMenu);

   if (CPlayerWin_IsMedia(pme))
   {
      // Video/Image
      if (!pme->m_bPlayRec)
         MP_DrawImage(pme->m_pIdleImage, &pme->m_rectImage, TRUE);

      // Progress bar...
      CPlayerWin_UpdateProgCtl(pme, 0, 0, 0);
   }
   else if (CPlayerWin_IsImage(pme))
   {
      IIMAGECTL_Redraw(pme->m_pImageCtl);
      IDISPLAY_Update(pme->m_pIDisplay);
   }
}

/*===========================================================================
   This function handles the events routed to the player window.
   If the event is
   (1) CLR or OK from error dialog: 
       (a) If app is in plugin mode, it is closed.
       (b) If media playback is on, then file list window is opened
       (c) If recording is on, then main window is opened

   (2) AVK_0 or EVT_COMMAND(IDM_PM_FULLSCRN): Full screen is toggled
   (3) EVT_CREATEMEDIA: Media is created using AEEMediaUtil_CreateMedia()
   (4) EVT_CREATEMEDIA_QCP: IMediaQCP is created and its media data is set 
       to put it in Ready state.
   (5) EVT_COMMAND: processes the menu commands.
   (6) EVT_KEY: routed appropriately to the controls.
===========================================================================*/
static boolean CPlayerWin_HandleEvent(IWindow * po, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
   CPlayerWin *   pme = (CPlayerWin *)po;
   boolean        bRet = TRUE;

   // Process CLR or OK from error dlg
   if (MP_ISCLR(eCode) || MP_ISCMD(eCode, IDS_OK))
   {
      // Stop the app if it is in plugin mode.
      if (MP_ISPLUGIN(pme))
         ISHELL_CloseApplet(pme->m_pIShell, FALSE);
      else
      {
         if (pme->m_eWin == MPPW_PLAY || pme->m_eWin == MPPW_IMAGE)
            CMediaPlayer_SetWindow(pme->m_pOwner, MPW_FILELIST, 0);
         else if (pme->m_eWin == MPPW_RECORD)
            CMediaPlayer_SetWindow(pme->m_pOwner, MPW_MAIN, 0);
      }

      return TRUE;
   }

   // Process EVT_KEY...
   if (MP_ISEVTKEY(eCode))
   {
      if (wParam == AVK_0)
         CPlayerWin_FullScreen(pme, (boolean)!pme->m_bFullScreen, FALSE);
      else if (CPlayerWin_IsMedia(pme))
         return IMENUCTL_HandleEvent(pme->m_pPlayerMenu, eCode, wParam, dwParam);
      else if (CPlayerWin_IsImage(pme))
      {
         if (wParam !=  AVK_SELECT)
            return IIMAGECTL_HandleEvent(pme->m_pImageCtl, eCode, wParam, dwParam);
         else
            return IMENUCTL_HandleEvent(pme->m_pPlayerMenu, eCode, wParam, dwParam);
      }

      return TRUE;
   }

   // Process create media events...
   if (eCode == EVT_CREATEMEDIA || eCode == EVT_CREATEMEDIA_QCP)
   {
      int   nRet;

      if (eCode == EVT_CREATEMEDIA)
         nRet = AEEMediaUtil_CreateMedia(pme->m_pIShell, &pme->m_md, &pme->m_pMedia);
      else
      {
         nRet = ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_MEDIAQCP, (void **)&pme->m_pMedia);
         if (nRet == SUCCESS)
            nRet = IMEDIA_SetMediaData(pme->m_pMedia, &pme->m_md);
      }

      if (SUCCESS != nRet)
         MP_ErrorDlg(pme->m_pOwner, IDS_ERR_CREATEMEDIA);
      else if (SUCCESS != IMEDIA_RegisterNotify(pme->m_pMedia, CPlayerWin_MediaNotify, pme))
         MP_ErrorDlg(pme->m_pOwner, IDS_ERR_MEDIA);
      else
      {
         uint32   dwCaps;

         IMEDIA_GetMediaParm(pme->m_pMedia, MM_PARM_CAPS, (int32 *)&dwCaps, 0);
         pme->m_bImage = (dwCaps & MM_CAPS_VIDEO) ? TRUE : FALSE;
         if (!pme->m_bImage)
            IMENUCTL_DeleteItem(pme->m_pPlayerMenu, IDM_PM_FULLSCRN);
         else
            IMEDIA_SetRect(pme->m_pMedia, &pme->m_rectImage, NULL);

         CMediaPlayer_Redraw(pme->m_pOwner, TRUE);
      }

      return TRUE;
   }

   // If this event is not EVT_COMMAND, then return...
   if (!MP_ISEVTCMD(eCode))
      return FALSE;

   if ( (CPlayerWin_IsMedia(pme) && !pme->m_pMedia) ||
        (CPlayerWin_IsImage(pme) && !pme->m_pImageCtl) )
      return TRUE;

   // Process EVT_COMMAND...
   switch (wParam)
   {
      case IDM_PM_PLAY:
         if (CPlayerWin_IsPause(pme))
            IMEDIA_Resume(pme->m_pMedia);
         else if (SUCCESS != IMEDIA_GetTotalTime(pme->m_pMedia))
         {
            if (AEE_SUCCESS == IMEDIA_Play(pme->m_pMedia))
               pme->m_bPlayRec = TRUE;
         }
         break;

      case IDM_PM_RECORD:
         if (CPlayerWin_IsPause(pme))
            IMEDIA_Resume(pme->m_pMedia);
         else if (AEE_SUCCESS == IMEDIA_Record(pme->m_pMedia))
               pme->m_bPlayRec = TRUE;
         break;

      case IDM_PM_STOP:
         IMEDIA_Stop(pme->m_pMedia);
         break;

      case IDM_PM_REWIND:
         IMEDIA_Rewind(pme->m_pMedia, MP_SEEK_TIME);
         break;

      case IDM_PM_FF:
         IMEDIA_FastForward(pme->m_pMedia, MP_SEEK_TIME);
         break;

      case IDM_PM_PAUSE:
         if (CPlayerWin_IsPause(pme))
            IMEDIA_Resume(pme->m_pMedia);
         else 
            IMEDIA_Pause(pme->m_pMedia);
         break;

      case IDM_PM_UP:
      {

         uint16   wVol = pme->m_wVolume + MP_VOLUME_STEP;
         wVol = (wVol > AEE_MAX_VOLUME) ? AEE_MAX_VOLUME : wVol;
         IMEDIA_SetVolume(pme->m_pMedia, wVol);
         break;
      }

      case IDM_PM_DOWN:
      {
         int16    nVol = (int16)pme->m_wVolume - MP_VOLUME_STEP;
         nVol = (nVol < 0) ? 0 : nVol;
         IMEDIA_SetVolume(pme->m_pMedia, (uint16)nVol);
         break;
      }

      case IDM_PM_FULLSCRN:
         CPlayerWin_FullScreen(pme, (boolean)!pme->m_bFullScreen, FALSE);
         break;

      default:
         bRet = FALSE;
         break;
   }

   return bRet;
}

/*===========================================================================
   This function indicates IMedia is in paused state.
===========================================================================*/
static boolean CPlayerWin_IsPause(CPlayerWin * pme)
{
   boolean  bChg;
   int      nState = IMEDIA_GetState(pme->m_pMedia, &bChg);

   if (bChg)
      return FALSE;

   return (nState == MM_STATE_PLAY_PAUSE || nState == MM_STATE_RECORD_PAUSE);
}
 
/*===========================================================================
   This function saves the file name and sets the media data locally.
   Assumption: pmd is not NULL.
===========================================================================*/
static boolean CPlayerWin_SetMediaData(CPlayerWin * pme, AEEMediaData * pmd)
{
   if (!pmd->pData)
      return FALSE;

   pme->m_pszFile = (char *)pmd->pData;
   MEMCPY(&pme->m_md, pmd, sizeof(AEEMediaData));

   return TRUE;
}

/*===========================================================================
   This function toggles full screen mode and does a redraw.
===========================================================================*/
static boolean CPlayerWin_FullScreen(CPlayerWin * pme, boolean bFull, boolean bDeferRedraw)
{
   if (!pme->m_bImage)
      return FALSE;

   if (!pme->m_bFullScreen)
   {
      SETAEERECT(&pme->m_rectImage, 0, 0, pme->m_pOwner->m_cxWidth, pme->m_pOwner->m_cyHeight);
   }
   else
   {
      MEMCPY(&pme->m_rectImage, &pme->m_rectImageCopy, sizeof(AEERect));
   }

   if (CPlayerWin_IsMedia(pme))
      IMEDIA_SetRect(pme->m_pMedia, &pme->m_rectImage, NULL);
   else if (CPlayerWin_IsImage(pme))
      IIMAGECTL_SetRect(pme->m_pImageCtl, &pme->m_rectImage);

   pme->m_bFullScreen = bFull;

   CMediaPlayer_Redraw(pme->m_pOwner, bDeferRedraw);
   
   return TRUE;
}

/*===============================================================================

                     ProgCtl Functions

=============================================================================== */
/*===========================================================================
   This funtion initializes the ProgCtl.
   ProgCtl contains two rectangles: main rectangle and within it the progress
   bar rectangle. The title is displayed on left of the progress bar within
   the main rectangle.
===========================================================================*/
static boolean CProgCtl_Init(CProgCtl * pme, AEERect * pRectMain)
{
   AEERect  rect;
   int      x, y;
   int      dxProg, dyProg;

   MEMCPY(&pme->m_rectMain, pRectMain, sizeof(AEERect));

   if (ISHELL_CreateInstance(pme->m_pIShell, AEECLSID_STATIC, (void **)&pme->m_pTitle))
      return FALSE;

   // ProgBar rect
   dxProg = pRectMain->dx / MP_PROGBAR_DX_DIV;
   dyProg = pRectMain->dy / MP_PROGBAR_DY_DIV;
   x = pRectMain->x + pRectMain->dx - dxProg + 1;
   y = pRectMain->y + pRectMain->dy / 2 - dyProg/2;

   if (dxProg > MP_PROGBAR_DX_OFFSET)
      dxProg -= MP_PROGBAR_DX_OFFSET;
   SETAEERECT(&pme->m_rectBar, x, y, dxProg, dyProg);

   SETAEERECT(&rect, pRectMain->x + 1, pRectMain->y + 1, pRectMain->dx - dxProg, pRectMain->dy);
   ISTATIC_SetRect(pme->m_pTitle, &rect);
   ISTATIC_SetProperties(pme->m_pTitle, ST_CENTERTEXT | ST_NOSCROLL);

   return TRUE;
}

/*===========================================================================
   This function updates the progress bar and the title.
===========================================================================*/
static void CProgCtl_SetPos(CProgCtl * pme, AECHAR * psz, uint16 wPct)
{
   MP_FitStaticText(pme->m_pIDisplay, pme->m_pTitle, AEE_FONT_NORMAL, psz);
   ISTATIC_Redraw(pme->m_pTitle);
   CProgCtl_DrawHist(pme, wPct);
   MP_FrameRect(pme->m_pIDisplay, &pme->m_rectMain);
}

/*===========================================================================
   This function deletes the ProgCtl
===========================================================================*/
static void CProgCtl_Release(CProgCtl * pme)
{
   MP_RELEASEIF(pme->m_pTitle);
}

/*===========================================================================
   This function fills wPct of  progress bar rectangle.
===========================================================================*/
static void CProgCtl_DrawHist(CProgCtl * pme, uint16 wPct)
{
   AEERect  rc;

   MEMCPY(&rc, &pme->m_rectBar, sizeof(AEERect));

   if(wPct > 100)
      wPct = 100;

   IDISPLAY_DrawFrame(pme->m_pIDisplay, &rc, AEE_FT_RAISED, CLR_SYS_SCROLLBAR);
   rc.dx = (rc.dx * wPct) / 100;
   IDISPLAY_FillRect(pme->m_pIDisplay, &rc, MP_CLR_PROG_FILL);
}

/*===============================================================================

                     Local Functions

=============================================================================== */
/*===========================================================================
   This function draws the image and centers it within the specified 
   rectangle if bCenter is TRUE.
===========================================================================*/
static void MP_DrawImage(IImage * pImage, AEERect * pRect, boolean bCenter)
{
   AEEImageInfo   ii;
   int            x;
   int            y;

   IIMAGE_GetInfo(pImage, &ii);

   // Do not display if image does not fit in the allocated rectangle.
   if (ii.cx > pRect->dx || ii.cy > pRect->dy)
      return;

   if (bCenter)
   {
      x = pRect->x + (pRect->dx / 2) - (ii.cxFrame / 2);
      y = pRect->y + (pRect->dy / 2) - (ii.cy / 2);
   }
   else
   {
      x = pRect->x;
      y = pRect->y;
   }

   IIMAGE_Start(pImage, x, y);
}

/*===========================================================================
   This function adds one item to the specified IMenuCtl.
===========================================================================*/
static boolean MP_AddMenuItem(IMenuCtl * pMenu, uint16 wTextID, AECHAR * pText, uint16 wImageID, uint16 wItemID, uint32 dwData)
{
   CtlAddItem  ai;

   // Fill in the CtlAddItem structure values
   ai.pText = pText;
   ai.pImage = NULL;
   ai.pszResImage = NAVIGATE_RES_FILE;
   ai.pszResText = NAVIGATE_RES_FILE;
   ai.wText = wTextID;
   ai.wFont = AEE_FONT_NORMAL;
   ai.wImage = wImageID;
   ai.wItemID = wItemID;
   ai.dwData = dwData;

   // Add the item to the menu control
   return IMENUCTL_AddItemEx( pMenu, &ai );
}

/*===========================================================================
   This function adds the extension string to the given string.
===========================================================================*/
static boolean MP_AddExtension(char ** ppszExtList, char * psz)
{
   char *   pExt;
   
   if (!ppszExtList || !psz)
      return FALSE;

   if (*ppszExtList)
      pExt = REALLOC(*ppszExtList, STRLEN(*ppszExtList) + STRLEN(psz) + STRLEN(MP_EXT_SEPARATOR) + 1);
   else
      pExt = MALLOC(STRLEN(psz) + STRLEN(MP_EXT_SEPARATOR) + 1);

   if (!pExt)
      return FALSE;

   STRCAT(pExt, psz);
   STRCAT(pExt, MP_EXT_SEPARATOR);
   *ppszExtList = pExt;
   return TRUE;
}

/*===========================================================================
   This function displays the specified error message and disables the 
   current window.
===========================================================================*/
static void MP_ErrorDlg(CMediaPlayer * pme, uint16 wResErrID)
{
   AEEPromptInfo  pi;
   uint16         wButtonIDs[] = {IDS_OK, 0};

   if (pme->m_pWin)
      CMediaPlayer_DisableWin(pme);

   pi.pszRes = NAVIGATE_RES_FILE;
   pi.pTitle = NULL;
   pi.pText = NULL;
   pi.wTitleID = 0; //IDS_ERR_BASE;
   pi.wTextID = wResErrID;
   pi.wDefBtn = IDS_OK;
   pi.pBtnIDs = wButtonIDs;
   pi.dwProps = ST_MIDDLETEXT | ST_CENTERTITLE;
   pi.fntTitle = AEE_FONT_BOLD;
   pi.fntText = AEE_FONT_NORMAL;
   pi.dwTimeout = 10000;  
   if (ISHELL_Prompt(pme->a.m_pIShell, &pi) == FALSE )
	   return;

   MP_DrawImage(pme->m_pHdrImage, &pme->m_rectHdr, TRUE);
   IDISPLAY_Update(pme->a.m_pIDisplay);
}

/*===========================================================================
   This function frames the rectangle with CLR_USER_FRAME and 
   CLR_SYS_DK_SHADOW
===========================================================================*/
static void MP_FrameRect(IDisplay * pd, AEERect * pRect)
{
   RGBVAL   clr = IDISPLAY_SetColor(pd, CLR_USER_FRAME, CLR_SYS_DK_SHADOW);
   IDISPLAY_FrameRect(pd, pRect);
   IDISPLAY_SetColor(pd, CLR_USER_FRAME, clr);
}

/*===========================================================================
   This function fits the text within the IStatic rectangle so that the
   scroll bar is not drawn.
===========================================================================*/
static void MP_FitStaticText(IDisplay * pd, IStatic * ps, AEEFont font, AECHAR * pszText)
{
   int      nFits;
   AEERect  rect;
   int      nLen = WSTRLEN(pszText);
   AECHAR   chSave = (AECHAR)0;

   ISTATIC_GetRect(ps, &rect);
   IDISPLAY_MeasureTextEx(pd, font, pszText, -1, rect.dx,  &nFits);
   if (nFits < nLen)
   {
      chSave = pszText[nFits];
      pszText[nFits] = (AECHAR)0;
   }
   ISTATIC_SetText(ps, NULL, pszText, AEE_FONT_NORMAL, font);
   if (nFits < nLen)
      pszText[nFits] = chSave;
}

/*===========================================================================
   This function returns the file name portion of the path.
===========================================================================*/
static char * MP_GetFileName(const char * psz)
{
   char *   pszName = STRRCHR(psz, (int)DIRECTORY_CHAR);

   if (pszName)
      pszName++;
   else
      pszName = (char *)psz;

   return pszName;
}

/*===========================================================================
   This function sets the menu attributes based on BREW Style Sheet for
   menu control.
===========================================================================*/
static void MP_SetMenuAttr(IMenuCtl * pMenu, AEECLSID clsMenu, uint16 nColorDepth, AEERect * pRect, uint32 dwProps)
{
	AEEItemStyle		sel, normal;
	AEEMenuColors		col;

	// Menu Style
	normal.ft = MENU8_FT;
	normal.xOffset = 0;
	normal.yOffset = 0;
	normal.roImage = MENU8_RO;

   sel.ft = MENU8_SELECT_FT;
	sel.xOffset = 0;
	sel.yOffset = 0;
	sel.roImage = MENU8_SELECT_RO;

	// Menu Colors
   col.cSelText = MENU8_SELECT_TEXT;
	col.wMask = MENU8_COLOR_MASK;

   if (clsMenu == AEECLSID_MENUCTL)
   {
	   col.cBack = MENU8_BACKGROUND;
	   col.cSelBack = MENU8_SELECT_BACKGROUND;

      dwProps |= IMENUCTL_GetProperties(pMenu);
   }
   else if (clsMenu == AEECLSID_SOFTKEYCTL || clsMenu == AEECLSID_ICONVIEWCTL)
   {
	   col.cBack = TB8_BACKGROUND;
	   col.cSelBack = TB8_SELECT_BACKGROUND;

      dwProps |= MP_ICON_TEXT_TOP | MP_NO_ARROWS;
   }

   if (clsMenu == AEECLSID_MENUCTL || clsMenu == AEECLSID_SOFTKEYCTL || clsMenu == AEECLSID_ICONVIEWCTL)
   {
	   IMENUCTL_SetStyle(pMenu, &normal, &sel);
	   IMENUCTL_SetColors(pMenu, &col);
	   IMENUCTL_SetProperties(pMenu, dwProps);
      if (pRect)
         IMENUCTL_SetRect(pMenu, pRect);
   }
}

/*===============================================================================

                     IMedia Callback Function

=============================================================================== */
/*===========================================================================
   This is the main callback notification function for the IMedia object
   within the player window.
   (1) Handles Play, Record, GetTotalTime, GetMediaParm and 
       SetMediaParm commands and corresponding status for those commands.
   (2) Updates the ProgCtl, if required.
===========================================================================*/
static void CPlayerWin_MediaNotify(void * pUser, AEEMediaCmdNotify * pCmdNotify)
{
   CPlayerWin *   pme = (CPlayerWin *)pUser;
   uint16         nTextID = 0;

   switch (pCmdNotify->nStatus)
   {
      case MM_STATUS_SEEK_FAIL:
      case MM_STATUS_PAUSE_FAIL:
      case MM_STATUS_RESUME_FAIL:
         nTextID = IDS_ERR_PLAYCTL_CMD;
         break;
   }

   if (pCmdNotify->nCmd == MM_CMD_PLAY || pCmdNotify->nCmd == MM_CMD_RECORD)  // IMEDIA_Play/IMEDIA_Record events
   { 
      nTextID = pCmdNotify->nCmd == MM_CMD_PLAY ? IDS_PLAY : IDS_RECORD;
      switch (pCmdNotify->nStatus)
      {
         case MM_STATUS_START:
            IMENUCTL_SetSel(pme->m_pPlayerMenu, IDM_PM_STOP);
            break;

         case MM_STATUS_MEDIA_SPEC:
            // pCmdNotify->pCmdData may point to media specs structure
            // defined in AEEMediaFormats.h. Save this info.
            break;

         case MM_STATUS_TICK_UPDATE:  // Typcally, one-second update
            pme->m_dwPlayPos++;
            if (!pme->m_dwTotalTime) // If no total time increase by 20!
               pme->m_wPct = (pme->m_wPct + 20) % 100;
            break; 

         case MM_STATUS_SEEK:
            nTextID = IDS_SEEK;
            if (pCmdNotify->dwSize)
               pme->m_dwPlayPos = (uint32)pCmdNotify->pCmdData / 1000;
            break;

         case MM_STATUS_PAUSE:
            nTextID = IDS_PAUSE;
            if (pCmdNotify->dwSize)
               pme->m_dwPlayPos = (uint32)pCmdNotify->pCmdData / 1000;
            break;

         case MM_STATUS_RESUME:
            if (pCmdNotify->dwSize)
               pme->m_dwPlayPos = (uint32)pCmdNotify->pCmdData / 1000;
            break;

         case MM_STATUS_DONE:    // playback done
         case MM_STATUS_ABORT:   // playback aborted
         {
            nTextID = (pCmdNotify->nStatus == MM_STATUS_DONE) ? IDS_STOP : IDS_ABORT;
            pme->m_bPlayRec = FALSE;
            pme->m_dwPlayPos = 0;
            pme->m_wPct = 0;

            IMENUCTL_SetSel(pme->m_pPlayerMenu, IDM_PM_PLAY);
            CMediaPlayer_Redraw(pme->m_pOwner, TRUE);           
            break;
         }
      }

      CPlayerWin_UpdateProgCtl(pme, pCmdNotify->nCmd, pCmdNotify->nSubCmd, nTextID);
   } // MM_CMD_PLAY
   else if (pCmdNotify->nCmd == MM_CMD_GETTOTALTIME)
   {
      if (pCmdNotify->nStatus == MM_STATUS_DONE || pCmdNotify->nStatus == MM_STATUS_ABORT)
      {
         pme->m_dwTotalTime = ((uint32)pCmdNotify->pCmdData) / 1000;
         if (AEE_SUCCESS == IMEDIA_Play(pme->m_pMedia))
            pme->m_bPlayRec = TRUE;
         CPlayerWin_UpdateProgCtl(pme, pCmdNotify->nCmd, pCmdNotify->nSubCmd, nTextID);
      }
   } // MM_CMD_GETTOTALTIME
   else if (pCmdNotify->nCmd == MM_CMD_SETMEDIAPARM)
   {
      if (pCmdNotify->nSubCmd == MM_PARM_VOLUME && pCmdNotify->nStatus == MM_STATUS_DONE)
      {
         IMEDIA_GetMediaParm(pme->m_pMedia, MM_PARM_VOLUME, NULL, NULL);
         return;
      }
   } // MM_CMD_SETMEDIAPARM
   else if (pCmdNotify->nCmd == MM_CMD_GETMEDIAPARM) // IMEDIA_GetMediaParm() events
   {
      if (pCmdNotify->nSubCmd == MM_PARM_VOLUME && pCmdNotify->nStatus == MM_STATUS_DONE)
      {
         pme->m_wVolume = (uint16)(uint32)pCmdNotify->pCmdData;

         pme->m_bProgTimer = FALSE;
         CPlayerWin_UpdateProgCtl(pme, pCmdNotify->nCmd, pCmdNotify->nSubCmd, IDS_VOLUME);
         pme->m_bProgTimer = TRUE;
         // Set the prog timer so that the display stays for few seconds.
         ISHELL_SetTimer(pme->m_pIShell, MP_PROG_TIMER, (PFNNOTIFY)CPlayerWin_ProgTimerNotify, pme);
      }
   }  // MM_CMD_GETMEDIAPARM
}

/*===========================================================================
   This function updates the ProgCtl if full screen mode is not active
   or if progress timer is not running.
===========================================================================*/
static void CPlayerWin_UpdateProgCtl(CPlayerWin * pme, int nCmd, int nSubCmd, uint16 wResID)
{
   char     sz[32];
   AECHAR   szBuf[32];

   if (MP_ISFULLSCRN(pme))
      return;

   if (pme->m_bProgTimer)
      return;

   if ( !ISHELL_LoadResString(pme->m_pIShell, NAVIGATE_RES_FILE, wResID, pme->m_szText, sizeof(pme->m_szText)) )
      pme->m_szText[0] = (AECHAR)0;

   if (nCmd == MM_CMD_PLAY || nCmd == MM_CMD_RECORD)
   {
      SPRINTF(sz, " %02d:%02d/%02d:%02d", pme->m_dwPlayPos/60, pme->m_dwPlayPos%60, pme->m_dwTotalTime/60, pme->m_dwTotalTime%60);
      STRTOWSTR(sz, szBuf, sizeof(szBuf));
      WSTRCAT(pme->m_szText, szBuf);

      if (pme->m_dwTotalTime)
         pme->m_wPct = (uint16)((100 * pme->m_dwPlayPos) / pme->m_dwTotalTime);
   }
   else if (nCmd == MM_CMD_GETTOTALTIME)
   {
      SPRINTF(sz, " %02d:%02d/%02d:%02d", pme->m_dwPlayPos/60, pme->m_dwPlayPos%60, pme->m_dwTotalTime/60, pme->m_dwTotalTime%60);
      STRTOWSTR(sz, szBuf, sizeof(szBuf));
      WSTRCAT(pme->m_szText, szBuf);
   }
   else if (nCmd == MM_CMD_GETMEDIAPARM && nSubCmd == MM_PARM_VOLUME)
   {
      SPRINTF(sz, "[%u]", pme->m_wVolume);
      STRTOWSTR(sz, szBuf, sizeof(szBuf));
      WSTRCAT(pme->m_szText, szBuf);
      pme->m_wPct = (100 * pme->m_wVolume) / AEE_MAX_VOLUME;
   }
   else
   {
      pme->m_wPct = 0;
   }

   CProgCtl_SetPos(&pme->m_ProgCtl, pme->m_szText, pme->m_wPct);

   IDISPLAY_Update(pme->m_pIDisplay);
}

/*===========================================================================
   This clears the progress timer.
===========================================================================*/
static void CPlayerWin_ProgTimerNotify(CPlayerWin * pme)
{
   pme->m_bProgTimer = FALSE;
   if (!pme->m_bPlayRec)
      CMediaPlayer_Redraw(pme->m_pOwner, TRUE);
}

/*===========================================================================
   This function redraws when the image is fully decoded.
===========================================================================*/
static void CPlayerWin_ImageNotify(void * pUser, IImage * pImage, AEEImageInfo * pi, int nErr)
{
   CPlayerWin *   pme = (CPlayerWin *)pUser;

   if (pme->m_pImage == pImage && !nErr)
      CMediaPlayer_Redraw(pme->m_pOwner, FALSE);
}

/*===========================================================================
   This function releases IBase.
===========================================================================*/
static void MP_FreeIF(IBase ** ppif)
{
   if (ppif && *ppif)
   {
      IBASE_Release(*ppif);
      *ppif = NULL;
   }
}

/*===========================================================================
   This function releases IWindow.
===========================================================================*/
static void MP_FreeWin(IWindow ** ppif)
{
   if (ppif && *ppif)
   {
      IWINDOW_Delete(*ppif);
      *ppif = NULL;
   }
}

/*========================End of File ================================================*/