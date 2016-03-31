/******************************************************************************
FILE:  GMenuCtl.c

SERVICES:
Implements a custom control that can be used for displaying monetary
values adjusted for the two places of the decimal.

GENERAL DESCRIPTION:
This is NOT a true BREW control.  It simply uses an interface that makes
it behave like one.  The primary difference between this control
and true BREW controls is that this control is not derived from IControl.
Therefore, special handling is occasionally required when using this control.

This decimal precision of the control can be increased or reduced by
changing the value of CGMENUCTL_DECIMAL_PRECISION defined in CGMeuCtl.h.

The maximum number of digits the control can display can be increased or
decreased by changing the value of CGMENUCTL_MAX_DIGITS defined in
CGMeuCtl.h.

PUBLIC CLASSES AND STATIC FUNCTIONS:
List the class(es) and static functions included in this file

INITIALIZATION & SEQUENCING REQUIREMENTS:

See Exported Routines

Copyright ?2002 QUALCOMM Incorporated.
All Rights Reserved.
QUALCOMM Proprietary/GTDR
******************************************************************************/
#include "AEEModGen.h"
#include "AEEAppGen.h"
#include "AEE.h"
#include "AEEDisp.h"
#include "AEEStdLib.h"
#include "AEEShell.h"
#include "nmdef.h"

#include "GMenuCtl.h"

#define GM_RELEASEIF(p)                GM_FreeIF((IBase **)&(p))

static void GM_FreeIF(IBase ** ppif);

static void CGMENUCTL_FreeImage(CGMeuCtl* pMe);

/*===========================================================================

FUNCTION: CGMENUCTL_CreateInstance

DESCRIPTION:
This function creates an instance of the CGMeuCtl and returns its pointer.

PARAMETERS:
pIShell [in] - A pointer to the application's IShell instance.

DEPENDENCIES:
None

RETURN VALUE:
Returns a pointer to the created control or NULL if the control could
not be created successfully.

SIDE EFFECTS:
None
===========================================================================*/

extern CGMeuCtl* CGMENUCTL_CreateInstance(IShell* pIShell)
{
	CGMeuCtl* pCtl = MALLOC(sizeof(CGMeuCtl));

	if (!pCtl)
		return NULL;

	if (!TQueueList_Init(&(pCtl->m_pDataList)))
	{
		FREE(pCtl);
		return NULL;
	}

	pCtl->m_pageSize = 1000;
	
	pCtl->m_bSelAble = TRUE;

	return pCtl;
}

/*===========================================================================

FUNCTION: CGMENUCTL_HandleEvent

DESCRIPTION:
The event handler for a CGMeuCtl.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

eCode [in] - Event code

wParam [in] - 16-bit event data

dwParam [in] - 32-bit event data

DEPENDENCIES:
None

RETURN VALUE:
TRUE - If the event was handled by the control
FALSE - if the event was not handled

SIDE EFFECTS:
None
===========================================================================*/
extern boolean CGMENUCTL_HandleEvent(CGMeuCtl* pCtl, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
	AEEApplet* pApp = (AEEApplet*)GETAPPINSTANCE();
	GMenuItem *item = NULL;

	DBGPRINTF("MENU: eCode:%x wParam:%x", eCode, wParam);

	// Only handle the event of the control is currently active
	if (pCtl && pCtl->m_bIsActive)
	{
		switch (eCode)
		{
		// Update the control's display is a digit is pressed.
		case EVT_KEY_PRESS:
			switch (wParam)
			{
			case AVK_SELECT:
				//通知上层
				item = (GMenuItem *)TQueueList_GetElem(pCtl->m_pDataList, pCtl->m_Index);
				if (item == NULL)
				{
					DBGPRINTF("TQueueList_GetElem Failed at pos:%d total:%d", pCtl->m_Index, pCtl->m_pageSize);
					return FALSE;
				}
				DBGPRINTF("TQueueList_GetElem pos:%d wText:%d", pCtl->m_Index, item->wText);
				ISHELL_PostEvent(pApp->m_pIShell, pApp->clsID, EVT_COMMAND, item->wText, 0);
				return TRUE;

			case AVK_DOWN:

				pCtl->m_Index ++;
				if (pCtl->m_Index >= pCtl->m_pageSize - 1)
					pCtl->m_Index = pCtl->m_pageSize - 1;


				CGMENUCTL_Redraw(pCtl);				

				return TRUE;

			case AVK_UP:
				pCtl->m_Index --;
				if (pCtl->m_Index <= 0)
					pCtl->m_Index = 0;

				CGMENUCTL_Redraw(pCtl);

				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

/*===========================================================================

FUNCTION: CGMENUCTL_SetActive

DESCRIPTION:
Sets the active state of the control.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

bActive [in] - A boolean value indicating if the control state is active (TRUE)
or inactive (FALSE)

DEPENDENCIES:
None

RETURN VALUE:
None

SIDE EFFECTS:
None
===========================================================================*/
extern void CGMENUCTL_SetActive(CGMeuCtl* pCtl, boolean bActive)
{
	if (pCtl)
	{
		pCtl->m_bIsActive = bActive;
		CGMENUCTL_Redraw(pCtl);
	}
}

extern void CGMENUCTL_SetSelAble(CGMeuCtl* pCtl, boolean bSelAble)
{
	if (pCtl)
	{
		pCtl->m_bSelAble = bSelAble;
		CGMENUCTL_Redraw(pCtl);
	}
}


/*===========================================================================

FUNCTION: CGMENUCTL_SetActive

DESCRIPTION:
Returns the current active state of the control.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

DEPENDENCIES:
None

RETURN VALUE:
TRUE - If the control is currently active
FALSE - If the control is currently inactive

SIDE EFFECTS:
None
===========================================================================*/
extern boolean CGMENUCTL_IsActive(CGMeuCtl* pCtl)
{
	if (pCtl)
		return pCtl->m_bIsActive;
	else
		return FALSE;
}

/*===========================================================================

FUNCTION: CGMENUCTL_Release

DESCRIPTION:
Deletes the control and all associated memory.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

DEPENDENCIES:
None

RETURN VALUE:
None

SIDE EFFECTS:
None
===========================================================================*/
extern void CGMENUCTL_Release(CGMeuCtl* pCtl)
{
	if (pCtl)
	{

		if (pCtl->m_pImageBk)
		{
			GM_RELEASEIF(pCtl->m_pImageBk);
		}

		if (pCtl->m_pImageSe)
		{
			GM_RELEASEIF(pCtl->m_pImageSe);
		}

		//
		CGMENUCTL_FreeImage(pCtl);

		if (pCtl->m_pDataList)
		{
			TQueueList_Destroy(&(pCtl->m_pDataList));
		}
		
		FREEIF(pCtl);
	}
}

/*===========================================================================

FUNCTION: CGMENUCTL_Reset

DESCRIPTION:
Resets the value of the control to zero.  This function redraws the control.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

DEPENDENCIES:
None

RETURN VALUE:
None

SIDE EFFECTS:
None
===========================================================================*/
extern void CGMENUCTL_Reset(CGMeuCtl* pCtl)
{
	if (pCtl)
	{
		if (pCtl->m_pDataList)
			TQueueList_Destroy(&(pCtl->m_pDataList));
		
		pCtl->m_Index = 0;
		pCtl->m_startIndex = 0;
		pCtl->m_pageSize = 0;
		
		pCtl->m_textPos = 0;
		
		//pCtl->m_isActive = FALSE;
		
		//MEMSET(&pCtl->m_Rect, 0, sizeof(AEERect));

		//MEMSET(&pCtl->m_Colors, 0, sizeof(GMenuColors));
		
		//pCtl->m_bSelAble;
		
		DBGPRINTF("GMENU RESET");
	}
}


/*===========================================================================

FUNCTION: CGMENUCTL_Redraw

DESCRIPTION:
Redraws the control on the device screen.  If the control currently has
focus, the control is drawn with a border.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

DEPENDENCIES:
None

RETURN VALUE:
None

SIDE EFFECTS:
Updates the device screen.
===========================================================================*/
extern void CGMENUCTL_Redraw(CGMeuCtl* pMe)
{
	AEEApplet* pApp = (AEEApplet*)GETAPPINSTANCE();

	if (pMe)
	{
		int i, j, height, h, a, b;
		AEEImageInfo infSe, infIc;
		AEERect rec;
		int xx, yy, dxx, dyy;
		RGBVAL oldColor;

		GMenuItem * pData = NULL;
		TQueueList * p = pMe->m_pDataList;

		ZEROAT(&infSe);

		IDISPLAY_EraseRect(pApp->m_pIDisplay, &pMe->m_Rect);

		h = IDISPLAY_GetFontMetrics(pApp->m_pIDisplay, AEE_FONT_NORMAL, &a, &b);

		//绘制背景图
		if (pMe->m_pImageBk)
		{
			IIMAGE_SetDrawSize(pMe->m_pImageBk, pMe->m_Rect.dx, pMe->m_Rect.dy);
			IIMAGE_Draw(pMe->m_pImageBk, pMe->m_Rect.x, pMe->m_Rect.y);
		}

		//确定菜单高度
		if (pMe->m_pImageSe && pMe->m_bSelAble)
		{
			IIMAGE_GetInfo(pMe->m_pImageSe, &infSe);
			IIMAGE_SetDrawSize(pMe->m_pImageSe, pMe->m_Rect.dx, infSe.cy);
		}
		else
		{
			infSe.cx = pMe->m_Rect.dx;
			infSe.cy = h;
		}

		//绘制菜单项
		i = 0;
		j = 0;
		height = pMe->m_Rect.y + 5;
		while (p)
		{
			AECHAR	iTemText[256];

			if (i < pMe->m_startIndex)
			{
				p = p->pNext;
				i++;
				continue;
			}

			if (j >= pMe->m_pageSize)
			{
				break;
			}

			pData = (GMenuItem*)p->pData;

			//加载菜单ICON
			if (pData->pImage == NULL && pData->pszResImage != NULL && pData->wImage != 0)
			{
				pData->pImage = ISHELL_LoadResImage(pApp->m_pIShell, pData->pszResImage, pData->wImage);
			}

			//加载菜单Text
			if (pData->pszResText != NULL && pData->wText != 0)
			{
				MEMSET(iTemText, 0, sizeof(AECHAR)*256);
				
				ISHELL_LoadResString(pApp->m_pIShell, pData->pszResText, pData->wText, iTemText, sizeof(AECHAR)* 256);

				pData->pText = iTemText;
			}

			if (i == pMe->m_Index)
			{
				if (pMe->m_pImageSe && pMe->m_bSelAble)
					IIMAGE_Draw(pMe->m_pImageSe, pMe->m_Rect.x, height + 2);

				ZEROAT(&infIc);
				if (pData->pImage)
				{
					IIMAGE_GetInfo(pData->pImage, &infIc);
					IIMAGE_Draw(pData->pImage, pMe->m_Rect.x, height + (infSe.cy - infIc.cy) / 2);
				}

				xx = pMe->m_Rect.x + infIc.cx + 2;
				yy = height + (infSe.cy - h) / 2 + 2;
				dxx = pMe->m_Rect.x + pMe->m_Rect.dx - xx;
				dyy = h;
				SETAEERECT(&rec, xx, yy, dxx, dyy);

				oldColor = IDISPLAY_SetColor(pApp->m_pIDisplay, CLR_USER_TEXT, pMe->m_Colors.cSelText);
				if (pMe->m_Properties & 0x02)
					IDISPLAY_DrawText(pApp->m_pIDisplay, AEE_FONT_NORMAL, pData->pText + pMe->m_textPos, -1, xx + 1, yy, &rec, IDF_TEXT_TRANSPARENT);
				IDISPLAY_DrawText(pApp->m_pIDisplay, AEE_FONT_NORMAL, pData->pText + pMe->m_textPos, -1, xx, yy, &rec, IDF_TEXT_TRANSPARENT);

				if (pMe->m_isActive)
				{
					if (IDISPLAY_MeasureText(pApp->m_pIDisplay, AEE_FONT_NORMAL, pData->pText) > dxx)
					{
						if (IDISPLAY_MeasureText(pApp->m_pIDisplay, AEE_FONT_NORMAL, pData->pText + pMe->m_textPos) > dxx)
						{
							pMe->m_textPos++;
						}
						else
							pMe->m_textPos = 0;

						//单行太长的话，动态显示
						ISHELL_SetTimer(pApp->m_pIShell, 300, (PFNNOTIFY)CGMENUCTL_Redraw, (void*)pMe);
					}
				}

				height += infSe.cy;
				IDISPLAY_SetColor(pApp->m_pIDisplay, CLR_USER_TEXT, oldColor);
			}
			else
			{
				ZEROAT(&infIc);
				if (pData->pImage)
				{
					IIMAGE_GetInfo(pData->pImage, &infIc);
					IIMAGE_Draw(pData->pImage, pMe->m_Rect.x, height + (infSe.cy - infIc.cy) / 2);
				}

				xx = pMe->m_Rect.x + infIc.cx + 2;
				yy = height + (infSe.cy - h) / 2 + 2;
				dxx = pMe->m_Rect.x + pMe->m_Rect.dx - xx;
				dyy = h;
				SETAEERECT(&rec, xx, yy, dxx, dyy);

				oldColor = IDISPLAY_SetColor(pApp->m_pIDisplay, CLR_USER_TEXT, pMe->m_Colors.cText);
				if (pMe->m_Properties & 0x01)
					IDISPLAY_DrawText(pApp->m_pIDisplay, AEE_FONT_NORMAL, pData->pText, -1, xx + 1, yy, &rec, IDF_TEXT_TRANSPARENT);
				IDISPLAY_DrawText(pApp->m_pIDisplay, AEE_FONT_NORMAL, pData->pText, -1, xx, yy, &rec, IDF_TEXT_TRANSPARENT);

				height += infSe.cy;
				IDISPLAY_SetColor(pApp->m_pIDisplay, CLR_USER_TEXT, oldColor);
			}

			p = p->pNext;
			i++;
			j++;
		}

		IDISPLAY_Update(pApp->m_pIDisplay);
		return ;
	}
}

/*===========================================================================

FUNCTION: CGMENUCTL_SetRect

DESCRIPTION:
Sets the rectangular coordinates of the control.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

pRect [in] - A pointer to the new rectangle coordinates.

DEPENDENCIES:
None

RETURN VALUE:
None

SIDE EFFECTS:
None
===========================================================================*/
extern void CGMENUCTL_SetRect(CGMeuCtl* pCtl, AEERect* pRect)
{
	if (pCtl && pRect)
	{
		pCtl->m_Rect = *pRect;
	}
}

/*===========================================================================

FUNCTION: CGMENUCTL_GetRect

DESCRIPTION:
Get the rectangular coordinates of the control.

PARAMETERS:
pCtl [in] - A pointer to a CGMeuCtl instance

pRect [out] - A pointer to the new rectangle coordinates.

DEPENDENCIES:
None

RETURN VALUE:
None

SIDE EFFECTS:
None
===========================================================================*/
extern void CGMENUCTL_GetRect(CGMeuCtl* pCtl, AEERect* pRect)
{
	if (pCtl && pRect)
	{
		*pRect = pCtl->m_Rect;
	}
}

extern void	CGMENUCTL_AddItem(CGMeuCtl* pCtl, GMenuItem* item)
{
	//if (pCtl->m_pDataList)
	{
		elemType elem = (elemType)MALLOC(sizeof(GMenuItem));
		if (elem == NULL)
		{
			DBGPRINTF("MALLOC FAILED!");
			return;
		}
		
		MEMCPY(elem, item, sizeof(GMenuItem));

		TQueueList_Insert(&(pCtl->m_pDataList), elem);

		//FREE(elem);

		pCtl->m_pageSize = TQueueList_GetCount(pCtl->m_pDataList);
		DBGPRINTF("MENU ITEM COUNT:%d", pCtl->m_pageSize);
	}
}

extern void	CGMENUCTL_SetColors(CGMeuCtl* pCtl, GMenuColors* cs)
{
	if (cs != NULL)
		pCtl->m_Colors = *cs;
}


extern void	CGMENUCTL_SetBGImage(CGMeuCtl* pCtl, const char * pszResFile, uint16 nResID)
{
	AEEApplet* pApp = (AEEApplet*)GETAPPINSTANCE();

	if (pCtl->m_pImageBk)
	{
		GM_RELEASEIF(pCtl->m_pImageBk);
	}

	pCtl->m_pImageBk = ISHELL_LoadResImage(pApp->m_pIShell, pszResFile, nResID);
	if (!pCtl->m_pImageBk) {
		return ;
	}
}

extern void	CGMENUCTL_SetScImage(CGMeuCtl* pCtl, const char * pszResFile, uint16 nResID)
{
	AEEApplet* pApp = (AEEApplet*)GETAPPINSTANCE();

	if (pCtl->m_pImageSe)
	{
		GM_RELEASEIF(pCtl->m_pImageSe);
	}

	pCtl->m_pImageSe = ISHELL_LoadResImage(pApp->m_pIShell, pszResFile, nResID);
	if (!pCtl->m_pImageSe) {
		return;
	}
}

/*===========================================================================
This function releases IBase.
===========================================================================*/
static void GM_FreeIF(IBase ** ppif)
{
	if (ppif && *ppif)
	{
		IBASE_Release(*ppif);
		*ppif = NULL;
	}
}


static void CGMENUCTL_FreeImage(CGMeuCtl* pMe)
{
	int i = 0;
	GMenuItem * pData = NULL;
	TQueueList * p = pMe->m_pDataList;

	while (p)
	{
		if (i >= pMe->m_pageSize)
		{
			break;
		}

		pData = (GMenuItem*)p->pData;

		//释放菜单ICON
		GM_RELEASEIF(pData->pImage);

		i ++;
		p = p->pNext;
	}
}