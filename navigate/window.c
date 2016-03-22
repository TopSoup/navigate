#include "window.h"
/*===============================================================================

                     CWindow Functions

=============================================================================== */
/*===========================================================================
   This function is the base class constructor of an IWindow object.
   It allocates memory for the interface and sets the vtbl.
===========================================================================*/
IWindow * CWindow_New(int16 nSize, CTopSoupApp * pOwner, VTBL(IWindow) * pvt)
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
boolean CWindow_ProcessEnable(IWindow * po, boolean bEnable)
{
   CWindow *   pme = (CWindow *)po;
   boolean     bProc = TRUE;

   if (!bEnable)
   {
      if (pme->m_bActive)
      {
         pme->m_bActive = FALSE;
         CTopSoupApp_CancelRedraw(pme->m_pOwner);
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