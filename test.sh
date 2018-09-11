#!/bin/sh

function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

./aqcc test
[ $? -eq 0 ] || fail "./aqcc test"

# compile testutil.o, stdlib.o and system.o
./aqcc cs testutil.c _testutil.s
./aqcc so _testutil.s _testutil.o
./aqcc cs stdlib.c _stdlib.s
./aqcc so _stdlib.s _stdlib.o
./aqcc so system.s _system.o

./aqcc cs test_define.c _test_define.s || fail "./aqcc cs test_define.c _test_define.s"
./aqcc so _test_define.s _test_define.o || fail "./aqcc so _test_define.s _test_define.o"
gcc _test_define.o _testutil.o _stdlib.o _system.o -o _test_define_exe.o -static -nostdlib
./_test_define_exe.o
[ $? -eq 0 ] || fail "./_test_define_exe.o"

gcc -E -P test.c -o _test.c
./aqcc cs _test.c _test.s || fail "./aqcc cs _test.c _test.s"
./aqcc so _test.s _test.o || fail "./aqcc so _test.s _test.o"
gcc _test.o _testutil.o _stdlib.o _system.o -o _test_exe.o -static -nostdlib
./_test_exe.o
[ $? -eq 0 ] || fail "./_test_exe.o"

./aqcc cs test_link.c _test_link_c.s
./aqcc so _test_link_c.s _test_link_c.o
./aqcc so test_link.s _test_link_s.o
./aqcc oe _test_link_c.o _test_link_s.o _test_exe.o
./_test_exe.o
[ $? -eq 5 ] || fail "./_test_exe.o (link)"
