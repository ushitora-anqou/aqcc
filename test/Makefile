AQCC_CC=../cc/cc
AQCC_AS=../as/as
AQCC_LD=../ld/ld
AQCC_CC_SELF=bin/cc_self
AQCC_AS_SELF=bin/as_self
AQCC_LD_SELF=bin/ld_self
AQCC_CC_SELFSELF=bin/cc_selfself
AQCC_AS_SELFSELF=bin/as_selfself
AQCC_LD_SELFSELF=bin/ld_selfself
AQCC_ENV=\
		 AQCC_CC=$(realpath $(AQCC_CC))\
		 AQCC_AS=$(realpath $(AQCC_AS))\
		 AQCC_LD=$(realpath $(AQCC_LD))
AQCC_SELF_ENV=\
			  AQCC_CC=$(realpath $(AQCC_CC_SELF))\
			  AQCC_AS=$(realpath $(AQCC_AS_SELF))\
			  AQCC_LD=$(realpath $(AQCC_LD_SELF))
AQCC_SELFSELF_ENV=\
				  AQCC_CC=$(realpath $(AQCC_CC_SELFSELF))\
				  AQCC_AS=$(realpath $(AQCC_AS_SELFSELF))\
				  AQCC_LD=$(realpath $(AQCC_LD_SELFSELF))

test: $(AQCC_CC) $(AQCC_AS) $(AQCC_LD)
	$(AQCC_ENV) ./test.sh

self_test: $(AQCC_CC_SELF) $(AQCC_AS_SELF) $(AQCC_LD_SELF)
	$(AQCC_SELF_ENV) ./test.sh

selfself_test: $(AQCC_CC_SELFSELF) $(AQCC_AS_SELFSELF) $(AQCC_LD_SELFSELF)
	$(AQCC_SELFSELF_ENV) ./test.sh
	cmp $(AQCC_CC_SELF) $(AQCC_CC_SELFSELF)
	cmp $(AQCC_AS_SELF) $(AQCC_AS_SELFSELF)
	cmp $(AQCC_LD_SELF) $(AQCC_LD_SELFSELF)

$(AQCC_CC):
	cd ../cc && make

$(AQCC_AS):
	cd ../as && make

$(AQCC_LD):
	cd ../ld && make

$(AQCC_CC_SELF): $(AQCC_CC) $(AQCC_AS) $(AQCC_LD)
	mkdir -p bin
	cd ../cc && make CC=../aqcc FLAGS=-v TARGET=../test/$@ $(AQCC_ENV)

$(AQCC_AS_SELF): $(AQCC_CC) $(AQCC_AS) $(AQCC_LD)
	mkdir -p bin
	cd ../as && make CC=../aqcc FLAGS=-v TARGET=../test/$@ $(AQCC_ENV)

$(AQCC_LD_SELF): $(AQCC_CC) $(AQCC_AS) $(AQCC_LD)
	mkdir -p bin
	cd ../ld && make CC=../aqcc FLAGS=-v TARGET=../test/$@ $(AQCC_ENV)

$(AQCC_CC_SELFSELF): $(AQCC_CC_SELF) $(AQCC_AS_SELF) $(AQCC_LD_SELF)
	mkdir -p bin
	cd ../cc && make CC=../aqcc FLAGS=-v TARGET=../test/$@ $(AQCC_SELF_ENV)

$(AQCC_AS_SELFSELF): $(AQCC_CC_SELF) $(AQCC_AS_SELF) $(AQCC_LD_SELF)
	mkdir -p bin
	cd ../as && make CC=../aqcc FLAGS=-v TARGET=../test/$@ $(AQCC_SELF_ENV)

$(AQCC_LD_SELFSELF): $(AQCC_CC_SELF) $(AQCC_AS_SELF) $(AQCC_LD_SELF)
	mkdir -p bin
	cd ../ld && make CC=../aqcc FLAGS=-v TARGET=../test/$@ $(AQCC_SELF_ENV)

clean:
	rm -rf bin/
	rm -rf _test.c _test_define_exe.o  _test_exe.o

.PHONY: test self_test selfself_test
