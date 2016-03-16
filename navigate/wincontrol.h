#ifndef TS_WINCONTROL_H
#define TS_WINCONTROL_H

#include "navigatepch.h"


#include "AEEStdLib.h"       
#include "AEEMenu.h"          // AEE Menu Services
#include "AEEImageCtl.h"      // AEE ImageCtl Services

typedef struct CTopSoupApp CTopSoupApp;


//image
void       TS_DrawImage(IImage * pImage, AEERect * pRect, boolean bCenter);

//menu
boolean    TS_AddMenuItem(IMenuCtl * pMenu, uint16 wTextID, AECHAR * pText, uint16 wImageID, uint16 wItemID, uint32 dwData);
void       TS_SetMenuAttr(IMenuCtl * pMenu, AEECLSID clsMenu, uint16 nColorDepth, AEERect * pRect, uint32 dwProps);

//static
void       TS_FitStaticText(IDisplay * pd, IStatic * ps, AEEFont font, AECHAR * pszText);

//rect
void       TS_FrameRect(IDisplay * pd, AEERect * pRect);

//dlg
void       TS_ErrorDlg(CTopSoupApp * pme, uint16 wResErrID);

//file
char *     TS_GetFileName(const char * psz);


//XXX
#define TS_DRAWHEADER(pme)             TS_DrawImage((pme)->m_pOwner->m_pHdrImage, &(pme)->m_pOwner->m_rectHdr, TRUE)

#endif