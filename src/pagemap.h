/*****************************************************************************************************************************
  This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
  Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

  FileName�� pagemap.h
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
#ifndef PAGEMAP_H
#define PAGEMAP_H 10000

#include <sys/types.h>

#include "initialize.h"

#define MAX_INT64 0x7fffffffffffffffll

void file_assert(int error, char *s);
void alloc_assert(void *p, char *s);
void trace_assert(int64_t time_t, int device, unsigned int lsn, int size, int ope);

struct local *find_location(struct ssd_info *ssd, unsigned int ppn);
unsigned int find_ppn(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, unsigned int block, unsigned int page);
struct ssd_info *pre_process_page(struct ssd_info *ssd);
unsigned int get_ppn_for_pre_process(struct ssd_info *ssd, unsigned int lsn);
struct ssd_info *get_ppn(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, struct sub_request *sub);
unsigned int gc(struct ssd_info *ssd, unsigned int channel, unsigned int flag);
int gc_direct_erase(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane);
int uninterrupt_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane);
int interrupt_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, struct gc_operation *gc_node);
int decide_gc_invoke(struct ssd_info *ssd, unsigned int channel);
int set_entry_state(struct ssd_info *ssd, unsigned int lsn, unsigned int size);
unsigned int get_ppn_for_gc(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, int page_type);

int erase_operation(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane, unsigned int block);
int erase_planes(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die1, unsigned int plane1, unsigned int command);
Status move_page(struct ssd_info *ssd, struct local *location, int page_type, unsigned int *transfer_size, int *page_id);
int gc_for_channel(struct ssd_info *ssd, unsigned int channel);
int delete_gc_node(struct ssd_info *ssd, unsigned int channel, struct gc_operation *gc_node);

// add trace file read func
int read_fdata(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope);
int read_ndata(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope);
int read_udata(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope);
int read_data(char *buffer, int64_t *time_t, int *lsn, int *size, int *ope);
int get_read_time(struct ssd_info *ssd, int page_id);
int get_prog_time(struct ssd_info *ssd, int page_id);
int get_next_write_page_for_gc(struct ssd_info *ssd, int last_write_page);
void print_location(struct local location);
int get_sub_number(struct sub_request *head, struct sub_request *tail);
void get_read_sub_number_for_all(struct ssd_info *ssd);
void get_read_sub_number_for_channel(struct ssd_info *ssd, int channel);
void get_write_sub_number_for_dy(struct ssd_info *ssd);
void get_write_sub_number_for_all(struct ssd_info *ssd);
void get_write_sub_number_for_channel(struct ssd_info *ssd, int channel);
void request_sub_distribute(struct ssd_info *ssd);
int get_lpn_for_make_aged(struct ssd_info *ssd, int max_lpn);

extern int gc_block_num; //垃圾回收从后往前遍历的块数,专门用于垃圾回收
extern int gc_extra_num; //因垃圾回收块满而扩展的块数

extern int second_choice_num;
extern int third_choice_num;
#endif
