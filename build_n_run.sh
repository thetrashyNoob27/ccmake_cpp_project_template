#!/usr/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${SCRIPT_DIR}";
if [ -d ".build" ]
then
#just cd to it.
:
else
rm -rv .build;
mkdir .build;
fi

cd .build;
cmake ..;
if [[ $? -ne 0 ]];
then 
echo "cmake fail";
exit;
else
clear;
fi

make;
if [[ $? -ne 0 ]];
then 
echo "build fail";
exit;
else
clear;
fi
./"cmake_cpp_project_template"  "$@";

