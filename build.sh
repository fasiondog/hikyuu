#!/bin/sh

cp boost-build.jam.linux boost-build.jam
cp Jamroot.linux Jamroot

bjam -j 4 link=shared

work_dir=/home/fasiondog/workspace/hikyuu
build_dir=$work_dir/build
toolset=clang-linux-3.3

#cd ./libs
#doxygen
#cd ..

cp -s -v -f $build_dir/hikyuu_python/$toolset/release/*.so $work_dir/tools/hikyuu
cp -s -v -f $build_dir/hikyuu_python/$toolset/release/indicator/*.so $work_dir/tools/hikyuu/indicator
cp -s -v -f $build_dir/hikyuu_python/$toolset/release/trade_manage/*.so $work_dir/tools/hikyuu/trade_manage
cp -s -v -f $build_dir/hikyuu_python/$toolset/release/trade_sys/*.so $work_dir/tools/hikyuu/trade_sys

cp -s -v -f $build_dir/hikyuu/$toolset/release/*.so $work_dir/bin
cp -s -v -f $build_dir/hikyuu_utils/$toolset/release/*.so $work_dir/bin

cp -s -v -f $work_dir/bin/*.so /home/fasiondog/lib

python $work_dir/tools/hikyuu/test/test.py

#cp -s -v -f $build_dir/hikyuu/$toolset/release/*.so $work_dir/tools/cstock/cstock
#cp -s -v -f $build_dir/cstock/$toolset/release/indicator/*.so $work_dir/tools/cstock/cstock/indicator
#cp -s -v -f $build_dir/cstock/$toolset/release/tradesys/*.so $work_dir/tools/cstock/cstock/tradesys
#cp -s -v -f $build_dir/cstock/$toolset/release/instance/*.so $work_dir/tools/cstock/cstock/instance



