# AQCC
AQCC=../../aqcc

TARGET=nqueen
SRC=main.c
OBJS=$(SRC:.c=.o)

$(TARGET) : $(OBJS)
	gcc $^ -o $@

%.o : %.c
	$(AQCC) $< $@

clean:
	rm -f $(TARGET) $(ASSEMBLES)

.PHONY: clean
