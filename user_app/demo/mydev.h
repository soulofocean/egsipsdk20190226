#ifndef _MYDEV_H
#define _MYDEV_H

#include <egsip_sdk.h>
#include <egsip_util.h>
#include <camera/egsip_camera.h>

extern egsip_dev_info g_mydev_info;                 // 设备信息
extern egsip_camera_cb_tbl g_srv_req_cb_tbl;        // 服务器请求回调函数表
extern egsip_camera_if_tbl g_mydev_req_if_tbl;      // 设备请求接口函数表


// 设备初始化函数
void mydev_init();

// 设备上报报警函数
void mydev_report_alarm(int handle);

// 设备状态回调函数
void mydev_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info);

#endif

