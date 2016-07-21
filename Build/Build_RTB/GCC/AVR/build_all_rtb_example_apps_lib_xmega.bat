REM $Id: build_all_rtb_example_apps_lib_xmega.bat 34245 2013-02-11 12:32:24Z sschneid $

REM ***********************************************
REM Build all RTB Example Applications based on lib
REM ***********************************************


rm -f build_all_rtb_example_apps_lib.log
rm -f build_all_rtb_example_apps_lib.err


REM ***********************************************
REM RTB Examples based on lib
REM ***********************************************

CALL build_rtb_eval_app_lib_xmega.bat >> build_all_rtb_example_apps_lib.log 2>> build_all_rtb_example_apps_lib.err
CALL build_rtb_star_demo_lib_xmega.bat >> build_all_rtb_example_apps_lib.log 2>> build_all_rtb_example_apps_lib.err
CALL build_rtb_tal_app_lib_xmega.bat >> build_all_rtb_example_apps_lib.log 2>> build_all_rtb_example_apps_lib.err

