# Needleman-Wunsch Makefile

#------------------ Working directories ------------------#
DSRC=src
DINC=src
DOBJ=obj
DTST=test
DPROTO=prototype
EXE=nw

#------------------ Compilation options ------------------#
CC=gcc
CFLAGS=-std=gnu99 -fopenmp -Wall -O3 -g -I$(DINC)

ifeq ($(SYS),freebsd)
	LDFLAGS=-lm -lc -rpath=/usr/local/lib/gcc5 -lgomp
else
	LDFLAGS=-lm -lc -lgomp
endif
	


#--------------------- Main rules ------------------------#
all: init $(EXE) tests prototypes

$(EXE):		$(DOBJ)/main.o				\
		$(DOBJ)/matrix.o			\
		$(DOBJ)/nw.o				\
		$(DOBJ)/alignment.o			\
		$(DOBJ)/bench.o             \
		$(DOBJ)/validate.o       
	$(CC) $^ -o $(EXE) $(LDFLAGS)

$(DOBJ)/%.o: 	$(DSRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


#------------------------- Tests -------------------------#
tests:		$(DTST)/matrix.test

$(DTST)/%.test:	$(DSRC)/%.c
	$(CC) $(CFLAGS) -DTEST $^ -o $@ $(LDFLAGS)

#----------------------- Prototypes ----------------------#
prototypes:	$(DPROTO)/ex_tim.proto			\
		$(DPROTO)/ex_tim_map.proto

$(DPROTO)/%.proto:	$(DPROTO)/%.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(DPROTO)/ex_tim_map.proto:	$(DPROTO)/ex_tim.c
	$(CC) $(CFLAGS) -DMAP $^ -o $@ $(LDFLAGS)

#--------------------- Utils rules -----------------------#
init: $(DOBJ) $(DTST)

$(DOBJ):
	mkdir -p $(DOBJ)

$(DTST):
	mkdir -p $(DTST)

clean:
	rm -rf $(DOBJ)/*.o
	rm -rf $(DTST)/*.test
	rm -rf $(DPROTO)/*.proto
	rm -f $(EXE)



