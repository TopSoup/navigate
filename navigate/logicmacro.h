#ifndef  TS_LOGICMACRO_H
#define  TS_LOGICMACRO_H

#include "navigatepch.h"

typedef struct _IWindow  IWindow;

#define TS_RELEASEIF(p)                TS_FreeIF((IBase **)&(p))
#define TS_RELEASEWIN(p)               TS_FreeWin((IWindow **)&(p))
#define TS_WINERR_RETURN(p)            { TS_RELEASEWIN(p); return NULL; }

void       TS_FreeIF(IBase ** ppif);
void       TS_FreeWin(IWindow ** ppif);

//key
#define TS_ISEVTKEY(e)                 ((e) == EVT_KEY)
#define TS_ISCLR(e)                    (TS_ISEVTKEY(e) && wParam == AVK_CLR)
#define TS_ISEVTCMD(e)                 ((e) == EVT_COMMAND)
#define TS_ISCMD(e, c)                 (TS_ISEVTCMD(e) && (c) == wParam)
#define TS_ISSOFT(e)					(TS_ISEVTKEY(e) && ( wParam == AVK_SOFT1 || wParam == AVK_SOFT2 ) )

#endif