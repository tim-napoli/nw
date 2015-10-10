
DSRC=src
DINC=src
DOBJ=obj
DTST=test
EXE=nw

CC=gcc
CFLAGS=-Wall -g -I$(DINC)
LDFLAGS=-lm -lc


all: init $(EXE) tests

$(EXE):		$(DOBJ)/nw.o
	$(CC) $^ -o $(EXE) $(LDFLAGS)

init: $(DOBJ) $(DTST)

$(DOBJ):
	mkdir -p $(DOBJ)

$(DTST):
	mkdir -p $(DTST)

$(DOBJ)/%.o: 	$(DSRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

tests:		$(DTST)/matrix.test

$(DTST)/%.test:	$(DSRC)/%.c
	$(CC) $(CFLAGS) -DTEST $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(DOBJ)/*.o
	rm -rf $(DTST)/*.test
	rm -f $(EXE)



