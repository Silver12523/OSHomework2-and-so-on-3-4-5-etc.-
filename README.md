# OSHomework2-and-so-on-3-4-5-etc.-
simple sudoku verification using pthread and Openkylin

## OSHomework2 P & S
shell_test.sh是测试脚本，在终端中输出。osh_test_result是osh的输出日志，另存为文件，见群中图。
### P1
这次使用了bash脚本做测试，刚开始将注释写在了EOF块内部，如下:
```c
# 1. 基础空行测试

# 2. 基础外部命令
ls
pwd
echo "OS Shell Test"

# 3. 后台运行命令 & 测试
sleep 1 &

# 4. 初始无历史测试 !!
!!

# 5. 初始无历史测试 !N
!1

# 6. 写入测试命令
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

# 7. 测试历史满10条后，第11条覆盖
echo test11

# 8. 查看历史
history

# 9. 测试 !! 执行最后一条命令
!!

# 10. 测试 !N 执行指定历史命令
!5

# 11. 测试非法历史命令
!abc

# 12. 测试超界历史命令
!999

# 13. 测试不存在的外部命令
wrongcmd_123456

# 14. 退出 Shell
exit
EOF
```

结果出现了大量execvp: No such file or directory报错和中文乱码，且部分测试用例判断失败。这是因为EOF中间的所有内容都会被写入命令文件，osh无法识别注释，会把这些内容当成无效命令执行，引发报错。后面就把注释删掉乐...

### P2
删去注释后，重新进行测试，发现结果有问题：
终端输出：
```
[PASS] Shell 启动成功 | exit 正常退出
[FAIL] 基础命令 ls 执行失败
osh> [3256] running in background
[3257] running in background
[PASS] 后台运行 & 功能正常
[FAIL] 无历史 !! 错误提示缺失
[PASS] 历史命令10条覆盖机制正常
osh> 8 echo test2
9 echo test3
10 echo test4
11 echo test5
12 echo test6
13 echo test7
14 echo test8
15 echo test9
16 echo test10
17 echo test11
[PASS] history 命令正常输出
[PASS] !! 执行最近命令成功
[FAIL] !N 功能异常
[PASS] 非法历史命令提示正常
[PASS] 超界历史命令提示正常
[PASS] 无效命令错误提示正常

完整输出日志已保存到：osh_test_result.log
所有测试用例执行完成
```
出问题了！但经过分析发现不是osh代码的问题，全是测试代码的问题（...干笑)具体如下：
#### P2.1.[FAIL] 基础命令 ls 执行失败
我查看了当时输出的log，ls命令执行成功了，屏幕输出了文件列表，终端也输出了后台进程[3256] running in background，这里显示错误是因为当时判断ls是否成功的代码有问题。
```c
if grep -qi "total" $OUT_FILE; then
    echo "[PASS] ls 成功"
else
    echo "[FAIL] ls 失败"
fi
```
total只有在ls -l才会输出，而我只执行了ls，输出只有文件名，这是我没搞清楚指令导致脚本判断条件写错了。改成这个就好啦：
```c
if grep -qi "osh.c" $OUT_FILE; then
```
#### P2.2.[FAIL] 无历史 !! 错误提示缺失
这一段的脚本是这样的：
```c
ls               # 先执行命令
pwd
echo "OS Shell Test"
sleep 1 &
!!               # 后执行!!
!1
```
呃，我们可以看到脚本先执行了ls/pwd/echo等命令，已经生成了历史记录，因此!!无法触发No commands in history错误提示，脚本grep不到关键字，判定失败。只要调整命令顺序，把!!/!1放在所有命令最前面，先触发无历史场景就好啦。

所以说不能想到哪个场景就写哪个场景，要先整理好它们之间的逻辑关系才行...
#### P2.3.[FAIL] !N 功能异常
这里调用的是!5，但这个适配的是某个之前的版本，查阅log发现现版本历史是8-17，如下：
```
osh> 8 echo test2
9 echo test3
10 echo test4
11 echo test5
12 echo test6
13 echo test7
14 echo test8
15 echo test9
16 echo test10
17 echo test11
[PASS] history 命令正常输出
[PASS] !! 执行最近命令成功
[FAIL] !N 功能异常
```
所以修改为!9。感觉很若只但我真的犯了这样的错所以还是写了哈哈（挠头）
之后测试就顺利通过了库啵~
