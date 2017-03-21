#include <AEEStdLib.h>
#include <AEEFile.h>



#define INI_FILETYPE                  IFile*
#define ini_openread(filename,file)   ((*(file) = IFILEMGR_OpenFile((IFileMgr*)FileMgr,filename,_OFM_READ)) != NULL)
#define ini_openwrite(filename,file)  ((*(file) = IFILEMGR_OpenFile((IFileMgr*)FileMgr,filename,(IFILEMGR_Test((IFileMgr*)FileMgr,filename) == SUCCESS ? _OFM_READWRITE : _OFM_CREATE) )) != NULL)
#define ini_close(file)               (IFILE_Release(*file))
#define ini_read(buffer,size,file)    (ini_brew_readline(*(file),buffer,size) > 0)
#define ini_write(buffer,file)        (IFILE_Write(*(file),buffer,STRLEN(buffer)) > 0)
#define ini_rename(source,dest,FileMgr)       (IFILEMGR_Rename((IFileMgr*)FileMgr,source,dest) == AEE_SUCCESS)
#define ini_remove(filename,FileMgr)          (IFILEMGR_Remove((IFileMgr*)FileMgr,filename) == AEE_SUCCESS)

#define INI_FILEPOS                   int32
#define ini_tell(file,pos)            (*(pos) = IFILE_Seek(*(file),_SEEK_CURRENT,0))
#define ini_seek(file,pos)            ( IFILE_Seek(*(file),_SEEK_START,*(pos)) == AEE_SUCCESS)

/* for floating-point support, define additional types and functions */
#define ini_ftoa(string,value)        SPRINTF((string),"%f",(value))
#define ini_atof(string)              (INI_REAL)STRTOD((string),NULL)



  #define TCHAR     char
  #define __T(s)    s
  #define _tcscat   STRCAT
  #define _tcschr   STRCHR
  #define _tcscmp   STRCMP
  #define _tcscpy   STRCPY
  #define _tcsicmp  STRICMP
  #define _tcslen   STRLEN
  #define _tcsncmp  STRNCMP
  #define _tcsnicmp STRNICMP
  #define _tcsrchr  STRRCHR
  #define _tcstol   STRTOUL
  #define _tcstod   STRTOD
  #define _stprintf SPRINTF
  //#define _tfgets   fgets
  //#define _tfputs   fputs
  //#define _tfopen   fopen
  //#define _tremove  remove
  //#define _trename  rename

