#ifndef _MYDEV_H
#define _MYDEV_H

#include <egsip_sdk.h>
#include <egsip_util.h>
#include <enter_machine/egsip_enter_machine.h>

#define  MAX_USERS 5   //每个IPC最多5路流
#define  MAX_CERT  512 //最多保存512个凭证条目

int g_mydev_handle;
egsip_dev_info g_mydev_info;                        // 设备信息
egsip_enter_machine_cb_tbl g_srv_req_cb_tbl;        // 服务器请求回调函数表
egsip_enter_machine_if_tbl g_mydev_req_if_tbl;      // 设备请求接口函数表

typedef struct _egsip_cert_param
{
    int using;
    egsip_dev_cb_certificate_param cert_param;
}egsip_cert_param;

// 设备初始化函数
void mydev_init();

// 设备上报报警函数
void mydev_report_alarm(int handle);

// 设备状态回调函数
void mydev_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info);

int mydev_del();

#endif

