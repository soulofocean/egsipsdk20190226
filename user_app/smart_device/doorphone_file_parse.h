#ifndef _DOORPHONE_FILE_PARSE_H
#define _DOORPHONE_FILE_PARSE_H
#include "mydev_json.h"
#include "doorphone_dev.h"



int user_file_load_doorphone_command_config(doorphone_command_info          *mydev_command_info);
int user_file_load_doorphone_device_config();
int user_file_load_doorphone_parameters(doorphone_parameters_info* dev_parameters);
int user_file_store_doorphone_parameters(doorphone_parameters_info* dev_parameters);
int user_file_del_doorphone_parameters(doorphone_parameters_info* dev_para);
#endif
