# AQCC
AQCC=../../aqcc

TARGET=nqueen
SRC=main.c
ASSEMBLES=$(SRC:.c=.s)

$(TARGET) : $(ASSEMBLES)
	gcc $^ -o $@

%.s : %.c
	$(AQCC) $< > $@

clean:
	rm -f $(TARGET) $(ASSEMBLES)

.PHONY: clean
