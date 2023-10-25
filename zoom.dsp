# Microsoft Developer Studio Project File - Name="zoom" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=zoom - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zoom.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zoom.mak" CFG="zoom - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zoom - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zoom - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zoom - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "libsys" /I "libpic" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpic.lib libpng.lib libtiff.lib libjpeg.lib pnm.lib ppm.lib pgm.lib pbm.lib /nologo /subsystem:console /machine:I386 /libpath:"libpic/Release" /libpath:"libpic/libpng/msvc/libpng___Win32_Release" /libpath:"libpic/libpng/msvc/zlib___Win32_Release" /libpath:"libpic/jpeg-6b/Release" /libpath:"libpic/tiff-v3.5.5/libtiff/Release" /libpath:"libpic/netpbm/pnm/Release" /libpath:"libpic/netpbm/ppm/Release" /libpath:"libpic/netpbm/pgm/Release" /libpath:"libpic/netpbm/pbm/Release"

!ELSEIF  "$(CFG)" == "zoom - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "libsys" /I "libpic" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpic.lib libpng.lib zlib.lib libtiff.lib libjpeg.lib pnm.lib ppm.lib pgm.lib pbm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"libpic/Debug" /libpath:"libpic/libpng/msvc/libpng___Win32_Debug" /libpath:"libpic/libpng/msvc/zlib___Win32_Debug" /libpath:"libpic/jpeg-6b/Debug" /libpath:"libpic/tiff-v3.5.5/libtiff/Debug" /libpath:"libpic/netpbm/pnm/Debug" /libpath:"libpic/netpbm/ppm/Debug" /libpath:"libpic/netpbm/pgm/Debug" /libpath:"libpic/netpbm/pbm/Debug"

!ENDIF 

# Begin Target

# Name "zoom - Win32 Release"
# Name "zoom - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\zoom\filt.c
# End Source File
# Begin Source File

SOURCE=.\zoom\scanline.c
# End Source File
# Begin Source File

SOURCE=.\zoom\zoom.c
# End Source File
# Begin Source File

SOURCE=.\zoom\zoom_main.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\zoom\filt.h
# End Source File
# Begin Source File

SOURCE=.\zoom\scanline.h
# End Source File
# Begin Source File

SOURCE=.\libsys\simple.h
# End Source File
# Begin Source File

SOURCE=.\zoom\zoom.h
# End Source File
# End Group
# Begin Group "Documentation Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\README
# End Source File
# Begin Source File

SOURCE=.\zoom\zoom.1
# End Source File
# Begin Source File

SOURCE=.\ZOOM.TXT
# End Source File
# End Group
# End Target
# End Project
