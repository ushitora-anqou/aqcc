# AQCC
AQCC=../../aqcc_detail

TARGET=nqueen
SRC=main.c
OBJS=$(SRC:.c=.o)

$(TARGET) : $(OBJS)
	gcc $^ -o $@

%.s : %.c
	$(AQCC) cs $< $@

%.o : %.s
	$(AQCC) so $< $@

clean:
	rm -f $(TARGET) $(ASSEMBLES)

.PHONY: clean
