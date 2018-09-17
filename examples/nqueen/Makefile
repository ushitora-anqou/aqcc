# AQCC
AQCC=../../aqcc

TARGET=nqueen
SRC=main.c
OBJS=$(SRC:.c=.o)

$(TARGET) : $(SRC) ../../system.s ../../stdlib.c
	$(AQCC) $^  -o $@

clean:
	rm -f $(TARGET) $(ASSEMBLES) $(OBJS)

.PHONY: clean
