# MMS Description file for pbm tools.
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
# Last Updated:  2-SEP-1993 by Rick Dyson
#               16-SEP-1993 by Rick Dyson for netpbm release/Alpha03

# Default values
INSTALLBINARIES =	PBMplus_Root:[Exe]
INSTALLMANUALS  =	PBMplus_Root:[TeX]

PBMLIB =	libpbm.olb
LIBS =          $(PBMLIB)/Library
OPT =		PBMplusSHR/Option
CFLAGS = 	$(CFLAGS) /Define = (PBMPLUS_RAWBITS,LIBTIFF) /Include_Directory = [-]

BINARIES =	atktopbm.exe brushtopbm.exe cmuwmtopbm.exe g3topbm.exe \
		gemtopbm.exe icontopbm.exe macptopbm.exe mgrtopbm.exe \
		pbmlife.exe pbmmake.exe pbmmask.exe pbmreduce.exe \
		pbmtext.exe \
		pbmto10x.exe pbmto4425.exe pbmtoascii.exe pbmtoatk.exe \
		pbmtobbnbg.exe pbmtocmuwm.exe pbmtoepson.exe pbmtog3.exe \
		pbmtogem.exe pbmtogo.exe pbmtoicon.exe pbmtolj.exe \
		pbmtomacp.exe pbmtomgr.exe pbmtopi3.exe pbmtoplot.exe \
		pbmtoptx.exe pbmtox10bm.exe pbmtoxbm.exe pbmtoybm.exe \
		pbmtozinc.exe pbmupc.exe pi3topbm.exe xbmtopbm.exe ybmtopbm.exe pbmtoln03.exe \
                pbmclean.exe pbmpscale.exe pbmtoepsi.exe pbmtolps.exe pbmtopk.exe pktopbm.exe

MANUALS1 =	atktopbm.tex brushtopbm.tex cmuwmtopbm.tex g3topbm.tex \
		gemtopbm.tex icontopbm.tex macptopbm.tex mgrtopbm.tex \
		pbmlife.tex pbmmake.tex pbmmask.tex pbmreduce.tex \
		pbmtext.tex \
		pbmto10x.tex pbmto4425.tex pbmtoascii.tex pbmtoatk.tex \
		pbmtobbnbg.tex pbmtocmuwm.tex pbmtoepson.tex pbmtog3.tex \
		pbmtogem.tex pbmtogo.tex pbmtoicon.tex pbmtolj.tex \
		pbmtomacp.tex pbmtomgr.tex pbmtopi3.tex pbmtoplot.tex \
		pbmtoptx.tex pbmtox10bm.tex pbmtoxbm.tex pbmtoybm.tex \
		pbmtozinc.tex pbmupc.tex pi3topbm.tex xbmtopbm.tex ybmtopbm.tex pbmtoln03.tex \
                pbmclean.tex pbmpscale.tex pbmtoepsi.tex pbmtolps.tex pbmtopk.tex pktopbm.tex
MANUALS3 =	libpbm.tex
MANUALS5 =	pbm.tex

MANUALS = 	$(MANUALS1) $(MANUALS3) $(MANUALS5)

.suffixes :	.tex .1 .3 .5

.first
	@ PBMPLUS_PATH = F$Element (0, "]", F$Environment ("DEFAULT")) - ".PBM" + ".]"
	@ If F$TrnLnm ("PBMplus_Root") .eqs. "" Then -
	Define /Translation_Attributes = Concealed PBMplus_Root "''PBMPLUS_PATH'"
	@ If F$TrnLnm ("PBMplus_Dir") .eqs. "" Then -
	Define PBMplus_Dir PBMplus_Root:[000000]
	@ If F$TrnLnm ("Sys") .eqs. "" Then Define Sys Sys$Library

all : 		binaries
	@ Continue

install : 	installbinaries
	@ Continue

binaries :	$(PBMLIB) $(BINARIES)
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
#	$(LINK) $(LINKFLAGS) $*.obj,$(LIBS)/Library,$(OPT)

# And libraries.
lib :           $(PBMLIB)
	@ Continue

$(PBMLIB) :	libpbm1.obj libpbm2.obj libpbm3.obj libpbm4.obj libpbm5.obj
	Library /Create $(PBMLIB) libpbm%.obj

libpbm1.obj :	libpbm1.c pbm.h [-]pbmplus.h libpbm.h [-]version.h includes.h
libpbm2.obj :	libpbm2.c pbm.h [-]pbmplus.h libpbm.h
libpbm3.obj :	libpbm3.c pbm.h [-]pbmplus.h libpbm.h
libpbm4.obj :	libpbm4.c pbm.h [-]pbmplus.h libpbm.h
libpbm5.obj :	libpbm5.c pbm.h [-]pbmplus.h

# Object file dependencies
atktopbm.obj :		atktopbm.c pbm.h [-]pbmplus.h
brushtopbm.obj :	brushtopbm.c pbm.h [-]pbmplus.h
cmuwmtopbm.obj :	cmuwmtopbm.c cmuwm.h pbm.h [-]pbmplus.h
g3topbm.obj :		g3topbm.c g3.h pbm.h [-]pbmplus.h
gemtopbm.obj :		gemtopbm.c pbm.h [-]pbmplus.h
icontopbm.obj :		icontopbm.c pbm.h [-]pbmplus.h
macptopbm.obj :		macptopbm.c macp.h pbm.h [-]pbmplus.h
mgrtopbm.obj :		mgrtopbm.c mgr.h pbm.h [-]pbmplus.h
pbmlife.obj :		pbmlife.c pbm.h [-]pbmplus.h
pbmmake.obj :		pbmmake.c pbm.h [-]pbmplus.h
pbmmask.obj :		pbmmask.c pbm.h [-]pbmplus.h
pbmreduce.obj :		pbmreduce.c pbm.h [-]pbmplus.h
pbmtext.obj :		pbmtext.c pbm.h pbmfont.h [-]pbmplus.h
pbmto10x.obj :		pbmto10x.c pbm.h [-]pbmplus.h
pbmto4425.obj :		pbmto4425.c pbm.h [-]pbmplus.h
pbmtoascii.obj :	pbmtoascii.c pbm.h [-]pbmplus.h
pbmtoatk.obj :		pbmtoatk.c pbm.h [-]pbmplus.h
pbmtobbnbg.obj :	pbmtobbnbg.c pbm.h [-]pbmplus.h
pbmtocmuwm.obj :	pbmtocmuwm.c cmuwm.h pbm.h [-]pbmplus.h
pbmtoepson.obj :	pbmtoepson.c pbm.h [-]pbmplus.h
pbmtog3.obj :		pbmtog3.c g3.h pbm.h [-]pbmplus.h
pbmtogem.obj :		pbmtogem.c pbm.h [-]pbmplus.h
pbmtogo.obj :		pbmtogo.c pbm.h [-]pbmplus.h
pbmtoicon.obj :		pbmtoicon.c pbm.h [-]pbmplus.h
pbmtolj.obj :		pbmtolj.c pbm.h [-]pbmplus.h
pbmtomacp.obj :		pbmtomacp.c macp.h pbm.h [-]pbmplus.h
pbmtomgr.obj :		pbmtomgr.c mgr.h pbm.h [-]pbmplus.h
pbmtopi3.obj :		pbmtopi3.c pbm.h [-]pbmplus.h
pbmtoplot.obj :		pbmtoplot.c pbm.h [-]pbmplus.h
pbmtoptx.obj :		pbmtoptx.c pbm.h [-]pbmplus.h
pbmtox10bm.obj :	pbmtox10bm.c pbm.h [-]pbmplus.h
pbmtoxbm.obj :		pbmtoxbm.c pbm.h [-]pbmplus.h
pbmtoybm.obj :		pbmtoybm.c pbm.h [-]pbmplus.h
pbmtozinc.obj :		pbmtozinc.c pbm.h [-]pbmplus.h
pbmupc.obj :		pbmupc.c pbm.h [-]pbmplus.h
pi3topbm.obj :		pi3topbm.c pbm.h [-]pbmplus.h
xbmtopbm.obj :		xbmtopbm.c pbm.h [-]pbmplus.h
ybmtopbm.obj :		ybmtopbm.c pbm.h [-]pbmplus.h
pbmtoln03.obj :		pbmtoln03.c pbm.h [-]pbmplus.h
pbmclean.obj :          pbmclean.c pbm.h [-]pbmplus.h
pbmpscale.obj :         pbmpscale.c pbm.h [-]pbmplus.h
pbmtoepsi.obj :         pbmtoepsi.c pbm.h [-]pbmplus.h
pbmtolps.obj :          pbmtolps.c pbm.h [-]pbmplus.h
pbmtopk.obj :           pbmtopk.c pbm.h [-]pbmplus.h
pktopbm.obj :           pktopbm.c pbm.h [-]pbmplus.h


# Binary dependencies, someone may want to build just a single image
atktopbm.exe :		atktopbm.obj $(PBMLIB)
brushtopbm.exe :	brushtopbm.obj $(PBMLIB)
cmuwmtopbm.exe :	cmuwmtopbm.obj $(PBMLIB)
g3topbm.exe :		g3topbm.obj $(PBMLIB)
gemtopbm.exe :		gemtopbm.obj $(PBMLIB)
icontopbm.exe :		icontopbm.obj $(PBMLIB)
macptopbm.exe :		macptopbm.obj $(PBMLIB)
mgrtopbm.exe :		mgrtopbm.obj $(PBMLIB)
pbmlife.exe :		pbmlife.obj $(PBMLIB)
pbmmake.exe :		pbmmake.obj $(PBMLIB)
pbmmask.exe :		pbmmask.obj $(PBMLIB)
pbmreduce.exe :		pbmreduce.obj $(PBMLIB)
pbmtext.exe :		pbmtext.obj $(PBMLIB)
pbmto10x.exe :		pbmto10x.obj $(PBMLIB)
pbmtoascii.exe :	pbmtoascii.obj $(PBMLIB)
pbmtoatk.exe :		pbmtoatk.obj $(PBMLIB)
pbmtobbnbg.exe :	pbmtobbnbg.obj $(PBMLIB)
pbmtocmuwm.exe :	pbmtocmuwm.obj $(PBMLIB)
pbmtoepson.exe :	pbmtoepson.obj $(PBMLIB)
pbmtog3.exe :		pbmtog3.obj $(PBMLIB)
pbmtogem.exe :		pbmtogem.obj $(PBMLIB)
pbmtogo.exe :		pbmtogo.obj $(PBMLIB)
pbmtoicon.exe :		pbmtoicon.obj $(PBMLIB)
pbmtolj.exe :		pbmtolj.obj $(PBMLIB)
pbmtomacp.exe :		pbmtomacp.obj $(PBMLIB)
pbmtomgr.exe :		pbmtomgr.obj $(PBMLIB)
pbmtopi3.exe :		pbmtopi3.obj $(PBMLIB)
pbmtoplot.exe :		pbmtoplot.obj $(PBMLIB)
pbmtoptx.exe :		pbmtoptx.obj $(PBMLIB)
pbmtox10bm.exe :	pbmtox10bm.obj $(PBMLIB)
pbmtoxbm.exe :		pbmtoxbm.obj $(PBMLIB)
pbmtoybm.exe :		pbmtoybm.obj $(PBMLIB)
pbmtozinc.exe :		pbmtozinc.obj $(PBMLIB)
pbmupc.exe :		pbmupc.obj $(PBMLIB)
pi3topbm.exe :		pi3topbm.obj $(PBMLIB)
xbmtopbm.exe :		xbmtopbm.obj $(PBMLIB)
ybmtopbm.exe :		ybmtopbm.obj $(PBMLIB)
pbmtoln03.exe :		pbmtoln03.obj $(PBMLIB)
pbmclean.exe :          pbmclean.obj $(PBMLIB)
pbmpscale.exe :         pbmpscale.obj $(PBMLIB)
pbmtoepsi.exe :         pbmtoepsi.obj $(PBMLIB)
pbmtolps.exe :          pbmtolps.obj $(PBMLIB)
pbmtopk.exe :           pbmtopk.obj $(PBMLIB)
pktopbm.exe :           pktopbm.obj $(PBMLIB)

# TeX documentation dependencies
atktopbm.tex :		atktopbm.1
brushtopbm.tex :	brushtopbm.1
cmuwmtopbm.tex :	cmuwmtopbm.1
g3topbm.tex :		g3topbm.1
gemtopbm.tex :		gemtopbm.1
icontopbm.tex :		icontopbm.1
macptopbm.tex :		macptopbm.1
mgrtopbm.tex :		mgrtopbm.1
pbmlife.tex :		pbmlife.1
pbmmake.tex :		pbmmake.1
pbmmask.tex :		pbmmask.1
pbmreduce.tex :		pbmreduce.1
pbmtext.tex :		pbmtext.1
pbmto10x.tex :		pbmto10x.1
pbmtoascii.tex :	pbmtoascii.1
pbmtoatk.tex :		pbmtoatk.1
pbmtobbnbg.tex :	pbmtobbnbg.1
pbmtocmuwm.tex :	pbmtocmuwm.1
pbmtoepson.tex :	pbmtoepson.1
pbmtog3.tex :		pbmtog3.1
pbmtogem.tex :		pbmtogem.1
pbmtogo.tex :		pbmtogo.1
pbmtoicon.tex :		pbmtoicon.1
pbmtolj.tex :		pbmtolj.1
pbmtomacp.tex :		pbmtomacp.1
pbmtomgr.tex :		pbmtomgr.1
pbmtopi3.tex :		pbmtopi3.1
pbmtoplot.tex :		pbmtoplot.1
pbmtoptx.tex :		pbmtoptx.1
pbmtox10bm.tex :	pbmtox10bm.1
pbmtoxbm.tex :		pbmtoxbm.1
pbmtoybm.tex :		pbmtoybm.1
pbmtozinc.tex :		pbmtozinc.1
pbmupc.tex :		pbmupc.1
pi3topbm.tex :		pi3topbm.1
xbmtopbm.tex :		xbmtopbm.1
ybmtopbm.tex :		ybmtopbm.1
pbmtoln03.tex :		pbmtoln03.1
pbmclean.tex :          pbmclean.1
pbmpscale.tex :         pbmpscale.1
pbmtoepsi.tex :         pbmtoepsi.1
pbmtolps.tex :          pbmtolps.1
pbmtopk.tex :           pbmtopk.1
pktopbm.tex :           pktopbm.1

clean :
	- Set Protection = Owner:RWED *.obj;*,*.*;-1
	- Purge /NoLog /NoConfirm *.*
	- Delete /NoLog /NoConfirm *.obj;
