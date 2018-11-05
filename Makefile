all:
	cd cc && make
	cd as && make
	cd ld && make

test:
	cd test && make test

self_test:
	cd test && make self_test

selfself_test:
	cd test && make selfself_test

clean:
	cd cc && make clean
	cd as && make clean
	cd ld && make clean
	cd test && make clean

.PHONY: all test clean
