/*****************************************************************************************************************************
  This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
  Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

  FileName： pagemap.h
Author: Hu Yang		Version: 2.1	Date:2011/12/02
Description:

History:
<contributor>     <time>        <version>       <desc>                   <e-mail>
Yang Hu	        2009/09/25	      1.0		    Creat SSDsim       yanghu@foxmail.com
2010/05/01        2.x           Change
Zhiming Zhu     2011/07/01        2.0           Change               812839842@qq.com
Shuangwu Zhang  2011/11/01        2.1           Change               820876427@qq.com
Chao Ren        2011/07/01        2.0           Change               529517386@qq.com
Hao Luo         2011/01/01        2.0           Change               luohao135680@gmail.com
 *****************************************************************************************************************************/

#define _CRTDBG_MAP_ALLOC

#include "pagemap.h"

// trace read function
// 1126620 3693 kjournald 12672 8 W 7 0 5c91c9a46
int read_ndata(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope) // aligned trace
{
    double time_f = 0.0;
    int disk = 0;
    int lsn1 = 0;
    int size1 = 0;
    int rw = 0;

    sscanf(buffer, "%lf  %d %d %d %d", &time_f, &disk, &lsn1, &size1, &rw);
    if (lsn1 == 0 && size1 == 0 && time_f == 0)
    {
        return 0;
    }
    *ope = rw;
    *lsn = lsn1;
    *size = size1;
    *time_t = (int64_t)(time_f * 1000000); // 将ms表示的trace转换成ssdsim内部的ns表示
    //*time_t = (int64_t)(time_f * 800000);
    //*time_t = (int64_t)(time_f * 300000);
    // *time_t = (int64_t)(time_f);
    return 1;
}

int read_udata(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope) // aligned trace
{
    char seq_random[4];
    int inter_time = 0;
    int device_time = 0;
    int idle_time = 0;

    char rw[2];
    double time_f = 0;
    int lsn1 = 0;
    int size1 = 0;

    sscanf(buffer, "%lf %s %d %d %s %d %d %d", &time_f, rw, &lsn1, &size1, seq_random, &inter_time, &device_time, &idle_time);

    if ((lsn1 == 0) || (size1 == 0) || (time_f == 0))
    {
        return 0;
    }

    if ((rw[0] == 'R') && (rw[1] = 'S'))
        *ope = 1;
    else
        *ope = 0;

    *lsn = lsn1;

    *size = size1;

    if (*size % 8 != 0)
        *size = (*size / 8 + 1) * 8;

    if (*size <= 8)
        *size = 8;

    //*time_t = (int64_t)(time_f * 1000000000);

    //*time_t = (int64_t)(time_f * 100000000);
    *time_t = (int64_t)(time_f * 100000000);

    // printf("%lf %s %d %d %s %d %d %d", time_f, rw,&lsn1, size1,seq_random,inter_time,device_time,idle_time);

    return 1;
}

int read_fdata(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope) // Financial
{
    int temp;
    char rw;
    double time_f;
    int lsn1;
    int size1;
    sscanf(buffer, "%d,%d,%d,%c,%lf", &temp, &lsn1, &size1, &rw, &time_f);
    if (temp == 0 && lsn1 == 0 && size1 == 0 && time_f == 0)
    {
        return 0;
    }
    if (rw == 'R' || rw == 'r')
        *ope = 0;
    else
        *ope = 1;
    *lsn = lsn1;
    size1 = size1 / 512;
    *size = size1;

    if (size1 % 8 != 0)
        *size = (size1 / 8 + 1) * 8;

    if (*size == 0)
        *size = 8;

    *time_t = (int64_t)(time_f * 1000000000);
    return 1;
}

int read_data(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope) // trace文件读取函数
{
    // read_wdata(buffer, time_t, lsn, size, ope);
    // return read_fdata(buffer, time_t, lsn, size, ope);
    // read_ddata(buffer, time_t, lsn, size, ope);
    return read_ndata(buffer, time_t, lsn, size, ope);

    // return read_udata(buffer, time_t, lsn, size, ope);
}

// 支持MLC TLC的读写时间统计
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
void print_location(struct local loc)
{
    printf("channel %d chip %d die %d plane %d block %d page %d\n", loc.channel, loc.chip, loc.die, loc.plane, loc.block, loc.page);
}
int get_sub_number(struct sub_request *head, struct sub_request *tail)
{
    if (head == NULL)
        return 0;
    int len = 1;
    while (head != tail)
    {
        len++;
        head = head->next_node;
    }
    return len;
}
void get_read_sub_number_for_all(struct ssd_info *ssd)
{
    for (int chan = 0; chan < ssd->parameter->channel_number; chan++)
    {
        int sub_number = get_sub_number(ssd->channel_head[chan].subs_r_head, ssd->channel_head[chan].subs_r_tail);
        printf("channel %d read sub %d\n", chan, sub_number);
    }
}
void get_read_sub_number_for_channel(struct ssd_info *ssd, int channel)
{
    int sub_number = get_sub_number(ssd->channel_head[channel].subs_r_head, ssd->channel_head[channel].subs_r_tail);
    printf("request queue : %d \n", ssd->request_queue_length);
    printf("channel %d read sub %d\n", channel, sub_number);
    if (sub_number == 0)
    {
        ssd->gc_read_sub_zero_cnt++;
    }
    else
    {
        ssd->gc_read_sub_sum_cnt += sub_number;
    }
}
void get_write_sub_number_for_dy(struct ssd_info *ssd)
{
    int sub_number = get_sub_number(ssd->subs_w_head, ssd->subs_w_tail);
    printf("write sub %d\n", sub_number);
}
void get_write_sub_number_for_all(struct ssd_info *ssd)
{
    for (int chan = 0; chan < ssd->parameter->channel_number; chan++)
    {
        int sub_number = get_sub_number(ssd->channel_head[chan].subs_w_head, ssd->channel_head[chan].subs_w_tail);
        printf("channel %d write sub %d\n", chan, sub_number);
    }
}
void get_write_sub_number_for_channel(struct ssd_info *ssd, int channel)
{
    int sub_number = get_sub_number(ssd->channel_head[channel].subs_w_head, ssd->channel_head[channel].subs_w_tail);
    printf("channel %d write sub %d\n", channel, sub_number);
}

void request_sub_distribute(struct ssd_info *ssd)
{
    struct request *req = ssd->request_queue;
    struct sub_request *sub;
    while (req != NULL)
    {
        sub = req->subs;
        while (sub != NULL)
        {
            if ((sub->current_state == SR_COMPLETE) || ((sub->next_state == SR_COMPLETE) && (sub->next_state_predict_time <= ssd->current_time)))
            {
            }
            else
            {
                print_location(*(sub->location));
            }
            sub = sub->next_subs;
        }
        req = req->next_node;
    }
}
int get_lpn_for_make_aged(struct ssd_info *ssd, int max_lpn)
{ // 在0~max_lpn之间随机选取一逻辑页号，此时该逻辑页号尚未被映射
    int lpn;
    lpn = rand() % max_lpn + 1;
    while (ssd->dram->map->map_entry[lpn].pn != 0)
    {
        lpn = rand() % max_lpn;
    }
    return lpn;
}
/************************************************
 *断言,当打开文件失败时，输出“open 文件名 error”
 *************************************************/
void file_assert(int error, char *s)
{
    if (error == 0)
        return;
    printf("open %s error\n", s);
    getchar();
    exit(-1);
}

/*****************************************************
 *断言,当申请内存空间失败时，输出“malloc 变量名 error”
 ******************************************************/
void alloc_assert(void *p, char *s) //断言
{
    if (p != NULL)
        return;
    printf("malloc %s error\n", s);
    getchar();
    exit(-1);
}

/*********************************************************************************
 *断言
 *A，读到的time_t，device，lsn，size，ope都<0时，输出“trace error:.....”
 *B，读到的time_t，device，lsn，size，ope都=0时，输出“probable read a blank line”
 **********************************************************************************/
void trace_assert(int64_t time_t, int device, unsigned int lsn, int size, int ope) //断言
{
    if (time_t < 0 || device < 0 || lsn < 0 || size < 0 || ope < 0)
    {
        printf("trace error:%lld %d %d %d %d\n", time_t, device, lsn, size, ope);
        getchar();
        exit(-1);
    }
    if (time_t == 0 && device == 0 && lsn == 0 && size == 0 && ope == 0)
    {
        printf("probable read a blank line\n");
        getchar();
    }
}

/************************************************************************************
 *函数的功能是根据物理页号ppn查找该物理页所在的channel，chip，die，plane，block，page
 *得到的channel，chip，die，plane，block，page放在结构location中并作为返回值
 *************************************************************************************/
struct local *find_location(struct ssd_info *ssd, unsigned int ppn)
{
    struct local *location = NULL;
    unsigned int i = 0;
    int pn, ppn_value = ppn;
    int page_plane = 0, page_die = 0, page_chip = 0, page_channel = 0;

    pn = ppn;

#ifdef DEBUG
    printf("enter find_location\n");
#endif

    location = (struct local *)malloc(sizeof(struct local));
    alloc_assert(location, "location");
    memset(location, 0, sizeof(struct local));

    page_plane = ssd->parameter->page_block * ssd->parameter->block_plane;
    page_die = page_plane * ssd->parameter->plane_die;
    page_chip = page_die * ssd->parameter->die_chip;
    page_channel = page_chip * ssd->parameter->chip_channel[0];

    /*******************************************************************************
     *page_channel是一个channel中page的数目， ppn/page_channel就得到了在哪个channel中
     *用同样的办法可以得到chip，die，plane，block，page
     ********************************************************************************/
    location->channel = ppn / page_channel;
    location->chip = (ppn % page_channel) / page_chip;
    location->die = ((ppn % page_channel) % page_chip) / page_die;
    location->plane = (((ppn % page_channel) % page_chip) % page_die) / page_plane;
    location->block = ((((ppn % page_channel) % page_chip) % page_die) % page_plane) / ssd->parameter->page_block;
    location->page = (((((ppn % page_channel) % page_chip) % page_die) % page_plane) % ssd->parameter->page_block) % ssd->parameter->page_block;

    return location;
}

/*****************************************************************************
 *这个函数的功能是根据参数channel，chip，die，plane，block，page，找到该物理页号
 *函数的返回值就是这个物理页号
 ******************************************************************************/
unsigned int find_ppn(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, unsigned int block, unsigned int page)
{
    unsigned int ppn = 0;
    unsigned int i = 0;
    int page_plane = 0, page_die = 0, page_chip = 0;
    int page_channel[100]; /*这个数组存放的是每个channel的page数目*/

#ifdef DEBUG
    printf("enter find_psn,channel:%d, chip:%d, die:%d, plane:%d, block:%d, page:%d\n", channel, chip, die, plane, block, page);
#endif

    /*********************************************
     *计算出plane，die，chip，channel中的page的数目
     **********************************************/
    page_plane = ssd->parameter->page_block * ssd->parameter->block_plane;
    page_die = page_plane * ssd->parameter->plane_die;
    page_chip = page_die * ssd->parameter->die_chip;
    while (i < ssd->parameter->channel_number)
    {
        page_channel[i] = ssd->parameter->chip_channel[i] * page_chip;
        i++;
    }

    /****************************************************************************
     *计算物理页号ppn，ppn是channel，chip，die，plane，block，page中page个数的总和
     *****************************************************************************/
    i = 0;
    while (i < channel)
    {
        ppn = ppn + page_channel[i];
        i++;
    }
    ppn = ppn + page_chip * chip + page_die * die + page_plane * plane + block * ssd->parameter->page_block + page;

    return ppn;
}

/********************************
 *函数功能是获得一个读子请求的状态
 *********************************/
int set_entry_state(struct ssd_info *ssd, unsigned int lsn, unsigned int size)
{
    int temp, state, move;

    temp = ~(0xffffffff << size);
    move = lsn % ssd->parameter->subpage_page;
    state = temp << move;

    return state;
}

/**************************************************
 *读请求预处理函数，当读请求所读得页里面没有数据时，
 *需要预处理网该页里面写数据，以保证能读到数据
 ***************************************************/
struct ssd_info *pre_process_page(struct ssd_info *ssd)
{
    int fl = 0;
    unsigned int device, lsn, size, ope, lpn, full_page;
    unsigned int largest_lsn, sub_size, ppn, add_size = 0;
    unsigned int i = 0, j, k;
    int map_entry_new, map_entry_old, modify;
    int flag = 0;
    char buffer[20000];
    struct local *location;
    int64_t time;
    int result;

    printf("\n");
    printf("begin pre_process_page.................\n");

    ssd->tracefile = fopen(ssd->tracefilename, "r");
    if (ssd->tracefile == NULL) /*打开trace文件从中读取请求*/
    {
        printf("the trace file can't open\n");
        return NULL;
    }

    full_page = ~(0xffffffff << (ssd->parameter->subpage_page));
    /*计算出这个ssd的最大逻辑扇区号*/
    largest_lsn = (unsigned int)((ssd->parameter->chip_num * ssd->parameter->die_chip * ssd->parameter->plane_die * ssd->parameter->block_plane * ssd->parameter->page_block * ssd->parameter->subpage_page) * (1 - ssd->parameter->overprovide));

    while (fgets(buffer, 20000, ssd->tracefile))
    {
        device = 0;
        result = read_data(buffer, &time, &lsn, &size, &ope); // 读取trace文件
        fl++;
        trace_assert(time, device, lsn, size, ope); /*断言，当读到的time，device，lsn，size，ope不合法时就会处理*/

        add_size = 0; /*add_size是这个请求已经预处理的大小*/

        // 最好使用READ WRITE宏而不是0或者1
        if (ope == READ) /*这里只是读请求的预处理，需要提前将相应位置的信息进行相应修改*/
        {
            while (add_size < size)
            {
                lsn = lsn % largest_lsn; /*防止获得的lsn比最大的lsn还大*/
                sub_size = ssd->parameter->subpage_page - (lsn % ssd->parameter->subpage_page);
                if (add_size + sub_size >= size) /*只有当一个请求的大小小于一个page的大小时或者是处理一个请求的最后一个page时会出现这种情况*/
                {
                    sub_size = size - add_size;
                    add_size += sub_size;
                }

                if ((sub_size > ssd->parameter->subpage_page) || (add_size > size)) /*当预处理一个子大小时，这个大小大于一个page或是已经处理的大小大于size就报错*/
                {
                    printf("pre_process sub_size:%d\n", sub_size);
                }

                /*******************************************************************************************************
                 *利用逻辑扇区号lsn计算出逻辑页号lpn
                 *判断这个dram中映射表map中在lpn位置的状态
                 *A，这个状态==0，表示以前没有写过，现在需要直接将ub_size大小的子页写进去写进去
                 *B，这个状态>0，表示，以前有写过，这需要进一步比较状态，因为新写的状态可以与以前的状态有重叠的扇区的地方
                 ********************************************************************************************************/
                lpn = lsn / ssd->parameter->subpage_page;
                if (ssd->dram->map->map_entry[lpn].state == 0) /*状态为0的情况*/
                {
                    /**************************************************************
                     *获得利用get_ppn_for_pre_process函数获得ppn，再得到location
                     *修改ssd的相关参数，dram的映射表map，以及location下的page的状态
                     ***************************************************************/
                    ppn = get_ppn_for_pre_process(ssd, lsn);
                    location = find_location(ssd, ppn);
                    ssd->program_count++;
                    ssd->channel_head[location->channel].program_count++;
                    ssd->channel_head[location->channel].chip_head[location->chip].program_count++;
                    ssd->dram->map->map_entry[lpn].pn = ppn;
                    ssd->dram->map->map_entry[lpn].state = set_entry_state(ssd, lsn, sub_size); // 0001
                    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn = lpn;
                    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state = ssd->dram->map->map_entry[lpn].state;
                    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state = ((~ssd->dram->map->map_entry[lpn].state) & full_page);

                    free(location);
                    location = NULL;
                }                                                  // if(ssd->dram->map->map_entry[lpn].state==0)
                else if (ssd->dram->map->map_entry[lpn].state > 0) /*状态不为0的情况*/
                {
                    map_entry_new = set_entry_state(ssd, lsn, sub_size); /*得到新的状态，并与原来的状态相或的到一个状态*/
                    map_entry_old = ssd->dram->map->map_entry[lpn].state;
                    modify = map_entry_new | map_entry_old;
                    ppn = ssd->dram->map->map_entry[lpn].pn;
                    location = find_location(ssd, ppn);

                    ssd->program_count++;
                    ssd->channel_head[location->channel].program_count++;
                    ssd->channel_head[location->channel].chip_head[location->chip].program_count++;
                    ssd->dram->map->map_entry[lsn / ssd->parameter->subpage_page].state = modify;
                    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state = modify;
                    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state = ((~modify) & full_page);

                    free(location);
                    location = NULL;
                }                     // else if(ssd->dram->map->map_entry[lpn].state>0)
                lsn = lsn + sub_size; /*下个子请求的起始位置*/
                add_size += sub_size; /*已经处理了的add_size大小变化*/
            }
        }
    }

    printf("\n");
    printf("pre_process is complete!\n");

    fclose(ssd->tracefile);

    for (i = 0; i < ssd->parameter->channel_number; i++)
    {
        for (j = 0; j < ssd->parameter->die_chip; j++)
        {
            for (k = 0; k < ssd->parameter->plane_die; k++)
            {
                fprintf(ssd->outputfile, "chip:%d,die:%d,plane:%d have free page: %d\n", i, j, k, ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);
                fflush(ssd->outputfile);
            }
        }
    }
    return ssd;
}
/**************************************
 *函数功能是为预处理函数获取物理页号ppn
 *获取页号分为动态获取和静态获取
 **************************************/
unsigned int get_ppn_for_pre_process(struct ssd_info *ssd, unsigned int lsn)
{
    unsigned int channel = 0, chip = 0, die = 0, plane = 0;
    unsigned int ppn, lpn;
    unsigned int active_block;
    unsigned int channel_num = 0, chip_num = 0, die_num = 0, plane_num = 0;

#ifdef DEBUG
    printf("enter get_psn_for_pre_process\n");
#endif

    channel_num = ssd->parameter->channel_number;
    chip_num = ssd->parameter->chip_channel[0];
    die_num = ssd->parameter->die_chip;
    plane_num = ssd->parameter->plane_die;
    lpn = lsn / ssd->parameter->subpage_page;

    if (ssd->parameter->allocation_scheme == 0) /*动态方式下获取ppn*/
    {
        if (ssd->parameter->dynamic_allocation == 0) /*表示全动态方式下，也就是channel，chip，die，plane，block等都是动态分配*/
        {
            channel = ssd->token;
            ssd->token = (ssd->token + 1) % ssd->parameter->channel_number;
            chip = ssd->channel_head[channel].token;
            ssd->channel_head[channel].token = (chip + 1) % ssd->parameter->chip_channel[0];
            die = ssd->channel_head[channel].chip_head[chip].token;
            ssd->channel_head[channel].chip_head[chip].token = (die + 1) % ssd->parameter->die_chip;
            plane = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
            ssd->channel_head[channel].chip_head[chip].die_head[die].token = (plane + 1) % ssd->parameter->plane_die;
        }
        else if (ssd->parameter->dynamic_allocation == 1) /*表示半动态方式，channel静态给出，package，die，plane动态分配*/
        {
            channel = lpn % ssd->parameter->channel_number;
            chip = ssd->channel_head[channel].token;
            ssd->channel_head[channel].token = (chip + 1) % ssd->parameter->chip_channel[0];
            die = ssd->channel_head[channel].chip_head[chip].token;
            ssd->channel_head[channel].chip_head[chip].token = (die + 1) % ssd->parameter->die_chip;
            plane = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
            ssd->channel_head[channel].chip_head[chip].die_head[die].token = (plane + 1) % ssd->parameter->plane_die;
        }
    }
    else if (ssd->parameter->allocation_scheme == 1) /*表示静态分配，同时也有0,1,2,3,4,5这6中不同静态分配方式*/
    {
        switch (ssd->parameter->static_allocation)
        {
        case 0: {
            channel = (lpn / (plane_num * die_num * chip_num)) % channel_num;
            chip = lpn % chip_num;
            die = (lpn / chip_num) % die_num;
            plane = (lpn / (die_num * chip_num)) % plane_num;
            break;
        }
        case 1: {
            channel = lpn % channel_num;
            chip = (lpn / channel_num) % chip_num;
            die = (lpn / (chip_num * channel_num)) % die_num;
            plane = (lpn / (die_num * chip_num * channel_num)) % plane_num;

            break;
        }
        case 2: {
            channel = lpn % channel_num;
            chip = (lpn / (plane_num * channel_num)) % chip_num;
            die = (lpn / (plane_num * chip_num * channel_num)) % die_num;
            plane = (lpn / channel_num) % plane_num;
            break;
        }
        case 3: {
            channel = lpn % channel_num;
            chip = (lpn / (die_num * channel_num)) % chip_num;
            die = (lpn / channel_num) % die_num;
            plane = (lpn / (die_num * chip_num * channel_num)) % plane_num;
            break;
        }
        case 4: {
            channel = lpn % channel_num;
            chip = (lpn / (plane_num * die_num * channel_num)) % chip_num;
            die = (lpn / (plane_num * channel_num)) % die_num;
            plane = (lpn / channel_num) % plane_num;

            break;
        }
        case 5: {
            channel = lpn % channel_num;
            chip = (lpn / (plane_num * die_num * channel_num)) % chip_num;
            die = (lpn / channel_num) % die_num;
            plane = (lpn / (die_num * channel_num)) % plane_num;

            break;
        }
        default:
            return 0;
        }
    }

    /******************************************************************************
     *根据上述分配方法找到channel，chip，die，plane后，再在这个里面找到active_block
     *接着获得ppn
     ******************************************************************************/
    if (find_active_block(ssd, channel, chip, die, plane) == FAILURE)
    {
        printf("the read operation is expand the capacity of SSD\n");
        return 0;
    }

    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
    if (write_page(ssd, channel, chip, die, plane, active_block, &ppn) == ERROR)
    {
        return 0;
    }

    return ppn;
}

/***************************************************************************************************
 *函数功能是在所给的channel，chip，die，plane里面找到一个active_block然后再在这个block里面找到一个页，
 *再利用find_ppn找到ppn。
 ****************************************************************************************************/
struct ssd_info *get_ppn(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, struct sub_request *sub)
{
    int old_ppn = -1;
    unsigned int ppn, lpn, full_page;
    unsigned int active_block;
    unsigned int block;
    unsigned int page, flag = 0, flag1 = 0;
    unsigned int old_state = 0, state = 0, copy_subpage = 0;
    struct local *location;
    struct direct_erase *direct_erase_node, *new_direct_erase;
    struct gc_operation *gc_node;

    unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;

#ifdef DEBUG
    printf("enter get_ppn,channel:%d, chip:%d, die:%d, plane:%d\n", channel, chip, die, plane);
#endif

    full_page = ~(0xffffffff << (ssd->parameter->subpage_page));
    lpn = sub->lpn;

    /*************************************************************************************
     *利用函数find_active_block在channel，chip，die，plane找到活跃block
     *并且修改这个channel，chip，die，plane，active_block下的last_write_page和free_page_num
     **************************************************************************************/
    if (find_active_block(ssd, channel, chip, die, plane) == FAILURE)
    {
        printf("ERROR :there is no free page in channel:%d, chip:%d, die:%d, plane:%d\n", channel, chip, die, plane);
        return ssd;
    }

    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page++;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;

    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page > ssd->parameter->page_block)
    {
        printf("error! the last write page larger than page number!!\n");
        while (1)
        {
        }
    }

    block = active_block;
    page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page;

    if (ssd->dram->map->map_entry[lpn].state == 0) /*this is the first logical page*/
    {
        if (ssd->dram->map->map_entry[lpn].pn != 0)
        {
            printf("Error in get_ppn()\n");
        }
        ssd->dram->map->map_entry[lpn].pn = find_ppn(ssd, channel, chip, die, plane, block, page);
        ssd->dram->map->map_entry[lpn].state = sub->state;
    }
    else /*这个逻辑页进行了更新，需要将原来的页置为失效*/
    {
        ppn = ssd->dram->map->map_entry[lpn].pn;
        location = find_location(ssd, ppn);
        if (ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn != lpn)
        {
            printf("\nError in get_ppn()\n");
        }
        ssd->update_write_count++;                                                                                                                                                              //记录更新写次数
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state = 0; /*表示某一页失效，同时标记valid和free状态都为0*/
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state = 0;  /*表示某一页失效，同时标记valid和free状态都为0*/
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn = 0;
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;

        /*******************************************************************************************
         *该block中全是invalid的页，可以直接删除，就在创建一个可擦除的节点，挂在location下的plane下面
         ********************************************************************************************/
        if (ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num == ssd->parameter->page_block)
        {
            new_direct_erase = (struct direct_erase *)malloc(sizeof(struct direct_erase));
            alloc_assert(new_direct_erase, "new_direct_erase");
            memset(new_direct_erase, 0, sizeof(struct direct_erase));

            new_direct_erase->block = location->block;
            new_direct_erase->next_node = NULL;
            direct_erase_node = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
            if (direct_erase_node == NULL)
            {
                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node = new_direct_erase;
            }
            else
            {
                new_direct_erase->next_node = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node = new_direct_erase;
            }
        }

        free(location);
        location = NULL;
        ssd->dram->map->map_entry[lpn].pn = find_ppn(ssd, channel, chip, die, plane, block, page);
        ssd->dram->map->map_entry[lpn].state = (ssd->dram->map->map_entry[lpn].state | sub->state);
    }

    sub->ppn = ssd->dram->map->map_entry[lpn].pn; /*修改sub子请求的ppn，location等变量*/
    sub->location->channel = channel;
    sub->location->chip = chip;
    sub->location->die = die;
    sub->location->plane = plane;
    sub->location->block = active_block;
    sub->location->page = page;

    ssd->program_count++; /*修改ssd的program_count,free_page等变量*/
    ssd->channel_head[channel].program_count++;
    ssd->channel_head[channel].chip_head[chip].program_count++;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].lpn = lpn;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].valid_state = sub->state;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].free_state = ((~(sub->state)) & full_page);
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].written_count++;
    ssd->write_flash_count++;

    if (ssd->parameter->active_write == 0) /*如果没有主动策略，只采用gc_hard_threshold，并且无法中断GC过程*/
    {                                      /*如果plane中的free_page的数目少于gc_hard_threshold所设定的阈值就产生gc操作*/
        if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page < (ssd->parameter->page_block * ssd->parameter->block_plane * ssd->parameter->gc_hard_threshold))
        {
            gc_node = (struct gc_operation *)malloc(sizeof(struct gc_operation));
            alloc_assert(gc_node, "gc_node");
            memset(gc_node, 0, sizeof(struct gc_operation));

            gc_node->next_node = NULL;
            gc_node->chip = chip;
            gc_node->die = die;
            gc_node->plane = plane;
            gc_node->block = 0xffffffff;
            gc_node->page = 0;
            gc_node->state = GC_WAIT;
            gc_node->priority = GC_UNINTERRUPT;
            gc_node->next_node = ssd->channel_head[channel].gc_command;
            ssd->channel_head[channel].gc_command = gc_node;
            ssd->gc_request++;
        }
    }

    return ssd;
}
// gc时write page一次增加3，保持其page类型不变
int get_next_write_page_for_gc(struct ssd_info *ssd, int last_write_page)
{
    if (last_write_page == -1)
        return 0; //初始时为-1
    int page_id = last_write_page + 3;
    if (page_id >= ssd->parameter->page_block)
    {
        page_id = last_write_page % 3 + 1;
        if (page_id == ERRORTYPE)
        {
            return -1;
        }
    }
    return page_id;
}
/*****************************************************************************************
 *这个函数功能是为gc操作寻找新的ppn，因为在gc操作中需要找到新的物理块存放原来物理块上的数据
 *在gc中寻找新物理块的函数，不会引起循环的gc操作
 ******************************************************************************************/
unsigned int get_ppn_for_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, int page_type)
{
    unsigned int ppn;
    unsigned int active_block, block, page;
    int new_page_type; //如果未找到指定类型的页
    int last_write_page, next_page;
#ifdef DEBUG
    printf("enter get_ppn_for_gc,channel:%d, chip:%d, die:%d, plane:%d\n", channel, chip, die, plane);
#endif

    if (find_active_block(ssd, channel, chip, die, plane) != SUCCESS)
    {
        printf("\n\n Error int get_ppn_for_gc().\n");
        return 0xffffffff;
    }
    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
    block = -1;
    block = active_block;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page++;
    page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page;

    ppn = find_ppn(ssd, channel, chip, die, plane, block, page);

    ssd->program_count++;
    ssd->channel_head[channel].program_count++;
    ssd->channel_head[channel].chip_head[chip].program_count++;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].written_count++;
    ssd->write_flash_count++;
    return ppn;
}

/*********************************************************************************************************************
 * 朱志明 于2011年7月28日修改
 *函数的功能就是erase_operation擦除操作，把channel，chip，die，plane下的block擦除掉
 *也就是初始化这个block的相关参数，eg：free_page_num=page_block，invalid_page_num=0，last_write_page=-1，erase_count++
 *还有这个block下面的每个page的相关参数也要修改。
 *********************************************************************************************************************/

Status erase_operation(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, unsigned int block)
{
    unsigned int i = 0;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num = ssd->parameter->page_block;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num = 0;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page = -1;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].erase_count++;
    for (i = 0; i < ssd->parameter->page_block; i++)
    {
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state = PG_SUB;
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state = 0;
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].lpn = -1;
    }
    ssd->erase_count++;
    ssd->channel_head[channel].erase_count++;
    ssd->channel_head[channel].chip_head[chip].erase_count++;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page += ssd->parameter->page_block;

    return SUCCESS;
}

/**************************************************************************************
 *这个函数的功能是处理INTERLEAVE_TWO_PLANE，INTERLEAVE，TWO_PLANE，NORMAL下的擦除的操作。
 ***************************************************************************************/
Status erase_planes(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die1, unsigned int plane1, unsigned int command)
{
    unsigned int die = 0;
    unsigned int plane = 0;
    unsigned int block = 0;
    struct direct_erase *direct_erase_node = NULL;
    unsigned int block0 = 0xffffffff;
    unsigned int block1 = 0;

    if ((ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node == NULL) || ((command != INTERLEAVE_TWO_PLANE) && (command != INTERLEAVE) && (command != TWO_PLANE) && (command != NORMAL))) /*如果没有擦除操作，或者command不对，返回错误*/
    {
        return ERROR;
    }

    /************************************************************************************************************
     *处理擦除操作时，首先要传送擦除命令，这是channel，chip处于传送命令的状态，即CHANNEL_TRANSFER，CHIP_ERASE_BUSY
     *下一状态是CHANNEL_IDLE，CHIP_IDLE。
     *************************************************************************************************************/
    block1 = ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node->block;

    ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
    ssd->channel_head[channel].current_time = ssd->current_time;
    ssd->channel_head[channel].next_state = CHANNEL_IDLE;

    ssd->channel_head[channel].chip_head[chip].current_state = CHIP_ERASE_BUSY;
    ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
    ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;

    if (command == INTERLEAVE_TWO_PLANE) /*高级命令INTERLEAVE_TWO_PLANE的处理*/
    {
        for (die = 0; die < ssd->parameter->die_chip; die++)
        {
            block0 = 0xffffffff;
            if (die == die1)
            {
                block0 = block1;
            }
            for (plane = 0; plane < ssd->parameter->plane_die; plane++)
            {
                direct_erase_node = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
                if (direct_erase_node != NULL)
                {
                    block = direct_erase_node->block;

                    if (block0 == 0xffffffff)
                    {
                        block0 = block;
                    }
                    else
                    {
                        if (block != block0)
                        {
                            continue;
                        }
                    }
                    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node = direct_erase_node->next_node;
                    erase_operation(ssd, channel, chip, die, plane, block); /*真实的擦除操作的处理*/
                    free(direct_erase_node);
                    direct_erase_node = NULL;
                    ssd->direct_erase_count++;
                }
            }
        }

        ssd->interleave_mplane_erase_count++; /*发送了一个interleave two plane erase命令,并计算这个处理的时间，以及下一个状态的时间*/
        ssd->channel_head[channel].next_state_predict_time = ssd->current_time + 18 * ssd->parameter->time_characteristics.tWC + ssd->parameter->time_characteristics.tWB;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time - 9 * ssd->parameter->time_characteristics.tWC + ssd->parameter->time_characteristics.tBERS;
    }
    else if (command == INTERLEAVE) /*高级命令INTERLEAVE的处理*/
    {
        for (die = 0; die < ssd->parameter->die_chip; die++)
        {
            for (plane = 0; plane < ssd->parameter->plane_die; plane++)
            {
                if (die == die1)
                {
                    plane = plane1;
                }
                direct_erase_node = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
                if (direct_erase_node != NULL)
                {
                    block = direct_erase_node->block;
                    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node = direct_erase_node->next_node;
                    erase_operation(ssd, channel, chip, die, plane, block);
                    free(direct_erase_node);
                    direct_erase_node = NULL;
                    ssd->direct_erase_count++;
                    break;
                }
            }
        }
        ssd->interleave_erase_count++;
        ssd->channel_head[channel].next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tBERS;
    }
    else if (command == TWO_PLANE) /*高级命令TWO_PLANE的处理*/
    {
        for (plane = 0; plane < ssd->parameter->plane_die; plane++)
        {
            direct_erase_node = ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane].erase_node;
            if ((direct_erase_node != NULL))
            {
                block = direct_erase_node->block;
                if (block == block1)
                {
                    ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane].erase_node = direct_erase_node->next_node;
                    erase_operation(ssd, channel, chip, die1, plane, block);
                    free(direct_erase_node);
                    direct_erase_node = NULL;
                    ssd->direct_erase_count++;
                }
            }
        }

        ssd->mplane_erase_conut++;
        ssd->channel_head[channel].next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tBERS;
    }
    else if (command == NORMAL) /*普通命令NORMAL的处理*/
    {
        direct_erase_node = ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node;
        block = direct_erase_node->block;
        ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node = direct_erase_node->next_node;
        free(direct_erase_node);
        direct_erase_node = NULL;
        erase_operation(ssd, channel, chip, die1, plane1, block);

        ssd->direct_erase_count++;
        ssd->channel_head[channel].next_state_predict_time = ssd->current_time + 5 * ssd->parameter->time_characteristics.tWC;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tWB + ssd->parameter->time_characteristics.tBERS;
    }
    else
    {
        return ERROR;
    }

    direct_erase_node = ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node;

    if (((direct_erase_node) != NULL) && (direct_erase_node->block == block1))
    {
        return FAILURE;
    }
    else
    {
        return SUCCESS;
    }
}

/*******************************************************************************************************************
 *GC操作由某个plane的free块少于阈值进行触发，当某个plane被触发时，GC操作占据这个plane所在的die，因为die是一个独立单元。
 *对一个die的GC操作，尽量做到四个plane同时erase，利用interleave erase操作。GC操作应该做到可以随时停止（移动数据和擦除
 *时不行，但是间隙时间可以停止GC操作），以服务新到达的请求，当请求服务完后，利用请求间隙时间，继续GC操作。可以设置两个
 *GC阈值，一个软阈值，一个硬阈值。软阈值表示到达该阈值后，可以开始主动的GC操作，利用间歇时间，GC可以被新到的请求中断；
 *当到达硬阈值后，强制性执行GC操作，且此GC操作不能被中断，直到回到硬阈值以上。
 *在这个函数里面，找出这个die所有的plane中，有没有可以直接删除的block，要是有的话，利用interleave two plane命令，删除
 *这些block，否则有多少plane有这种直接删除的block就同时删除，不行的话，最差就是单独这个plane进行删除，连这也不满足的话，
 *直接跳出，到gc_parallelism函数进行进一步GC操作。该函数寻找全部为invalid的块，直接删除，找到可直接删除的返回1，没有找
 *到返回-1。
 *********************************************************************************************************************/
int gc_direct_erase(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane)
{
    unsigned int lv_die = 0, lv_plane = 0; /*为避免重名而使用的局部变量 Local variables*/
    unsigned int interleaver_flag = FALSE, muilt_plane_flag = FALSE;
    unsigned int normal_erase_flag = TRUE;

    struct direct_erase *direct_erase_node1 = NULL;
    struct direct_erase *direct_erase_node2 = NULL;

    direct_erase_node1 = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
    if (direct_erase_node1 == NULL)
    {
        return FAILURE;
    }

    if ((normal_erase_flag == TRUE)) /*不是每个plane都有可以直接删除的block，只对当前plane进行普通的erase操作，或者只能执行普通命令*/
    {
        if (erase_planes(ssd, channel, chip, die, plane, NORMAL) == SUCCESS)
        {
            return SUCCESS;
        }
        else
        {
            return FAILURE; /*目标的plane没有可以直接删除的block，需要寻找目标擦除块后在实施擦除操作*/
        }
    }
    return SUCCESS;
}

Status move_page(struct ssd_info *ssd, struct local *location, int page_type, unsigned int *transfer_size, int *page_id)
{
    struct local *new_location = NULL;
    unsigned int free_state = 0, valid_state = 0;
    unsigned int lpn = 0, old_ppn = 0, ppn = 0;

    lpn = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn;
    valid_state = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state;
    free_state = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state;
    old_ppn = find_ppn(ssd, location->channel, location->chip, location->die, location->plane, location->block, location->page); /*记录这个有效移动页的ppn，对比map或者额外映射关系中的ppn，进行删除和添加操作*/
    // 获取特定类型的页
    ppn = get_ppn_for_gc(ssd, location->channel, location->chip, location->die, location->plane, page_type); /*找出来的ppn一定是在发生gc操作的plane中,才能使用copyback操作，为gc操作获取ppn*/

    new_location = find_location(ssd, ppn); /*根据新获得的ppn获取new_location*/
    *page_id = new_location->page;          //返回写入页号

    (*transfer_size) += size(valid_state);
    ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state = free_state;
    ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn = lpn;
    ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state = valid_state;

    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state = 0;
    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn = 0;
    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state = 0;
    ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;

    if (old_ppn == ssd->dram->map->map_entry[lpn].pn) /*修改映射表*/
    {
        ssd->dram->map->map_entry[lpn].pn = ppn;
    }

    free(new_location);
    new_location = NULL;

    return SUCCESS;
}

/*******************************************************************************************************************************************
 *目标的plane没有可以直接删除的block，需要寻找目标擦除块后在实施擦除操作，用在不能中断的gc操作中，成功删除一个块，返回1，没有删除一个块返回-1
 *在这个函数中，不用考虑目标channel,die是否是空闲的,擦除invalid_page_num最多的block
 ********************************************************************************************************************************************/
int uninterrupt_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane)
{
    unsigned int i = 0, invalid_page = 0;
    unsigned int block, active_block, transfer_size, free_page, page_move_count = 0; /*记录失效页最多的块号*/
    struct local *location = NULL;
    unsigned int total_invalid_page_num = 0;
    int write_page_id;                      //垃圾回收写入页的页号
    int64_t time_R_AND_PROG = 0;            //读写总时间
    int page_allocation_type;               //分配给该垃圾回收的写入页类型
    int LSBnum = 0, CSBnum = 0, MSBnum = 0; //各种类型页的数目
    int device_queue_depth;                 //设备队列长度
    int vaild_page_num;                     //块内有效页的数目
    int type_block_size;
    int rand_value;

    if (find_active_block(ssd, channel, chip, die, plane) != SUCCESS) /*获取活跃块*/
    {
        printf("\n\n Error in uninterrupt_gc().\n");
        return ERROR;
    }
    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;

    invalid_page = 0;
    transfer_size = 0;
    block = -1;
    for (i = 0; i < ssd->parameter->block_plane; i++) /*查找最多invalid_page的块号，以及最大的invalid_page_num*/
    {
        total_invalid_page_num += ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
        if ((active_block != i) && (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num > invalid_page))
        {
            invalid_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
            block = i;
        }
    }
    if (block == -1)
    {
        return 1;
    }
    free_page = 0;
    unsigned int mask = ~(0xffffffff << (ssd->parameter->subpage_page)); // 每1 bit表示 1个subpage
    for (i = 0; i < ssd->parameter->page_block; i++)                     /*逐个检查每个page，如果有有效数据的page需要移动到其他地方存储*/
    {
        if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state & PG_SUB) == mask)
        {
            free_page++;
        }
        if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state > 0) /*该页是有效页，需要copyback操作*/
        {
            location = (struct local *)malloc(sizeof(struct local));
            alloc_assert(location, "location");
            memset(location, 0, sizeof(struct local));

            location->channel = channel;
            location->chip = chip;
            location->die = die;
            location->plane = plane;
            location->block = block;
            location->page = i;
            move_page(ssd, location, page_allocation_type, &transfer_size, &write_page_id); /*真实的move_page操作*/
            time_R_AND_PROG += get_read_time(ssd, i) + get_prog_time(ssd, write_page_id);   // 计算迁移页所用时间
            page_move_count++;

            free(location);
            location = NULL;
        }
    }
    if (free_page != 0)
    {
        printf("too much free page. channel %d chip %d die %d plane %d  block %d  pages: %d\n", channel, chip, die, plane, block, free_page);
    }
    erase_operation(ssd, channel, chip, die, plane, block); /*执行完move_page操作后，就立即执行block的擦除操作*/

    ssd->channel_head[channel].current_state = CHANNEL_GC;
    ssd->channel_head[channel].current_time = ssd->current_time;
    ssd->channel_head[channel].next_state = CHANNEL_IDLE;
    ssd->channel_head[channel].chip_head[chip].current_state = CHIP_ERASE_BUSY;
    ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
    ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;

    ssd->channel_head[channel].next_state_predict_time = ssd->current_time + page_move_count * (7 * ssd->parameter->time_characteristics.tWC + 7 * ssd->parameter->time_characteristics.tWC) + transfer_size * SECTOR * (ssd->parameter->time_characteristics.tWC + ssd->parameter->time_characteristics.tRC) + time_R_AND_PROG;
    ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tBERS;
    ssd->gc_sum_time += page_move_count * (7 * ssd->parameter->time_characteristics.tWC + 7 * ssd->parameter->time_characteristics.tWC) + transfer_size * SECTOR * (ssd->parameter->time_characteristics.tWC + ssd->parameter->time_characteristics.tRC) + time_R_AND_PROG + ssd->parameter->time_characteristics.tBERS;
    ssd->gc_count++;
    ssd->gc_page_move_count += page_move_count;
    return 1;
}

/*******************************************************************************************************************************************
 *目标的plane没有可以直接删除的block，需要寻找目标擦除块后在实施擦除操作，用在可以中断的gc操作中，成功删除一个块，返回1，没有删除一个块返回-1
 *在这个函数中，不用考虑目标channel,die是否是空闲的
 ********************************************************************************************************************************************/
int interrupt_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, struct gc_operation *gc_node)
{
    unsigned int i, block, active_block, transfer_size, invalid_page = 0;
    struct local *location;
    int write_page_id;
    int64_t time_R_AND_PROG = 0;

    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
    transfer_size = 0;
    if (gc_node->block >= ssd->parameter->block_plane)
    {
        for (i = 0; i < ssd->parameter->block_plane; i++)
        {
            if ((active_block != i) && (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num > invalid_page))
            {
                invalid_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
                block = i;
            }
        }
        gc_node->block = block;
    }

    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].invalid_page_num != ssd->parameter->page_block) /*还需要执行copyback操作*/
    {
        for (i = gc_node->page; i < ssd->parameter->page_block; i++)
        {
            if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].page_head[i].valid_state > 0)
            {
                location = (struct local *)malloc(sizeof(struct local));
                alloc_assert(location, "location");
                memset(location, 0, sizeof(struct local));

                location->channel = channel;
                location->chip = chip;
                location->die = die;
                location->plane = plane;
                location->block = block;
                location->page = i;
                transfer_size = 0;

                move_page(ssd, location, 0, &transfer_size, &write_page_id);
                time_R_AND_PROG += get_read_time(ssd, i) + get_prog_time(ssd, write_page_id);

                free(location);
                location = NULL;

                gc_node->page = i + 1;
                ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].invalid_page_num++;
                ssd->channel_head[channel].current_state = CHANNEL_C_A_TRANSFER;
                ssd->channel_head[channel].current_time = ssd->current_time;
                ssd->channel_head[channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[channel].chip_head[chip].current_state = CHIP_COPYBACK_BUSY;
                ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;

                ssd->channel_head[channel].next_state_predict_time = ssd->current_time + (7 + transfer_size * SECTOR) * ssd->parameter->time_characteristics.tWC + ssd->parameter->time_characteristics.tR + (7 + transfer_size * SECTOR) * ssd->parameter->time_characteristics.tWC;
                ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;

                return 0;
            }

            else
            {
                erase_operation(ssd, channel, chip, die, plane, gc_node->block);

                ssd->channel_head[channel].current_state = CHANNEL_C_A_TRANSFER;
                ssd->channel_head[channel].current_time = ssd->current_time;
                ssd->channel_head[channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[channel].next_state_predict_time = ssd->current_time + 5 * ssd->parameter->time_characteristics.tWC;

                ssd->channel_head[channel].chip_head[chip].current_state = CHIP_ERASE_BUSY;
                ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
                ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tBERS;

                return 1; /*该gc操作完成，返回1，可以将channel上的gc请求节点删除*/
            }

            printf("there is a problem in interrupt_gc\n");
            return 1;
        }
    }
}
/*************************************************************
 *函数的功能是当处理完一个gc操作时，需要把gc链上的gc_node删除掉
 **************************************************************/
int delete_gc_node(struct ssd_info *ssd, unsigned int channel, struct gc_operation *gc_node)
{
    struct gc_operation *gc_pre = NULL;
    if (gc_node == NULL)
    {
        return ERROR;
    }

    if (gc_node == ssd->channel_head[channel].gc_command)
    {
        ssd->channel_head[channel].gc_command = gc_node->next_node;
    }
    else
    {
        gc_pre = ssd->channel_head[channel].gc_command;
        while (gc_pre->next_node != NULL)
        {
            if (gc_pre->next_node == gc_node)
            {
                gc_pre->next_node = gc_node->next_node;
                break;
            }
            gc_pre = gc_pre->next_node;
        }
    }
    free(gc_node);
    gc_node = NULL;
    ssd->gc_request--;
    return SUCCESS;
}

/***************************************
 *这个函数的功能是处理channel的每个gc操作
 ****************************************/
Status gc_for_channel(struct ssd_info *ssd, unsigned int channel)
{
    int flag_direct_erase = 1, flag_gc = 1, flag_invoke_gc = 1;
    unsigned int chip, die, plane, flag_priority = 0;
    unsigned int current_state = 0, next_state = 0;
    long long next_state_predict_time = 0;
    struct gc_operation *gc_node = NULL, *gc_p = NULL;

    /*******************************************************************************************
     *查找每一个gc_node，获取gc_node所在的chip的当前状态，下个状态，下个状态的预计时间
     *如果当前状态是空闲，或是下个状态是空闲而下个状态的预计时间小于当前时间，并且是不可中断的gc
     *那么就flag_priority令为1，否则为0
     ********************************************************************************************/
    gc_node = ssd->channel_head[channel].gc_command;
    while (gc_node != NULL)
    {
        current_state = ssd->channel_head[channel].chip_head[gc_node->chip].current_state;
        next_state = ssd->channel_head[channel].chip_head[gc_node->chip].next_state;
        next_state_predict_time = ssd->channel_head[channel].chip_head[gc_node->chip].next_state_predict_time;
        if ((current_state == CHIP_IDLE) || ((next_state == CHIP_IDLE) && (next_state_predict_time <= ssd->current_time)))
        {
            if (gc_node->priority == GC_UNINTERRUPT) /*这个gc请求是不可中断的，优先服务这个gc操作*/
            {
                flag_priority = 1;
                break;
            }
        }
        gc_node = gc_node->next_node;
    }
    if (flag_priority != 1) /*没有找到不可中断的gc请求，首先执行队首的gc请求*/
    {
        gc_node = ssd->channel_head[channel].gc_command;
        while (gc_node != NULL)
        {
            current_state = ssd->channel_head[channel].chip_head[gc_node->chip].current_state;
            next_state = ssd->channel_head[channel].chip_head[gc_node->chip].next_state;
            next_state_predict_time = ssd->channel_head[channel].chip_head[gc_node->chip].next_state_predict_time;
            /**********************************************
             *需要gc操作的目标chip是空闲的，才可以进行gc操作
             ***********************************************/
            if ((current_state == CHIP_IDLE) || ((next_state == CHIP_IDLE) && (next_state_predict_time <= ssd->current_time)))
            {
                break;
            }
            gc_node = gc_node->next_node;
        }
    }
    if (gc_node == NULL)
    {
        return FAILURE;
    }

    chip = gc_node->chip;
    die = gc_node->die;
    plane = gc_node->plane;

    if (gc_node->priority == GC_UNINTERRUPT)
    {
        flag_direct_erase = gc_direct_erase(ssd, channel, chip, die, plane);
        if (flag_direct_erase != SUCCESS)
        {
            flag_gc = uninterrupt_gc(ssd, channel, chip, die, plane); /*当一个完整的gc操作完成时（已经擦除一个块，回收了一定数量的flash空间），返回1，将channel上相应的gc操作请求节点删除*/
            if (flag_gc == 1)
            {
                delete_gc_node(ssd, channel, gc_node);
            }
        }
        else
        {
            delete_gc_node(ssd, channel, gc_node);
        }
        return SUCCESS;
    }
    /*******************************************************************************
     *可中断的gc请求，需要首先确认该channel上没有子请求在这个时刻需要使用这个channel，
     *没有的话，在执行gc操作，有的话，不执行gc操作
     ********************************************************************************/
    else
    {
        flag_invoke_gc = decide_gc_invoke(ssd, channel); /*判断是否有子请求需要channel，如果有子请求需要这个channel，那么这个gc操作就被中断了*/

        if (flag_invoke_gc == 1)
        {
            flag_direct_erase = gc_direct_erase(ssd, channel, chip, die, plane);
            if (flag_direct_erase == -1)
            {
                flag_gc = interrupt_gc(ssd, channel, chip, die, plane, gc_node); /*当一个完整的gc操作完成时（已经擦除一个块，回收了一定数量的flash空间），返回1，将channel上相应的gc操作请求节点删除*/
                if (flag_gc == 1)
                {
                    delete_gc_node(ssd, channel, gc_node);
                }
            }
            else if (flag_direct_erase == 1)
            {
                delete_gc_node(ssd, channel, gc_node);
            }
            return SUCCESS;
        }
        else
        {
            return FAILURE;
        }
    }
}

/************************************************************************************************************
 *flag用来标记gc函数是在ssd整个都是idle的情况下被调用的（1），还是确定了channel，chip，die，plane被调用（0）
 *进入gc函数，需要判断是否是不可中断的gc操作，如果是，需要将一整块目标block完全擦除后才算完成；如果是可中断的，
 *在进行GC操作前，需要判断该channel，die是否有子请求在等待操作，如果没有则开始一步一步的操作，找到目标
 *块后，一次执行一个copyback操作，跳出gc函数，待时间向前推进后，再做下一个copyback或者erase操作
 *进入gc函数不一定需要进行gc操作，需要进行一定的判断，当处于硬阈值以下时，必须进行gc操作；当处于软阈值以下时，
 *需要判断，看这个channel上是否有子请求在等待(有写子请求等待就不行，gc的目标die处于busy状态也不行)，如果
 *有就不执行gc，跳出，否则可以执行一步操作
 ************************************************************************************************************/
unsigned int gc(struct ssd_info *ssd, unsigned int channel, unsigned int flag)
{
    unsigned int i;
    int flag_direct_erase = 1, flag_gc = 1, flag_invoke_gc = 1;
    unsigned int flag_priority = 0;
    struct gc_operation *gc_node = NULL, *gc_p = NULL;

    if (flag == 1) /*整个ssd都是IDEL的情况*/
    {
        for (i = 0; i < ssd->parameter->channel_number; i++)
        {
            flag_priority = 0;
            flag_direct_erase = 1;
            flag_gc = 1;
            flag_invoke_gc = 1;
            gc_node = NULL;
            gc_p = NULL;
            if ((ssd->channel_head[i].current_state == CHANNEL_IDLE) || (ssd->channel_head[i].next_state == CHANNEL_IDLE && ssd->channel_head[i].next_state_predict_time <= ssd->current_time))
            {
                channel = i;
                if (ssd->channel_head[channel].gc_command != NULL)
                {
                    gc_for_channel(ssd, channel);
                }
            }
        }
        return SUCCESS;
    }
    else /*只需针对某个特定的channel，chip，die进行gc请求的操作(只需对目标die进行判定，看是不是idle）*/
    {
        gc_for_channel(ssd, channel);
        return SUCCESS;
    }
}

/**********************************************************
 *判断是否有子请求血药channel，若果没有返回1就可以发送gc操作
 *如果有返回0，就不能执行gc操作，gc操作被中断
 ***********************************************************/
int decide_gc_invoke(struct ssd_info *ssd, unsigned int channel)
{
    struct sub_request *sub;
    struct local *location;

    if ((ssd->channel_head[channel].subs_r_head == NULL) && (ssd->channel_head[channel].subs_w_head == NULL)) /*这里查找读写子请求是否需要占用这个channel，不用的话才能执行GC操作*/
    {
        return 1; /*表示当前时间这个channel没有子请求需要占用channel*/
    }
    else
    {
        if (ssd->channel_head[channel].subs_w_head != NULL)
        {
            return 0;
        }
        else if (ssd->channel_head[channel].subs_r_head != NULL)
        {
            sub = ssd->channel_head[channel].subs_r_head;
            while (sub != NULL)
            {
                if (sub->current_state == SR_WAIT) /*这个读请求是处于等待状态，如果他的目标die处于idle，则不能执行gc操作，返回0*/
                {
                    location = find_location(ssd, sub->ppn);
                    if ((ssd->channel_head[location->channel].chip_head[location->chip].current_state == CHIP_IDLE) || ((ssd->channel_head[location->channel].chip_head[location->chip].next_state == CHIP_IDLE) && (ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time <= ssd->current_time)))
                    {
                        free(location);
                        location = NULL;
                        return 0;
                    }
                    free(location);
                    location = NULL;
                }
                else if (sub->next_state == SR_R_DATA_TRANSFER)
                {
                    location = find_location(ssd, sub->ppn);
                    if (ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time <= ssd->current_time)
                    {
                        free(location);
                        location = NULL;
                        return 0;
                    }
                    free(location);
                    location = NULL;
                }
                sub = sub->next_node;
            }
        }
        return 1;
    }
}
