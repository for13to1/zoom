# Microsoft Developer Studio Project File - Name="pgm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pgm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pgm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pgm.mak" CFG="pgm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pgm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pgm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pgm - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /Zi /O2 /I ".." /I "../pbm" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pgm - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /I "../pbm" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /GZ /c
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

# Name "pgm - Win32 Release"
# Name "pgm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Programs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\asciitopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\bioradtopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\fstopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\hipstopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\lispmtopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmbentley.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmcrater.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmedge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmenhance.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmhist.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmkernel.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmmerge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmnoise.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmnorm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmoil.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmramp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmtexture.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmtofs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmtolispm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\psidtopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rawtopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\spottopgm.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libpgm1.c
# End Source File
# Begin Source File

SOURCE=.\libpgm2.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dithers.h
# End Source File
# Begin Source File

SOURCE=.\libpgm.h
# End Source File
# Begin Source File

SOURCE=.\pgm.h
# End Source File
# End Group
# Begin Group "Documentation Files"

# PROP Default_Filter ""
# Begin Group "Program Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\asciitopgm.1
# End Source File
# Begin Source File

SOURCE=.\bioradtopgm.1
# End Source File
# Begin Source File

SOURCE=.\fstopgm.1
# End Source File
# Begin Source File

SOURCE=.\hipstopgm.1
# End Source File
# Begin Source File

SOURCE=.\lispmtopgm.1
# End Source File
# Begin Source File

SOURCE=.\pbmtopgm.1
# End Source File
# Begin Source File

SOURCE=.\pgmbentley.1
# End Source File
# Begin Source File

SOURCE=.\pgmcrater.1
# End Source File
# Begin Source File

SOURCE=.\pgmedge.1
# End Source File
# Begin Source File

SOURCE=.\pgmenhance.1
# End Source File
# Begin Source File

SOURCE=.\pgmhist.1
# End Source File
# Begin Source File

SOURCE=.\pgmkernel.1
# End Source File
# Begin Source File

SOURCE=.\pgmnoise.1
# End Source File
# Begin Source File

SOURCE=.\pgmnorm.1
# End Source File
# Begin Source File

SOURCE=.\pgmoil.1
# End Source File
# Begin Source File

SOURCE=.\pgmramp.1
# End Source File
# Begin Source File

SOURCE=.\pgmtexture.1
# End Source File
# Begin Source File

SOURCE=.\pgmtofs.1
# End Source File
# Begin Source File

SOURCE=.\pgmtolispm.1
# End Source File
# Begin Source File

SOURCE=.\pgmtopbm.1
# End Source File
# Begin Source File

SOURCE=.\psidtopgm.1
# End Source File
# Begin Source File

SOURCE=.\rawtopgm.1
# End Source File
# Begin Source File

SOURCE=.\spottopgm.1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libpgm.3
# End Source File
# Begin Source File

SOURCE=.\pgm.5
# End Source File
# End Group
# End Target
# End Project
