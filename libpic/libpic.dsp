# Microsoft Developer Studio Project File - Name="libpic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libpic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libpic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libpic.mak" CFG="libpic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libpic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libpic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libpic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "../libsys" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libpic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../libsys" /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libpic - Win32 Release"
# Name "libpic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bmp.c
# End Source File
# Begin Source File

SOURCE=.\dump.c
# End Source File
# Begin Source File

SOURCE=.\gif.c
# End Source File
# Begin Source File

SOURCE=.\iris.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\iris_pic.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\jpg.c
# ADD CPP /I "jpeg-6b"
# End Source File
# Begin Source File

SOURCE=.\mallocNd.c
# End Source File
# Begin Source File

SOURCE=.\pic.c
# End Source File
# Begin Source File

SOURCE=.\pic_all.c
# End Source File
# Begin Source File

SOURCE=.\pic_file.c
# End Source File
# Begin Source File

SOURCE=.\png.c
# ADD CPP /I "libpng" /I "zlib"
# End Source File
# Begin Source File

SOURCE=.\pnm.c
# ADD CPP /I "netpbm" /I "netpbm/pbm" /I "netpbm/pgm" /I "netpbm/ppm" /I "netpbm/pnm" /D "MSDOS"
# End Source File
# Begin Source File

SOURCE=.\readGIF.c
# End Source File
# Begin Source File

SOURCE=.\rle.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rle_pic.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rle_random.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\swap.c
# End Source File
# Begin Source File

SOURCE=.\tif.c
# ADD CPP /I "tiff-v3.5.5/libtiff"
# End Source File
# Begin Source File

SOURCE=.\window.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dump.h
# End Source File
# Begin Source File

SOURCE=.\gif.h
# End Source File
# Begin Source File

SOURCE=.\iris.h
# End Source File
# Begin Source File

SOURCE=.\pic.h
# End Source File
# Begin Source File

SOURCE=.\picrle.h
# End Source File
# Begin Source File

SOURCE=.\picrle_random.h
# End Source File
# Begin Source File

SOURCE=.\pixel.h
# End Source File
# Begin Source File

SOURCE=.\swap.h
# End Source File
# Begin Source File

SOURCE=.\window.h
# End Source File
# End Group
# Begin Group "Documentation Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pic.3
# End Source File
# End Group
# End Target
# End Project
