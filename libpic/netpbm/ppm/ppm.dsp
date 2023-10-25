# Microsoft Developer Studio Project File - Name="ppm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ppm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ppm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ppm.mak" CFG="ppm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ppm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ppm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ppm - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /Zi /O2 /I ".." /I "../pbm" /I "../pgm" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ppm - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /I "../pbm" /I "../pgm" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSDOS" /D "__STDC__" /YX /FD /GZ /c
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

# Name "ppm - Win32 Release"
# Name "ppm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Programs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bitio.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\bmptoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\gouldtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\hpcdtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ilbmtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\imgtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mtvtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pcxtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pgmtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pi1toppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\picttoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pjtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppm3d.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmbrighten.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmchange.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmdim.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmdist.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmdither.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmflash.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmforge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmhist.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmmake.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmmerge.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmmix.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmnorm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmntsc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmpat.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmquant.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmqvga.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmrelief.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmshift.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmspread.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtoacad.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtobmp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtogif.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtoicr.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtoilbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtomap.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtomitsu.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopcx.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopgm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopi1.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopict.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopj.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopjxl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtopuzz.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtorgb3.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtosixel.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtotga.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtouil.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtoxpm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtoyuv.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ppmtoyuvsplit.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\qrttoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rawtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rgb3toppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sldtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\spctoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sputoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\tgatoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ximtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xpmtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xvminitoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\yuvsplittoppm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\yuvtoppm.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libppm1.c
# End Source File
# Begin Source File

SOURCE=.\libppm2.c
# End Source File
# Begin Source File

SOURCE=.\libppm3.c
# End Source File
# Begin Source File

SOURCE=.\libppm4.c
# End Source File
# Begin Source File

SOURCE=.\libppm5.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\autocad.h
# End Source File
# Begin Source File

SOURCE=.\bitio.h
# End Source File
# Begin Source File

SOURCE=.\bmp.h
# End Source File
# Begin Source File

SOURCE=.\ilbm.h
# End Source File
# Begin Source File

SOURCE=.\libppm.h
# End Source File
# Begin Source File

SOURCE=.\lum.h
# End Source File
# Begin Source File

SOURCE=.\mitsu.h
# End Source File
# Begin Source File

SOURCE=.\ppm.h
# End Source File
# Begin Source File

SOURCE=.\ppmcmap.h
# End Source File
# Begin Source File

SOURCE=.\ppmdraw.h
# End Source File
# Begin Source File

SOURCE=.\tga.h
# End Source File
# Begin Source File

SOURCE=.\xim.h
# End Source File
# End Group
# Begin Group "Documentation Files"

# PROP Default_Filter ""
# Begin Group "Program Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bmptoppm.1
# End Source File
# Begin Source File

SOURCE=.\gouldtoppm.1
# End Source File
# Begin Source File

SOURCE=.\hpcdtoppm.1
# End Source File
# Begin Source File

SOURCE=.\ilbmtoppm.1
# End Source File
# Begin Source File

SOURCE=.\imgtoppm.1
# End Source File
# Begin Source File

SOURCE=.\mtvtoppm.1
# End Source File
# Begin Source File

SOURCE=.\pcxtoppm.1
# End Source File
# Begin Source File

SOURCE=.\pgmtoppm.1
# End Source File
# Begin Source File

SOURCE=.\pi1toppm.1
# End Source File
# Begin Source File

SOURCE=.\picttoppm.1
# End Source File
# Begin Source File

SOURCE=.\pjtoppm.1
# End Source File
# Begin Source File

SOURCE=.\ppm3d.1
# End Source File
# Begin Source File

SOURCE=.\ppmbrighten.1
# End Source File
# Begin Source File

SOURCE=.\ppmchange.1
# End Source File
# Begin Source File

SOURCE=.\ppmdim.1
# End Source File
# Begin Source File

SOURCE=.\ppmdist.1
# End Source File
# Begin Source File

SOURCE=.\ppmdither.1
# End Source File
# Begin Source File

SOURCE=.\ppmflash.1
# End Source File
# Begin Source File

SOURCE=.\ppmforge.1
# End Source File
# Begin Source File

SOURCE=.\ppmhist.1
# End Source File
# Begin Source File

SOURCE=.\ppmmake.1
# End Source File
# Begin Source File

SOURCE=.\ppmmix.1
# End Source File
# Begin Source File

SOURCE=.\ppmnorm.1
# End Source File
# Begin Source File

SOURCE=.\ppmntsc.1
# End Source File
# Begin Source File

SOURCE=.\ppmpat.1
# End Source File
# Begin Source File

SOURCE=.\ppmquant.1
# End Source File
# Begin Source File

SOURCE=.\ppmquantall.1
# End Source File
# Begin Source File

SOURCE=.\ppmqvga.1
# End Source File
# Begin Source File

SOURCE=.\ppmrelief.1
# End Source File
# Begin Source File

SOURCE=.\ppmshift.1
# End Source File
# Begin Source File

SOURCE=.\ppmspread.1
# End Source File
# Begin Source File

SOURCE=.\ppmtoacad.1
# End Source File
# Begin Source File

SOURCE=.\ppmtobmp.1
# End Source File
# Begin Source File

SOURCE=.\ppmtogif.1
# End Source File
# Begin Source File

SOURCE=.\ppmtoicr.1
# End Source File
# Begin Source File

SOURCE=.\ppmtoilbm.1
# End Source File
# Begin Source File

SOURCE=.\ppmtomap.1
# End Source File
# Begin Source File

SOURCE=.\ppmtomitsu.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopcx.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopgm.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopi1.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopict.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopj.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopjxl.1
# End Source File
# Begin Source File

SOURCE=.\ppmtopuzz.1
# End Source File
# Begin Source File

SOURCE=.\ppmtorgb3.1
# End Source File
# Begin Source File

SOURCE=.\ppmtosixel.1
# End Source File
# Begin Source File

SOURCE=.\ppmtotga.1
# End Source File
# Begin Source File

SOURCE=.\ppmtouil.1
# End Source File
# Begin Source File

SOURCE=.\ppmtoxpm.1
# End Source File
# Begin Source File

SOURCE=.\ppmtoyuv.1
# End Source File
# Begin Source File

SOURCE=.\ppmtoyuvsplit.1
# End Source File
# Begin Source File

SOURCE=.\qrttoppm.1
# End Source File
# Begin Source File

SOURCE=.\rawtoppm.1
# End Source File
# Begin Source File

SOURCE=.\rgb3toppm.1
# End Source File
# Begin Source File

SOURCE=.\sldtoppm.1
# End Source File
# Begin Source File

SOURCE=.\spctoppm.1
# End Source File
# Begin Source File

SOURCE=.\sputoppm.1
# End Source File
# Begin Source File

SOURCE=.\tgatoppm.1
# End Source File
# Begin Source File

SOURCE=.\ximtoppm.1
# End Source File
# Begin Source File

SOURCE=.\xpmtoppm.1
# End Source File
# Begin Source File

SOURCE=.\xvminitoppm.1
# End Source File
# Begin Source File

SOURCE=.\yuvsplittoppm.1
# End Source File
# Begin Source File

SOURCE=.\yuvtoppm.1
# End Source File
# End Group
# Begin Source File

SOURCE=.\libppm.3
# End Source File
# Begin Source File

SOURCE=.\ppm.5
# End Source File
# Begin Source File

SOURCE=.\xpmtoppm.README
# End Source File
# End Group
# End Target
# End Project
