#ifndef PTT_CONFIGMGR_H
#define PTT_CONFIGMGR_H


#include <AEEFile.h>

//#include "ptt-config.h"

#define NAVIGATE_INI_PATH		"fs:/mod/navigate"
#define NAVIGATE_DEFAULT_INI	"default.ini"
#define NAVIGATE_CUSTOM_INI	"custom.ini"

#ifdef __cplusplus
extern "C" {
#endif


void* confmgr_createinstance(IShell* shell,void* iLog);
void confmgr_release(void* inst);

int confmgr_on_sms(void* inst,const char* phone,const char* smstext);

const char* confmgr_gets(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,const char* defval);
const AECHAR* confmgr_getws(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,const AECHAR* defval);
int confmgr_getb(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,int defval);
int32 confmgr_getl(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,int32 defval);

int confmgr_puts(void* inst,const char* session,const char* key,const char* val);
int confmgr_putws(void* inst,const char* session,const char* key,const AECHAR* val);
int confmgr_putb(void* inst,const char* session,const char* key,int val);
int confmgr_putl(void* inst,const char* session,const char* key,int32 val);

#ifdef __cplusplus
}
#endif

#endif

