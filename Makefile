CSPICETOP=$(shell ( [ -d cspice/. ] && echo cspice ) || ( [ -d $(HOME)/cspice/. ] && echo $(HOME)/cspice ) || ( [ -d /usr/local/cspice/. ] && echo /usr/local/cspice ) || echo CSPICE_DIRECTORY_NOT_FOUND)
CPPFLAGS=-I$(CSPICETOP)/include
LDLIBS=$(CSPICETOP)/lib/cspice.a -lm

EXES=smrc_000

all: $(EXES)

clean:
	$(RM) $(EXES)
