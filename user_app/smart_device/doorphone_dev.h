#ifndef _DOORPHONE_DEV_H
#define _DOORPHONE_DEV_H

#include <egsip_sdk.h>
#include <egsip_util.h>
#include <enter_machine/egsip_enter_machine.h>


#define  MAX_CERT  512 //最多保存512个凭证条目

int g_doorphone_dev_handle;
egsip_dev_info g_doorphone_dev_info;                        // 设备信息
egsip_dev_status_callback g_doorphone_status_cb;	// 设备状态回调函数
egsip_enter_machine_cb_tbl g_doorphone_srv_req_cb_tbl;        // 服务器请求回调函数表
egsip_enter_machine_if_tbl g_doorphone_req_if_tbl;      // 设备请求接口函数表
#define  MAX_COMMAND 5   //每个IPC一次最多发送5条告警


typedef struct _egsip_doorphone_cert_param
{
    int using;
    egsip_dev_cb_certificate_param cert_param;
}egsip_doorphone_cert_param;

typedef struct _doorphone_command_info
{
    int alarm_count;
    egsip_acs_alarm_info alarm_info[MAX_COMMAND];
}doorphone_command_info;

typedef struct _doorphone_parameters_info
{
    int  pic_url_en;
    char pic_url[128];
    int  door_param_en;
    egsip_door_param_type door_pa;
    int  cert_param_en[MAX_CERT];
    egsip_doorphone_cert_param      cert_pa[MAX_CERT];
}doorphone_parameters_info;


void init_doorphone();
// 设备初始化函数
void camera_init_doorphone();

// 设备上报报警函数
void camera_report_alarm(int handle);
int doorphone_alarm_report(doorphone_command_info *mydev_command_info);

// 设备状态回调函数
void doorphone_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info);

int doorphone_del_doorphone();

#endif

