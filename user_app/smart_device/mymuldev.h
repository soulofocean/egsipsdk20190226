#ifndef _MYMULDEV_H
#define _MYMULDEV_H

#include "mydev_list.h"
#include <egsip_util.h>
#include "egsip_sdk.h"
#define egsip_log_user(fmt, args...)  egsip_log_print(EGSIP_LOG_INFO,"[EGSIP] "fmt, ##args)
typedef struct _user_dev_info
{
    int dev_handle;                 //设备handle
    int status;                     //设备运行状态
    int magic_num;                  //设备唯一性编码
    int updated;                    //设备是否已升级
    int update_delay;               //设备升级延迟时间
    struct list_head node;          //设备链表节点
    egsip_dev_info dev_info;         //设备信息

    void *status_cb_func;           //设备状态回调接口
    void *srv_req_cb_tbl;           //服务器请求命令入口
    void *dev_req_if_tbl;           //设备请求命令入口
}user_dev_info;
int muldev_init();
int process_loop_msg();
#endif