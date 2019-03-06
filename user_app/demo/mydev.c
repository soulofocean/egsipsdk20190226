#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "mydev.h"

egsip_dev_info g_mydev_info;              // 设备信息
egsip_camera_cb_tbl g_srv_req_cb_tbl;     // 服务器请求回调函数表
egsip_camera_if_tbl g_mydev_req_if_tbl;   // 设备请求接口函数表

int g_sess_id = 0;


// 设备状态回调函数实现
void mydev_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info)
{
    egsip_log_debug("handle(%d); status(%s).\n", handle, status);
}

// 被呼叫的回调函数实现
EGSIP_RET_CODE mydev_called_cb(int handle, int sess_id, egsip_dev_call_info *call_info)
{
    egsip_log_debug("handle(%d).\n", handle);

    return EGSIP_RET_SUCCESS;
}

// 呼叫被应答回调函数实现
EGSIP_RET_CODE mydev_call_answered_cb(int handle, int sess_id)
{
    egsip_log_debug("handle(%d)\n", handle);
    
    return EGSIP_RET_SUCCESS;
}

// 流传输开始通知回调函数实现
EGSIP_RET_CODE mydev_stream_started_cb(int handle, int sess_id, EGSIP_DEV_STREAM_TYPE stream_flag)
{
    egsip_log_debug("handle(%d)\n", handle);
    
    return EGSIP_RET_SUCCESS;
}

// 呼叫被停止回调函数实现
EGSIP_RET_CODE  mydev_call_stopped_cb(int handle, int sess_id, int status)
{
    egsip_log_debug("handle(%d)\n", handle);
    
    return EGSIP_RET_SUCCESS;
}

// 设备初始化函数
void mydev_init()
{
    egsip_log_debug("mydev_init\n");

    // 设置设备信息
    memset(&g_mydev_info, 0, sizeof(g_mydev_info));
    strcpy(g_mydev_info.srv_addr, "192.168.1.100:21000");
    strcpy(g_mydev_info.local_addr, "192.168.1.199:5060");
    g_mydev_info.dev_type = EGSIP_TYPE_CAMERA;
    g_mydev_info.vendor_num = EGSIP_VENDOR_NUM_EVERGRANDE;
    strcpy(g_mydev_info.mac, "0010FA43D532");
    g_mydev_info.call_dev_type = EGSIP_CALL_DEV_ENTRA_MACHINE;
    strcpy(g_mydev_info.addr_code, "10120000");
    g_mydev_info.dev_number = 1;

    strcpy(g_mydev_info.dev_type_desc, "IP Camera");
    strcpy(g_mydev_info.vendor_name, "Evergrande");
    strcpy(g_mydev_info.model, "DS-2C2D2EFF");
    strcpy(g_mydev_info.fw_version, "V2.1.0build20181201");
    
    g_mydev_info.subdev_count = 0;
    g_mydev_info.subdev_info = NULL;

    // 设置服务器请求回调函数表
    memset(&g_srv_req_cb_tbl, 0, sizeof(g_srv_req_cb_tbl));
    g_srv_req_cb_tbl.called_cb = mydev_called_cb;
    g_srv_req_cb_tbl.call_answered_cb = mydev_call_answered_cb;
    g_srv_req_cb_tbl.call_stopped_cb =mydev_call_stopped_cb;
    g_srv_req_cb_tbl.stream_started_cb =mydev_stream_started_cb;
}

// 报警上报接口的服务器回复回调函数实现
void mydev_alarm_report_res_cb(int handle, int sess_id, EGSIP_RET_CODE ret)
{
    egsip_log_debug("handle(%d); sess_id(%s); ret(%d).\n", handle, sess_id, ret);
}

// 设备报警上报函数
void mydev_report_alarm(int handle)
{
    if(g_mydev_req_if_tbl.dev_alarm_report_if)
    {
        egsip_dev_alarm_info alarm_info={0};
        g_sess_id++;
        (*g_mydev_req_if_tbl.dev_alarm_report_if)(handle, g_sess_id, mydev_alarm_report_res_cb, &alarm_info);
    }
}



