#!/bin/bash

#####################################################
# Unix jenkins script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2013.03.16
#
# Description:
#
#    Generate lcov coverage reports.
#    Assumes all data has been downloaded by another script.
#
#
#####################################################

set -x
WORKSPACE=$1
CX_DEBUG_DIR=$WORKSPACE/working/CustusX3/build_Debug

# ==========================================================
# configure and build all libs with debug info
./working/CustusX3/CustusX3/install/Shared/script/cxInstaller.py --full --all -t Debug -j4 -s --isb_password=sintefsvn -u custusx --external_dir=$WORKSPACE/external --working_dir=$WORKSPACE/working --cmake_args="-DBUILD_DOCUMENTATION:BOOL=OFF" --coverage

# ==========================================================
# make the CustusX project in order to provoke a build failure.
cd $CX_DEBUG_DIR
make -j4
if [ $? == 0 ]
then
    echo "CustusX build success"
#    exit 0
else
    echo "CustusX build failure, terminating"
    exit 1
fi


# ==========================================================
# run lcov and generate html. Coverage info will now be in CX_DEBUG_DIR/
cd $WORKSPACE
./working/CustusX3/CustusX3/testing/coverage/cxCoverage.py --initialize --run_tests --ctest_args="-D ExperimentalTest --no-compress-output" --post_test $CX_DEBUG_DIR

# ==========================================================
# (this is done by the coverage.py script)
#
# Run all tests and write them in xml format to ./CTestResults.xml
#cd $CX_DEBUG_DIR
#rm -rf Testing/201*
#ctest -D ExperimentalTest --no-compress-output
#cp Testing/`head -n 1 Testing/TAG`/Test.xml ./CTestResults.xml

# ==========================================================
# run cppcheck
CX_SOURCE_DIR=$WORKSPACE/working/CustusX3/CustusX3
cppcheck --enable=all --xml-version=2 -i$CX_SOURCE_DIR/externals/ssc/Code/3rdParty/ $CX_SOURCE_DIR/source $CX_SOURCE_DIR/externals/ssc/Code/ 2> $WORKSPACE/cppcheck-result.xml

# ==========================================================
# run line counter
sloccount --duplicates --wide --details $CX_SOURCE_DIR >$WORKSPACE/sloccount_raw.sc
./working/CustusX3/CustusX3/testing/jenkins/clean_sloccount.py --remove="3rdParty/ config/ install/ /data/" $WORKSPACE/sloccount_raw.sc $WORKSPACE/sloccount.sc 

# post-op requirements:
#   - publish coverage data 
#   - publish ctest data using xUnit
#   - publish cppcheck and sloccount


