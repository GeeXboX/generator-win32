######################################
# GeeXboX Win32 GUI Generator        #
# Zores Benjamin - (C) 2003	     #
######################################

CC       = i586-mingw32msvc-gcc
WRES     = i586-mingw32msvc-windres
STRIP    = i586-mingw32msvc-strip
UPX	 = upx

APP      = generator
EXEFILE  = $(APP).exe
OBJFILE  = $(APP).o
COFFFILE = $(APP).coff
SRCFILE  = $(APP).c langconf.c
RESFILE  = $(APP).rc

CFLAGS	 = -Wall -Os

all:	coff $(SRCFILE)
	$(CC) $(CFLAGS) $(COFFFILE) $(SRCFILE) -o $(EXEFILE)
	$(STRIP) $(EXEFILE)
	$(UPX) --best --crp-ms=999999 --nrv2b $(EXEFILE)
	rm -f $(COFFFILE)

coff:	$(RESFILE)
	$(WRES) $(RESFILE) -O coff $(COFFFILE)

clean:
	rm -f $(COFFFILE) $(EXEFILE)
