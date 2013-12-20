############################################################################
# This code was compiled using                                             #
#    IBM CSET/2 at CSD level 046                                           #
#    OS/2 2.0 toolkit                                                      #
#    IBM TCP/IP 1.2.1 w/ current CSD's                                     #
#      Base:        UN37938                                                #
#      Programming: UN37942                                                #
#                                                                          #
# I make no claims as to compatibility with any other systems.             #
############################################################################
OSTYPE       = OS2
ARCHITECTURE = i486
COMPILER     = CSET2

CC    = icc
ICC   =
COPTS = -Sp2 -Q+ -N30 -W3 -Sm -Kb+ -G4
LOPTS = -B"/NOI"
DEFS  = -D$(OSTYPE) -D$(ARCHITECTURE) -D$(COMPILER)
LIBS  = tcpipdll.lib

# Inference rules
.SUFFIXES:
.SUFFIXES: .obj .c

.c.obj:
	$(CC) $(COPTS) $(DEFS) -C+ -Fo$@ $<

# ---------------------------------------------------------------------------
nistime.exe : nistime.obj 
	$(CC) $(COPTS) $(DEFS) $(LOPTS) -Fe$@ $** $(LIBS)
