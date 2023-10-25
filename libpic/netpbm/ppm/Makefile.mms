# MMS Description file for ppm tools.
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

# CONFIGURE: 
PGMDIR =	[-.pgm]
PGMLIB =	$(PGMDIR)libpgm.olb
PGMINC =	$(PGMDIR)pgm.h
PGMLIBINC =	$(PGMDIR)libpgm.h

# PBM is required, don't comment these lines out.
PBMDIR =	[-.pbm]
PBMLIB =	$(PBMDIR)libpbm.olb
PBMINC =	$(PBMDIR)pbm.h
PBMLIBINC =	$(PBMDIR)libpbm.h

PPMLIB =	libppm.olb
LIBS = 		$(PPMLIB)/Library,$(PGMLIB)/Library,$(PBMLIB)/Library
OPT =		[-]PBMplusSHR.OPT/Option
CFLAGS =	$(CFLAGS) /Define = (PPM,PGM,PBM,PBMPLUS_RAWBITS,LIBTIFF) /Include_Directory = ([-],[-.pgm],[-.pbm])

PORTBINARIES =	gouldtoppm.exe hpcdtoppm.exe ilbmtoppm.exe imgtoppm.exe \
		mtvtoppm.exe pcxtoppm.exe pgmtoppm.exe pi1toppm.exe \
		picttoppm.exe pjtoppm.exe ppm3d.exe ppmchange.exe ppmdim.exe \
		ppmdither.exe ppmflash.exe ppmforge.exe ppmtomitsu.exe \
		ppmhist.exe ppmmake.exe ppmmix.exe ppmntsc.exe \
		ppmquant.exe ppmrelief.exe ppmshift.exe ppmspread.exe \
		ppmtoacad.exe ppmtogif.exe ppmtoicr.exe ppmtoilbm.exe \
		ppmtopcx.exe ppmtopgm.exe ppmtopi1.exe ppmtopict.exe \
		ppmtopj.exe ppmtopuzz.exe ppmtorgb3.exe ppmtosixel.exe \
		ppmtotga.exe ppmtouil.exe ppmtoxpm.exe ppmtoyuv.exe \
		qrttoppm.exe rawtoppm.exe rgb3toppm.exe sldtoppm.exe \
		spctoppm.exe sputoppm.exe tgatoppm.exe ximtoppm.exe \
		xpmtoppm.exe yuvtoppm.exe ppmtopjxl.exe xvminitoppm.exe \
                bmptoppm.exe ppmbrighten.exe ppmdist.exe ppmnorm.exe \
                ppmqvga.exe ppmtobmp.exe ppmtomap.exe \
                ppmtoyuvsplit.exe yuvsplittoppm.exe
MATHBINARIES =	ppmpat.exe

BINARIES =      $(PORTBINARIES) $(MATHBINARIES)

MANUALS1 =	gouldtoppm.tex hpcdtoppm.tex ilbmtoppm.tex imgtoppm.tex \
		mtvtoppm.tex pcxtoppm.tex pgmtoppm.tex pi1toppm.tex \
		picttoppm.tex pjtoppm.tex ppm3d.tex ppmdim.tex ppmdim.tex ppmdither.tex \
		ppmflash.tex ppmforge.tex ppmhist.tex ppmmake.tex ppmmix.tex \
		ppmntsc.tex ppmquant.tex ppmrelief.tex ppmtomitsu.tex \
		ppmshift.tex ppmspread.tex \
		ppmtoacad.tex ppmtogif.tex ppmtoicr.tex ppmtoilbm.tex \
		ppmtopcx.tex ppmtopgm.tex ppmtopi1.tex ppmtopict.tex \
		ppmtopj.tex ppmtopuzz.tex ppmtorgb3.tex ppmtosixel.tex \
		ppmtotga.tex ppmtouil.tex ppmtoxpm.tex ppmtoyuv.tex \
		qrttoppm.tex rawtoppm.tex rgb3toppm.tex sldtoppm.tex \
		spctoppm.tex sputoppm.tex tgatoppm.tex ximtoppm.tex \
		xpmtoppm.tex yuvtoppm.tex ppmpat.tex ppmtopjxl.tex \
                bmptoppm.tex ppmbrighten.tex ppmdist.tex ppmnorm.tex \
                ppmqvga.tex ppmtobmp.tex ppmtomap.tex xvminitoppm.tex \
                ppmtoyuvsplit.tex yuvsplittoppm.tex
MANUALS3 =	libppm.tex
MANUALS5 =	ppm.tex

MANUALS = $(MANUALS1) $(MANUALS3) $(MANUALS5)

.suffixes :	.tex .1 .3 .5

.first
	@ PBMPLUS_PATH = F$Element (0, "]", F$Environment ("DEFAULT")) - ".PPM" + ".]"
	@ If F$TrnLnm ("PBMplus_Root") .eqs. "" Then -
	Define /Translation_Attributes = Concealed PBMplus_Root "''PBMPLUS_PATH'"
	@ If F$TrnLnm ("PBMplus_Dir") .eqs. "" Then -
	Define PBMplus_Dir PBMplus_Root:[000000]
	@ If F$TrnLnm ("Sys") .eqs. "" Then Define Sys Sys$Library

all :		binaries
	@ continue

install :	installbinaries
	@ continue

binaries :	$(PPMLIB) $(PGMLIB) $(PBMLIB) $(BINARIES)
	@ Set Protection = (System:RWE, Owner:RWE, Group:RE, World:RE) *.exe

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
#	$(LINK) $(LINKFLAGS) $*.obj,$(LIBS),$(OPT)

# And libraries.
lib :		$(PPMLIB)
	@ Continue

$(PPMLIB) :	libppm1.obj libppm2.obj libppm3.obj libppm4.obj libppm5.obj bitio.obj
	Library /Create $(PPMLIB) libppm%.obj,bitio.obj

libppm1.obj :	libppm1.c ppm.h $(PGMINC) $(PBMINC) libppm.h libppm1.c
libppm2.obj :	libppm2.c ppm.h $(PGMINC) $(PBMINC) libppm.h $(PGMLIBINC) $(PBMLIBINC)
libppm3.obj :	libppm3.c ppm.h $(PGMINC) $(PBMINC) ppmcmap.h libppm.h
libppm4.obj :	libppm4.c ppm.h $(PGMINC) $(PBMINC)
libppm5.obj :	libppm5.c ppm.h $(PGMINC) $(PBMINC) ppmdraw.h

$(PBMLIB) :
	Set Default [-.pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.ppm]

$(PGMLIB) :
	Set Default [-.pgm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.ppm]

# Object file dependencies
gouldtoppm.obj :	gouldtoppm.c ppm.h $(PGMINC) $(PBMINC)
hpcdtoppm.obj :		hpcdtoppm.c ppm.h $(PGMINC) $(PBMINC)
ilbmtoppm.obj :		ilbmtoppm.c ilbm.h ppm.h $(PGMINC) $(PBMINC)
imgtoppm.obj :		imgtoppm.c ppm.h $(PGMINC) $(PBMINC)
mtvtoppm.obj :		mtvtoppm.c ppm.h $(PGMINC) $(PBMINC)
pcxtoppm.obj :		pcxtoppm.c ppm.h $(PGMINC) $(PBMINC)
pgmtoppm.obj :		pgmtoppm.c ppm.h $(PGMINC) $(PBMINC)
pi1toppm.obj :		pi1toppm.c ppm.h $(PGMINC) $(PBMINC)
picttoppm.obj :		picttoppm.c ppm.h $(PGMINC) $(PBMINC)
pjtoppm.obj :		pjtoppm.c ppm.h $(PGMINC) $(PBMINC)
ppm3d.obj :		ppm3d.c ppm.h $(PGMINC) $(PBMINC)
ppmchange.obj :		ppmchange.c ppm.h $(PGMINC) $(PBMINC)
ppmdim.obj :		ppmdim.c ppm.h $(PGMINC) $(PBMINC)
ppmdither.obj :		ppmdither.c ppm.h $(PGMINC) $(PBMINC)
ppmflash.obj :		ppmflash.c ppm.h $(PGMINC) $(PBMINC)
ppmforge.obj :		ppmforge.c ppm.h $(PGMINC) $(PBMINC)
ppmhist.obj :		ppmhist.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmmake.obj :		ppmmake.c ppm.h $(PGMINC) $(PBMINC)
ppmmix.obj :		ppmmix.c ppm.h $(PGMINC) $(PBMINC)
ppmntsc.obj :		ppmntsc.c ppm.h $(PGMINC) $(PBMINC)
ppmquant.obj :		ppmquant.c $(PGMDIR)dithers.h ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmrelief.obj :		ppmrelief.c ppm.h $(PGMINC) $(PBMINC)
ppmshift.obj :		ppmshift.c ppm.h $(PGMINC) $(PBMINC)
ppmspread.obj :		ppmspread.c ppm.h $(PGMINC) $(PBMINC)
ppmtoacad.obj :		ppmtoacad.c autocad.h ppm.h $(PGMINC) $(PBMINC)
ppmtogif.obj :		ppmtogif.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtoicr.obj :		ppmtoicr.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtoilbm.obj :		ppmtoilbm.c ilbm.h ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtomitsu.obj :	ppmtomitsu.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtopcx.obj :		ppmtopcx.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtopgm.obj :		ppmtopgm.c ppm.h $(PGMINC) $(PBMINC)
ppmtopi1.obj :		ppmtopi1.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtopict.obj :		ppmtopict.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtopj.obj :		ppmtopj.c ppm.h $(PGMINC) $(PBMINC)
ppmtopuzz.obj :		ppmtopuzz.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtorgb3.obj :		ppmtorgb3.c ppm.h $(PGMINC) $(PBMINC)
ppmtosixel.obj :	ppmtosixel.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtotga.obj :		ppmtotga.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtoxpm.obj :		ppmtoxpm.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtouil.obj :		ppmtouil.c ppmcmap.h ppm.h $(PGMINC) $(PBMINC)
ppmtoyuv.obj :		ppmtoyuv.c ppm.h $(PGMINC) $(PBMINC)
qrttoppm.obj :		qrttoppm.c ppm.h $(PGMINC) $(PBMINC)
rawtoppm.obj :		rawtoppm.c ppm.h $(PGMINC) $(PBMINC)
rgb3toppm.obj :		rgb3toppm.c ppm.h $(PGMINC) $(PBMINC)
sldtoppm.obj :		sldtoppm.c ppm.h $(PGMINC) $(PBMINC)
spctoppm.obj :		spctoppm.c ppm.h $(PGMINC) $(PBMINC)
sputoppm.obj :		sputoppm.c ppm.h $(PGMINC) $(PBMINC)
tgatoppm.obj :		tgatoppm.c tga.h ppm.h $(PGMINC) $(PBMINC)
ximtoppm.obj :		ximtoppm.c xim.h ppm.h $(PGMINC) $(PBMINC)
xpmtoppm.obj :		xpmtoppm.c ppm.h $(PGMINC) $(PBMINC)
xvminitoppm.obj :	xvminitoppm.c ppm.h $(PGMINC) $(PBMINC)
yuvtoppm.obj :		yuvtoppm.c ppm.h $(PGMINC) $(PBMINC)
ppmpat.obj :		ppmpat.c ppmdraw.h ppm.h $(PGMINC) $(PBMINC)
ppmtopmxl.obj :		ppmtopjxl.c ppm.h $(PGMINC) $(PBMINC)
bitio.obj :		bitio.c bitio.h

# Binary dependencies, someone may want to build just a single image
hpcdtoppm.exe :		hpcdtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
gouldtoppm.exe :	gouldtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ilbmtoppm.exe :		ilbmtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
imgtoppm.exe :		imgtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
mtvtoppm.exe :		mtvtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
pcxtoppm.exe :		pcxtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
pgmtoppm.exe :		pgmtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
pi1toppm.exe :		pi1toppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
picttoppm.exe :		picttoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
pjtoppm.exe :		pjtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmchange.exe :		ppmchange.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmdim.exe :		ppmdim.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmdither.exe :		ppmdither.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmflash.exe :		ppmflash.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmforge.exe :		ppmforge.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmhist.exe :		ppmhist.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmmake.exe :		ppmmake.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmmix.exe :		ppmmix.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmntsc.exe :		ppmntsc.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmquant.exe :		ppmquant.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmrelief.exe :		ppmrelief.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmshift.exe :		ppmshift.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmspread.exe :		ppmspread.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtoacad.exe :		ppmtoacad.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtogif.exe :		ppmtogif.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtoicr.exe :		ppmtoicr.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtoilbm.exe :		ppmtoilbm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtomitsu.exe :	ppmtomitsu.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopcx.exe :		ppmtopcx.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopgm.exe :		ppmtopgm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopi1.exe :		ppmtopi1.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopict.exe :		ppmtopict.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopj.exe :		ppmtopj.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopuzz.exe :		ppmtopuzz.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtorgb3.exe :		ppmtorgb3.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtosixel.exe :	ppmtosixel.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtotga.exe :		ppmtotga.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtoxpm.exe :		ppmtoxpm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtoyuv.exe :		ppmtoyuv.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
qrttoppm.exe :		qrttoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
rawtoppm.exe :		rawtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
rgb3toppm.exe :		rgb3toppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
sldtoppm.exe :		sldtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
spctoppm.exe :		spctoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
sputoppm.exe :		sputoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
tgatoppm.exe :		tgatoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ximtoppm.exe :		ximtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
xpmtoppm.exe :		xpmtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
xvminitoppm.exe :	xvminitoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
yuvtoppm.exe :		yuvtoppm.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmpat.exe :		ppmpat.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)
ppmtopmxl.exe :		ppmtopjxl.obj $(PPMLIB) $(PGMLIB) $(PBMLIB)

# TeX documentation dependencies
hpcdtoppm.tex :		hpcdtoppm.1
gouldtoppm.tex :	gouldtoppm.1
ilbmtoppm.tex :		ilbmtoppm.1
imgtoppm.tex :		imgtoppm.1
mtvtoppm.tex :		mtvtoppm.1
pcxtoppm.tex :		pcxtoppm.1
pgmtoppm.tex :		pgmtoppm.1
pi1toppm.tex :		pi1toppm.1
picttoppm.tex :		picttoppm.1
pjtoppm.tex :		pjtoppm.1
ppmchange.tex :		ppmchange.1
ppmdim.tex :		ppmdim.1
ppmdither.tex :		ppmdither.1
ppmflash.tex :		ppmflash.1
ppmforge.tex :		ppmforge.1
ppmhist.tex :		ppmhist.1
ppmmake.tex :		ppmmake.1
ppmmix.tex :		ppmmix.1
ppmntsc.tex :		ppmntsc.1
ppmquant.tex :		ppmquant.1
ppmrelief.tex :		ppmrelief.1
ppmshift.tex :		ppmshift.1
ppmspread.tex :		ppmspread.1
ppmtoacad.tex :		ppmtoacad.1
ppmtogif.tex :		ppmtogif.1
ppmtoicr.tex :		ppmtoicr.1
ppmtoilbm.tex :		ppmtoilbm.1
ppmtomitsu.tex :	ppmtomitsu.1
ppmtopcx.tex :		ppmtopcx.1
ppmtopgm.tex :		ppmtopgm.1
ppmtopi1.tex :		ppmtopi1.1
ppmtopict.tex :		ppmtopict.1
ppmtopj.tex :		ppmtopj.1
ppmtopuzz.tex :		ppmtopuzz.1
ppmtorgb3.tex :		ppmtorgb3.1
ppmtosixel.tex :	ppmtosixel.1
ppmtotga.tex :		ppmtotga.1
ppmtoxpm.tex :		ppmtoxpm.1
ppmtoyuv.tex :		ppmtoyuv.1
qrttoppm.tex :		qrttoppm.1
rawtoppm.tex :		rawtoppm.1
rgb3toppm.tex :		rgb3toppm.1
sldtoppm.tex :		sldtoppm.1
spctoppm.tex :		spctoppm.1
sputoppm.tex :		sputoppm.1
tgatoppm.tex :		tgatoppm.1
ximtoppm.tex :		ximtoppm.1
xpmtoppm.tex :		xpmtoppm.1
xvminitoppm.tex :	xvminitoppm.1
yuvtoppm.tex :		yuvtoppm.1
ppmpat.tex :		ppmpat.1
ppmtopmxl.tex :		ppmtopjxl.1
libppm.tex :		libppm.3
ppm.tex :		ppm.5

clean :
	- Set Protection = Owner:RWED *.obj;*,*.*;-1
	- Purge /NoLog /NoConfirm *.*
	- Delete /NoLog /NoConfirm *.obj;
