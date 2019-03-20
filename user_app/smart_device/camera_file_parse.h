#ifndef _CAMERA_FILE_PARSE_H
#define _CAMERA_FILE_PARSE_H
#include "mydev_json.h"
#include "camera_dev.h"

#define  MAX_COMMAND 5   //每个IPC一次最多发送5条告警

typedef struct _command_info
{
    int alarm_count;
    egsip_dev_alarm_info alarm_info[MAX_COMMAND];
}command_info;

int user_file_load_camera_command_config(command_info          *mydev_command_info);
int user_file_load_camera_device_config();
int user_file_load_camera_parameters(char *pic_url);
int user_file_store_camera_parameters(char *pic_url);
#endif
