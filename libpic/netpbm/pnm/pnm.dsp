# Microsoft Developer Studio Project File - Name="pnm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pnm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pnm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pnm.mak" CFG="pnm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pnm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pnm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pnm - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /Zi /O2 /I ".." /I "../pbm" /I "../pgm" /I "../ppm" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pnm - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /I "../pbm" /I "../pgm" /I "../ppm" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /GZ /c
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

# Name "pnm - Win32 Release"
# Name "pnm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Programs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fitstopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\giftopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmalias.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmarith.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmcat.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmcomp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmconvol.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmconvol.c.jef
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmcrop.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmcut.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmdepth.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmenlarge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmfile.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmflip.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmgamma.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmhisteq.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmhistmap.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnminvert.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmmerge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmnlfilt.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmnoraw.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmpad.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmpaste.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmrotate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmscale.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmshear.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmsmooth.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtile.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtoddif.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtofits.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtops.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtorast.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtosgi.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtosir.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtotiff.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnmtoxwd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rasttopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sgitopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sirtopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\tifftopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xwdtopnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zeisstopnm.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libpnm1.c
# End Source File
# Begin Source File

SOURCE=.\libpnm2.c
# End Source File
# Begin Source File

SOURCE=.\libpnm3.c
# End Source File
# Begin Source File

SOURCE=.\libpnm4.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\pnm.h
# End Source File
# Begin Source File

SOURCE=.\rast.h
# End Source File
# Begin Source File

SOURCE=.\sgi.h
# End Source File
# Begin Source File

SOURCE=.\x10wd.h
# End Source File
# Begin Source File

SOURCE=.\x11wd.h
# End Source File
# End Group
# Begin Group "Documentation Files"

# PROP Default_Filter ""
# Begin Group "Program Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\anytopnm.1
# End Source File
# Begin Source File

SOURCE=.\fitstopnm.1
# End Source File
# Begin Source File

SOURCE=.\giftopnm.1
# End Source File
# Begin Source File

SOURCE=.\pnmalias.1
# End Source File
# Begin Source File

SOURCE=.\pnmarith.1
# End Source File
# Begin Source File

SOURCE=.\pnmcat.1
# End Source File
# Begin Source File

SOURCE=.\pnmcomp.1
# End Source File
# Begin Source File

SOURCE=.\pnmconvol.1
# End Source File
# Begin Source File

SOURCE=.\pnmconvol.1.jef
# End Source File
# Begin Source File

SOURCE=.\pnmcrop.1
# End Source File
# Begin Source File

SOURCE=.\pnmcut.1
# End Source File
# Begin Source File

SOURCE=.\pnmdepth.1
# End Source File
# Begin Source File

SOURCE=.\pnmenlarge.1
# End Source File
# Begin Source File

SOURCE=.\pnmfile.1
# End Source File
# Begin Source File

SOURCE=.\pnmflip.1
# End Source File
# Begin Source File

SOURCE=.\pnmgamma.1
# End Source File
# Begin Source File

SOURCE=.\pnmhisteq.1
# End Source File
# Begin Source File

SOURCE=.\pnmhistmap.1
# End Source File
# Begin Source File

SOURCE=.\pnmindex.1
# End Source File
# Begin Source File

SOURCE=.\pnminvert.1
# End Source File
# Begin Source File

SOURCE=.\pnmmargin.1
# End Source File
# Begin Source File

SOURCE=.\pnmnlfilt.1
# End Source File
# Begin Source File

SOURCE=.\pnmnoraw.1
# End Source File
# Begin Source File

SOURCE=.\pnmpad.1
# End Source File
# Begin Source File

SOURCE=.\pnmpaste.1
# End Source File
# Begin Source File

SOURCE=.\pnmrotate.1
# End Source File
# Begin Source File

SOURCE=.\pnmscale.1
# End Source File
# Begin Source File

SOURCE=.\pnmshear.1
# End Source File
# Begin Source File

SOURCE=.\pnmsmooth.1
# End Source File
# Begin Source File

SOURCE=.\pnmtile.1
# End Source File
# Begin Source File

SOURCE=.\pnmtoddif.1
# End Source File
# Begin Source File

SOURCE=.\pnmtofits.1
# End Source File
# Begin Source File

SOURCE=.\pnmtops.1
# End Source File
# Begin Source File

SOURCE=.\pnmtorast.1
# End Source File
# Begin Source File

SOURCE=.\pnmtosgi.1
# End Source File
# Begin Source File

SOURCE=.\pnmtosir.1
# End Source File
# Begin Source File

SOURCE=.\pnmtotiff.1
# End Source File
# Begin Source File

SOURCE=.\pnmtoxwd.1
# End Source File
# Begin Source File

SOURCE=.\pstopnm.1
# End Source File
# Begin Source File

SOURCE=.\rasttopnm.1
# End Source File
# Begin Source File

SOURCE=.\sgitopnm.1
# End Source File
# Begin Source File

SOURCE=.\sirtopnm.1
# End Source File
# Begin Source File

SOURCE=.\tifftopnm.1
# End Source File
# Begin Source File

SOURCE=.\xwdtopnm.1
# End Source File
# Begin Source File

SOURCE=.\zeisstopnm.1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libpnm.3
# End Source File
# Begin Source File

SOURCE=.\pnm.5
# End Source File
# Begin Source File

SOURCE=.\pnmconvol.README
# End Source File
# Begin Source File

SOURCE=.\pnmsmooth.README
# End Source File
# End Group
# End Target
# End Project
