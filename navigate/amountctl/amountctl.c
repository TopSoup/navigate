/******************************************************************************
FILE:  ET_AmountCtl.c

SERVICES:  
   Implements a custom control that can be used for displaying monetary
   values adjusted for the two places of the decimal.  

GENERAL DESCRIPTION:
   This is NOT a true BREW control.  It simply uses an interface that makes 
   it behave like one.  The primary difference between this control 
   and true BREW controls is that this control is not derived from IControl.
   Therefore, special handling is occasionally required when using this control.

   This decimal precision of the control can be increased or reduced by 
   changing the value of CAMOUNTCTL_DECIMAL_PRECISION defined in CAmountCtl.h.

   The maximum number of digits the control can display can be increased or
   decreased by changing the value of CAMOUNTCTL_MAX_DIGITS defined in 
   CAmountCtl.h.

PUBLIC CLASSES AND STATIC FUNCTIONS:
	List the class(es) and static functions included in this file

INITIALIZATION & SEQUENCING REQUIREMENTS:

	See Exported Routines

       Copyright © 2002 QUALCOMM Incorporated.
               All Rights Reserved.
            QUALCOMM Proprietary/GTDR
******************************************************************************/
#include "AEEModGen.h"
#include "AEEAppGen.h"
#include "amountctl.h"
#include "AEE.h"
#include "AEEDisp.h"
#include "AEEStdLib.h"
#include "AEEShell.h"
#include "nmdef.h"

/*===========================================================================

FUNCTION: CAMOUNTCTL_CreateInstance

DESCRIPTION:
	This function creates an instance of the CAmountCtl and returns its pointer.

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

extern CAmountCtl* CAMOUNTCTL_CreateInstance( IShell* pIShell )
{
  CAmountCtl* pAmtCtl = MALLOC( sizeof( CAmountCtl ) );

  if( !pAmtCtl )
    return NULL;
  else
  {
    pAmtCtl->m_nAmount = 0;
    pAmtCtl->m_bIsActive = FALSE;

    if( ISHELL_CreateInstance( pIShell, AEECLSID_STATIC, (void**)&pAmtCtl->m_pAmount ) != SUCCESS )
    {
      FREEIF( pAmtCtl );
      return NULL;
    }
    else
    {
      ISTATIC_SetProperties( pAmtCtl->m_pAmount, ST_MIDDLETEXT );
    }
  }
  return pAmtCtl;
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_HandleEvent

DESCRIPTION:
	The event handler for a CAmountCtl.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

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
extern boolean CAMOUNTCTL_HandleEvent( CAmountCtl* pCtl, AEEEvent eCode, uint16 wParam, uint32 dwParam )
{
  AEEApplet* pApp = (AEEApplet*) GETAPPINSTANCE();

  // Only handle the event of the control is currently active
  if( pCtl && pCtl->m_bIsActive )
  {
    switch( eCode )
    {
    case EVT_KEY:
      switch( wParam )
      {
      case AVK_CLR:  // Delete a digit if the Clr button is pressed.
                     // If all the digits have been cleared return FALSE
        if( pCtl->m_nAmount > 0 )
        {
          pCtl->m_nAmount /= 10;
          CAMOUNTCTL_Redraw( pCtl );
          return TRUE;
        }
        else
          return FALSE;

      // Send an EVT_CTL_TAB event if the left or right buttons are pressed.
      case AVK_RIGHT:
          ISHELL_PostEvent( pApp->m_pIShell, pApp->clsID, EVT_CTL_TAB, CTL_TAB_RIGHT, NULL );
          return TRUE;

      case AVK_LEFT:
          ISHELL_PostEvent( pApp->m_pIShell, pApp->clsID, EVT_CTL_TAB, CTL_TAB_LEFT, NULL );
          return TRUE;
      }
      break;

    // Update the control's display is a digit is pressed.
    case EVT_KEY_PRESS:
      switch( wParam )
      {
      case AVK_0:
      case AVK_1:
      case AVK_2:
      case AVK_3:
      case AVK_4:
      case AVK_5:
      case AVK_6:
      case AVK_7:
      case AVK_8:
      case AVK_9:
        CAMOUNTCTL_SetValue( pCtl, ( pCtl->m_nAmount * 10 ) + ( wParam - AVK_0 ) );
        CAMOUNTCTL_Redraw( pCtl );
        return TRUE;
      }
      break;
    }
  }
  return FALSE;
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_SetActive

DESCRIPTION:
	Sets the active state of the control.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

  bActive [in] - A boolean value indicating if the control state is active (TRUE)
    or inactive (FALSE)

DEPENDENCIES:
   None

RETURN VALUE:
   None

SIDE EFFECTS:
  None
===========================================================================*/
extern void CAMOUNTCTL_SetActive( CAmountCtl* pCtl, boolean bActive )
{
  if( pCtl )
  {
    pCtl->m_bIsActive = bActive;
    CAMOUNTCTL_Redraw( pCtl );
  }
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_SetActive

DESCRIPTION:
	Returns the current active state of the control.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

DEPENDENCIES:
   None

RETURN VALUE:
   TRUE - If the control is currently active
   FALSE - If the control is currently inactive

SIDE EFFECTS:
  None
===========================================================================*/
extern boolean CAMOUNTCTL_IsActive( CAmountCtl* pCtl )
{
  if( pCtl )
    return pCtl->m_bIsActive;
  else
    return FALSE;
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_Release

DESCRIPTION:
	Deletes the control and all associated memory.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

DEPENDENCIES:
  None

RETURN VALUE:
  None

SIDE EFFECTS:
  None
===========================================================================*/
extern void CAMOUNTCTL_Release( CAmountCtl* pCtl )
{
  if( pCtl )
  {
    ISTATIC_Release( pCtl->m_pAmount );
    FREEIF( pCtl );
  }
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_Reset

DESCRIPTION:
	Resets the value of the control to zero.  This function redraws the control.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

DEPENDENCIES:
  None

RETURN VALUE:
  None

SIDE EFFECTS:
  None
===========================================================================*/
extern void CAMOUNTCTL_Reset( CAmountCtl* pCtl )
{
  if( pCtl )
  {
    pCtl->m_nAmount = 0;
    pCtl->m_bIsActive = FALSE;
    ISTATIC_Reset( pCtl->m_pAmount );
  }
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_SetValue

DESCRIPTION:
	Sets the numerical value of the control.  This causes the control to redraw
  itself.

  The numerical value is passed in the nValue argument as an integer.  The
  control will format the integer by placing the decimal in the correct
  location and padding the value with leading zeros if necessary.

  For example, a value of "34567" pass in the nValue argument will be formatted
  and displayed in the control as "345.67".

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

  nValue [in] - The new numeric value of the control

DEPENDENCIES:
  None

RETURN VALUE:
  None

SIDE EFFECTS:
  None
===========================================================================*/
extern void CAMOUNTCTL_SetValue( CAmountCtl* pCtl, uint32 nValue )
{
  char psAmount[ CAMOUNTCTL_MAX_DIGITS + 1 ];

  if( pCtl )
  {
    SPRINTF( psAmount, "%i", nValue );
    if( STRLEN( psAmount ) < CAMOUNTCTL_MAX_DIGITS )
      pCtl->m_nAmount = nValue;
  }
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_SetValue

DESCRIPTION:
	Returns the current numeric value of the control.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

DEPENDENCIES:
  None

RETURN VALUE:
  Returns the value of the control's digits as an integer (decimal points
  and leading zeros will not be included).

  For example, if the control's value is "345.67", an integer value of 
  "34567" will be returned.

SIDE EFFECTS:
  None
===========================================================================*/
extern uint32 CAMOUNTCTL_GetValue( CAmountCtl* pCtl )
{
  if( pCtl )
    return pCtl->m_nAmount;
  else
    return 0;
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_ConvertValueToString

DESCRIPTION:
	This is a generic utility function which takes a value (specified as an
  integer) and returns a formatted string containing the decimal point 
  and leading zeros if required.

  NOTE: This function does not require a CAmountCtl instance.

  For example, a value of "23" will be formatted as "0.23".

PARAMETERS:
	nValue [in] - The integer value to be converted

  psValueBuf [out] - A pointer the string buffer where the formatted string
    will be written.

  nValueBufSize [in] - The number of bytes in the output buffer.

DEPENDENCIES:
  None

RETURN VALUE:
  None

SIDE EFFECTS:
  If the size of the string buffer is less that the formatted string, the
  value written to the buffer will be truncated.
===========================================================================*/
extern void CAMOUNTCTL_ConvertValueToString( uint32 nValue, AECHAR* psValueBuf, uint32 nValueBufSize )
{
  AECHAR psFormatString[] = { '%','i','.','%','.','2','i','\0' };
  uint32 nPrecision = 1;
  uint32 nIntAmount = 0;
  uint32 nFloatAmount = 0;
  int i;

  for( i = 0; i < CAMOUNTCTL_DECIMAL_PRECISION; i ++ )
    nPrecision = nPrecision * 10;
  nIntAmount = nValue / nPrecision;
  nFloatAmount = nValue - ( nIntAmount * nPrecision );
  WSPRINTF( psValueBuf, nValueBufSize, psFormatString, nIntAmount, nFloatAmount );
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_Redraw

DESCRIPTION:
	Redraws the control on the device screen.  If the control currently has
  focus, the control is drawn with a border.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

DEPENDENCIES:
  None

RETURN VALUE:
  None

SIDE EFFECTS:
  Updates the device screen.
===========================================================================*/
extern void CAMOUNTCTL_Redraw( CAmountCtl* pCtl )
{
  AEEApplet* pApp = (AEEApplet*) GETAPPINSTANCE();
  AECHAR psAmountString[ CAMOUNTCTL_MAX_DIGITS + 2 ];
  AEERect rRect;

  if( pCtl )
  {
    CAMOUNTCTL_ConvertValueToString( pCtl->m_nAmount, psAmountString, ( CAMOUNTCTL_MAX_DIGITS + 2 ) * sizeof( AECHAR ) );
    ISTATIC_SetText( pCtl->m_pAmount, NULL, psAmountString, AEE_FONT_NORMAL, AEE_FONT_NORMAL );
    ISTATIC_Redraw( pCtl->m_pAmount );
    ISTATIC_GetRect( pCtl->m_pAmount, &rRect );
    rRect.x--;
    if( pCtl->m_bIsActive )
    {
      IDISPLAY_SetColor( pApp->m_pIDisplay, CLR_USER_FRAME, RGB_BLACK );
      IDISPLAY_FrameRect( pApp->m_pIDisplay, &rRect );
    }
    else
    {
      IDISPLAY_FrameRect( pApp->m_pIDisplay, &rRect );
    }
    IDISPLAY_Update( pApp->m_pIDisplay );
    IDISPLAY_SetColor( pApp->m_pIDisplay, CLR_USER_FRAME, RGB_WHITE );
  }
}

/*===========================================================================

FUNCTION: CAMOUNTCTL_SetRect

DESCRIPTION:
	Sets the rectangular coordinates of the control.

PARAMETERS:
	pCtl [in] - A pointer to a CAmountCtl instance

  pRect [in] - A pointer to the new rectangle coordinates.

DEPENDENCIES:
  None

RETURN VALUE:
  None

SIDE EFFECTS:
  None
===========================================================================*/
extern void CAMOUNTCTL_SetRect( CAmountCtl* pCtl, AEERect* pRect )
{
  if( pCtl && pRect )
  { pRect->y += 2;  // Minor adjustment to put the baseline of the text in line with the label text
    ISTATIC_SetRect( pCtl->m_pAmount, pRect );
  }
}
