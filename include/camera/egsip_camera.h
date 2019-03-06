#ifndef _EGSIP_CAMERA_H_
#define _EGSIP_CAMERA_H_

#include "../egsip_command.h"

// IPC枪机 EGSIP_TYPE_CAMERA  -------------------------------------------------

// *** 设备专有参数 *****

// 设备额外信息
typedef struct _egsip_camera_extra_info
{
    int max_camera;
    int max_alarm;
}egsip_camera_extra_info;


// 服务器请求回调函数表
typedef struct _egsip_camera_cb_tbl
{
    egsip_dev_called_cb called_cb;
    egsip_dev_call_answered_cb call_answered_cb;
    egsip_dev_call_stopped_cb call_stopped_cb;
    egsip_dev_stream_started_cb stream_started_cb;
    egsip_dev_device_upgrade_cb device_upgrade_cb;
    egsip_dev_get_pic_storage_cb get_pic_storage_cb;
    egsip_dev_set_pic_storage_cb set_pic_storage_cb;
}egsip_camera_cb_tbl;

// 设备请求接口函数表

typedef struct _egsip_camera_if_tbl
{
    egsip_dev_call_if  call_if;
    egsip_dev_answer_call_if answer_call_if;
    egsip_dev_stop_call_if stop_call_if;
    egsip_dev_alarm_report_if dev_alarm_report_if;
}egsip_camera_if_tbl;

#endif
