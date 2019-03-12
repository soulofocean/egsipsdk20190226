#include "mydev_json.h"
#include "mydev.h"

#define  MAX_COMMAND 5   //每个IPC一次最多发送5条告警

typedef struct _command_info
{
    int alarm_count;
    egsip_acs_alarm_info alarm_info[MAX_COMMAND];
}command_info;

typedef struct _parameters_info
{
    int  pic_url_en;
    char pic_url[128];
    int  door_param_en;
    egsip_door_param_type door_pa;
    int  cert_param_en[MAX_CERT];
    egsip_cert_param      cert_pa[MAX_CERT];
}parameters_info;

int user_file_load_command_config(command_info          *mydev_command_info);
int user_file_load_device_config();
int user_file_load_parameters(parameters_info* dev_parameters);
int user_file_store_parameters(parameters_info* dev_parameters);
int user_file_del_parameters(parameters_info* dev_para);

