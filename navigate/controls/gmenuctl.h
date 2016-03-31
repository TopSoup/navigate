/******************************************************************************
FILE:  GMenuCtl.h

SERVICES:  Header file for IGMENUCTL

GENERAL DESCRIPTION:

PUBLIC CLASSES AND STATIC FUNCTIONS:
List the class(es) and static functions included in this file

INITIALIZATION & SEQUENCING REQUIREMENTS:

See Exported Routines

Copyright ?2002 QUALCOMM Incorporated.
All Rights Reserved.
QUALCOMM Proprietary/GTDR
******************************************************************************/
#include "TQueueList.h"

#define CGMENUCTL_MAX_DIGITS           8  // The max number of digits the control can display
#define CGMENUCTL_DECIMAL_PRECISION    2  // The decimal precision of the control's value

#define CTL_TAB_LEFT                    0  // The wParam value of a left key press in an EVT_CTL_TAB event
#define CTL_TAB_RIGHT                   1  // The wParam value of a right key press in an EVT_CTL_TAB event
#define CTL_TAB_UP						2
#define CTL_TAB_DOWN					3


// MenuItem
typedef struct
{
	const AECHAR * pText;         // Text
	IImage *       pImage;        // Image

	const char * pszResImage;
	const char * pszResText;
	uint16 wText;
	uint16 wImage;

	uint16         wItemID;
	uint32         dwData;

	uint16		   wItemHeight;

} GMenuItem;

// MenuColor
typedef struct
{
	RGBVAL        cText;
	RGBVAL        cSelText;
} GMenuColors;

// Control Structure
typedef struct
{
	IImage            *m_pImageBk;
	IImage            *m_pImageSe;

	TQueueList        *m_pDataList;

	int               m_Index;
	int               m_startIndex;
	int               m_pageSize;

	int               m_textPos;

	boolean           m_isActive;

	AEERect           m_Rect;
	GMenuColors       m_Colors;

	uint32            m_Properties;
	
	boolean           m_bSelAble;

	boolean           m_bIsActive;
} CGMeuCtl;

extern CGMeuCtl*	CGMENUCTL_CreateInstance(IShell* pIShell);
extern boolean      CGMENUCTL_HandleEvent(CGMeuCtl* pCtl, AEEEvent eCode, uint16 wParam, uint32 dwParam);
extern void         CGMENUCTL_SetActive(CGMeuCtl* pCtl, boolean bActive);
extern boolean      CGMENUCTL_IsActive(CGMeuCtl* pCtl);
extern void         CGMENUCTL_Release(CGMeuCtl* pCtl);
extern void         CGMENUCTL_Reset(CGMeuCtl* pCtl);


extern void         CGMENUCTL_Redraw(CGMeuCtl* pCtl);
extern void         CGMENUCTL_SetRect(CGMeuCtl* pCtl, AEERect* pRect);
extern void			CGMENUCTL_GetRect(CGMeuCtl* pCtl, AEERect* pRect);


extern void			CGMENUCTL_AddItem(CGMeuCtl* pCtl, GMenuItem* item);
extern void			CGMENUCTL_SetColors(CGMeuCtl* pCtl, GMenuColors* cs);

extern void         CGMENUCTL_SetBGImage(CGMeuCtl* pCtl, const char * pszResFile, uint16 nResID);
extern void         CGMENUCTL_SetScImage(CGMeuCtl* pCtl, const char * pszResFile, uint16 nResID);

extern void         CGMENUCTL_SetSelAble(CGMeuCtl* pCtl, boolean bSelAble);


/********** HOW TO ***********/


/*
调用
void OP_GMNUITEM(CAPP *pMe)
{
	//rect
	{
		AEERect rect;
		SETAEERECT(&rect, 0, 0, 240, 295);
		CGMENUCTL_SetRect(pMe->m_gMenuCtl, &rect);
	}

	//color
	{
	GMenuColors menuCol;
	menuCol.cSelText = MAKE_RGB(255, 255, 255);
	menuCol.cText = MAKE_RGB(255, 255, 255);

	CGMENUCTL_SetColors(pMe->m_gMenuCtl, &menuCol);
}

	//bg/select image
	{
		CGMENUCTL_SetBGImage(pMe->m_gMenuCtl, SAMPLEPOSDET_RES_FILE, IDP_BG);
		CGMENUCTL_SetScImage(pMe->m_gMenuCtl, SAMPLEPOSDET_RES_FILE, IDI_OBJECT_5002);
	}

	//menu item
	{
		GMenuItem item;
		MEMSET(&item, 0, sizeof(GMenuItem));

		item.pszResImage = item.pszResText = SAMPLEPOSDET_RES_FILE;
		//item.wImage = IDI_OBJECT_5004;
		item.wItemID = IDS_STRING_DESTINATION;
		//item.pText = L"TEST1";
		item.wText = IDS_STRING_DESTINATION;
		CGMENUCTL_AddItem(pMe->m_gMenuCtl, &item);

		MEMSET(&item, 0, sizeof(GMenuItem));
		item.pszResImage = item.pszResText = SAMPLEPOSDET_RES_FILE;
		//item.wImage = IDI_OBJECT_5004;
		item.wItemID = IDS_STRING_EMPTY;
		//item.pText = L"TEST2";
		item.wText = IDS_STRING_EMPTY;
		CGMENUCTL_AddItem(pMe->m_gMenuCtl, &item);

		MEMSET(&item, 0, sizeof(GMenuItem));
		item.pszResImage = item.pszResText = SAMPLEPOSDET_RES_FILE;
		//item.wImage = IDI_OBJECT_5004;
		item.wItemID = IDS_STRING_NO_MATHED;
		//item.pText = L"TEST3";
		item.wText = IDS_STRING_NO_MATHED;
		CGMENUCTL_AddItem(pMe->m_gMenuCtl, &item);
	}

	//redraw
	CGMENUCTL_SetActive(pMe->m_gMenuCtl, TRUE);
	//CGMENUCTL_Redraw(pMe->m_gMenuCtl);
}

*/