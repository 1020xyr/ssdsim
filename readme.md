**该版本的ssdsim将高级命令部分全部删去，如需要使用高级命令，请勿使用该版本**

由于未使用cache部分的代码，故ssdsim缓存部分的代码未做任何更改

src目录下是修复一些bug的ssdsim源码，trace目录下为常用的工作负载，program目录下是一些画图的python程序和shell脚本

运行方法

```bash
./ssd trace路径 生成文件路径
```

ssdsim原始版本为[https://github.com/jiangyu718/ssdsim](https://github.com/jiangyu718/ssdsim)，基于此版本的ssdsim进行了一些修改，详见修改说明。



可以使用以下这种方式将不同版本的实现代码分离开

```c
if (ssd->parameter->version == GC_OPTIMIZE) {
   //code
} else if (ssd->parameter->version == BASE) {
   //code
}
```



可以使用printf_sub_request_for_debug输出各个事务的信息，便于debug

```c
void printf_sub_request_for_debug(struct ssd_info *ssd, struct request *req)
{
    struct sub_request *sub;
    int64_t start_time, end_time;
    FILE *debug_file = fopen("debug.txt", "a+");	// 写入执行文件同目录的debug.txt中
    int flag = 1;
    sub = req->subs;
    while (sub != NULL)
    {
        if ((sub->current_state == SR_COMPLETE) || ((sub->next_state == SR_COMPLETE) && (sub->next_state_predict_time <= ssd->current_time))) // if any sub-request is not completed, the request is not completed
        {
            sub = sub->next_subs;
        }
        else
        {
            flag = 0;
            break;
        }
    }
    if (flag == 0)	// 若请求未完成
    {
        return;
    }

    sub = req->subs;
    while (sub != NULL)
    {
        struct local loc = *(sub->location);
        fprintf(debug_file, "lpn:%ld begin time:%ld complete time:%ld process time:%d\n", sub->lpn, sub->begin_time, sub->complete_time, sub->complete_time - sub->begin_time);
        fprintf(debug_file, "channel %d chip %d die %d plane %d block %d page %d\n", loc.channel, loc.chip, loc.die, loc.plane, loc.block, loc.page);
        sub = sub->next_subs;
    }
}
```



src目录下的result目录包含ssdsim执行test_trace负载的输出，可以运行一小段负载，测试ssdsim的准确性



 影响时间的因素有： 
    一：配置文件：t_RC  t_WC  t_PROG t_R t_BERS channel number等参数
    二：设备队列长度：p->queue_length=64;
    三：read_data中time size的放大倍数



如有任何问题，望请告知！

邮箱：r1523646952@163.com
