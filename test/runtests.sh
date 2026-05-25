#!/bin/sh  # 添加Shebang，指定用sh解释器
echo "Running unit tests:"

for i in "$1"/*.bin  # 给$1加引号，处理路径含空格的情况
do  # 确保do紧跟在for循环行的下一行，且换行符为LF
    if test -f "$i"  # 给$i加引号，处理文件名含空格的情况
    then
        if ./"$i"
        then
            echo "$i PASS"
        else
            echo "ERROR in test $i:"
            exit 1
        fi
    fi
done

txtbld=$(tput bold)
echo "${txtbld}$(tput setaf 2)All unit tests passed.$(tput sgr0)"
