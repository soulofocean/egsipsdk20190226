#ifndef _CAMERA_FILE_PARSE_H
#define _CAMERA_FILE_PARSE_H
#include "mydev_json.h"
#include "camera_dev.h"



int user_file_load_camera_command_config(camera_command_info          *mydev_command_info);
int user_file_load_camera_device_config();
int user_file_load_camera_parameters(char *pic_url);
int user_file_store_camera_parameters(char *pic_url);
#endif
