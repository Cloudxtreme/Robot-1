#
# Compiler settings
#
CCDEBUG = -g -DDEBUG -DUBUNTU
CC	= gcc
CFLAGS	= -Wall -DUBUNTU
LDFLAGS	= 
LDLIBS	= -lrt -lpthread


###############################################################################
#
# Source- and object-files
#

SRC	=	MasterControl.c ApiThread.c MainCtlThread.c TelnetThread.c \
		SubCtl_2Sonic.c SubCtl_2Motor.c
OBJ	=	$(SRC:.c=.o)
HDR	=	$(SRC:.c=.h)

PGM	=	MasterControl

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

forum:	$(SRC) $(HDR)
	tar cvzf /home/dirk/Raspi_forum/$(ARCHNAME) $(SRC) $(HDR) Makefile

