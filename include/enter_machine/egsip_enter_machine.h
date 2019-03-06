#ifndef _EGSIP_ENTER_MACHINE_H_
#define _EGSIP_ENTER_MACHINE_H_

#include "../egsip_command.h"

// 门口机 EGSIP_TYPE_ENTRA_MACHINE  -------------------------------------------------

// *** 设备专有参数 *****

// 服务器请求回调函数表
typedef struct _egsip_enter_machine_cb_tbl
{
    egsip_dev_called_cb called_cb;
    egsip_dev_call_answered_cb call_answered_cb;
    egsip_dev_call_stopped_cb call_stopped_cb;
    egsip_dev_stream_started_cb stream_started_cb;
    egsip_dev_load_certificate_cb load_certificate_cb;
    egsip_dev_read_certificate_cb read_certificate_cb;
    egsip_dev_delete_certificate_cb delete_certificate_cb;
    egsip_dev_acs_door_open_cb door_open_cb;
    egsip_dev_set_pic_storage_cb set_pic_storage_cb;
    egsip_dev_get_door_param_cb get_door_param_cb;
    egsip_dev_set_door_param_cb set_door_param_cb;
    egsip_dev_device_upgrade_cb device_upgrade_cb;
}egsip_enter_machine_cb_tbl;

// 设备请求接口函数表

typedef struct _egsip_enter_machine_if_tbl
{
    egsip_dev_call_if  call_if;
    egsip_dev_answer_call_if answer_call_if;
    egsip_dev_stop_call_if stop_call_if;
    egsip_dev_acs_record_report_if record_report_if;
    egsip_dev_acs_alarm_report_if acs_alarm_report_if;
    egsip_dev_intercom_record_report_if intercom_report_if;
    egsip_dev_elevator_control_if elev_control_if;
    egsip_dev_intercom_unlock_record_report_if intercom_unlock_report_if;
}egsip_enter_machine_if_tbl;

#endif
