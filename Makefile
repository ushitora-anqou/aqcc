test:
	cd cc && make test
	cd as && make test
	cd ld && make test

self_test:
	cd cc && make self_test
	cd as && make self_test
	cd ld && make self_test

selfself_test:
	cd cc && make selfself_test
	cd as && make selfself_test
	cd ld && make selfself_test

clean:
	cd cc && make clean
	cd as && make clean
	cd ld && make clean
