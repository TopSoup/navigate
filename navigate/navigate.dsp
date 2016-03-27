# Microsoft Developer Studio Project File - Name="navigate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=navigate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "navigate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "navigate.mak" CFG="navigate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "navigate - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "navigate - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "navigate"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "navigate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MEDIAPLAYER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../inc" /D "AEE_SIMULATOR" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MEDIAPLAYER_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"./navigate.dll"

!ELSEIF  "$(CFG)" == "navigate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MEDIAPLAYER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(BREWDIR)\inc" /I "$(BREWDIR)\sdk\inc" /I "." /I ".\location" /I ".\db" /I ".\amountctl" /D "AEE_SIMULATOR" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"./navigate.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "navigate - Win32 Release"
# Name "navigate - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AEEAppGen.c
# End Source File
# Begin Source File

SOURCE=.\AEEModGen.c
# End Source File
# Begin Source File

SOURCE=.\destinfowindow.c
# End Source File
# Begin Source File

SOURCE=.\destlistwindow.c
# End Source File
# Begin Source File

SOURCE=.\logicmacro.c
# End Source File
# Begin Source File

SOURCE=.\mainwindow.c
# End Source File
# Begin Source File

SOURCE=.\navigate.c
# End Source File
# Begin Source File

SOURCE=.\navigatedestwindow.c
# End Source File
# Begin Source File

SOURCE=.\navigatewindow.c
# End Source File
# Begin Source File

SOURCE=.\newdestwindow.c
# End Source File
# Begin Source File

SOURCE=.\soswindow.c
# End Source File
# Begin Source File

SOURCE=.\wherewindow.c
# End Source File
# Begin Source File

SOURCE=.\wincontrol.c
# End Source File
# Begin Source File

SOURCE=.\window.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\inc\AEEAppGen.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\AEEMediaUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\AEEModGen.h
# End Source File
# Begin Source File

SOURCE=.\destinfowindow.h
# End Source File
# Begin Source File

SOURCE=.\destlistwindow.h
# End Source File
# Begin Source File

SOURCE=.\logicmacro.h
# End Source File
# Begin Source File

SOURCE=.\mainwindow.h
# End Source File
# Begin Source File

SOURCE=.\mediaplayer_res.h
# End Source File
# Begin Source File

SOURCE=.\navigate.bid
# End Source File
# Begin Source File

SOURCE=.\navigate.h
# End Source File
# Begin Source File

SOURCE=.\navigatedestwindow.h
# End Source File
# Begin Source File

SOURCE=.\navigatepch.h
# End Source File
# Begin Source File

SOURCE=.\navigatewindow.h
# End Source File
# Begin Source File

SOURCE=.\newdestwindow.h
# End Source File
# Begin Source File

SOURCE=.\soswindow.h
# End Source File
# Begin Source File

SOURCE=.\wherewindow.h
# End Source File
# Begin Source File

SOURCE=.\wincontrol.h
# End Source File
# Begin Source File

SOURCE=.\window.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\backimg.brx

!IF  "$(CFG)" == "navigate - Win32 Release"

!ELSEIF  "$(CFG)" == "navigate - Win32 Debug"

# Begin Custom Build
InputPath=.\backimg.brx

BuildCmds= \
	"$(BREWDIR)\tools\ResourceEditor\brewrc.exe" -s -o ./backimg.bar -h ./backimg.brh $(InputPath)

"backimg.bar" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"backimg.brh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\kitimg.brx

!IF  "$(CFG)" == "navigate - Win32 Release"

!ELSEIF  "$(CFG)" == "navigate - Win32 Debug"

# Begin Custom Build
InputPath=.\kitimg.brx

BuildCmds= \
	"$(BREWDIR)\tools\ResourceEditor\brewrc.exe" -s -o ./kitimg.bar -h ./kitimg.brh $(InputPath)

"kitimg.bar" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"kitimg.brh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\navigate.brx

!IF  "$(CFG)" == "navigate - Win32 Release"

!ELSEIF  "$(CFG)" == "navigate - Win32 Debug"

# Begin Custom Build
InputPath=.\navigate.brx

BuildCmds= \
	"$(BREWDIR)\tools\ResourceEditor\brewrc.exe" -s -o ./navigate.bar -h ./navigate.brh $(InputPath)

"navigate.bar" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"navigate.brh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "location"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\location\e_asin.c
# End Source File
# Begin Source File

SOURCE=.\location\location.c
# End Source File
# Begin Source File

SOURCE=.\location\location.h
# End Source File
# Begin Source File

SOURCE=.\location\transform.c
# End Source File
# Begin Source File

SOURCE=.\location\transform.h
# End Source File
# End Group
# Begin Group "db"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\db\dbfunc.c
# End Source File
# Begin Source File

SOURCE=.\db\dbfunc.h
# End Source File
# End Group
# Begin Group "amountctl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\amountctl\amountctl.c
# End Source File
# Begin Source File

SOURCE=.\amountctl\amountctl.h
# End Source File
# End Group
# End Target
# End Project
