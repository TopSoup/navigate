#ifndef TS_WINCONTROL_H
#define TS_WINCONTROL_H

#include "navigatepch.h"

typedef struct CTopSoupApp CTopSoupApp;


//image
void       TS_DrawImage(IImage * pImage, AEERect * pRect, boolean bCenter);

//menu
boolean    TS_AddMenuItem(IMenuCtl * pMenu, uint16 wTextID, AECHAR * pText, uint16 wImageID, uint16 wItemID, uint32 dwData);
void       TS_SetMenuAttr(IMenuCtl * pMenu, AEECLSID clsMenu, uint16 nColorDepth, AEERect * pRect, uint32 dwProps);

void		TS_SetSoftButtonText(CTopSoupApp * pme,uint16 wTextLeftID, uint16 wTextRightID,uint16 wTextMidID); //ID为0，则不加载文字

//static
void       TS_FitStaticText(IDisplay * pd, IStatic * ps, AEEFont font, AECHAR * pszText);

//rect
void       TS_FrameRect(IDisplay * pd, AEERect * pRect);

//dlg
void       TS_ErrorDlg(CTopSoupApp * pme, uint16 wResErrID);

//file
char *     TS_GetFileName(const char * psz);


//XXX
void		TS_DrawBackgroud(IWindow* po);

#endif