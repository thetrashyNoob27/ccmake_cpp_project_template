#!/usr/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${SCRIPT_DIR}";
if [ -d ".build" ]
then
    rm -rv .build;
fi
mkdir -p .build

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

