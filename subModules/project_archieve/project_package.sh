#!/usr/bin/bash
OUTPUT_H_NAME=project_archieve_data

test_command_exist()
{
    the_command=$1;
    if command -v "$the_command" >/dev/null 2>&1; then
        echo "$the_command is installed"
        return 0
    else
        echo "$the_command is not installed"
        return 1
    fi
}
# OUTPUT_H_NAME=test
create_head_file()
{
    cat << EOF > "${OUTPUT_H_NAME}".h
#ifndef _PROJECT_TAR_H_
#define _PROJECT_TAR_H_
#include <cstdint>
extern unsigned char test_h[];
extern unsigned char test_h_end;
#endif
EOF
    cat << EOF > "${OUTPUT_H_NAME}".cpp
#include "${OUTPUT_H_NAME}.h"
unsigned char test_h[] = {
$1
};
unsigned char test_h_end;

EOF
}
# OUTPUT_H_NAME=test
# create_head_file "create_head_file"
# cat test

ROOT_PROJECT_PATH=$1;
if [[ -z "${ROOT_PROJECT_PATH}" ]];
then
    echo "root project path empty.quiting.";
    create_head_file;
    exit;
else
    pass
fi

# ROOT_PROJECT_PATH="/home/peter/Documents/programming/c++/cmake_cpp_project_template"


test_command_exist xxd;
if [[ $? -eq 1 ]]; then
    create_head_file;
    exit;
fi
test_command_exist tar;
if [[ $? -eq 1 ]]; then
    create_head_file;
    exit;
fi
echo "this is bash script.";
tar_data=$(tar -cvf - --exclude=".build" --exclude=".vscode" --exclude="${OUTPUT_H_NAME}" -C "$(dirname ${ROOT_PROJECT_PATH})" "$(basename ${ROOT_PROJECT_PATH})"|xxd -i);
create_head_file "${tar_data}";
echo "tar header create to ${OUTPUT_H_NAME}";