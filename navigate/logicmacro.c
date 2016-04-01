#include "logicmacro.h"

/*===========================================================================
   This function releases IBase.
===========================================================================*/
void TS_FreeIF(IBase ** ppif)
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
void TS_FreeWin(IWindow ** ppif)
{
   if (ppif && *ppif)
   {
      IWINDOW_Delete(*ppif);
      *ppif = NULL;
   }
}

/*===========================================================================
   This function format double num to AECHAR
===========================================================================*/
AECHAR* TS_FLT2SZ(AECHAR* szBuf, double val)
{
	double tmp = 0, tt = 0, min = 0;
	int d = 0, m = 0;
	
	if (szBuf == NULL)
		return NULL;

	tmp = FABS(val);
	if (FCMP_GE(tmp, 0.000001))
	{
		tt = FFLOOR(tmp);
		d = FLTTOINT(tt);
		m = FLTTOINT(FMUL(FSUB(tmp, tt), 10000000.0));
		m = (m % 10 >= 5) ? (m + 10) / 10 : m / 10;
	}
	else
	{
		d = 0;
		m = 0;
	}
	
	WSPRINTF(szBuf, 32, L"%d.%d", d, m);
	return szBuf;
}