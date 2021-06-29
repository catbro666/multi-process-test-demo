# 简介
This is a multi-process demo. You can use it as a simple performace test tool, and get statistic results.

这是一个多进程的demo测试程序。你可以用作简单的性能测试工具，统计测试结果。

你可以参考我的blog: [如何编写一个多进程性能测试程序](https://catbro666.github.io/posts/e416d910/)

# 源文件说明

- common.c common.c

封装错误判断及日志打印

- opt.c opt.h

命令行选项处理

- work.c work.h

具体测试相关代码

- multi-process.c

负责测试主控

# 添加自定义测试用例

1. 在`opt.c`和`opt.h`中，添加需要的命令行选项
2. 在`work.c`和`work.h`中，`global_init()`和`global_clean()`内进行全局的初始化及全局的清理工作
3. 在`work.c`和`work.h`中，添加自定义的测试函数，以及相应的测试参数和测试结果
4. 主控`multi-process.c`通过`test_init()`、`test_work()`、`test_clean()`调用测试相关的初始化、执行及清理工作
5. 主控`multi-process.c`中，修改测试结果的更新与统计操作。

主控`multi-process.c`中，`COMMON INIT`、`PARENT INIT`和`CHILD INIT`代码块处分别进行公共的初始化工作和父子进程特定的初始化工作。
`/* PARENT CLEANUP */`和`/* CHILD CLEANUP */`代码块处则分别进行对应的清理工作。
`/* DO YOUR WORK */`处执行具体的测试，`DO REAL-TIME STATISTICS`和`DO FINAL STATISTICS`处进行测试结果的统计。

