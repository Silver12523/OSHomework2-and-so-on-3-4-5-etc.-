#!/bin/bash
# 输出采用不同颜色表示状态，31m红是错误，32m绿是通过
# 初始化统计变量
pass=0
fail=0
fail_list=()

# 检查 osh 可执行文件是否存在
if [ ! -x "./osh" ]; then
    echo -e "\033[1;31m[ERROR] 当前目录未找到可执行文件 ./osh，请先编译：gcc osh.c -o osh\033[0m"
    exit 1
fi

# 临时文件配置
CMD_FILE="test_cmds.tmp"
OUT_FILE="osh_test_result.log"
rm -f $CMD_FILE $OUT_FILE

# 测试用例
cat > $CMD_FILE << EOF
!!
!1

ls
pwd
echo "OS Shell Test"
sleep 1 &
echo test1
echo test2
echo test3
echo test4
echo test5
echo test6
echo test7
echo test8
echo test9
echo test10
echo test11
history
!!
!9
!abc
!999
wrongcmd_123456
exit
EOF

echo "[测试启动] 正在运行 osh 并执行所有测试用例..."
echo ""

# 执行测试&捕获输出
./osh < $CMD_FILE > $OUT_FILE 2>&1

# 结果判断
echo "[测试结果明细]"

# 1. 启动与退出测试
if [ $? -eq 0 ]; then
    echo -e "\033[32m[PASS] Shell 启动成功 | exit 正常退出\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] Shell 异常退出/卡死\033[0m"
    ((fail++))
    fail_list+=("Shell 启动与退出")
fi

# 2. 基础命令ls测试
if grep -qi "osh.c" $OUT_FILE; then
    echo -e "\033[32m[PASS] 基础命令 ls 执行成功\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 基础命令 ls 执行失败\033[0m"
    ((fail++))
    fail_list+=("基础命令 ls")
fi

# 3. 后台命令 & 测试
if grep -E "\[.*\] running in background" $OUT_FILE; then
    echo -e "\033[32m[PASS] 后台运行 & 功能正常\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 后台运行 & 功能异常\033[0m"
    ((fail++))
    fail_list+=("后台运行功能")
fi

# 4. 无历史 !! 错误提示
if grep -qi "No commands in history." $OUT_FILE; then
    echo -e "\033[32m[PASS] 无历史时 !! 错误提示正常\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 无历史 !! 错误提示缺失\033[0m"
    ((fail++))
    fail_list+=("无历史 !! 错误提示")
fi

# 5. 历史命令存储+覆盖测试
if grep -qi "test11" $OUT_FILE; then
    echo -e "\033[32m[PASS] 历史命令10条覆盖机制正常\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 历史命令覆盖功能异常\033[0m"
    ((fail++))
    fail_list+=("历史命令覆盖机制")
fi

# 6. history 命令输出测试
if grep -E "[0-9]+ echo test" $OUT_FILE; then
    echo -e "\033[32m[PASS] history 命令正常输出\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] history 命令无输出\033[0m"
    ((fail++))
    fail_list+=("history 命令输出")
fi

# 7. !! 执行最近命令测试
if grep -qi "test11" $OUT_FILE; then
    echo -e "\033[32m[PASS] !! 执行最近命令成功\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] !! 功能异常\033[0m"
    ((fail++))
    fail_list+=("!! 执行功能")
fi

# 8. !N 执行指定命令测试
if grep -qi "echo test9" $OUT_FILE; then
    echo -e "\033[32m[PASS] !N 执行指定历史命令成功\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] !N 功能异常\033[0m"
    ((fail++))
    fail_list+=("!N 执行功能")
fi

# 9. 非法历史命令 !abc 测试
if grep -qi "Invalid history command." $OUT_FILE; then
    echo -e "\033[32m[PASS] 非法历史命令提示正常\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 非法历史命令未提示\033[0m"
    ((fail++))
    fail_list+=("非法历史命令提示")
fi

# 10. 超界 !999 错误提示
if grep -qi "No such command in history." $OUT_FILE; then
    echo -e "\033[32m[PASS] 超界历史命令提示正常\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 超界历史命令未提示\033[0m"
    ((fail++))
    fail_list+=("超界历史命令提示")
fi

# 11. 不存在命令错误处理测试
if grep -qi "execvp:" $OUT_FILE; then
    echo -e "\033[32m[PASS] 无效命令错误提示正常\033[0m"
    ((pass++))
else
    echo -e "\033[31m[FAIL] 无效命令未提示\033[0m"
    ((fail++))
    fail_list+=("无效命令错误提示")
fi

echo ""
echo "[测试用例总结]"
total=$((pass + fail))
echo "总测试用例数：$total"
echo "通过用例数：$pass"
echo "失败用例数：$fail"

if [ $fail -eq 0 ]; then
    echo -e "\033[32m全部测试用例通过！Shell 功能完全正常^_^\033[0m"
else
    echo "失败的测试项："
    for item in "${fail_list[@]}"; do
        echo "  - $item"
    done
fi

echo ""
echo -e "完整输出日志已保存到：$OUT_FILE"
echo -e "所有测试用例执行完成"

# 清理临时文件
rm -f $CMD_FILE
