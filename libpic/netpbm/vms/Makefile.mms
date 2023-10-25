# MMS Description file for pbmplus tools.
#
# Copyright (C) 1989 by Jef Poskanzer.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.  This software is provided "as is" without express or
# implied warranty.
#
# This is a MMS Description file for the DEC VMS MMS utility.
# Written by Rick Dyson (dyson@iowasp.physics.uiowa.edu) 10-NOV-1991
# originally based on one by Terry Poot (tp@mccall.com)
#
# Last Updated:  2-SEP-1993 by Rick Dyson
#               16-SEP-1993 by Rick Dyson for the netpbm Alpha03 release
#               27-Sep-1993 by Rick Dyson for merged VMS version of netpbm
#               30-NOV-1993 by Rick Dyson for NetPBM beta 29nov93

# CONFIGURE: Define the directory that you want the binaries copied to.
INSTALLBINARIES =	PBMplus_Root:[Exe]

# CONFIGURE: Define the directories that you want the manual sources copied to.
INSTALLMANUALS =	PBMplus_Root:[TeX]

WSO =	Write Sys$Output

PROTLIST = *.,README.*,SETUP.COM,ADD_LIST.COM,OTHER.SYSTEMS,PBMPLUS.HLB,PBMplus_Root:[.Exe]*.*

.first
	@ PBMPLUS_PATH = F$Element (0, "]", F$Environment ("DEFAULT")) + ".]"
	@ Define /NoLog /Translation_Attributes = Concealed PBMplus_Root "''PBMPLUS_PATH'"
	@ Define /NoLog PBMplus_Dir PBMplus_Root:[000000]
        @ Define /NoLog PBMPlusShr PBMPlus_Dir:PBMPlusShr

.last
	@ Set Default PBMplus_Dir
#	@- Set Protection = (System:RWE, Owner:RWE, Group:RE, World:RE) $(PROTLIST)

DEFAULT	:
	@ $(WSO) "You must specify which target to make. Valid targets are:"
	@ $(WSO) " "
	@ $(WSO) "Programs:"
	@ $(WSO) "      ALL             - make all PBMplus executables"
	@ $(WSO) "      PBM             - make just PBM executables"
	@ $(WSO) "      PGM             - make just PGM executables"
	@ $(WSO) "      PPM             - make just PPM executables"
	@ $(WSO) "      PNM             - make just PNM executables"
	@ $(WSO) "      LIBTIFF         - make just TIFF library"
	@ $(WSO) "      LIBSHR          - make just the shareable library"
	@ $(WSO) "      INSTALL         - move executables to $(INSTALLBINARIES)"
	@ $(WSO) " "
	@ $(WSO) "TeX Documentation: "
	@ $(WSO) "      MANUAL          - make all TeX files of troff man pages"
	@ $(WSO) "      INSTALLMANUAL   - move TeX files to $(INSTALLMANUALS)"
	@ $(WSO) "      HELP            - make the VMS HELP library"
	@ $(WSO) " "
	@ $(WSO) "Maintanence: "
	@ $(WSO) "      BUILD_PROC      - use MMS to create a VMS command procedure"
	@ $(WSO) "                        to build most of the PBMplus package without MMS"
	@ $(WSO) "      CLEAN           - purge all files and delete all object files"

NULL :
	@ Continue

ALL :   libshr pbm pgm ppm libtiff pnm help
	! All Finished with build of PBMplus!!!

LIBSHR :	libs
	@PBMplus_Dir:MAKE_PBMPLUSSHR.COM

LIBS :	COMPILE.H
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default PBMplus_Dir

COMPILE.H :	STAMP-DATE.COM
	@ @PBMplus_Dir:STAMP-DATE.COM
	@ Purge /NoLog /NoConfirm COMPILE.H

PBM :
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS all
	Set Default PBMplus_Dir

PGM :
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS all
	Set Default PBMplus_Dir

PPM :
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS all
	Set Default PBMplus_Dir

PNM :
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS all
	Set Default PBMplus_Dir

LIBTIFF :
	Set Default PBMplus_Root:[libtiff]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS all
	Set Default PBMplus_Dir

MANUAL :
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS manual
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS manual
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS manual
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS manual
	Set Default PBMplus_Dir

INSTALL :       ALL
	@ If F$Parse ("PBMplus_Root:[Exe]") .eqs. "" -
		Then Create /Directory PBMplus_Root:[Exe]
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS install
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS install
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS install
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS install
	Set Default PBMplus_Dir

INSTALLMANUAL :
	@ If F$Parse ("PBMplus_Root:[TeX]") .eqs. "" -
		Then Create /Directory PBMplus_Root:[TeX]
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS installmanual
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS installmanual
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS installmanual
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS installmanual
	Set Default PBMplus_Root:[TeX]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS all
	Set Default PBMplus_Dir

HELP :
	Library /Create /Help PBMPLUS.HLB PBMPLUS.HLP
	Set File /Truncate PBMPLUS.HLB

BUILD_PROC :
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:pbmlibbuild.com lib
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:pgmlibbuild.com lib
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:ppmlibbuild.com lib
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:pnmlibbuild.com lib
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:pbmallbuild.com all
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:pgmallbuild.com all
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:ppmallbuild.com all
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:pnmallbuild.com all
	Set Default PBMplus_Root:[libtiff]
	$(MMS) $(MMSQUALIFIERS)/NoAction /From_Sources /Output = PBMplus_Dir:libtiffbuild.com lib
	Set Default PBMplus_Dir
	Copy NL: MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Write Sys$Output ""Edit Me first!!!"""
	Write BP "$ Write Sys$Output ""Check out TeX stuff!!!"""
	Write BP "$ Write Sys$Output ""            Do YOU have these????"""
	Write BP "$ Exit"
	Write BP "$!"
	Write BP "$ If F$Mode () .eqs. ""INTERACTIVE"""
	Write BP "$    Then"
	Write BP "$        VERIFY = F$Verify (0)"
	Write BP "$    Else"
	Write BP "$        VERIFY = F$Verify (1)"
	Write BP "$ EndIf"
	Write BP "$ THIS_PATH = F$Element (0, ""]"", F$Environment (""PROCEDURE"")) + ""]""
	Write BP "$ Set Default 'THIS_PATH'"
	Write BP "$!"
	Write BP "$ PBMPLUS_PATH = F$Element (0, ""]"", F$Environment (""DEFAULT"")) + "".]""
	Write BP "$ Define /NoLog /Translation_Attributes = Concealed PBMplus_Root ""''PBMPLUS_PATH'""
	Write BP "$ Define /NoLog PBMplus_Dir PBMplus_Root:[000000]"
	Write BP "$ Define /NoLog PBMplusShr PBMplus_Dir:PBMplusShr"
	Write BP "$!"
	Write BP "$!            Make the Shareable Library"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Root:[pbm]"
	Close BP
	- Append PBMLIBBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[pgm]"
	Close BP
	- Append PGMLIBBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[ppm]"
	Close BP
	- Append PPMLIBBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[pnm]"
	Close BP
	- Append PNMLIBBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Dir"
	Write BP "$ @ PBMplus_Dir:MAKE_PBMplusShr.COM
	Write BP "$!"
	Write BP "$!		PBM"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Root:[pbm]"
	Close BP
	- Append PBMALLBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!		PGM"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Root:[pgm]"
	Close BP
	- Append PGMALLBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!		PPM"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Root:[ppm]"
	Close BP
	- Append PPMALLBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!            LIBTIFF"
	Write BP "$!" 
	Write BP "$ Set Default PBMplus_Root:[libtiff]"
	Close BP
	- Append LIBTIFFBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!		PNM"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Root:[pnm]"
	Close BP
	- Append PNMALLBUILD.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!		Install the binaries in a separate directory"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Dir"
	Write BP "$ If F$Parse (""PBMplus_Root:[Exe]"") .eqs. """" Then Create /Directory PBMplus_Root:[Exe]"
	Close BP
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pbminstall.com install
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pgminstall.com install
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:ppminstall.com install
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pnminstall.com install
	Set Default PBMplus_Dir
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PBM]"
	Close BP
	- Append PBMINSTALL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PGM]"
	Close BP
	- Append PGMINSTALL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PPM]"
	Close BP
	- Append PPMINSTALL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PNM]"
	Close BP
	- Append PNMINSTALL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!		Build the VMS Help Library"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Dir"
	Write BP "$ Library /Create /Help PBMPLUS.HLB PBMPLUS.HLP"
	Write BP "$ Set File /Truncate PBMPLUS.HLB"
	Close BP
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pbmmanual.com manual
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pgmmanual.com manual
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:ppmmanual.com manual
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pnmmanual.com manual
	Set Default PBMplus_Dir
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$!"
	Write BP "$!	Translation of troff man pages to LaTeX files using tr2TeX from  "
	Write BP "$!	the DECUS tapes.  It's also available from many TeX ftp sites.   "
	Write BP "$!			VERY SYSTEM DEPENDENT!!!!!!!"
	Write BP "$!"
	Write BP "$ Set Default PBMplus_Root:[pbm]"
	Close BP
	- Append PBMMANUAL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PGM]"
	Close BP
	- Append PGMMANUAL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PPM]"
	Close BP
	- Append PPMMANUAL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[PNM]"
	Close BP
	- Append PNMMANUAL.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Dir"
	Write BP "$!"
	Write BP "$!	Make a TeX doc area and put all TeX files there."
	Write BP "$!"
	Write BP "$ If F$Parse (""PBMplus_Root:[TeX]"") .eqs. """" Then Create /Directory PBMplus_Root:[TeX]"
	Close BP
	Set Default PBMplus_Root:[pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pbminstman.com installmanual
	Set Default PBMplus_Root:[pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pgminstman.com installmanual
	Set Default PBMplus_Root:[ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:ppminstman.com installmanual
	Set Default PBMplus_Root:[pnm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:pnminstman.com installmanual
	Set Default PBMplus_Root:[TeX]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS /NoAction /From_Sources /Output = PBMplus_Dir:tex.com all
	Set Default PBMplus_Dir
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[pbm]"
	Close BP
	- Append PBMINSTMAN.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[pgm]"
	Close BP
	- Append PGMINSTMAN.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[ppm]"
	Close BP
	- Append PPMINSTMAN.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[pnm]"
	Close BP
	- Append PNMINSTMAN.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Root:[TeX]"
	Close BP
	- Append TEX.COM MAKE_PBMPLUS.COM
	Open /Append BP MAKE_PBMPLUS.COM
	Write BP "$ Set Default PBMplus_Dir
	Write BP "$ Exit"
	Close BP
	Delete /NoConfirm PBMLIBBUILD.COM;*
	Delete /NoConfirm PGMLIBBUILD.COM;*
	Delete /NoConfirm PPMLIBBUILD.COM;*
	Delete /NoConfirm PNMLIBBUILD.COM;*
	Delete /NoConfirm PBMALLBUILD.COM;*
	Delete /NoConfirm PGMALLBUILD.COM;*
	Delete /NoConfirm PPMALLBUILD.COM;*
	Delete /NoConfirm PNMALLBUILD.COM;*
	Delete /NoConfirm LIBTIFFBUILD.COM;*
	Delete /NoConfirm PBMINSTALL.COM;*
	Delete /NoConfirm PGMINSTALL.COM;*
	Delete /NoConfirm PPMINSTALL.COM;*
	Delete /NoConfirm PNMINSTALL.COM;*
	Delete /NoConfirm PBMMANUAL.COM;*
	Delete /NoConfirm PGMMANUAL.COM;*
	Delete /NoConfirm PPMMANUAL.COM;*
	Delete /NoConfirm PNMMANUAL.COM;*
	Delete /NoConfirm PBMINSTMAN.COM;*
	Delete /NoConfirm PGMINSTMAN.COM;*
	Delete /NoConfirm PPMINSTMAN.COM;*
	Delete /NoConfirm PNMINSTMAN.COM;*
	Delete /NoConfirm TEX.COM;*

CLEAN :
	Set Default PBMplus_Root:[pbm]
	- $(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS clean
	Set Default PBMplus_Root:[pgm]
	- $(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS clean
	Set Default PBMplus_Root:[ppm]
	- $(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS clean
	Set Default PBMplus_Root:[pnm]
	- $(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS clean
	Set Default PBMplus_Root:[libtiff]
	- $(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS clean
	- Set Default PBMplus_Root:[TeX]
	- $(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS clean
	- Set Default PBMplus_Root:[exe]
	- Set Protection = Owner:RWED *.*;-1
	- Purge /NoLog /NoConfirm *.*
	Set Default PBMplus_Dir
	- Set File /Truncate PBMPLUS.HLB
	- Set Protection = Owner:RWED *.*;-1,*.obj;
	- Purge /NoLog /NoConfirm *.*
	- Delete /NoLog /NoConfirm *.obj;,*.map;
