#!/bin/sh

cp boost-build.jam.linux boost-build.jam
cp Jamroot.linux Jamroot

b2 -j 4 link=shared

#toolset=clang-linux-3.8.0
toolset=gcc-5.4.0

#cd ./libs
#doxygen
#cd ..

rm -v -f ./tools/hikyuu/*.so
rm -v -f ./tools/hikyuu/libboost*

cp -v -f ${BOOST_ROOT}/bin.v2/libs/python/build/${toolset}/release/libboost_python3.so.* ./tools/hikyuu/
cp -v -f ${BOOST_ROOT}/stage/lib/libboost_python.so.* ./tools/hikyuu/
cp -v -f ${BOOST_ROOT}/stage/lib/libboost_chrono.so.* ./tools/hikyuu/
cp -v -f ${BOOST_ROOT}/stage/lib/libboost_date_time.so.* ./tools/hikyuu/
cp -v -f ${BOOST_ROOT}/stage/lib/libboost_filesystem.so.* ./tools/hikyuu/
cp -v -f ${BOOST_ROOT}/stage/lib/libboost_system.so.* ./tools/hikyuu/
cp -v -f ${BOOST_ROOT}/stage/lib/libboost_serialization.so.* ./tools/hikyuu/

cp -v -f ./build/hikyuu/$toolset/release/libhikyuu.so ./tools/hikyuu
cp -v -f ./build/hikyuu_utils/$toolset/release/libhikyuu_utils.so ./tools/hikyuu

cp -v -f ./build/importdata/$toolset/release/importdata ./tools/hikyuu

cp -v -f ./build/hikyuu_python/$toolset/release/_hikyuu.so ./tools/hikyuu
cp -v -f ./build/hikyuu_python/$toolset/release/indicator/_indicator.so ./tools/hikyuu/indicator
cp -v -f ./build/hikyuu_python/$toolset/release/trade_manage/_trade_manage.so ./tools/hikyuu/trade_manage
cp -v -f ./build/hikyuu_python/$toolset/release/trade_sys/_trade_sys.so ./tools/hikyuu/trade_sys
cp -v -f ./build/hikyuu_python/$toolset/release/trade_instance/_trade_instance.so ./tools/hikyuu/trade_instance



cd ./test
b2 link=shared

python ../tools/hikyuu/test/test.py

cd ..




