
DSRC=src
DINC=src
DOBJ=obj
EXE=nw

CC=gcc5
CFLAGS=-Wall -g -I$(DINC)
LDFLAGS=-lm -lc


all: init $(EXE)

$(EXE):		$(DOBJ)/nw.o
	$(CC) $^ -o $(EXE) $(LDFLAGS)

init: $(DOBJ)

$(DOBJ):
	mkdir -p $(DOBJ)

$(DOBJ)/%.o: 	$(DSRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf $(DOBJ)/*.o
	rm $(EXE)



