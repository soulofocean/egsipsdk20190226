#ifndef _CAMERA_H
#define _CAMERA_H

#include <egsip_sdk.h>
#include <egsip_util.h>
#include <camera/egsip_camera.h>
#define  MAX_COMMAND 5   //每个IPC一次最多发送5条告警

typedef struct _camera_command_info
{
    int alarm_count;
    egsip_dev_alarm_info alarm_info[MAX_COMMAND];
}camera_command_info;


egsip_dev_info g_camera_dev_info;              // 设备信息
egsip_dev_status_callback g_camera_status_cb;	// 设备状态回调函数
egsip_camera_cb_tbl g_camera_srv_req_cb_tbl;     // 服务器请求回调函数表
egsip_camera_if_tbl g_camera_req_if_tbl;   // 设备请求接口函数表

void init_camera();
int start_camera_test_mul();
// 设备初始化函数
void mydev_init_camera();

int camera_del();

// 设备上报报警函数
void camera_alarm_report_by_id(int handle);
int camera_alarm_report_by_file(char *arg);
int camera_alarm_report(camera_command_info *mydev_command_info);


// 设备状态回调函数
void camera_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info);

#endif

