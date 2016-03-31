#include "wincontrol.h"

/*===============================================================================

                     Local Functions

=============================================================================== */
/*===========================================================================
   This function draws the image and centers it within the specified 
   rectangle if bCenter is TRUE.
   - note£ºreturn if image bot fit in the rect
===========================================================================*/
void TS_DrawImage(IImage * pImage, AEERect * pRect, boolean bCenter)
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
boolean TS_AddMenuItem(IMenuCtl * pMenu, uint16 wTextID, AECHAR * pText, uint16 wImageID, uint16 wItemID, uint32 dwData)
{
   CtlAddItem  ai;

   // Fill in the CtlAddItem structure values
   ai.pText = pText;
   ai.pImage = NULL;
   ai.pszResImage = KITIMG_RES_FILE;
   ai.pszResText = NAVIGATE_RES_FILE;
   ai.wText = wTextID;
   ai.wFont = AEE_FONT_LARGE;
   ai.wImage = wImageID;
   ai.wItemID = wItemID;
   ai.dwData = dwData;

   // Add the item to the menu control
   return IMENUCTL_AddItemEx( pMenu, &ai );
}

/*===========================================================================
   This function adds the extension string to the given string.
===========================================================================*/
#define MP_EXT_SEPARATOR      ", "
boolean TS_AddExtension(char ** ppszExtList, char * psz)
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
void TS_ErrorDlg(CTopSoupApp * pme, uint16 wResErrID)
{
   AEEPromptInfo  pi;
   uint16         wButtonIDs[] = {IDS_OK, 0};

   if (pme->m_pWin)
      CTopSoupApp_DisableWin(pme);

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

   //TS_DrawImage(pme->m_pHdrImage, &pme->m_rectHdr, TRUE);
   IDISPLAY_Update(pme->a.m_pIDisplay);
}

/*===========================================================================
   This function frames the rectangle with CLR_USER_FRAME and 
   CLR_SYS_DK_SHADOW
===========================================================================*/
void TS_FrameRect(IDisplay * pd, AEERect * pRect)
{
   RGBVAL   clr = IDISPLAY_SetColor(pd, CLR_USER_FRAME, CLR_SYS_DK_SHADOW);
   IDISPLAY_FrameRect(pd, pRect);
   IDISPLAY_SetColor(pd, CLR_USER_FRAME, clr);
}

/*===========================================================================
   This function fits the text within the IStatic rectangle so that the
   scroll bar is not drawn.
===========================================================================*/
void TS_FitStaticText(IDisplay * pd, IStatic * ps, AEEFont font, AECHAR * pszText)
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

   ISTATIC_Redraw(ps);
}

/*===========================================================================
   This function returns the file name portion of the path.
===========================================================================*/
char * TS_GetFileName(const char * psz)
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
// Based on Menu style sheet:
#define MENU8_FT                 AEE_FT_NONE
#define MENU8_SELECT_FT          AEE_FT_NONE//AEE_FT_RAISED
#define MENU8_RO                 AEE_RO_TRANSPARENT
#define MENU8_SELECT_RO          AEE_RO_TRANSPARENT
#define MENU8_COLOR_MASK 	      (MC_BACK | MC_SEL_BACK | MC_SEL_TEXT)
#define MENU8_BACKGROUND	      MAKE_RGB(255,255,204)
#define MENU8_SELECT_BACKGROUND	MAKE_RGB(153, 204, 204)
#define MENU8_SELECT_TEXT	      RGB_BLACK

#define TB8_BACKGROUND	         MAKE_RGB(192,192,192)
#define TB8_SELECT_BACKGROUND	   MAKE_RGB(192, 192, 192)

void TS_SetMenuAttr(IMenuCtl * pMenu, AEECLSID clsMenu, uint16 nColorDepth, AEERect * pRect, uint32 dwProps)
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

void TS_DrawBackgroud(IWindow* po) {
	CWindow* pme = (CWindow*)po;

    TS_DrawImage(pme->m_pOwner->m_pBackImage,&pme->m_pOwner->m_rectBack,TRUE);
	
	TS_DrawImage(pme->m_pOwner->m_pHdrImage, &pme->m_pOwner->m_rectHdr, TRUE);
	TS_FitStaticText(pme->m_pOwner->a.m_pIDisplay, pme->m_pOwner->m_pHdrStatic, AEE_FONT_LARGE, pme->m_pOwner->m_pHdrText);
	
	TS_DrawImage(pme->m_pOwner->m_pBottomImage,&pme->m_pOwner->m_rectBtm,TRUE);
    TS_FitStaticText(pme->m_pOwner->a.m_pIDisplay,pme->m_pOwner->m_pLeftSoftStatic,AEE_FONT_LARGE,pme->m_pOwner->m_pLeftSoftText);
	TS_FitStaticText(pme->m_pOwner->a.m_pIDisplay,pme->m_pOwner->m_pRightSoftStatic,AEE_FONT_LARGE,pme->m_pOwner->m_pRightSoftText);
	TS_FitStaticText(pme->m_pOwner->a.m_pIDisplay,pme->m_pOwner->m_pMidSoftStatic,AEE_FONT_LARGE,pme->m_pOwner->m_pMidSoftText);
}

void TS_SetSoftButtonText(CTopSoupApp * pme,uint16 wTextLeftID, uint16 wTextRightID,uint16 wTextMidID)
{
	MEMSET(pme->m_pLeftSoftText,0,sizeof(pme->m_pLeftSoftText));
	MEMSET(pme->m_pRightSoftText,0,sizeof(pme->m_pRightSoftText));
	MEMSET(pme->m_pMidSoftText,0,sizeof(pme->m_pMidSoftText));
	if(wTextLeftID)
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,wTextLeftID,pme->m_pLeftSoftText,sizeof(pme->m_pLeftSoftText));
	if(wTextRightID)
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,wTextRightID,pme->m_pRightSoftText,sizeof(pme->m_pRightSoftText));
	if(wTextMidID)
		ISHELL_LoadResString(pme->a.m_pIShell,NAVIGATE_RES_FILE,wTextMidID,pme->m_pMidSoftText,sizeof(pme->m_pMidSoftText));
}

static void CTopSoupApp_onSplashDrawOver(void * po) 
{
	CTopSoupApp* pme = (CTopSoupApp*)po;

	if(pme->m_pWin)
		CTopSoupApp_Redraw(pme, TRUE);

}

/*===========================================================================
   This function draws the splash screen and brings up the main window
   after the splash timer runs out.
===========================================================================*/
void  TS_DrawSplash(CTopSoupApp * pme,AECHAR* prompt,int msTimeout,PFNNOTIFY on_splashOver)
{
	if( NULL == prompt )
		return;
	
	if (pme->m_pWin)
		CTopSoupApp_DisableWin(pme);
   
   {
      IImage * pi = ISHELL_LoadResImage(pme->a.m_pIShell, NAVIGATE_RES_FILE, IDJ_OBJECT_PROMPT);
	  IStatic * pInfoStatic = NULL;

      if (pi)
      {
         AEERect  rect;
		 AEEImageInfo      info;
		 int16 x,y;

		 IIMAGE_GetInfo(pi,&info);
		 x = ( pme->m_rectWin.dx - info.cx ) / 2;
		 y = pme->m_rectWin.y + ( pme->m_rectWin.dy - info.cy ) / 2;
         SETAEERECT(&rect,x,y,info.cx,info.cy);
         TS_DrawImage(pi, &rect, TRUE);


		 ISHELL_CreateInstance(pme->a.m_pIShell, AEECLSID_STATIC, (void **)&pInfoStatic);
		 ISTATIC_SetRect(pInfoStatic, &rect);
		 ISTATIC_SetProperties(pInfoStatic,  ST_MIDDLETEXT | ST_CENTERTEXT | ST_NOSCROLL);
         TS_FitStaticText(pme->a.m_pIDisplay, pInfoStatic, AEE_FONT_LARGE, prompt);

         TS_RELEASEIF(pi);
		 TS_RELEASEIF(pInfoStatic);
      }

      // start the timer.
	  if ( on_splashOver )
		  ISHELL_SetTimer(pme->a.m_pIShell, msTimeout, (PFNNOTIFY)on_splashOver, pme);  
	  else
		  ISHELL_SetTimer(pme->a.m_pIShell, msTimeout, (PFNNOTIFY)CTopSoupApp_onSplashDrawOver, pme);  
   }
}

