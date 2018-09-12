#!/bin/sh

function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

./aqcc_detail test
[ $? -eq 0 ] || fail "./aqcc_detail test"

# compile testutil.o, stdlib.o and system.o
./aqcc_detail cs testutil.c _testutil.s
./aqcc_detail so _testutil.s _testutil.o
./aqcc_detail cs stdlib.c _stdlib.s
./aqcc_detail so _stdlib.s _stdlib.o
./aqcc_detail so system.s _system.o

./aqcc_detail cs test_define.c _test_define.s || fail "./aqcc_detail cs test_define.c _test_define.s"
./aqcc_detail so _test_define.s _test_define.o || fail "./aqcc_detail so _test_define.s _test_define.o"
#gcc _test_define.o _testutil.o _stdlib.o _system.o -o _test_define_exe.o -static -nostdlib
./aqcc_detail oe _test_define.o _testutil.o _stdlib.o _system.o _test_define_exe.o
chmod +x _test_define_exe.o
./_test_define_exe.o
[ $? -eq 0 ] || fail "./_test_define_exe.o"

gcc -E -P test.c -o _test.c
./aqcc_detail cs _test.c _test.s || fail "./aqcc_detail cs _test.c _test.s"
./aqcc_detail so _test.s _test.o || fail "./aqcc_detail so _test.s _test.o"
#gcc _test.o _testutil.o _stdlib.o _system.o -o _test_exe.o -static -nostdlib
./aqcc_detail oe _test.o _testutil.o _stdlib.o _system.o _test_exe.o
chmod +x _test_exe.o
./_test_exe.o
[ $? -eq 0 ] || fail "./_test_exe.o"

./aqcc_detail cs test_link.c _test_link_c.s
./aqcc_detail so _test_link_c.s _test_link_c.o
./aqcc_detail so test_link.s _test_link_s.o
./aqcc_detail oe _test_link_c.o _test_link_s.o _test_exe.o
chmod +x _test_exe.o
./_test_exe.o
[ $? -eq 5 ] || fail "./_test_exe.o (link)"
