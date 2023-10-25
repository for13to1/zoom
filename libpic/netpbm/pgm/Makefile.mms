# MMS Description file for pgm tools.
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

# PBM is required, don't comment these lines out.
PBMDIR =	[-.pbm]
PBMLIB =	$(PBMDIR)libpbm.olb
PBMINC =	$(PBMDIR)pbm.h [-]pbmplus.h
PBMLIBDEF =	$(PBMDIR)libpbm.h

PGMLIB =	libpgm.olb
LIBS =          $(PGMLIB)/Library,$(PBMLIB)/Library
OPT =		[-]PBMplusSHR.OPT/Option
CFLAGS = 	$(CFLAGS) /Define = (PBMPLUS_RAWBITS,LIBTIFF) /Include_Directory = ([-.pbm],[-])

BINARIES =	fstopgm.exe hipstopgm.exe lispmtopgm.exe \
		pgmbentley.exe pgmcrater.exe pgmedge.exe pgmenhance.exe \
		pgmhist.exe pgmnoise.exe pgmnorm.exe pgmoil.exe pgmramp.exe \
		pgmkernel.exe \
		pgmtofs.exe pgmtolispm.exe pgmtopbm.exe psidtopgm.exe \
		rawtopgm.exe pgmtexture.exe asciitopgm.exe bioradtopgm.exe \
		pbmtopgm.exe spottopgm.exe

MANUALS1 =	fstopgm.tex hipstopgm.tex lispmtopgm.tex \
		pgmbentley.tex pgmcrater.tex pgmedge.tex pgmenhance.tex \
		pgmhist.tex pgmnoise.tex pgmnorm.tex pgmoil.tex pgmramp.tex \
		pgmtofs.tex pgmtolispm.tex pgmtopbm.tex psidtopgm.tex \
		rawtopgm.tex pgmtexture.tex asciitopgm.exe bioradtopgm.tex \
		pbmtopgm.tex spottopgm.tex pgmkernel.tex 
MANUALS3 =	libpgm.tex
MANUALS5 =	pgm.tex

MANUALS = 	$(MANUALS1) $(MANUALS3) $(MANUALS5)

.suffixes :	.tex .1 .3 .5

.first
	@ PBMPLUS_PATH = F$Element (0, "]", F$Environment ("DEFAULT")) - ".PGM" + ".]"
	@ If F$TrnLnm ("PBMplus_Root") .eqs. "" Then -
	Define /Translation_Attributes = Concealed PBMplus_Root "''PBMPLUS_PATH'"
	@ If F$TrnLnm ("PBMplus_Dir") .eqs. "" Then -
	Define PBMplus_Dir PBMplus_Root:[000000]
	@ If F$TrnLnm ("Sys") .eqs. "" Then Define Sys Sys$Library

all :		binaries
	@ continue

install :	installbinaries
	@ continue

binaries :	$(PGMLIB) $(PBMLIB) $(BINARIES)
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
lib :		$(PGMLIB)
	@ Continue

$(PGMLIB) :	libpgm1.obj libpgm2.obj
	Library /Create $(PGMLIB) libpgm%.obj

libpgm1.obj :	libpgm1.c pgm.h $(PBMINC) libpgm.h
libpgm2.obj :	libpgm2.c pgm.h $(PBMINC) libpgm.h $(PBMLIBDEF)

$(PBMLIB) :
	Set Default [-.pbm]
	$(MMS) $(MMSQUALIFIERS) /Description = Makefile.MMS lib
	Set Default [-.pgm]

# Object file dependencies
fstopgm.obj :		fstopgm.c pgm.h $(PBMINC)
hipstopgm.obj :		hipstopgm.c pgm.h $(PBMINC)
lispmtopgm.obj :	lispmtopgm.c pgm.h $(PBMINC)
pgmbentley.obj :	pgmbentley.c pgm.h $(PBMINC)
pgmcrater.obj :		pgmcrater.c pgm.h $(PBMINC)
pgmedge.obj :		pgmedge.c pgm.h $(PBMINC)
pgmenhance.obj :	pgmenhance.c pgm.h $(PBMINC)
pgmhist.obj :		pgmhist.c pgm.h $(PBMINC)
pgmkernel.obj :		pgmkernel.c pgm.h $(PBMINC)
pgmnoise.obj :		pgmnoise.c pgm.h $(PBMINC)
pgmnorm.obj :		pgmnorm.c pgm.h $(PBMINC)
pgmoil.obj :		pgmoil.c pgm.h $(PBMINC)
pgmramp.obj :		pgmramp.c pgm.h $(PBMINC)
pgmtexture.obj :	pgmtexture.c pgm.h $(PBMINC)
pgmtofs.obj :		pgmtofs.c pgm.h $(PBMINC)
pgmtopbm.obj :		pgmtopbm.c dithers.h $(PBMINC) pgm.h
pgmtolispm.obj :	pgmtolispm.c pgm.h $(PBMINC)
psidtopgm.obj :		psidtopgm.c pgm.h $(PBMINC)
rawtopgm.obj :		rawtopgm.c pgm.h $(PBMINC)
asciitopgm.obj :        asciitopgm.c pgm.h $(PBMINC)
bioradtopgm.obj :       bioradtopgm.c pgm.h $(PBMINC)
pbmtopgm.obj :          pbmtopgm.c pgm.h $(PBMINC)
spottopgm.obj :         spottopgm.c pgm.h $(PBMINC)

# Binary dependencies, someone may want to build just a single image
fstopgm.exe :		fstopgm.obj $(PGMLIB) $(PBMLIB)
hipstopgm.exe :		hipstopgm.obj $(PGMLIB) $(PBMLIB)
lispmtopgm.exe :	lispmtopgm.obj $(PGMLIB) $(PBMLIB)
pgmbentley.exe :	pgmbentley.obj $(PGMLIB) $(PBMLIB)
pgmcrater.exe :		pgmcrater.obj $(PGMLIB) $(PBMLIB)
pgmedge.exe :		pgmedge.obj pgm.h $(PBMINC)
pgmenhance.exe :	pgmenhance.obj $(PGMLIB) $(PBMLIB)
pgmhist.exe :		pgmhist.obj $(PGMLIB) $(PBMLIB)
pgmkernel.exe :		pgmkernel.obj $(PGMLIB) $(PBMLIB)
pgmnoise.exe :		pgmnoise.obj $(PGMLIB) $(PBMLIB)
pgmnorm.exe :		pgmnorm.obj $(PGMLIB) $(PBMLIB)
pgmoil.exe :		pgmoil.obj $(PGMLIB) $(PBMLIB)
pgmramp.exe :		pgmramp.obj $(PGMLIB) $(PBMLIB)
pgmtexture.exe :	pgmtexture.obj $(PGMLIB) $(PBMLIB)
pgmtofs.exe :		pgmtofs.obj $(PGMLIB) $(PBMLIB)
pgmtopbm.exe :		pgmtopbm.obj $(PGMLIB) $(PBMLIB)
pgmtolispm.exe :	pgmtolispm.obj $(PGMLIB) $(PBMLIB)
psidtopgm.exe :		psidtopgm.obj $(PGMLIB) $(PBMLIB)
rawtopgm.exe :		rawtopgm.obj $(PGMLIB) $(PBMLIB)
asciitopgm.exe :        asciitopgm.obj $(PGMLIB) $(PBMLIB)
bioradtopgm.exe :       bioradtopgm.obj $(PGMLIB) $(PBMLIB)
pbmtopgm.exe :          pbmtopgm.obj $(PGMLIB) $(PBMLIB)
spottopgm.exe :         spottopgm.obj $(PGMLIB) $(PBMLIB)

# TeX documentation dependencies
fstopgm.tex :		fstopgm.1
hipstopgm.tex :		hipstopgm.1
lispmtopgm.tex :	lispmtopgm.1
pgmbentley.tex :	pgmbentley.1
pgmcrater.tex :		pgmcrater.1
pgmedge.tex :		pgmedge.1
pgmenhance.tex :	pgmenhance.1
pgmhist.tex :		pgmhist.1
pgmkernel.tex :		pgmkernel.1
pgmnoise.tex :		pgmnoise.1
pgmnorm.tex :		pgmnorm.1
pgmoil.tex :		pgmoil.1
pgmramp.tex :		pgmramp.1
pgmtexture.tex :	pgmtexture.1
pgmtofs.tex :		pgmtofs.1
pgmtopbm.tex :		pgmtopbm.1
pgmtolispm.tex :	pgmtolispm.1
psidtopgm.tex :		psidtopgm.1
rawtopgm.tex :		rawtopgm.1
libpgm.tex :		libpgm.3
pgm.tex :		pgm.5
asciitopgm.tex :        asciitopgm.1
bioradtopgm.tex :       bioradtopgm.1
pbmtopgm.tex :          pbmtopgm.1
spottopgm.tex :         spottopgm.1

clean :
	- Set Protection = Owner:RWED *.obj;*,*.*;-1
	- Purge /NoLog /NoConfirm *.*
	- Delete /NoLog /NoConfirm *.obj;
