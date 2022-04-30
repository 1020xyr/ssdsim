注释

```c
long int ftell(FILE *stream) 返回给定流 stream 的当前文件位置。
int fseek(FILE *stream, long offset, int fromwhere);函数设置文件指针stream的位置。
 
```

使用示例
将trace文件放在trace文件夹中，运行
```sh
sh run.sh
```

一个32位的整型数据的每一位代表一个子页，32/ssd->parameter->subpage_page就表示有多少页，这里的每一页的状态都存放在了 req->need_distr_flag中，也就是complt中，通过比较complt的每一项与full_page，就可以知道，这一页是否处理完成。如果没处理完成则通过creat_sub_request函数创建子请求。 

get_requests函数进行了两次feof(ssd->tracefile)判断，第二次判断是考虑了trace文件末尾有空行的情况，如果trace文件没有空行，那trace最后一行则不会读入，故这是程序兼容性考虑。
```c
if (feof(ssd->tracefile))
{
    request1 = NULL;
    return 0;
}
```
修改日志

一：trace读取格式问题

```c
sscanf(buffer_request,"%lld %d %d %d %d",&time,&device,&lsn,&size,&ope);
修改成
device = 0;
int result = read_data(buffer, &time_t, &lsn, &size, &ope);  

在pagemap.h pagemap.c中声明并实现read函数
在pre_process_page函数，get_request函数中修改
```

二：make_aged函数增加有效页

```c
struct ssd_info *make_aged(struct ssd_info *ssd)
{
    unsigned int i, j, k, l, m, n, ppn;
    int threshould, flag = 0;

    //一个块中的有效页数无效页占比 20%
    int vaild_page_ratio = 5;
    //有效页的逻辑页号
    int crt_lpn = 0;
    int lpn, valid_state;
    int rand_value;
    int large_lpn = (ssd->parameter->chip_num * ssd->parameter->die_chip * ssd->parameter->plane_die * ssd->parameter->block_plane * ssd->parameter->page_block) * (1 - ssd->parameter->overprovide);
    if (ssd->parameter->aged == 1)
    {
        //threshold表示一个plane中有多少页需要提前置为失效或有效
        threshould = (int)(ssd->parameter->block_plane * ssd->parameter->page_block * ssd->parameter->aged_ratio);
        for (i = 0; i < ssd->parameter->channel_number; i++)
            for (j = 0; j < ssd->parameter->chip_channel[i]; j++)
                for (k = 0; k < ssd->parameter->die_chip; k++)
                    for (l = 0; l < ssd->parameter->plane_die; l++)
                    {
                        flag = 0;
                        for (m = 0; m < ssd->parameter->block_plane; m++)
                        {
                            if (flag >= threshould)
                            {
                                break;
                            }
                            for (n = 0; n < ssd->parameter->page_block; n++)
                            {
                                rand_value = rand() % 10;
                                //printf("%d\n",rand_value);
                                if (rand_value < vaild_page_ratio)
                                {
                                    lpn = get_lpn_for_make_aged(ssd,large_lpn);
                                    valid_state = ~(0xffffffff << (ssd->parameter->subpage_page));
                                    ssd->dram->map->map_entry[lpn].pn = find_ppn(ssd, i, j, k, l, m, n);
                                    ssd->dram->map->map_entry[lpn].state = ~(0xffffffff << (ssd->parameter->subpage_page));
                                }
                                else{
                                    lpn = 0;
                                    valid_state = 0;
                                    ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].invalid_page_num++;
                                }
                                flag++;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].valid_state = valid_state;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].free_state = 0;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].lpn = lpn; //把valid_state free_state lpn都置为0表示页失效，检测的时候三项都检测，单独lpn=0可以是有效页
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_page_num--;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_page++;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].free_page--;                                
                                ppn = find_ppn(ssd, i, j, k, l, m, n);   
                            }
                            int tmp = 0;        //gdb断点测试
                            //printf("%d\n",ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].invalid_page_num);                           
                        }
                    }
    }
    else
    {
        return ssd;
    }

    return ssd;
}
int get_lpn_for_make_aged(struct ssd_info * ssd,int max_lpn){
    int lpn;
    lpn = rand() % max_lpn + 1;
    while(ssd->dram->map->map_entry[lpn].pn!=0){
        lpn = rand() % max_lpn;
    }
    return lpn;
}

```
随机数生成问题：[blog](https://blog.csdn.net/u014520797/article/details/88895514)

srand()函数是初始化随机数产生器，它产生随机数种子，说白了就是初始化随机数。
srand((unsigned)time(NULL));取的是系统时间，也就是距离1970.1.1午夜有多少秒。
而for循环每循环一次时间远远小于1秒，这就导致了srand((unsigned)time(NULL))产生的种子并没有改变。 那么，相同的随机数种子所产生的随机数肯定是一样的了。
每初始化一次相同的随机数种子srand，产生的rand随机数组成序列是一样的
lpn设置成随机选取而不是从0开始一直加一

三： 支持SLC，MLC，TLC

```c
//设置TLC参数
struct ac_time_characteristics{
    int trLSB;      // MLC/TLC SSD
    int trMSB;
    int trCSB;
    int tpLSB;
    int tpMSB;
    int tpCSB;
};
struct parameter_value{
    //增加的参数
    int flash_type; 

};
//读取这些参数
// in load_parameters function
else if((res_eql=strcmp(buf,"flash type")) ==0){
    sscanf(buf + next_eql,"%d",&p->flash_type); 
    printf("test  type:%d\n",p->flash_type);
}
else if((res_eql=strcmp(buf,"t_rLSB")) ==0){
    sscanf(buf + next_eql,"%d",&p->time_characteristics.trLSB); 
    printf("time:%d\n",p->time_characteristics.trLSB);

}else if((res_eql=strcmp(buf,"t_rMSB")) ==0){
    sscanf(buf + next_eql,"%d",&p->time_characteristics.trMSB); 

}else if((res_eql=strcmp(buf,"t_rCSB")) ==0){
    sscanf(buf + next_eql,"%d",&p->time_characteristics.trCSB); 

}else if((res_eql=strcmp(buf,"t_pLSB")) ==0){
    sscanf(buf + next_eql,"%d",&p->time_characteristics.tpLSB); 

}else if((res_eql=strcmp(buf,"t_pMSB")) ==0){
    sscanf(buf + next_eql,"%d",&p->time_characteristics.tpMSB); 

}else if((res_eql=strcmp(buf,"t_pCSB")) ==0){
    sscanf(buf + next_eql,"%d",&p->time_characteristics.tpCSB); 
} 

//用新的读写时间代替之前的ssd->parameter->time_characteristics.tR和ssd->parameter->time_characteristics.tPROG

//在pagemap.h pagemap.c中声明并定义读写时间方法
int get_read_time(struct ssd_info *ssd, int page_id)
{
    int t_R;
    if (ssd->parameter->flash_type == SLC)
    {
        t_R = ssd->parameter->time_characteristics.tR;
    }
    else if (ssd->parameter->flash_type == MLC)
    {
        if (page_id % 2 == 0)
        {
            t_R = ssd->parameter->time_characteristics.trLSB;
        }
        else
        {
            t_R = ssd->parameter->time_characteristics.trMSB;
        }
    }
    else if (ssd->parameter->flash_type == TLC)
    {
        if (page_id % 3 == 0)
        {
            t_R = ssd->parameter->time_characteristics.trLSB;
        }
        else if (page_id % 3 == 1)
        {
            t_R = ssd->parameter->time_characteristics.trCSB;
        }
        else
        {
            t_R = ssd->parameter->time_characteristics.trMSB;
        }
    }
    else
    {
        printf("error flash type\n");
    }
    return t_R;
}

int get_prog_time(struct ssd_info *ssd, int page_id)
{
    int t_PROG;
    if (ssd->parameter->flash_type == SLC)
    {
        t_PROG = ssd->parameter->time_characteristics.tPROG;
    }
    else if (ssd->parameter->flash_type == MLC)
    {
        if (page_id % 2 == 0)
        {
            t_PROG = ssd->parameter->time_characteristics.tpLSB;
        }
        else
        { 
            t_PROG = ssd->parameter->time_characteristics.tpMSB;
        }
    }
    else if (ssd->parameter->flash_type == TLC)
    {
        if (page_id % 3 == 0)
        {
            t_PROG = ssd->parameter->time_characteristics.tpLSB;
        }
        else if (page_id % 3 == 1)
        {
            t_PROG = ssd->parameter->time_characteristics.tpCSB;
        }
        else
        {
            t_PROG = ssd->parameter->time_characteristics.tpMSB;
        }
    }
    else
    {
        printf("error flash type\n");
    } 
    return t_PROG;  
}
// 替换旧的时间测量方式 copyback高级命令相关的时间均未修改
// 已修改
static_write:页号计算应于get_ppn之后
get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub);
page_id = sub->location->page;
t_PROG = get_prog_time(ssd, page_id);
go_one_step command==NORMAL分支    
uninterrupt_gc
修改move_page函数，返回写入页的页号
Status move_page(struct ssd_info *ssd, struct local *location, unsigned int *transfer_size,int* page_id);    
```

四：修改固定的页数 63

```c
ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page>63
修改成
ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page > ssd->parameter->page_block    
```

五： trace末尾时间驱动问题

在将p->queue_length从5改成64后，由于simulate函数在trace文件读取完后ssd->current_time不变，无法处理设备队列中的请求，if (flag == 0 && ssd->request_queue == NULL)永远不会满足，故程序陷入死循环。

```c
//修改 即使读完trace文件依然推动
get_request函数中
if (feof(ssd->tracefile)){
    nearest_event_time = find_nearest_event(ssd);
    if(nearest_event_time!=MAX_INT64){
            ssd->current_time = nearest_event_time;
    }  
    return 0;
}
```
六：static_write函数中更新读子请求重复计算
```c
// allocate_location
if (ssd->dram->map->map_entry[sub_req->lpn].state != 0)
{ /*这个写回的子请求的逻辑页不可以覆盖之前被写回的数据 需要产生读请求*/
    if ((sub_req->state & ssd->dram->map->map_entry[sub_req->lpn].state) != ssd->dram->map->map_entry[sub_req->lpn].state)
    {
        ssd->read_count++;
        ssd->update_read_count++;
        update = (struct sub_request *)malloc(sizeof(struct sub_request));
        alloc_assert(update, "update");
        memset(update, 0, sizeof(struct sub_request));

        if (update == NULL)
        {
            return ERROR;
        }
        update->location = NULL;
        update->next_node = NULL;
        update->next_subs = NULL;
        update->update = NULL;
        location = find_location(ssd, ssd->dram->map->map_entry[sub_req->lpn].pn);
        update->location = location;
        update->begin_time = ssd->current_time;
        update->current_state = SR_WAIT;
        update->current_time = MAX_INT64;
        update->next_state = SR_R_C_A_TRANSFER;
        update->next_state_predict_time = MAX_INT64;
        update->lpn = sub_req->lpn;
        update->state = ((ssd->dram->map->map_entry[sub_req->lpn].state ^ sub_req->state) & 0x7fffffff);
        update->size = size(update->state);
        update->ppn = ssd->dram->map->map_entry[sub_req->lpn].pn;
        update->operation = READ;

        if (ssd->channel_head[location->channel].subs_r_tail != NULL)
        {
            ssd->channel_head[location->channel].subs_r_tail->next_node = update;
            ssd->channel_head[location->channel].subs_r_tail = update;
        }
        else
        {
            ssd->channel_head[location->channel].subs_r_tail = update;
            ssd->channel_head[location->channel].subs_r_head = update;
        }
    }
allocate_location函数中已经生成了相应的读子请求，static_write函数中就不应该再计算一次读请求时间，而是效仿go_one_step中的方法，在处理写请求时先判断对应的读子请求是否完成
//services_2_write 
 while (sub != NULL)
{
    if ((sub->current_state == SR_WAIT) && (sub->location->channel == channel) && (sub->location->chip == chip) && (sub->location->die == die)) /*该子请求就是当前die的请求*/
    {
        //增加对更新读子请求的判断
        if (sub->update != NULL) /*如果有需要提前读出的页*/
        {
            if ((sub->update->current_state == SR_COMPLETE) || ((sub->update->next_state == SR_COMPLETE) && (sub->update->next_state_predict_time <= ssd->current_time))) //被更新的页已经被读出
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    sub = sub->next_node;
} 
//static_write
// 在生成写请求时已经计算生成了update读请求，此时为冗余计算，删去
sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC + (sub->size * ssd->parameter->subpage_capacity) * ssd->parameter->time_characteristics.tWC;
sub->complete_time = sub->next_state_predict_time;
time = sub->complete_time;
```
七：错误的写操作时间计算
t_PROG应该赋值给sub->next_state_predict_time而不是ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time
```c
sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC + (sub->size * ssd->parameter->subpage_capacity) * ssd->parameter->time_characteristics.tWC+ t_PROG;
sub->complete_time = sub->next_state_predict_time;
time = sub->complete_time;

ssd->channel_head[location->channel].current_state = CHANNEL_TRANSFER;
ssd->channel_head[location->channel].current_time = ssd->current_time;
ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
ssd->channel_head[location->channel].next_state_predict_time = time;

ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_WRITE_BUSY;
ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_IDLE;
// ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=time+ssd->parameter->time_characteristics.tPROG;
ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = time;
```
八：get_requests函数末尾的无用计算，删去。ssd->next_request_time 没用过
```c
filepoint = ftell(ssd->tracefile);
fgets(buffer, 200, ssd->tracefile); //寻找下一条请求的到达时间
//sscanf(buffer,"%lld %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
device = 0;
result = read_data(buffer, &time_t, &lsn, &size, &ope);
ssd->next_request_time = time_t;
fseek(ssd->tracefile, filepoint, 0);
```
九：统计gc
```c
struct ssd_info{
    unsigned long gc_count;              //垃圾回收次数
    unsigned long gc_page_move_count;
    unsigned long update_write_count;
    int64_t gc_sum_time;           //gc操作总时间
}
//uninterrupt_gc    
ssd->gc_sum_time += page_move_count * (7 * ssd->parameter->time_characteristics.tWC + 7 * ssd->parameter->time_characteristics.tWC) + transfer_size * SECTOR * (ssd->parameter->time_characteristics.tWC + ssd->parameter->time_characteristics.tRC) + time_R_AND_PROG + ssd->parameter->time_characteristics.tBERS;
ssd->gc_count++;
ssd->gc_page_move_count += page_move_count;
//get_ppn
ssd->update_write_count++;      //记录更新写次数
//statistic_output
fprintf(ssd->statisticfile, "update write count: %lld\n",ssd->update_write_count);
if(ssd->gc_count!=0){
    fprintf(ssd->statisticfile, "gc average response time: %lld\n", ssd->gc_sum_time / ssd->gc_count);
    fprintf(ssd->statisticfile, "gc count: %13d\n", ssd->gc_count); 
    fprintf(ssd->statisticfile, "gc average page move count: %13d\n", ssd->gc_page_move_count / ssd->gc_count);        
}
```
十：修改时间测量方式
```c
ssd->read_avg = ssd->read_avg + (req->response_time - req->time);
ssd->read_avg = ssd->read_avg + (end_time - req->time);
改至
ssd->read_avg = ssd->read_avg + (req->response_time - req->begin_time);
ssd->read_avg = ssd->read_avg + (end_time - start_time);
```
十一：gc时空闲页计算方式、
```c
free_page = 0;
unsigned int mask=~(0xffffffff<<(ssd->parameter->subpage_page)); // 每1 bit表示 1个subpage
for (i = 0; i < ssd->parameter->page_block; i++) /*逐个检查每个page，如果有有效数据的page需要移动到其他地方存储*/
{
    if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state & PG_SUB) == mask)
    {
        free_page++;
        printf("page number %d\n",i);
    }
```

十二：更好调整gc阈值
```c
// 0时代表不需要gc，不为0则动态调整gc阈值
if(ssd->parameter->gc_hard_threshold!=0){
    float new_gc_hard = (ssd->channel_head[0].chip_head[0].die_head[0].plane_head[0].free_page - 20);
    ssd->parameter->gc_hard_threshold = new_gc_hard  / (ssd->parameter->page_block * ssd->parameter->block_plane);       
}
printf("gc hard threshold=%lf  free page:%lf",ssd->parameter->gc_hard_threshold, ssd->parameter->page_block * ssd->parameter->block_plane * ssd->parameter->gc_hard_threshold);
```
十三：gc时读写队列判断
```c
// gc函数中
//注释掉这一段
if ((ssd->parameter->allocation_scheme == 1) || ((ssd->parameter->allocation_scheme == 0) && (ssd->parameter->dynamic_allocation == 1)))
{
    if ((ssd->channel_head[channel].subs_r_head != NULL) || (ssd->channel_head[channel].subs_w_head != NULL)) /*队列上有请求，先服务请求*/
    {
        return 0;
    }
}
```
小bug
```c
//trace里没有读或写
if(ssd->read_request_count!=0){
    fprintf(ssd->outputfile, "read request average response time: %lld\n", ssd->read_avg / ssd->read_request_count);
}else{
    printf("read count=0\n");
}
if(ssd->write_request_count!=0){
    fprintf(ssd->outputfile, "write request average response time: %lld\n", ssd->write_avg / ssd->write_request_count);
}else{
    printf("write count=0\n");
}
//statistic_output函数输出格式
fprintf(ssd->outputfile, "the %d channel, 0 chip, %d die, %d plane has : %13d erase operations\n", i, j, k,plane_erase);
fprintf(ssd->statisticfile, "the %d channel, 0 chip, %d die, %d plane has : %13d erase operations\n", i, j, k,plane_erase);
```

**实验部分**

一：增加page_allocation参数，指示使用QDS+US 还是QDS+UBS(不使用)  每个plane记录上次分配的gc page类型

```c
else if((res_eql=strcmp(buf,"page_allocation")) ==0){
    sscanf(buf + next_eql,"%d",&p->page_allocation); 
    printf("page allocation: %d\n",p->page_allocation);
} 
struct plane_info{
    int add_reg_ppn;                    //read，write时把地址传送到该变量，该变量代表地址寄存器。die由busy变为idle时，清除地址 //有可能因为一对多的映射，在一个读请求时，有多个相同的lpn，所以需要用ppn来区分  
    unsigned int free_page;             //该plane中有多少free page
    unsigned int ers_invalid;           //记录该plane中擦除失效的块数
    unsigned int active_block;          //if a die has a active block, 该项表示其物理块号
    int can_erase_block;                //记录在一个plane中准备在gc操作中被擦除操作的块,-1表示还没有找到合适的块
    struct direct_erase *erase_node;    //用来记录可以直接删除的块号,在获取新的ppn时，每当出现invalid_page_num==64时，将其添加到这个指针上，供GC操作时直接删除
    struct blk_info *blk_head;

    //增加的参数
    int last_gc_page_type;              //上次gc分配的页类型
};

```

二：修改move函数  get_ppn_for_gc参数，增加page_type参数

```c
Status move_page(struct ssd_info *ssd, struct local *location, int page_type, unsigned int *transfer_size,int* page_id);
unsigned int get_ppn_for_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, int page_type);
```

三：增加get_next_write_page_for_gc函数，计算下一次的write page页号

```c
//gc时write page一次增加3，保持其page类型不变
int get_next_write_page_for_gc(struct ssd_info *ssd,int last_write_page){
    if(last_write_page==-1) return 0;               //初始时为-1
    int page_id = last_write_page + 3;
    if(page_id>=ssd->parameter->page_block){
        page_id = last_write_page%3 + 1;
        if(page_id==ERRORTYPE){
            return -1;
        }
    }
    return page_id;
}
```
四：修改find_active_block函数，使其只能在限定范围寻找active_block
```c
Status find_active_block(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane)
{
    unsigned int active_block;
    unsigned int free_page_num = 0;
    unsigned int count = 0;
    int block_size_for_write = ssd->parameter->block_plane - gc_block_num;

    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
    free_page_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
    // last_write_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
    while ((free_page_num == 0) && (count < block_size_for_write))
    {
        active_block = (active_block + 1) % block_size_for_write;
        free_page_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
        count++;
    }
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block = active_block;
    if (count < block_size_for_write)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}
```

