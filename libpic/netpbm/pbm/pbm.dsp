# Microsoft Developer Studio Project File - Name="pbm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pbm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pbm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pbm.mak" CFG="pbm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pbm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pbm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pbm - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /Zi /O2 /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pbm - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /GZ /c
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

# Name "pbm - Win32 Release"
# Name "pbm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Programs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\atktopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\brushtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\cmuwmtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\g3topbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\gemtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\icontopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\macptopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mgrtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmclean.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmlife.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmmake.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmmask.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmmerge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmpscale.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmreduce.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtext.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmto10x.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmto4425.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoascii.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoatk.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtobbnbg.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtocmuwm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoepsi.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoepson.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtog3.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtogem.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtogo.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoicon.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtolj.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoln03.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtolps.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtomacp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtomgr.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtopi3.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtopk.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoplot.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoptx.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtox10bm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoxbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtoybm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmtozinc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pbmupc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pi3topbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pktopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xbmtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ybmtopbm.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libpbm1.c
# End Source File
# Begin Source File

SOURCE=.\libpbm2.c
# End Source File
# Begin Source File

SOURCE=.\libpbm3.c
# End Source File
# Begin Source File

SOURCE=.\libpbm4.c
# End Source File
# Begin Source File

SOURCE=.\libpbm5.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bitreverse.h
# End Source File
# Begin Source File

SOURCE=.\cmuwm.h
# End Source File
# Begin Source File

SOURCE=.\g3.h
# End Source File
# Begin Source File

SOURCE=.\includes.h
# End Source File
# Begin Source File

SOURCE=.\libpbm.h
# End Source File
# Begin Source File

SOURCE=.\macp.h
# End Source File
# Begin Source File

SOURCE=.\mgr.h
# End Source File
# Begin Source File

SOURCE=.\pbm.h
# End Source File
# Begin Source File

SOURCE=.\pbmfont.h
# End Source File
# End Group
# Begin Group "Documentation Files"

# PROP Default_Filter ""
# Begin Group "Program Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\atktopbm.1
# End Source File
# Begin Source File

SOURCE=.\brushtopbm.1
# End Source File
# Begin Source File

SOURCE=.\cmuwmtopbm.1
# End Source File
# Begin Source File

SOURCE=.\g3topbm.1
# End Source File
# Begin Source File

SOURCE=.\gemtopbm.1
# End Source File
# Begin Source File

SOURCE=.\icontopbm.1
# End Source File
# Begin Source File

SOURCE=.\macptopbm.1
# End Source File
# Begin Source File

SOURCE=.\mgrtopbm.1
# End Source File
# Begin Source File

SOURCE=.\pbmclean.1
# End Source File
# Begin Source File

SOURCE=.\pbmfilters.1
# End Source File
# Begin Source File

SOURCE=.\pbmlife.1
# End Source File
# Begin Source File

SOURCE=.\pbmmake.1
# End Source File
# Begin Source File

SOURCE=.\pbmmask.1
# End Source File
# Begin Source File

SOURCE=.\pbmpscale.1
# End Source File
# Begin Source File

SOURCE=.\pbmreduce.1
# End Source File
# Begin Source File

SOURCE=.\pbmtext.1
# End Source File
# Begin Source File

SOURCE=.\pbmto10x.1
# End Source File
# Begin Source File

SOURCE=.\pbmto4425.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoascii.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoatk.1
# End Source File
# Begin Source File

SOURCE=.\pbmtobbnbg.1
# End Source File
# Begin Source File

SOURCE=.\pbmtocmuwm.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoepsi.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoepson.1
# End Source File
# Begin Source File

SOURCE=.\pbmtog3.1
# End Source File
# Begin Source File

SOURCE=.\pbmtogem.1
# End Source File
# Begin Source File

SOURCE=.\pbmtogo.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoicon.1
# End Source File
# Begin Source File

SOURCE=.\pbmtolj.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoln03.1
# End Source File
# Begin Source File

SOURCE=.\pbmtolps.1
# End Source File
# Begin Source File

SOURCE=.\pbmtomacp.1
# End Source File
# Begin Source File

SOURCE=.\pbmtomgr.1
# End Source File
# Begin Source File

SOURCE=.\pbmtopi3.1
# End Source File
# Begin Source File

SOURCE=.\pbmtopk.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoplot.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoptx.1
# End Source File
# Begin Source File

SOURCE=.\pbmtox10bm.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoxbm.1
# End Source File
# Begin Source File

SOURCE=.\pbmtoybm.1
# End Source File
# Begin Source File

SOURCE=.\pbmtozinc.1
# End Source File
# Begin Source File

SOURCE=.\pbmupc.1
# End Source File
# Begin Source File

SOURCE=.\pi3topbm.1
# End Source File
# Begin Source File

SOURCE=.\pktopbm.1
# End Source File
# Begin Source File

SOURCE=.\xbmtopbm.1
# End Source File
# Begin Source File

SOURCE=.\ybmtopbm.1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libpbm.3
# End Source File
# Begin Source File

SOURCE=.\pbm.5
# End Source File
# End Group
# End Target
# End Project
