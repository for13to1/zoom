# MMS Description file for pnm tools.
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
# Written by Rick Dyson (dyson@iowasp.physics.uiowa.edu) 10-NOV-1991
# originally based on one by Terry Poot (tp@mccall.com)
#

# Default values
INSTALLBINARIES =	PBMplus_Root:[Exe]
INSTALLMANUALS  =	PBMplus_Root:[TeX]

# CONFIGURE: You can compile PNM without PPM.  If you don't want PPM,
# comment out the next four lines.  This will make the PNM programs use
# less memory.
PPMDIR =	[-.ppm]
PPMLIB =	$(PPMDIR)libppm.olb
PPMDEF =	$(PPMDIR)ppm.h [-]pbmplus.h
PPMLIBDEF =	$(PPMDIR)libppm.h

# CONFIGURE: Likewise here: if you don't have PGM, comment these lines out.
PGMDIR =	[-.pgm]
PGMLIB =	$(PGMDIR)libpgm.olb
PGMDEF =	$(PGMDIR)pgm.h
PGMLIBDEF =	$(PGMDIR)libpgm.h

# PBM is required, don't comment these lines out.
PBMDIR =	[-.pbm]
PBMLIB =	$(PBMDIR)libpbm.olb
PBMDEF =	$(PBMDIR)pbm.h
PBMLIBDEF =	$(PBMDIR)libpbm.h

# TIFF is not required for most of the PNM package, just for 
# the specific TIFF programs.
TIFFFLAGS = 	$(CFLAGS) /Define = (PPM,PGM,PBM,PBMPLUS_RAWBITS,LIBTIFF,"BSDTYPES=1","USE_VARARGS=1","USE_PROTOTYPES=0","USE_CONST",__STDC__)
TIFFINC =	/Include_Directory = ([-],[-.ppm],[-.pgm],[-.pbm],[-.libtiff])
TIFFLIB =       [-.libtiff]libtiff.olb

PNMLIB =	libpnm.olb
LIBS = 		$(PNMLIB)/Library,$(PPMLIB)/Library,$(PGMLIB)/Library,$(PBMLIB)/Library
OPT =		[-]PBMplusSHR.OPT/Option
CFLAGS =	$(CFLAGS) /Define = (PPM,PGM,PBM,PBMPLUS_RAWBITS,LIBTIFF) /Include_Directory = ([-],[-.ppm],[-.pgm],[-.pbm],[-.libtiff])

PORTBINARIES =	pnmarith.exe pnmcat.exe pnmconvol.exe pnmcrop.exe pnmcut.exe \
		pnmdepth.exe pnmenlarge.exe pnmfile.exe pnmflip.exe \
		pnmhistmap.exe pnminvert.exe pnmnoraw.exe pnmpaste.exe \
		pnmtile.exe pnmalias.exe pnmtofits.exe fitstopnm.exe \
		pnmtoddif.exe pnmtops.exe pnmtorast.exe \
		pnmtoxwd.exe rasttopnm.exe xwdtopnm.exe pnmcomp.exe \
                pnmtosir.exe sirtopnm.exe giftopnm.exe pnmtosgi.exe sgitopnm.exe \
                pnmcomp.exe pnmnlfilt.exe pnmpad.exe zeisstopnm.exe
MATHBINARIES =	pnmgamma.exe pnmrotate.exe pnmscale.exe pnmshear.exe
TIFFBINARIES =  tifftopnm.exe pnmtotiff.exe

BINARIES =      $(PORTBINARIES) $(MATHBINARIES)

MANUALS1 =	pnmarith.tex pnmcat.tex pnmconvol.tex pnmcrop.tex pnmcut.tex \
		pnmdepth.tex pnmenlarge.tex pnmfile.tex pnmflip.tex \
		pnmhistmap.tex pnminvert.tex pnmnoraw.tex pnmpaste.tex \
		pnmscale.tex pnmtile.tex pnmtofits.tex fitstopnm.tex \
		pnmtoddif.tex pnmtops.tex pnmtorast.tex \
		pnmtoxwd.tex rasttopnm.tex xwdtopnm.tex \
		pnmgamma.tex pnmrotate.tex pnmalias.tex \
		pnmshear.tex pnmcomp.tex pnmtosir.tex sirtopnm.tex \
                pnmcomp.tex pnmnlfilt.tex pnmpad.tex zeisstopnm.tex \
                giftopnm.tex
MANUALS3 =	libpnm.tex
MANUALS5 =	pnm.tex
TIFFMANUALS =   tifftopnm.tex pnmtotiff.tex

MANUALS  =	$(MANUALS1) $(MANUALS3) $(MANUALS5) $(TIFFMANUALS)

.suffixes :	.tex .1 .3 .5

.first
	@ PBMPLUS_PATH = F$Element (0, "]", F$Environment ("DEFAULT")) - ".PNM" + ".]"
	@ If F$TrnLnm ("PBMplus_Root") .eqs. "" Then -
	Define /Translation_Attributes = Concealed PBMplus_Root "''PBMPLUS_PATH'"
	@ If F$TrnLnm ("PBMplus_Dir") .eqs. "" Then -
	Define PBMplus_Dir PBMplus_Root:[000000]
	@ If F$TrnLnm ("Sys") .eqs. "" Then Define Sys Sys$Library

all :		binaries tiff
	@ continue

install :	installbinaries
	@ continue

binaries :	$(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB) $(BINARIES)
	@ Set Protection = (System:RWE, Owner:RWE, Group:RE, World:RE) *.exe

tiff :		$(TIFFLIB) $(TIFFBINARIES)
	@ continue

tifftopnm.exe :		tifftopnm.c $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB) $(TIFFLIB)
	$(CC) $(TIFFFLAGS) $(TIFFINC) tifftopnm.c
	$(LINK) $(LINKFLAGS) tifftopnm,$(TIFFLIB)/Library,$(OPT)

pnmtotiff.exe :		pnmtotiff.c $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB) $(TIFFLIB)
	$(CC) $(TIFFFLAGS) $(TIFFINC) pnmtotiff.c
	$(LINK) $(LINKFLAGS) pnmtotiff,$(TIFFLIB)/Library,$(OPT)

installbinaries :	$(BINARIES)
	@- Set Protection = (System:RWE, Owner:RWED, Group:RE, World:RE) *.exe
	@- Rename /Log *.exe $(INSTALLBINARIES)
	@- Set Protection = Owner:RWE PBMplus_Root:[Exe]*.exe

manual :	TeX $(MANUALS)
	@ Set Protection = (System:RWE, Owner:RWE, Group:RE, World:RE) *.tex

installmanual :	
	@- Set Protection = (System:RWE, Owner:RWED, Group:RE, World:RE) *.tex
	@- Rename *.tex $(INSTALLMANUALS)
	@- Set Protection = Owner:RWE PBMplus_Root:[TeX]*.tex

TeX :
	SETUP TeX

# Rules for creating TeX documentation from troff files.
.1.tex :
	tr2TeX -m -t -o $*.tex $*.1
.3.tex :
	tr2TeX -m -t -o $*.tex $*.3
.5.tex :
	tr2TeX -m -t -o $*.tex $*.5

# Rule for plain programs.
.obj.exe :
	$(LINK) $(LINKFLAGS) $*.obj,$(OPT)

# And libraries.
lib :		$(PNMLIB)
	@ Continue

$(PNMLIB) :	libpnm1.obj libpnm2.obj libpnm3.obj libpnm4.obj
	Library /Create $(PNMLIB) libpnm%.obj

libpnm1.obj :	libpnm1.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
libpnm2.obj :	libpnm2.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF) $(PPMLIBDEF) $(PGMLIBDEF) $(PBMLIBDEF)
libpnm3.obj :	libpnm3.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF) $(PPMLIBDEF) $(PGMLIBDEF) $(PBMLIBDEF)
libpnm4.obj :	libpnm4.c pnm.h rast.h $(PPMDEF) $(PGMDEF) $(PBMDEF)

$(PPMLIB) :
	Set Default [-.ppm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.pnm]

$(PGMLIB) :
	Set Default [-.pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.pnm]

$(PBMLIB) :
	Set Default [-.pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.pnm]

$(TIFFLIB) :
	Set Default [-.libtiff]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.pnm]

# Object file dependencies
pnmalias.obj :		pnmalias.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmarith.obj :		pnmarith.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmcat.obj :		pnmcat.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmcomp.obj :		pnmcomp.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmconvol.obj :		pnmconvol.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmcrop.obj :		pnmcrop.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmcut.obj :		pnmcut.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmdepth.obj :		pnmdepth.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmenlarge.obj :	pnmenlarge.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmfile.obj :		pnmflip.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmflip.obj :		pnmflip.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmgamma.obj :		pnmgamma.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmhistmap.obj :	pnmhistmap.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnminvert.obj :		pnminvert.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmnoraw.obj :		pnmnoraw.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmpaste.obj :		pnmpaste.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmrotate.obj :		pnmrotate.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmscale.obj :		pnmscale.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmshear.obj :		pnmshear.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtile.obj :		pnmtile.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtoddif.obj :		pnmtoddif.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtofits.obj :		pnmtofits.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtops.obj :		pnmtops.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtorast.obj :		pnmtorast.c rast.h pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtosgi.obj :		pnmtosgi.c pnm.h sgi.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtotiff.obj :		pnmtotiff.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
giftopnm.obj :		giftopnm.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
fitstopnm.obj :		fitstopnm.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
rasttopnm.obj :		rasttopnm.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
sgitopnm.obj :		sgitopnm.c pnm.h sgi.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
tifftopnm.obj :		tifftopnm.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
xwdtopnm.obj :		xwdtopnm.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
sirtopnm.obj :          sirtopnm.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtosir.obj :          pnmtosir.c pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)

# Binary dependencies, someone may want to build just a single image
pnmalias.exe :		pnmalias.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmarith.exe :		pnmarith.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmcat.exe :		pnmcat.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmcomp.exe :		pnmcomp.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmconvol.exe :		pnmconvol.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmcrop.exe :		pnmcrop.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmcut.exe :		pnmcut.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmdepth.exe :		pnmdepth.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmenlarge.exe :	pnmenlarge.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmfile.exe :		pnmfile.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmflip.exe :		pnmflip.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmgamma.exe :		pnmgamma.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnminvert.exe :		pnminvert.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmnoraw.exe :		pnmnoraw.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmpaste.exe :		pnmpaste.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmrotate.exe :		pnmrotate.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmscale.exe :		pnmscale.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmshear.exe :		pnmshear.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmtile.exe :		pnmtile.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmtoddif.exe :		pnmtoddif.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmtofits.exe :		pnmtofits.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmtops.exe :		pnmtops.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
pnmtorast.exe :		pnmtorast.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
giftopnm.exe :		giftopnm.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
fitstopnm.exe :		fitstopnm.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
rasttopnm.exe :		rasttopnm.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
xwdtopnm.exe :		xwdtopnm.obj $(PNMLIB) $(PPMLIB) $(PGMLIB) $(PBMLIB)
sgitopnm.exe :          sgitopnm.obj pnm.h sgi.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
sirtopnm.exe :          sirtopnm.obj pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtosir.exe :          pnmtosir.obj pnm.h $(PPMDEF) $(PGMDEF) $(PBMDEF)
pnmtosgi.exe :          pnmtosgi.obj pnm.h sgi.h $(PPMDEF) $(PGMDEF) $(PBMDEF)

# TeX documentation dependencies
pnmalias.tex :		pnmalias.1
pnmarith.tex :		pnmarith.1
pnmcat.tex :		pnmcat.1
pnmcomp.tex :		pnmcomp.1
pnmconvol.tex :		pnmconvol.1
pnmcrop.tex :		pnmcrop.1
pnmcut.tex :		pnmcut.1
pnmdepth.tex :		pnmdepth.1
pnmenlarge.tex :	pnmenlarge.1
pnmfile.tex :		pnmfile.1
pnmflip.tex :		pnmflip.1
pnmgamma.tex :          pnmgamma.1
pnminvert.tex :		pnminvert.1
pnmnoraw.tex :		pnmnoraw.1
pnmpaste.tex :		pnmpaste.1
pnmrotate.tex :         pnmrotate.1
pnmscale.tex :		pnmscale.1
pnmshear.tex :          pnmshear.1
pnmtile.tex :		pnmtile.1
pnmtoddif.tex :		pnmtoddif.1
pnmtofits.tex :		pnmtofits.1
pnmtops.tex :		pnmtops.1
pnmtorast.tex :		pnmtorast.1
pnmtotiff.tex :		pnmtotiff.1
pnmtosgi.tex :		pnmtosgi.1
pnmtoxwd.tex :		pnmtoxwd.1
giftopnm.tex :		giftopnm.1
fitstopnm.tex :		fitstopnm.1
rasttopnm.tex :		rasttopnm.1
tifftopnm.tex :		tifftopnm.1
sgitopnm.tex :		sgitopnm.1
xwdtopnm.tex :		xwdtopnm.1
libpnm.tex :		libpnm.3
pnm.tex :		pnm.5
sirtopnm.tex :          sirtopnm.1
pnmtosir.tex :          pnmtosir.1

clean :
	- Set Protection = Owner:RWED *.obj;*,*.*;-1
	- Purge /NoLog /NoConfirm *.*
	- Delete /NoLog /NoConfirm *.obj;
