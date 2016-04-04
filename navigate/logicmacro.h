#ifndef  TS_LOGICMACRO_H
#define  TS_LOGICMACRO_H

#include "navigatepch.h"

#define TS_HEADER_Y           17
#define TS_TITLE_Y	TS_HEADER_Y + 38	//38为标题图片高度


typedef struct _IWindow  IWindow;

#define TS_RELEASEIF(p)                TS_FreeIF((IBase **)&(p))
#define TS_RELEASEWIN(p)               TS_FreeWin((IWindow **)&(p))
#define TS_WINERR_RETURN(p)            { TS_RELEASEWIN(p); return NULL; }

void       TS_FreeIF(IBase ** ppif);
void       TS_FreeWin(IWindow ** ppif);

AECHAR*	   TS_FLT2SZ(AECHAR* szBuf, double val);

//绘制文字
void       TS_DrawText(IDisplay* pIDisplay, AEEFont nFont, AECHAR* pText, AEERect *rect);


/**
 * @brief
 */
typedef struct _ts_time_t {
	uint16			year;
	uint8			month;
	uint8			day;
	uint8			hour;
	uint8			minute;
	uint8			second;
	uint16			millisecond;
}ts_time_t;

	
/**
 * @brief 获得当前时间
 */
int			TS_GetTimeNow(ts_time_t* tw);

/**
 * @brief 经度校验[-180,180]
 */
boolean		TS_CheckLon(AECHAR* szLon);
/**
 * @brief 纬度校验[-90,90]
 */
boolean		TS_CheckLat(AECHAR* szLon);

//key
#define TS_ISEVTKEY(e)                 ((e) == EVT_KEY)
#define TS_ISCLR(e)                    (TS_ISEVTKEY(e) && wParam == AVK_CLR)
#define TS_ISEVTCMD(e)                 ((e) == EVT_COMMAND)
#define TS_ISCMD(e, c)                 (TS_ISEVTCMD(e) && (c) == wParam)	//TODO?
#define TS_ISSOFT(e)					(TS_ISEVTKEY(e) && ( wParam == AVK_SOFT1 || wParam == AVK_SOFT2 ) )
#define TS_ISSELCHG(e)                 ((e) == EVT_CTL_SEL_CHANGED)
#define TS_ISSEL(e, c)                 ((e) == EVT_KEY_PRESS && (c) == AVK_SELECT)

#endif