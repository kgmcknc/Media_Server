INCDIR = ./include
SRCDIR = ./src
OUTDIR = ./output

CC=gcc
CFLAGS=-I$(INCDIR)

DEPS = $(wildcard $(INCDIR)/*.h)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OUTDIR)/%.o,$(wildcard $(SRCDIR)/*.c))

$(OUTDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

web_control: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OUTDIR)/*.o *~ core $(INCDIR)/*~