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

//��������
void TS_DrawText(IDisplay* pIDisplay, AEEFont nFont, AECHAR* pText, AEERect *rect)
{
	RGBVAL oldColor;
	oldColor = IDISPLAY_SetColor(pIDisplay, CLR_USER_TEXT, MAKE_RGB(255, 255, 255));
	IDISPLAY_DrawText(pIDisplay, nFont, pText, -1, rect->x, rect->y, rect, IDF_TEXT_TRANSPARENT);
	IDISPLAY_SetColor(pIDisplay, CLR_USER_TEXT, oldColor);
}




/**
 * @brief ��õ�ǰʱ��
 */
int TS_GetTimeNow(ts_time_t* tw) {
	uint32 secs;
	JulianType julian;
	
	if( NULL == tw )
		return -1;
	
	secs = GETTIMESECONDS();
	GETJULIANDATE(secs,&julian);
	
	tw->year = julian.wYear;
	tw->month = (uint8)julian.wMonth;
	tw->day = (uint8)julian.wDay;
	tw->hour = (uint8)julian.wHour;
	tw->minute = (uint8)julian.wMinute;
	tw->second = (uint8)julian.wSecond;
	tw->millisecond = 0;
	
	return 0;
}


/**
 * @brief ����У��[-180,180]
 */
boolean		TS_CheckLon(AECHAR* szLon)
{
	double lon;
	lon = WSTRTOFLOAT(szLon);

	//У�龭γ��
	if (FCMP_L(lon, -180.0) || FCMP_G(lon, 180.0))
		return FALSE;

	return TRUE;
}

/**
 * @brief γ��У��[-90,90]
 */
boolean		TS_CheckLat(AECHAR* szLat)
{
	double lat;
	lat = WSTRTOFLOAT(szLat);

	//У�龭γ��
	if (FCMP_L(lat, -90.0) || FCMP_G(lat, 90.0))
		return FALSE;

	return TRUE;
}