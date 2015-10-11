#
# Compiler settings
#
CCDEBUG = -g -DDEBUG
CC	= gcc
CFLAGS	= -Wall -DUBUNTU
LDFLAGS	= 
LDLIBS	= -lrt -lpthread


###############################################################################
#
# Source- and object-files
#

SRC	=	CommProc.c CommRs232.c AllControl.c SubCtl_2Sonic.c
OBJ	=	$(SRC:.c=.o)
HDR	=	$(SRC:.c=.h)

PGM	=	AllControl

ARCHNAME = $(PGM).src.tgz

###############################################################################
#
# Rules
#
# The Target Build

all:	$(PGM)

debug:	CFLAGS = $(CCDEBUG)
debug:	all

$(PGM):	$(OBJ)

$(OBJ):	$(HDR)

.PHONEY:	clean
clean:
	rm -f $(OBJ) $(PGM)

archiv:	$(SRC) $(HDR)
	tar cvzf $(ARCHNAME) $(SRC) $(HDR)

transfer:	archiv
	scp $(ARCHNAME) robot@pi-lcurr:$(ARCHNAME)

