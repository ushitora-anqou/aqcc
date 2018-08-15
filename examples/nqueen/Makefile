CC=../../aqcc_self

TARGET=nqueen
SRC=main.c
ASSEMBLES=$(SRC:.c=.s)

$(TARGET) : $(ASSEMBLES)
	gcc $^ -o $@

%.s : %.c
	$(CC) $< > $@

clean:
	rm -f $(TARGET) $(ASSEMBLES)

.PHONY: clean
