#include"mydev_json.h"
#include"egsip_sdk.h"
#include<stdlib.h>
#include"myMQ.h"
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include"mymuldev.h"
#include"myProtocol.h"
#include<unistd.h>
#include <sys/wait.h>
#include "doorphone_dev.h"
#include "doorphone_file_parse.h"
static struct list_head s_mydev_cert_list_head;
static struct list_head s_mydev_dev_list_head;
mydev_json_obj s_device_config_obj = NULL;  //设备用户配置信息
/*设备状态*/
static int s_mydev_status = 0;
static int s_mydev_dev_magic_num;           //设备唯一MAGIC数字

#define MUL_DEVICE_CONFIG_FILE_NAME    "./device_config_hkipc"
static int user_dev_enqueue(struct list_head *head, user_dev_info *dev_obj, int *magic_num)
{
    if( (NULL == dev_obj) ||
        (NULL == head))
    {
        egsip_log_error("input head/req_obj NULL.\n");
        return EGSIP_RET_ERROR;
    }

    s_mydev_dev_magic_num++;
    dev_obj->magic_num = s_mydev_dev_magic_num;
    list_add(&(dev_obj->node), head);
    if(NULL != magic_num)
    {
        *magic_num = dev_obj->magic_num;
    }

    return EGSIP_RET_SUCCESS;
}
static int user_dev_get_type(struct list_head *head, EGSIP_DEV_TYPE dev_type, user_dev_info **dev_obj)
{
    user_dev_info *pos, *n;

    if( (NULL == dev_obj) ||
        (NULL == head))
    {
        egsip_log_error("input head/req_obj NULL.\n");
        return EGSIP_RET_ERROR;
    }

    list_for_each_entry_safe(pos,n,head,node)
    {
        if(pos->dev_info.dev_type == dev_type)
        {
            egsip_log_debug("dev(type:%d)(head:0x%x) de queue list success.\n", dev_type, head);
            *dev_obj = pos;
            return EGSIP_RET_SUCCESS;
        }
    }

    egsip_log_error("dev(type:%d) de queue list failed.\n", dev_type);

    return EGSIP_RET_ERROR;
}

static int user_check_vendor_num(int vendor_num)
{
    int valid = -1;
    switch(vendor_num)
    {
        case EGSIP_VENDOR_NUM_HIKVISION:     // 海康
        case EGSIP_VENDOR_NUM_DAHUA:         // 大华
        case EGSIP_VENDOR_NUM_JIESHUN:       // 捷顺
        case EGSIP_VENDOR_NUM_ANJUBAO:       // 安居宝
        case EGSIP_VENDOR_NUM_LEELEN:        // 立林
        case EGSIP_VENDOR_NUM_IBM:           // IBM
        case EGSIP_VENDOR_NUM_HONEYWELL:     // 霍尼韦尔
        case EGSIP_VENDOR_NUM_EVERGRANDE:    // 恒大
        case EGSIP_VENDOR_NUM_HITACHI:       // 日立电梯
        case EGSIP_VENDOR_NUM_QLDT:          // 三菱电梯
        case EGSIP_VENDOR_NUM_OTIS:          // 奥的斯电梯
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_device_type(int device_type)
{
    int valid = -1;
    switch(device_type)
    {
        case EGSIP_TYPE_CAMERA:                      // IPC枪机
        case EGSIP_TYPE_EAGLEEYE_CAM:                // 鹰眼摄像机
        case EGSIP_TYPE_BALLHEAD_CAM:                // 球机
        case EGSIP_TYPE_FACE_CAP_CAM:                // 人脸抓拍机
        case EGSIP_TYPE_PARKING_CTRL:                // 停车场控制器
        case EGSIP_TYPE_PARK_LPN_ID:                 // 停车场车牌识别仪
        case EGSIP_TYPE_DOOR_CTRL:                   // 门禁控制器
        case EGSIP_TYPE_GATE_CTRL:                   // 人行通道控制器
        case EGSIP_TYPE_ENTRA_MACHINE:               // 门口机
        case EGSIP_TYPE_FENCE_MACHINE:               // 围墙机
        case EGSIP_TYPE_INDOOR_MACHINE:              // 室内机
        case EGSIP_TYPE_MGMT_MACHINE:                // 管理机
        case EGSIP_TYPE_ELE_LINK_CTRL:               // 电梯联动控制器
        case EGSIP_TYPE_PATROL_DEV:                  // 巡更设备
        case EGSIP_TYPE_SCREEN_CTRL:                 // 信息发布屏控制器
        case EGSIP_TYPE_BROADCAST_CTRL:              // 广播控制器
        case EGSIP_TYPE_ELEVATOR_CTRL:               // 电梯厂商控制器
        case EGSIP_TYPE_SMART_CTRL_KB:               // 智能控制终端
        case EGSIP_TYPE_CARPARK_CAM:                 // 车位检测相机
        case EGSIP_TYPE_ELEC_LPN_CTRL:               // 电子车位控制器
        //case EGSIP_TYPE_PARKING_LOCK_CONTROLLER:     // 车位锁控制器
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_device_id(char *id)
{
    if((NULL != id) &&
        (strlen(id) == 12))
    {
        return 1;
    }

    return -1;
}

static int user_check_subdevice_type(int dev_type, int subdevice_type)
{
    int valid = -1;
    switch(dev_type)
    {
        case EGSIP_TYPE_ENTRA_MACHINE:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_PARKING_CTRL  = 2005, // 停车场控制器
                case EGSIP_SUBTYPE_ENTRANCE_BLUETOOTH_READER:             // 门口机蓝牙读卡器
                case EGSIP_SUBTYPE_ENTRANCE_CPU_CARD_READER:              // 门口机CPU读头
                case EGSIP_SUBTYPE_ENTRANCE_FACE_READER:                  // 门口机人脸识别读卡器
                case EGSIP_SUBTYPE_ENTRANCE_FINGER_READER:                // 门口机指纹识别读卡器
                case EGSIP_SUBTYPE_ENTRANCE_IC_CARD_READER:               // 门口机IC卡读卡器
                case EGSIP_SUBTYPE_ENTRANCE_PASSWORD_KEYBOARD:            // 门口机密码输入键盘
                case EGSIP_SUBTYPE_ENTRANCE_QR_READER:                    // 门口机二维码读卡器
                case EGSIP_SUBTYPE_ENTRANCE_READER:                       // 门口机读头
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_CAMERA:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_CAMERA     = 2001, // IPC枪机
                case EGSIP_SUBTYPE_CAMERA_VIDEO_CHANNEL:                         // IPC枪机通道
                case EGSIP_SUBTYPE_CAMERA_ALERT_IN_CHANNEL:                      // IPC枪机告警输入通道
                case EGSIP_SUBTYPE_CAMERA_ALERT_OUT_CHANNEL:                     // IPC枪机告警输出通道
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_GATE_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_GATE_CTRL     = 2010, // 人行通道控制器
                case EGSIP_SUBTYPE_GATE_MACHINE:                                 // 人证读卡器（闸机）
                case EGSIP_SUBTYPE_GATE_IQR_READER:                              // 二维码读卡器（闸机）
                case EGSIP_SUBTYPE_GATE_IC_CARD_READER:                          // IC卡读卡器（闸机）
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_ELE_LINK_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_ELE_LINK_CTRL = 2016, // 电梯联动控制器
                case EGSIP_SUBTYPE_ELEVATOR_SUB_CONTROLLER:                      // 电梯联动控制子设备（电梯联动控制器里的虚拟子设备）
                //case EGSIP_SUBTYPE_ELEVATOR_CAR:                                 // 电梯轿厢
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_SCREEN_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_SCREEN_CTRL   = 2018, // 信息发布屏控制器
                case EGSIP_SUBTYPE_INFORMATION_SCREEN:                           // 信息发布屏
                case EGSIP_SUBTYPE_INFORMATION_LED_SCREEN:                       // 信息LED大屏
                case EGSIP_SUBTYPE_INFORMATION_LCD_SCREEN:                       // 信息LCD大屏
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_ELEVATOR_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_ELEVATOR_CTRL = 2020, // 电梯厂商控制器
                case EGSIP_SUBTYPE_ELEVATOR_IC_CARD_READER:                      // 电梯IC卡读头
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_SMART_CTRL_KB:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_SMART_CTRL_KB = 2021, // 智能控制终端
                case EGSIP_SUBTYPE_ELECTRIC_FENCE:                               // 电子围栏
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSIP_TYPE_ELEC_LPN_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSIP_TYPE_ELEC_LPN_CTRL = 2023, // 电子车位控制器
                case EGSIP_SUBTYPE_ELECTRIC_LPN_DISPLAY:                         // 电子车位显示屏
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
//        case EGSIP_TYPE_PARKING_LOCK_CONTROLLER:
//        {
//            switch(subdevice_type)
//            {
//                //EGSIP_TYPE_PARKING_LOCK_CONTROLLER = 2025, // 车位锁控制器
//                case EGSIP_SUBTYPE_PARKING_LOCK:                                 // 车位锁
//                {
//                    valid = 1;
//                    break;
//                }
//                default:
//                {
//                    break;
//                }
//            }
//        }
        default:
        {
            break;
        }
    }

    return valid;
}

int load_mul_device_config()
{
    int index = 0;
    int error_cnt = 0;
    int subdev_cnt = 0;
    int ch = 0;
    int loop = 0;
    int conf_file_size = 0;
    int comment_start = 0;
    int comment_end = 0;
    int single_comment = 0;
    int multi_comment = 0;
    int array_size = 0;
    int array_index = 0;
    int valid_dev_cnt = 0;
    int valid_subdev_cnt = 0;
    user_dev_info *user_dev = NULL;
    mydev_json_obj dev_item_obj = NULL;
    mydev_json_obj subdev_array_obj = NULL;
    mydev_json_obj array_item_obj = NULL;
    char *param_buff = NULL;
    char dev_name[128] = "";
    FILE *fd_conf = NULL;                   //配置文件描述符
    egsip_subdev_info *subdev_info;

    egsip_log_debug("load device config start.\n");
    egsip_log_user("load device config file(%s).\n", MUL_DEVICE_CONFIG_FILE_NAME);
    fd_conf = fopen(MUL_DEVICE_CONFIG_FILE_NAME,"r");
    if(NULL == fd_conf)
    {
        egsip_log_error("parameters file open failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        egsip_log_user("no found device config file(%s), door will exit.\n", MUL_DEVICE_CONFIG_FILE_NAME);
        return EGSIP_RET_ERROR;
    }

    fseek(fd_conf,0L,SEEK_END);
    conf_file_size = ftell(fd_conf);

    param_buff = (char *)malloc(conf_file_size);
    if(NULL == param_buff)
    {
        egsip_log_error("malloc  failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        egsip_log_user("parser device config file(%s) failed, exit.\n", MUL_DEVICE_CONFIG_FILE_NAME);
        fclose(fd_conf);
        return EGSIP_RET_ERROR;
    }
    memset(param_buff, 0, conf_file_size);

    fseek(fd_conf,0L,SEEK_SET);
    while (1)
    {
        ch = fgetc(fd_conf);
        if (EOF == ch)
        {
            break;
        }
        if (' ' == ch)
        {
            continue;
        }

        if (('/' == ch) && (0 == comment_start))
        {
            comment_start = 1;
            continue;
        }

        if ((1 == comment_start) &&
            (0 == single_comment) &&
            (0 == multi_comment))
        {
            if ('/' == ch)
            {
                single_comment = 1;
            }
            else if ('*' == ch)
            {
                multi_comment = 1;
            }
            else
            {
                continue;
            }
        }

        if (1 == single_comment)
        {
            if ('\n' == ch)
            {
                comment_start = 0;
                single_comment = 0;
            }
            continue;
        }

        if (1 == multi_comment)
        {
            if (('*' == ch) && (0 == comment_end))
            {
                comment_end = 1;
            }
            if (('/' == ch) && (1 == comment_end))
            {
                comment_start = 0;
                multi_comment = 0;
            }
            continue;
        }

        if ('\n' == ch)
        {
            continue;
        }
        param_buff[index++] = ch;
    }

    param_buff[index] = '\0';
    egsip_log_debug("dev config(%s).\n", param_buff);

    s_device_config_obj = mydev_json_parse(param_buff);
    if(NULL == s_device_config_obj)
    {
        egsip_log_info("parser failed, skip(%s).\n", param_buff);
    }
    free(param_buff);

	int fatal_error = 0;
    mydev_json_get_array_size(s_device_config_obj, &array_size);
    for(array_index=0;array_index<array_size;array_index++)
    {
        dev_item_obj = mydev_json_get_array_item(s_device_config_obj,array_index);
        if(NULL != dev_item_obj)
        {
            memset(dev_name, 0, sizeof(dev_name));
            mydev_json_get_string(dev_item_obj, "name", dev_name, sizeof(dev_name));

            user_dev = (user_dev_info *)malloc(sizeof(user_dev_info));
			//memset(&user_dev, 0, sizeof(user_dev));
            if(NULL == user_dev)
            {
                egsip_log_info("malloc failed, skip parser dev(name:%s).\n", dev_name);
                continue;
            }
            memset(user_dev, 0, sizeof(user_dev_info));
            mydev_json_get_string(dev_item_obj, "server_addr", user_dev->dev_info.srv_addr, sizeof(user_dev->dev_info.srv_addr));
			mydev_json_get_string(dev_item_obj, "local_addr", user_dev->dev_info.local_addr, sizeof(user_dev->dev_info.local_addr));
			mydev_json_get_int(dev_item_obj, "dev_type", (int *)&user_dev->dev_info.dev_type);
            mydev_json_get_int(dev_item_obj, "call_dev_type", (int *)&user_dev->dev_info.call_dev_type);
            mydev_json_get_int(dev_item_obj, "vendor_num", (int *)&user_dev->dev_info.vendor_num);
            mydev_json_get_string(dev_item_obj, "mac", user_dev->dev_info.mac, sizeof(user_dev->dev_info.mac));
            mydev_json_get_string(dev_item_obj, "addr_code", user_dev->dev_info.addr_code, sizeof(user_dev->dev_info.addr_code));
            mydev_json_get_int(dev_item_obj, "dev_number", &user_dev->dev_info.dev_number);
            mydev_json_get_string(dev_item_obj, "dev_type_desc", user_dev->dev_info.dev_type_desc, sizeof(user_dev->dev_info.dev_type_desc));
            mydev_json_get_string(dev_item_obj, "vendor_name", user_dev->dev_info.vendor_name, sizeof(user_dev->dev_info.vendor_name));
            mydev_json_get_string(dev_item_obj, "model", user_dev->dev_info.model, sizeof(user_dev->dev_info.model));
            mydev_json_get_string(dev_item_obj, "fw_version", user_dev->dev_info.fw_version, sizeof(user_dev->dev_info.fw_version));
            mydev_json_get_int(dev_item_obj, "subdev_count", &user_dev->dev_info.subdev_count);
            error_cnt = 0;
            if(user_check_vendor_num(user_dev->dev_info.vendor_num)<0)
            {
                egsip_log_info("invalid device vendornum(%04d), skip parser dev(name:%s).\n", user_dev->dev_info.vendor_num, dev_name);
                error_cnt++;
            }
            if(user_check_device_type(user_dev->dev_info.dev_type)<0)
            {
                egsip_log_info("invalid device dev_type(%d), skip parser dev(name:%s).\n", user_dev->dev_info.dev_type, dev_name);
                error_cnt++;
            }
            if(user_check_device_id(user_dev->dev_info.mac)<0)
            {
                egsip_log_info("invalid device mac(%s), skip parser dev(name:%s).\n", user_dev->dev_info.mac, dev_name);
                error_cnt++;
            }
            if(error_cnt>0)
            {
                free(user_dev);
                user_dev = NULL;
				fatal_error = 1;//有错误就退出，后续有需要在此修改
				break;
                //continue;
            }
            subdev_array_obj = mydev_json_get_object(dev_item_obj, "subdev");
			valid_subdev_cnt = 0;
            if(NULL != subdev_array_obj)
            {
                mydev_json_get_array_size(subdev_array_obj, &user_dev->dev_info.subdev_count);
                subdev_cnt = user_dev->dev_info.subdev_count;
                if(subdev_cnt>0)
                {
                    user_dev->dev_info.subdev_info = (egsip_subdev_info *)malloc(sizeof(egsip_subdev_info)*user_dev->dev_info.subdev_count);
                    if(NULL != user_dev->dev_info.subdev_info)
                    {
                        for(loop=0;loop<subdev_cnt;loop++)
                        {
                            subdev_info = user_dev->dev_info.subdev_info + loop;
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
                                error_cnt = 0;
                                if(user_check_subdevice_type(user_dev->dev_info.dev_type, subdev_info->subdev_id.subdev_type)<0)
                                {
                                    egsip_log_info("invalid subdev_type(%04d), skip parser dev(name:%s).\n", subdev_info->subdev_id.subdev_type, subdev_info->name);
                                    error_cnt++;
                                }
                                if(user_check_device_id(subdev_info->subdev_id.mac)<0)
                                {
                                    egsip_log_info("invalid subdev mac(%s), skip parser dev(name:%s).\n", subdev_info->subdev_id.mac, subdev_info->name);
                                    error_cnt++;
                                }
                                if(error_cnt>0)
                                {
                                    free(user_dev->dev_info.subdev_info);
                                    user_dev->dev_info.subdev_info = NULL;
									fatal_error = 1;//有错误就退出，后续有需要在此修改
									break;
									//continue;
                                }
                                else
                                {
                                    valid_subdev_cnt++;
                                }
                            }
                        }
                        user_dev->dev_info.subdev_count = valid_subdev_cnt;
						if(fatal_error)
						{
							break;//有错误就退出，后续有需要在此修改
						}
                    }
                    else
                    {
                        user_dev->dev_info.subdev_count = 0;
                    }
                }
            }
            valid_dev_cnt++;
            user_dev_enqueue(&s_mydev_dev_list_head, user_dev, NULL);
        }
    }

    fclose(fd_conf);

	if(fatal_error)
	{
		egsip_log_user("fatal_error detected, breake parse device config!\n");
		return EGSIP_RET_ERROR;
	}

    if(valid_dev_cnt<=0)
    {
        egsip_log_user("no found valid device.\n");
        return EGSIP_RET_ERROR;
    }
    else
    {
        return EGSIP_RET_SUCCESS;
    }
}
int genDevIDs(char *devid,int idlens, int offset)
{
	int ret = EGSIP_RET_SUCCESS;
	const int minlen = 4;
	int result = 0;
	if(idlens<minlen){
		egsip_log_error("idlens[%d]<minlen[%d]\n",idlens,minlen);
		return EGSIP_RET_ERROR;
	}
	if(offset>0){
		char endStr[minlen+1];
		strcpy(endStr,devid+idlens-minlen);
		//TODO:进位超过10000可能会计算错误，初始值弄小点就好
		result = atoi(endStr)+offset;
		if(result>10000)
			return EGSIP_RET_ERROR;
		snprintf(endStr,sizeof(endStr),"%04d",result);
		egsip_log_debug("size=%d,len=%d,endStr=%s\n",sizeof(endStr),strlen(endStr),endStr);
		strcpy(devid+idlens-minlen,endStr);
	}
	return ret;
}

int muldev_init()
{
	int ret = 0;
    egsip_log_debug("enter\n");
	INIT_LIST_HEAD(&(s_mydev_cert_list_head));
    INIT_LIST_HEAD(&(s_mydev_dev_list_head));
    ret = load_mul_device_config();
    if(ret < 0)
    {
        egsip_log_user("load dev conf file failed, please check.\n");
        return ret;
    }
	int mqid_tmp;
	ret = create_mq(SOCKET_RSV_MQ_KEY, &mqid_tmp);
	ret = create_mq(SOCKET_SEND_MQ_KEY, &mqid_tmp);
	ret = create_mq(SOCKET_SEND_SHORT_MQ_KEY, &mqid_tmp);
	s_mydev_status = 1;
	return ret;
}
int init_arg_arr(char (*arg_arr)[ARG_LEN],int arg_count)
{
	int index;
	for(index = 0;index < arg_count;index++){
		memset(arg_arr[index],0,ARG_LEN);
	}
	return 0;
}
//验证dev_init [ARR_Index] [DEV_Type] [Number]参数合法性
int check_dev_init_arg(char (*arg_arr)[ARG_LEN],unsigned int *dev_arr, int dev_arr_count,int used_count,char *str_tmp)
{
	int ret = 0;
	if(used_count!=4){
		sprintf(str_tmp,"Invalid used_count [%d]",used_count);
		egsip_log_error("%s\n",str_tmp);
		ret = -1;
	}
	else if(atoi(arg_arr[1])<0||atoi(arg_arr[1])>=DEV_FORK_LIST_MAX_SIZE){
		sprintf(str_tmp,"Invalid Index [%d](0-%d)",atoi(arg_arr[1]),DEV_FORK_LIST_MAX_SIZE - 1);
		egsip_log_error("%s\n",str_tmp);
		ret = -1;
	}
	else if(user_check_device_type(atoi(arg_arr[2]))<0){
		sprintf(str_tmp,"Invalid device type [%d]",atoi(arg_arr[2]));
		egsip_log_error("%s\n",str_tmp);
		ret = -1;
	}
	else if(atoi(arg_arr[3])<=0 ||atoi(arg_arr[3])>DEV_MAX_COUNT){
		sprintf(str_tmp,"Invalid device count [%d](1-%d)",atoi(arg_arr[3]),DEV_MAX_COUNT);
		egsip_log_error("%s\n",str_tmp);
		ret = -1;
	}
	else{
		int arr_index = 0;
		int dev_type = 0;
		for(arr_index=0;arr_index<dev_arr_count;++arr_index)
		{
			dev_type = GetDevType(dev_arr[arr_index]);
			if(dev_type==atoi(arg_arr[2]))
			{
				sprintf(str_tmp,"Invalid duplicate dev_type [%d]",atoi(arg_arr[2]));
				egsip_log_error("%s\n",str_tmp);
				ret = -1;
				break;
			}
		}
	}
	return ret;
}
//验证dev_ctl [DEV_Type] [DEV_Index] [CMD] [CMD_ARG]参数正确性
int check_dev_ctl_arg(char (*arg_arr)[ARG_LEN],unsigned int *dev_arr, int dev_arr_count, int used_count,char *str_tmp)
{
	int ret = 0;
	if(used_count<4){
		sprintf(str_tmp,"Invalid used_count [%d]",used_count);
		egsip_log_error("%s\n",str_tmp);
		ret = -1;
		return ret;
	}
	//第一个参数0意思为针对所有的dev,此时第二个参数无效可以为任意值，不需要进行此校验
	//第二个参数为0意思为针对此Type的所有dev
	if (atoi(arg_arr[1])!=0)
	{
		int arr_index = 0;
		int dev_type = 0;
		int dev_count = 0;
		int is_match = 0;
		for(arr_index=0;arr_index<dev_arr_count;++arr_index)
		{
			dev_type = GetDevType(dev_arr[arr_index]);
			dev_count = GetDevCount(dev_arr[arr_index]);
			if(dev_type==atoi(arg_arr[1]))
			{
				is_match = 1;
				if(atoi(arg_arr[2])>dev_count || atoi(arg_arr[2])<0)
				{
					is_match = 0;
					sprintf(str_tmp,"Invalid dev_count [%d](0-%d)",atoi(arg_arr[2]),dev_count);
					break;
				}
			}
		}
		if(!is_match)
		{
			if(strlen(str_tmp)==0)
			{
				sprintf(str_tmp,"Invalid match dev_type [%d]",atoi(arg_arr[1]));
			}
			ret = -1;
			return ret;
		}
	}
	//校验命令是否存在
//	if(strncmp(arg_arr[3], "alarm", strlen("status")) != 0&&
//			strncmp(arg_arr[3], "record", strlen("record")) != 0&&
//			strncmp(arg_arr[3], "event", strlen("event")) != 0&&
//			strncmp(arg_arr[3], "result", strlen("result")) != 0&&
//			strncmp(arg_arr[3], "fac_status", strlen("fac_status")) != 0&&
//			strncmp(arg_arr[3], "elevator_record", strlen("elevator_record")) != 0&&
//			strncmp(arg_arr[3], "fac_ba_status", strlen("fac_ba_status")) != 0&&
//			strncmp(arg_arr[3], "intercom", strlen("intercom")) != 0)
//	{
//		sprintf(str_tmp,"Invalid match cmd [%s]",arg_arr[3]);
//		ret = -1;
//	}
	return ret;
}
//向目标为dest_type和dest_offset-1的子设备发送buff中的信息
//如果dest_type为0则向长度为arr_size的dev_arr中包含的所有设备的MQ发送buff信息
//如果dest_type不为0而dest_offset为0则向dev_arr中Type为dest_type的所有设备的MQ发送buff消息
int dispatch_rcv_msg(unsigned int dest_type,unsigned int dest_offset, unsigned int dev_arr[],int arr_size,char*buff)
{
	int arr_index = 0;
	egsip_log_info("buff=%s\n",buff);
	if(dest_type == 0 || dest_offset ==0){
		for(arr_index=0;arr_index<arr_size;++arr_index){
			if(!*(dev_arr+arr_index)){
				break;
			}
			unsigned int dev_type = GetDevType(*(dev_arr+arr_index));
			unsigned int dev_count = GetDevCount(*(dev_arr+arr_index));
			unsigned int dev_index = 0;
			egsip_log_info("dev_type:%d dev_count:%d\n",dev_type,dev_count);
			for(;dev_index<dev_count;++dev_index){
				if(dest_type==0||dest_type==dev_type){
					PutDispatchMQ(dev_type,dev_index,buff);
				}
			}
		}
	}
	else{
		PutDispatchMQ(dest_type,dest_offset-1,buff);
	}
	return 0;
}
int processUploadInfo(user_dev_info *user_dev,char * input_req_cmd)
{
	//char input_req_dev[30] = {0};
	//char input_req_cont[1024] = {0};
	//char sub_dev_id[30] = {0};
	//char dev_id[30] = {0};
	//char tmp_content[1024] = {0};
	//char data[1024]={0};
	//egsip_subdev_id tmp_sub = user_dev->dev_info.subdev_info->subdev_id;
	//egsip_dev_info *tmp_main = &(user_dev->dev_info);
	//sprintf(sub_dev_id,"%04d%s%04d",tmp_sub.subdev_type,tmp_sub.subdev_mac,tmp_sub.subdev_num);
	//sprintf(dev_id,"%04d%04d%s",tmp_main->dev_type,tmp_main->vendor_num,tmp_main->id);
	egsip_log_info("input_req_cmd = %s\n",input_req_cmd);
	//egsip_log_info("sub_dev_id = %s\n",sub_dev_id);
	//egsip_log_info("dev_id = %s\n",dev_id);
	EGSIP_RET_CODE ret = EGSIP_RET_SUCCESS;
	ret = user_dev_get_type(&s_mydev_dev_list_head, user_dev->dev_info.dev_type, &user_dev);
	//对带参数的命令进行参数拆分
	char arg_arr[ARG_ARR_COUNT][ARG_LEN];
	int used_count;
	ret = init_arg_arr(arg_arr, ARG_ARR_COUNT);
	split_arg_by_space(input_req_cmd, arg_arr, ARG_ARR_COUNT, &used_count);
	//为了兼容下面代码，将原命令拷贝回来
	memset(input_req_cmd,0,MQ_INFO_BUFF);
	strcpy(input_req_cmd,arg_arr[0]);
	if(strcmp(input_req_cmd, "alarm") == 0)
    {
		if(user_dev->dev_info.dev_type == EGSIP_TYPE_ENTRA_MACHINE)
		{
			//alarm [EventType] [SubdevType] [DESC] [ImgUrl] [UsetID] [UserType]
			egsip_log_info("Parse devtype:[%d] cmd\n",user_dev->dev_info.dev_type);
			command_info  mydev_command_info;
			memset(&mydev_command_info, 0 ,sizeof(mydev_command_info));
			mydev_command_info.alarm_count = 1;
			mydev_command_info.alarm_info[0].event_type = atoi(arg_arr[1]);
			mydev_command_info.alarm_info[0].sub_dev.subdev_type = atoi(arg_arr[2]);
			strcpy(mydev_command_info.alarm_info[0].sub_dev.mac,user_dev->dev_info.mac);
			mydev_command_info.alarm_info[0].sub_dev.subdev_num = 1;
			get_current_time_str(1, mydev_command_info.alarm_info[0].event_time);
			strcpy(mydev_command_info.alarm_info[0].desc,arg_arr[3]);
			strcpy(mydev_command_info.alarm_info[0].pic_url,arg_arr[4]);
			strcpy(mydev_command_info.alarm_info[0].user_id,arg_arr[5]);
			mydev_command_info.alarm_info[0].user_type  =atoi(arg_arr[6]);
			egsip_log_info("Parse devtype:[%d] cmd comlete\n",user_dev->dev_info.dev_type);
			ret = doorphone_alarm_report(&mydev_command_info);
			egsip_log_info("doorphone_alarm_report:ret = [%d]\n",ret);
		}
		else
		{
			egsip_log_error("Not support dev type [%d] in [record] cmd\n",user_dev->dev_info.dev_type);
			return EGSIP_RET_NO_SURPORT;
		}
    }
    else
    {
        egsip_log_user("no found req(%s), skip.\n", input_req_cmd);
		ret = EGSIP_RET_ERROR;
    }
	return ret;
}

void Child_process_loop(user_dev_info *user_dev,int dev_offset)
{
	egsip_log_debug("Enter.\n");
	int ret;
	msg_struct msgbuff;
	int mqid = 0;
	msgbuff.msgType = GetMQMsgType(user_dev->dev_info.dev_type,dev_offset);
	memset(msgbuff.msgData.info, 0, sizeof(msgbuff.msgData.info));
	//创建自己的MQ
	ret = create_mq(GetDispatchMQKey(msgbuff.msgType), &mqid);
	if(ret>=0){
		while(1)
    	{
			ret = GetDispatchMQ(msgbuff.msgType,&msgbuff);
			if(ret >= EGSIP_RET_SUCCESS){
				egsip_log_info("[id:%s] get msg:[type:%d,offset:%d info:%s]\n",user_dev->dev_info.mac,msgbuff.msgData.devType,msgbuff.msgData.offset,msgbuff.msgData.info);
				if(strncmp(msgbuff.msgData.info, "dev_stop", strlen("dev_stop"))==0){
					egsip_log_info("ready to break\n");
					break;
				}
				processUploadInfo(user_dev,msgbuff.msgData.info);
				memset(msgbuff.msgData.info, 0, sizeof(msgbuff.msgData.info));
        	}
			else{
				sleep(1);
			}
    	}
	}
	egsip_log_info("I'm out\n");
	ret = DelDispatchMQ(msgbuff.msgType);
}
int mydev_init_by_type(EGSIP_DEV_TYPE dev_type)
{
	switch (dev_type)
		{
			case EGSIP_TYPE_ENTRA_MACHINE:
			{
				init_doorphone();
				break;
			}
			case EGSIP_TYPE_CAMERA:
			{
			}
			default:
			{
				egsip_log_error("Not support devType:%d\n",dev_type);
				return EGSIP_RET_ERROR;
			}
		}
	return EGSIP_RET_SUCCESS;
}
static int mydev_create_single(user_dev_info *user_dev)
{
	egsip_log_debug("enter.\n");
    int ret = 0;
    int get_len = 0;
    int srv_req_cb_len = 0;
    int dev_req_if_len = 0;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSIP_TYPE_ENTRA_MACHINE:
        {
            user_dev->status_cb_func = (void *)g_doorphone_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&g_doorphone_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&g_doorphone_req_if_tbl;
            srv_req_cb_len = sizeof(g_doorphone_srv_req_cb_tbl);
            dev_req_if_len = sizeof(g_doorphone_req_if_tbl);
            break;
        }
        default :
        {
			egsip_log_error("Not support devType[%d]\n",user_dev->dev_info.dev_type);
            break;
        }
    }
	egsip_log_debug("Subdev_Count=[%d]\n",user_dev->dev_info.subdev_count);
	int loop = 0;
	int subdev_cnt = user_dev->dev_info.subdev_count;
	egsip_subdev_info *subdev_info;
	if(NULL != user_dev->dev_info.subdev_info)
    {
		for(loop=0;loop<subdev_cnt;loop++)
		{
			subdev_info = user_dev->dev_info.subdev_info + loop;
			egsip_log_debug("sub_dev_id=[%d%12s%04d]\n",subdev_info->subdev_id.subdev_type,subdev_info->subdev_id.mac,subdev_info->subdev_id.subdev_num);
		}
	}
	else
	{
		egsip_log_debug("SUBDEV is NULL!\n");
	}
    ret = egsip_dev_create(&user_dev->dev_info, (egsip_dev_status_callback)user_dev->status_cb_func, &user_dev->dev_handle);
    if(ret != EGSIP_RET_SUCCESS)
    {
        egsip_log_error("(sub_id:%s) egsip_dev_create failed.\n", user_dev->dev_info.mac);
        return ret;
    }
    ret = egsip_dev_func_register(user_dev->dev_handle, user_dev->srv_req_cb_tbl, srv_req_cb_len,
                                    user_dev->dev_req_if_tbl, dev_req_if_len, &get_len);
    if(ret != EGSIP_RET_SUCCESS || get_len != dev_req_if_len)
    {
        egsip_dev_delete(user_dev->dev_handle);
        egsip_log_error(" dev(id:%s) func_register failed, ret(%d).\n", user_dev->dev_info.mac, ret);
		egsip_sdk_uninit();
        return ret;
    }
    egsip_log_info("(id:%s) dev create success.\n", user_dev->dev_info.mac);
    return EGSIP_RET_SUCCESS;
}
int free_dev_list()
{
    struct list_head *head;
    user_dev_info *pos, *n;

    head = &s_mydev_dev_list_head;
    list_for_each_entry_safe(pos,n,head,node)
    {
        //egsip_dev_delete(pos->dev_handle);
        list_del(&pos->node);
        if(NULL != pos->dev_info.subdev_info)
        {
            free(pos->dev_info.subdev_info);
        }
        free(pos);
        pos = NULL;
    }
	egsip_log_info("Free list complete!\n");
    return 0;
}

int my_dev_single_init(EGSIP_DEV_TYPE dev_type, int dev_offset)
{
	int ret;
	int loop = 0;
	user_dev_info *user_dev = NULL;
	egsip_subdev_info *sub_devinfo = NULL;
	ret = user_dev_get_type(&s_mydev_dev_list_head, dev_type, &user_dev);
    if(user_dev == NULL)
    {
        egsip_log_user("no found dev_type(%d), skip report.\n", dev_type);
        return ret;
    }
	ret = genDevIDs(user_dev->dev_info.mac, strlen(user_dev->dev_info.mac),dev_offset);
	user_dev->dev_info.dev_number += dev_offset;
	if(user_dev->dev_info.subdev_count > 0)
    {
        for(loop=0; loop<user_dev->dev_info.subdev_count; loop++)
        {
            sub_devinfo = user_dev->dev_info.subdev_info+loop;
            ret = genDevIDs(sub_devinfo->subdev_id.mac,strlen(sub_devinfo->subdev_id.mac), dev_offset);
        }
    }
	if(ret != EGSIP_RET_SUCCESS){
		return ret;
	}
	egsip_log_debug("Processing Main_DEV_ID:[%4d%4d%s]\n",user_dev->dev_info.vendor_num,user_dev->dev_info.dev_type,user_dev->dev_info.mac);
	ret = egsip_sdk_init(printf);
    if(ret != EGSIP_RET_SUCCESS)
    {
        egsip_log_error("[%s]egsip_sdk_init failed\n",user_dev->dev_info.mac);
        return ret;
    }
	ret = mydev_init_by_type(user_dev->dev_info.dev_type);
	if(ret!=EGSIP_RET_SUCCESS){
		egsip_log_error("(id:%s) mydev_init_by_type failed.\n", user_dev->dev_info.mac);
        return ret;
	}
    //ret = mydev_create();
    ret = mydev_create_single(user_dev);
    if(EGSIP_RET_SUCCESS != ret){
		egsip_log_error(" dev(id:%s) create failed, ret(%d).\n", user_dev->dev_info.mac, ret);
        return ret;
    }
    //ret = mydev_start();
	ret = egsip_dev_start(user_dev->dev_handle);
	if(ret != EGSIP_RET_SUCCESS){
		egsip_log_error("(id:%s) egsip_dev_start failed, ret(%d).\n", user_dev->dev_info.mac, ret);
		return ret;
    }
	else{
		egsip_log_user("(id:%s) dev start success.\n", user_dev->dev_info.mac);
	}
	Child_process_loop(user_dev,dev_offset);
	if(user_dev->dev_info.dev_type == EGSIP_TYPE_ENTRA_MACHINE)
	{
		mydev_del_doorphone();
	}
	//sleep(1);//等1秒再结束，为了防止主进程无限等待下去，待验证是否有效
	egsip_log_info("[id:%s]DelDispatchMQ ret = %d\n",user_dev->dev_info.mac,ret);
    //ret = mydev_stop();
    egsip_dev_stop(user_dev->dev_handle);
	egsip_log_info("[id:%s]mydev_stop ret = %d\n",user_dev->dev_info.mac,ret);
    egsip_dev_delete(user_dev->dev_handle);
	free_dev_list();
	egsip_log_info("[id:%s]mydev_delete ret = %d\n",user_dev->dev_info.mac,ret);
	kill(getpid(),9);
    egsip_sdk_uninit();
	return EGSIP_RET_SUCCESS;
}


int process_loop_msg()
{
	int ret;
	int process_count = 1;
	msg_struct msgbuff;
	int main_pid = getpid();
	RsvMsgProcResultEnum result = SEND_MSG;
	msgQueenDataType myarg;
	char arg_arr[ARG_ARR_COUNT][ARG_LEN];
	char str_tmp[200] = {0};
	unsigned int dev_arr[DEV_FORK_LIST_MAX_SIZE]={0};
	int used_count = 0;
	int isStarted = 0;
	//目前dev_ctl [DEV_Type] [DEV_Index] [CMD] (CMD_ARG)最长为5个
	int arg_current_len = 4;
	ret = init_arg_arr(arg_arr,ARG_ARR_COUNT);
	while(1){//主进程在此循环中执行
		ret = GetRsvMQ(&msgbuff);
		memset(str_tmp,0,sizeof(str_tmp));
		result = SEND_MSG;
		if(ret < 0){
			sleep(1);
			continue;
		}
		//dev_init [ARR_Index] [DEV_Type] [Number]
		//dev_start/dev_stop
		//dev_ctl [DEV_Type] [DEV_Index] [CMD] (CMD_ARG)
		ret = split_arg_by_space(msgbuff.msgData.info,arg_arr,arg_current_len,&used_count);
		if(strcmp(arg_arr[0],"dev_init")==0){
			egsip_log_info("Process dev_init\n");
			if(isStarted){
				strcpy(str_tmp,"dev is Started Please stop first!");
			}
			else{
				ret = check_dev_init_arg(arg_arr,dev_arr,DEV_FORK_LIST_MAX_SIZE,used_count,str_tmp);
				if(ret == 0)
				{
					//验证通过，加入列表
					ret = Update_Dev_Fork_List(dev_arr, atoi(arg_arr[1]), atoi(arg_arr[2]), atoi(arg_arr[3]));
					sprintf(str_tmp,"dev_init ret = [%d]",ret);
				}
			}
		}
		else if(strcmp(arg_arr[0],"dev_start")==0){
			egsip_log_info("Process dev_start\n");
			//TODO:Fork 子设备进程
			if(isStarted){
				strcpy(str_tmp,"dev is Started Please stop first!");
			}
			else{
				ret = ForkMulDev(dev_arr,&myarg);
				sprintf(str_tmp,"dev_start ret = [%d]",ret);
				if(ret<0){
					isStarted = 0;
				}
				else{
					isStarted = 1;
				}
				if(getpid()!=main_pid){
					//子进程跳出父进程的循环，奔向自己的循环，父进程继续在此循环处理消息
					break;
				}
			}
		}
		else if(strcmp(arg_arr[0],"dev_stop")==0){
			egsip_log_info("Process dev_stop\n");
			//分发stop消息，让子设备退出
			dispatch_rcv_msg(0,0,dev_arr,DEV_FORK_LIST_MAX_SIZE,arg_arr[0]);
			wait(NULL);//和signal(SIGCHLD, SIG_IGN)搭配用,等待其他子进程退出
			memset(dev_arr,0,sizeof(dev_arr));
			ret = init_arg_arr(arg_arr,ARG_ARR_COUNT);
			isStarted = 0;
			strcpy(str_tmp,"dev_stop success!");
			egsip_log_info("%s\n",str_tmp);
		}
		else if(strcmp(arg_arr[0],"dev_ctl")==0){
			egsip_log_info("Process dev_ctl\n");
			//check arg
			if(!isStarted){
				strcpy(str_tmp,"dev is stopped Please start it first!");
			}
			else{
				ret = check_dev_ctl_arg(arg_arr,dev_arr,DEV_FORK_LIST_MAX_SIZE,used_count,str_tmp);
				egsip_log_info("ret=%d\n",ret);
				if(ret == 0)
				{
					//验证通过，向设备发送消息
					//arg_arr[4]和后面的暂不处理，后续有需要再进行补充
					ret = dispatch_rcv_msg(atoi(arg_arr[1]),atoi(arg_arr[2]),dev_arr,DEV_FORK_LIST_MAX_SIZE,arg_arr[3]);
					sprintf(str_tmp,"dev_ctl ret = [%d]",ret);
				}
			}
		}
		else{
			sprintf(str_tmp,"Invalid CMD_TYPE [%s]",arg_arr[0]);
			egsip_log_error("%s\n",str_tmp);
		}
		egsip_log_info("result=%d\n",result);
		switch (result)
		{
			case DSP_MSG:
			{
				PutDispatchNMQ(msgbuff,process_count);
				break;
			}
			case SEND_MSG:
			{
				PutSendMQ(str_tmp);
				break;
			}
			case No_Need_Rsp:
			default: break;
		}
	}
	if(getpid()!=main_pid){
		//子进程的初始化和循环
		//TODO:在回调函数中需要返回状态
		egsip_log_info("[Child:%d] devType=%d offset=%d\n",getpid(),myarg.devType,myarg.offset);
		ret = my_dev_single_init(myarg.devType,myarg.offset);
	}
	return ret;
}

