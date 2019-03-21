#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <errno.h>

#include "camera_file_parse.h"

#define CAMERA_DEVICE_CONFIG_FILE_NAME     "./device_config_hkipc"
#define CAMERA_COMMAND_CONFIG_FILE_NAME    "./command_config_hkipc"
#define CAMERA_PARAM_FILE_NAME             "./parameters_file_hkipc"

static int user_load_camera_alarm_config(camera_command_info          *mydev_command_info, mydev_json_obj dev_item_obj)
{

    int loop = 0;
    int valid_subdev_id = 0;
    mydev_json_obj subdev_array_obj = NULL;
    mydev_json_obj array_item_obj = NULL;
    mydev_json_obj subdev_item_obj = NULL;

    subdev_array_obj = mydev_json_get_object(dev_item_obj, "alarm");
    if(NULL != subdev_array_obj)
    {
        mydev_json_get_array_size(subdev_array_obj, &mydev_command_info->alarm_count);
        egsip_log_info("info  %d item\n", mydev_command_info->alarm_count);
        if(mydev_command_info->alarm_count>0)
        {
            for(loop=0;loop<mydev_command_info->alarm_count;loop++)
            {
                array_item_obj = mydev_json_get_array_item(subdev_array_obj,loop);
                if(NULL != array_item_obj)
                {
                    mydev_json_get_int(array_item_obj, "priority", (int *)&mydev_command_info->alarm_info[loop].priority);
                    mydev_json_get_string(array_item_obj, "time", 
                                            mydev_command_info->alarm_info[loop].time, 
                                            sizeof(mydev_command_info->alarm_info[loop].time));
                    mydev_json_get_int(array_item_obj, "method", (int *)&mydev_command_info->alarm_info[loop].method);
                    mydev_json_get_string(array_item_obj, "desc", 
                                            mydev_command_info->alarm_info[loop].desc, 
                                            sizeof(mydev_command_info->alarm_info[loop].desc));
                    mydev_json_get_double(array_item_obj, "longitude", (double *)&mydev_command_info->alarm_info[loop].longitude);
                    mydev_json_get_double(array_item_obj, "latitude", (double *)&mydev_command_info->alarm_info[loop].latitude);
                    mydev_json_get_int(array_item_obj, "alarm_type", (int *)&mydev_command_info->alarm_info[loop].alarm_type);
                    mydev_json_get_int(array_item_obj, "valid_subdev_id", (int *)&valid_subdev_id);
                    if(valid_subdev_id == 0)
                    {
                        mydev_command_info->alarm_info[loop].subdev_id = NULL;
                    }
                    else
                    {
                        subdev_item_obj = mydev_json_get_object(array_item_obj, "subdev");
                        if(NULL != subdev_item_obj)
                        {
                            mydev_json_get_int(subdev_item_obj, "subdev_type", 
                                                 (int *)&mydev_command_info->alarm_info[loop].subdev_id->subdev_type);
                            mydev_json_get_string(subdev_item_obj, "mac", 
                                                    mydev_command_info->alarm_info[loop].subdev_id->mac, 
                                                    sizeof(mydev_command_info->alarm_info[loop].subdev_id->mac));
                            mydev_json_get_int(subdev_item_obj, "subdev_num", 
                                                 (int *)&mydev_command_info->alarm_info[loop].subdev_id->subdev_num);
                        }
                        else
                        {
                            mydev_command_info->alarm_info[loop].subdev_id = NULL;
                        }
                    }
                    mydev_json_get_string(array_item_obj, "alarm_param", 
                                            mydev_command_info->alarm_info[loop].alarm_param, 
                                            sizeof(mydev_command_info->alarm_info[loop].alarm_param));
                }
            }
        }
    }

    return 0;
}

int user_file_load_camera_command_config(camera_command_info          *mydev_command_info)
{
    int ret = -1;
    int index = 0;
    int ch = 0;
    int conf_file_size = 0;
    int array_size = 0;
    int array_index = 0;
    int valid_info_cnt = 0;
    mydev_json_obj param_obj = NULL;
    mydev_json_obj dev_item_obj = NULL;
    char *param_buff = NULL;
    FILE *fd_conf = NULL;                   //配置文件描述符

    egsip_log_debug("load command config start.\n");
    egsip_log_info("load command config file(%s).\n", CAMERA_COMMAND_CONFIG_FILE_NAME);
    fd_conf = fopen(CAMERA_COMMAND_CONFIG_FILE_NAME,"r");
    if(NULL == fd_conf)
    {
        egsip_log_info("no found device config file(%s), door will exit.\n", CAMERA_COMMAND_CONFIG_FILE_NAME);
        return ret;
    }

    fseek(fd_conf,0L,SEEK_END);
    conf_file_size = ftell(fd_conf);

    param_buff = (char *)malloc(conf_file_size);
    if(NULL == param_buff)
    {
        egsip_log_error("malloc failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        egsip_log_info("parser command config file(%s) failed, exit.\n", CAMERA_COMMAND_CONFIG_FILE_NAME);
        fclose(fd_conf);
        return ret;
    }

    fseek(fd_conf,0L,SEEK_SET);
    while (1)
    {
        ch = fgetc(fd_conf);
        if (EOF == ch)
        {
            break;
        }
        if ('\n' == ch)
        {
            continue;
        }
        param_buff[index++] = ch;
    }

    param_buff[index] = '\0';
    egsip_log_debug("command config(%s).\n", param_buff);

    param_obj = mydev_json_parse(param_buff);
    if(NULL == param_obj)
    {
        egsip_log_info("parser failed, skip(%s).\n", param_buff);
    }

    mydev_json_get_array_size(param_obj, &array_size);
    for(array_index=0;array_index<array_size;array_index++)
    {
        dev_item_obj = mydev_json_get_array_item(param_obj,array_index);
        if(NULL != dev_item_obj)
        {
            mydev_json_get_int(dev_item_obj, "alarm_count", (int *)&mydev_command_info->alarm_count);
            if(mydev_command_info->alarm_count > MAX_COMMAND)
            {
                egsip_log_info("info count up to 5\n");
                mydev_command_info->alarm_count = MAX_COMMAND;
            }
            else if(mydev_command_info->alarm_count > 0)
            {
                user_load_camera_alarm_config(mydev_command_info, dev_item_obj);
            }
            valid_info_cnt++;
            ret = 0;
        }
    }

    if(NULL != param_obj)
    {
        mydev_json_clear(param_obj);
        param_obj = NULL;
    }

    fclose(fd_conf);
    free(param_buff);

    if(valid_info_cnt<=0)
    {
        egsip_log_info("no found valid info.\n");
        ret = -1;
    }

    return ret;
}

int user_file_load_camera_device_config(egsip_dev_info *mydev_info)
{
    int ret = -1;
    int index = 0;
    int ch = 0;
    int loop = 0;
    int conf_file_size = 0;
    int array_size = 0;
    int array_index = 0;
    int valid_dev_cnt = 0;
    mydev_json_obj param_obj = NULL;
    mydev_json_obj dev_item_obj = NULL;
    mydev_json_obj subdev_array_obj = NULL;
    mydev_json_obj array_item_obj = NULL;
    char *param_buff = NULL;
    FILE *fd_conf = NULL;                   //配置文件描述符
    egsip_subdev_info *subdev_info;

    egsip_log_debug("load device config start.\n");
    egsip_log_info("load device config file(%s).\n", CAMERA_DEVICE_CONFIG_FILE_NAME);
    fd_conf = fopen(CAMERA_DEVICE_CONFIG_FILE_NAME,"r");
    if(NULL == fd_conf)
    {
        egsip_log_info("no found device config file(%s), door will exit.\n", CAMERA_DEVICE_CONFIG_FILE_NAME);
        return ret;
    }

    fseek(fd_conf,0L,SEEK_END);
    conf_file_size = ftell(fd_conf);

    param_buff = (char *)malloc(conf_file_size);
    if(NULL == param_buff)
    {
        egsip_log_error("malloc  failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        egsip_log_info("parser device config file(%s) failed, exit.\n", CAMERA_DEVICE_CONFIG_FILE_NAME);
        fclose(fd_conf);
        return ret;
    }

    fseek(fd_conf,0L,SEEK_SET);
    while (1)
    {
        ch = fgetc(fd_conf);
        if (EOF == ch)
        {
            break;
        }
        if ('\n' == ch)
        {
            continue;
        }
        param_buff[index++] = ch;
    }

    param_buff[index] = '\0';
    egsip_log_debug("dev config(%s).\n", param_buff);

    param_obj = mydev_json_parse(param_buff);
    if(NULL == param_obj)
    {
        egsip_log_info("parser failed, skip(%s).\n", param_buff);
    }

    mydev_json_get_array_size(param_obj, &array_size);
    for(array_index=0;array_index<array_size;array_index++)
    {
        dev_item_obj = mydev_json_get_array_item(param_obj,array_index);
        if(NULL != dev_item_obj)
        {
            mydev_json_get_string(dev_item_obj, "server_addr", mydev_info->srv_addr, sizeof(mydev_info->srv_addr));
            mydev_json_get_string(dev_item_obj, "local_addr", mydev_info->local_addr, sizeof(mydev_info->local_addr));
            mydev_json_get_int(dev_item_obj, "dev_type", (int *)&mydev_info->dev_type);
            mydev_json_get_int(dev_item_obj, "call_dev_type", (int *)&mydev_info->call_dev_type);
            mydev_json_get_int(dev_item_obj, "vendor_num", (int *)&mydev_info->vendor_num);
            mydev_json_get_string(dev_item_obj, "mac", mydev_info->mac, sizeof(mydev_info->mac));
            mydev_json_get_string(dev_item_obj, "addr_code", mydev_info->addr_code, sizeof(mydev_info->addr_code));
            mydev_json_get_int(dev_item_obj, "dev_number", &mydev_info->dev_number);
            mydev_json_get_int(dev_item_obj, "subdev_count", &mydev_info->subdev_count);
            mydev_json_get_string(dev_item_obj, "dev_type_desc", mydev_info->dev_type_desc, sizeof(mydev_info->dev_type_desc));
            mydev_json_get_string(dev_item_obj, "vendor_name", mydev_info->vendor_name, sizeof(mydev_info->vendor_name));
            mydev_json_get_string(dev_item_obj, "model", mydev_info->model, sizeof(mydev_info->model));
            mydev_json_get_string(dev_item_obj, "fw_version", mydev_info->fw_version, sizeof(mydev_info->fw_version));
            if(mydev_info->subdev_count == 0)
            {
                mydev_info->subdev_info = NULL;
            }
            else
            {
                subdev_array_obj = mydev_json_get_object(dev_item_obj, "subdev");
                if(NULL != subdev_array_obj)
                {
                    //mydev_json_get_array_size(subdev_array_obj, &g_camera_dev_info.subdev_count);
                    if(mydev_info->subdev_count>0)
                    {
                        mydev_info->subdev_info = (egsip_subdev_info *)malloc(sizeof(egsip_subdev_info)*mydev_info->subdev_count);
                        for(loop=0;loop<mydev_info->subdev_count;loop++)
                        {
                            subdev_info = mydev_info->subdev_info + loop;
                            array_item_obj = mydev_json_get_array_item(subdev_array_obj,loop);
                            if(NULL != array_item_obj)
                            {
                                mydev_json_get_int(array_item_obj, "subdev_type", (int *)&subdev_info->subdev_id.subdev_type);
                                mydev_json_get_string(array_item_obj, "mac", subdev_info->subdev_id.mac, sizeof(subdev_info->subdev_id.mac));
                                mydev_json_get_int(array_item_obj, "subdev_num", (int *)&subdev_info->subdev_id.subdev_num);
                                mydev_json_get_string(array_item_obj, "name", subdev_info->name, sizeof(subdev_info->name));
                                mydev_json_get_string(array_item_obj, "vendor_name", subdev_info->vendor_name, sizeof(subdev_info->vendor_name));
                                mydev_json_get_string(array_item_obj, "model", subdev_info->model, sizeof(subdev_info->model));
                                mydev_json_get_string(array_item_obj, "owner", subdev_info->owner, sizeof(subdev_info->owner));
                                mydev_json_get_string(array_item_obj, "civilcode", subdev_info->civilcode, sizeof(subdev_info->civilcode));
                                mydev_json_get_string(array_item_obj, "address", subdev_info->address, sizeof(subdev_info->address));
                                mydev_json_get_int(array_item_obj, "safetyway", (int *)&subdev_info->safetyway);
                                mydev_json_get_int(array_item_obj, "registerway", (int *)&subdev_info->registerway);
                                mydev_json_get_int(array_item_obj, "secrecy", (int *)&subdev_info->secrecy);
                                mydev_json_get_int(array_item_obj, "status", (int *)&subdev_info->status);
                            }
                        }
                    }
                }
            }
            valid_dev_cnt++;
            ret = 0;
        }
    }

    if(NULL != param_obj)
    {
        mydev_json_clear(param_obj);
        param_obj = NULL;
    }

    fclose(fd_conf);
    free(param_buff);

    if(valid_dev_cnt<=0)
    {
        egsip_log_info("no found valid device.\n");
        ret = -1;
    }

    return ret;
}

int user_file_load_camera_parameters(char *pic_url)
{
    int ret = -1;
    int index = 0;
    int ch = 0;
    int conf_file_size = 0;
    mydev_json_obj param_obj = NULL;
    char *param_buff = NULL;
    FILE *fd_conf = NULL;                   //配置文件描述符

    egsip_log_debug("load device config start.\n");
    egsip_log_info("load device config file(%s).\n", CAMERA_DEVICE_CONFIG_FILE_NAME);
    fd_conf = fopen(CAMERA_DEVICE_CONFIG_FILE_NAME,"r");
    if(NULL == fd_conf)
    {
        egsip_log_error("no found device parameter file(%s), door will exit.\n", CAMERA_DEVICE_CONFIG_FILE_NAME);
        return ret;
    }

    fseek(fd_conf,0L,SEEK_END);
    conf_file_size = ftell(fd_conf);

    param_buff = (char *)malloc(conf_file_size);
    if(NULL == param_buff)
    {
        egsip_log_error("parser device v file(%s) failed, exit.\n", CAMERA_DEVICE_CONFIG_FILE_NAME);
        fclose(fd_conf);
        return ret;
    }

    fseek(fd_conf,0L,SEEK_SET);
    while (1)
    {
        ch = fgetc(fd_conf);
        if (EOF == ch)
        {
            break;
        }
        if ('\n' == ch)
        {
            continue;
        }
        param_buff[index++] = ch;
    }

    param_buff[index] = '\0';
    egsip_log_debug("dev parameter(%s).\n", param_buff);

    param_obj = mydev_json_parse(param_buff);
    if(NULL == param_obj)
    {
        egsip_log_info("parser failed, skip(%s).\n", param_buff);
    }

    mydev_json_add_string(param_obj, "pic_url", pic_url);

    if(NULL != param_obj)
    {
        mydev_json_clear(param_obj);
        param_obj = NULL;
    }

    fclose(fd_conf);
    free(param_buff);
    ret = 0;
    return ret;
}


int user_file_store_camera_parameters(char *pic_url)
{
    FILE *fd_conf = NULL;                   //文件描述符
    char *param_string = NULL;
    mydev_json_obj single_param_obj = NULL;
    int ret = -1;

    egsip_log_debug("store parameters start.\n");
    egsip_log_debug("store parameters file(%s).\n", CAMERA_PARAM_FILE_NAME);

    fd_conf = fopen(CAMERA_PARAM_FILE_NAME,"wb");
    if(NULL == fd_conf)
    {
        egsip_log_error("open boot load failed.\n");
        return ret;
    }

    single_param_obj = mydev_json_create_object();
    if(NULL == single_param_obj)
    {
        egsip_log_error("new create store param obj failed.\n");
        return ret;
    }

    mydev_json_add_string(single_param_obj, "pic_url", pic_url);

    param_string = mydev_json_print_tostring(single_param_obj);
    if(NULL != param_string)
    {
        fwrite(param_string, strlen(param_string), 1, fd_conf);
        free(param_string);
        param_string = NULL;
    }

    fclose(fd_conf);
    ret = 0;
    return ret;
}

