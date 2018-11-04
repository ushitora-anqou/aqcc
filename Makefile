all:
	cd cc && make
	cd as && make
	cd ld && make

clean:
	cd cc && make clean
	cd as && make clean
	cd ld && make clean
