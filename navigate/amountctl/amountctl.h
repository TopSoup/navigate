/******************************************************************************
FILE:  ET_AmountCtl.h

SERVICES:  Header file for IAMOUNTCTL

GENERAL DESCRIPTION:

PUBLIC CLASSES AND STATIC FUNCTIONS:
	List the class(es) and static functions included in this file

INITIALIZATION & SEQUENCING REQUIREMENTS:

	See Exported Routines

       Copyright © 2002 QUALCOMM Incorporated.
               All Rights Reserved.
            QUALCOMM Proprietary/GTDR
******************************************************************************/

#define CAMOUNTCTL_MAX_DIGITS           8  // The max number of digits the control can display
#define CAMOUNTCTL_DECIMAL_PRECISION    2  // The decimal precision of the control's value

#define CTL_TAB_LEFT                    0  // The wParam value of a left key press in an EVT_CTL_TAB event
#define CTL_TAB_RIGHT                   1  // The wParam value of a right key press in an EVT_CTL_TAB event

// Control Structure
typedef struct
{
  uint32                  m_nAmount;
  IStatic*                m_pAmount;
  boolean                 m_bIsActive;
} CAmountCtl;

extern CAmountCtl*   CAMOUNTCTL_CreateInstance( IShell* pIShell );
extern boolean       CAMOUNTCTL_HandleEvent( CAmountCtl* pCtl, AEEEvent eCode, uint16 wParam, uint32 dwParam );
extern void          CAMOUNTCTL_SetActive( CAmountCtl* pCtl, boolean bActive );
extern boolean       CAMOUNTCTL_IsActive( CAmountCtl* pCtl );
extern void          CAMOUNTCTL_Release( CAmountCtl* pCtl );
extern void          CAMOUNTCTL_Reset( CAmountCtl* pCtl );
extern void          CAMOUNTCTL_SetValue( CAmountCtl* pCtl, uint32 nValue );
extern uint32        CAMOUNTCTL_GetValue( CAmountCtl* pCtl );
extern void          CAMOUNTCTL_ConvertValueToString( uint32 nValue, AECHAR* psValueBuf, uint32 nValueBufSize );
extern void          CAMOUNTCTL_Redraw( CAmountCtl* pCtl );
extern void          CAMOUNTCTL_SetRect( CAmountCtl* pCtl, AEERect* pRect );

