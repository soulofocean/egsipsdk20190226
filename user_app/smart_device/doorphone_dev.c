#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#include "media_mgmt.h"
#include "mydev_json.h"
#include "doorphone_file_parse.h"

#include "myMQ.h"
#define  MAX_USERS 5   //每个IPC最多5路流

typedef struct _egsip_doorphone_dev_satus
{
    int using;
    int dev_call;
    int handle;
    int sess_id;
    char pic_url[128];
    EGSIP_DEV_STREAM_TYPE stream_flag;
    egsip_dev_call_info call_info;
}egsip_doorphone_dev_satus;


//static egsip_subdev_info   g_subdev[6] = {0};
static egsip_doorphone_dev_satus       mydev_status[MAX_CERT];
static int                   mydev_test_start;
static egsip_door_param_type mydev_door_pa;
static egsip_doorphone_cert_param      mydev_cert_pa[MAX_CERT];
static int                   video_format = 96;

void doorphone_record_report_cb(int handle, int msg_id, EGSIP_RET_CODE ret)
{
   egsip_log_debug("enter.\n");
   egsip_log_debug("handle(%d).\n", handle);
   egsip_log_debug("req_id(%d).\n", msg_id);
   egsip_log_debug("ret(%d).\n",ret);
}

// 门禁记录主动上报
EGSIP_RET_CODE  doorphone_record_report(int handle, int sess_id, egsip_acs_record_type *record)
{
    egsip_log_info("report info\n");
    g_doorphone_req_if_tbl.record_report_if(handle, sess_id, doorphone_record_report_cb, record);

    return 0;
}

void doorphone_call_report_cb(int handle, int msg_id, EGSIP_RET_CODE ret)
{
   egsip_log_debug("enter.\n");
   egsip_log_debug("handle(%d).\n", handle);
   egsip_log_debug("req_id(%d).\n", msg_id);
   egsip_log_debug("ret(%d).\n",ret);
}

// 对讲记录主动上报
EGSIP_RET_CODE  doorphone_call_report(int handle, int sess_id, egsip_intercom_record_report_info *record)
{
    egsip_log_info("report info\n");
    g_doorphone_req_if_tbl.intercom_report_if(handle, sess_id, doorphone_call_report_cb, record);

    return 0;
}

void doorphone_lock_report_cb(int handle, int msg_id, EGSIP_RET_CODE ret)
{
   egsip_log_debug("enter.\n");
   egsip_log_debug("handle(%d).\n", handle);
   egsip_log_debug("req_id(%d).\n", msg_id);
   egsip_log_debug("ret(%d).\n",ret);
}

// 开锁记录主动上报
EGSIP_RET_CODE  doorphone_lock_report(int handle, int sess_id, egsip_intercom_unlock_record_report_info *record)
{
    egsip_log_info("report info\n");
    g_doorphone_req_if_tbl.intercom_unlock_report_if(handle, sess_id, doorphone_lock_report_cb, record);

    return 0;
}

// 设备状态回调函数实现
void doorphone_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info)
{
	char msgTmp[1024] = {0}; 
	sprintf(msgTmp,"handle(%d) status=[%d] desc=[%s]\n", handle,status,desc_info);
    egsip_log_debug("%s\n", msgTmp);
	//PutSendMQ(msgTmp);
	DevMsgAck(status,msgTmp,USE_LONG_MSG);
    if (desc_info == NULL)
    {
        egsip_log_debug("handle(%d)  fail.\n", handle);
        return;
    }
    int i;
    switch(status)
    {
        case EGSIP_DEV_STATUS_STOPED:
            egsip_log_info("device init failed (%s)\n",desc_info);
            break;
        case EGSIP_DEV_STATUS_TO_REGISTER:
            for(i=0;i<MAX_USERS;i++)
            {
                if(mydev_status[i].handle == handle)
                {
                    memset(&(mydev_status[i]), 0, sizeof(egsip_doorphone_dev_satus));
                    break;
                }
            }
            egsip_log_info("device disconnect, reconnectting(%s)\n",desc_info);
            break;
        case EGSIP_DEV_STATUS_WORKING:
            egsip_log_info("device init success (%s)\n",desc_info);
            break;
        default:
            egsip_log_info("device status unknown (%s)\n",desc_info);
    }
}

// 被呼叫的回调函数实现
EGSIP_RET_CODE doorphone_called_cb(int handle, int sess_id, egsip_dev_call_info *call_info)
{
    if (call_info == NULL)
    {
        egsip_log_debug("handle(%d) sess_id(%d) called fail.\n", handle, sess_id);
        return EGSIP_RET_DATA_ERROR;
    }

    egsip_log_debug("handle(%d) sess_id(%d) be invited.\n", handle, sess_id);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].using == 0)
        {
            mydev_status[i].using = 1;
            //strncpy(mydev_status[i].call_id, call_id, sizeof(mydev_status[i].call_id));
            mydev_status[i].sess_id = sess_id;
            mydev_status[i].handle = handle;
            mydev_status[i].dev_call = 1;
            memcpy(&(mydev_status[i].call_info), call_info, sizeof(egsip_dev_call_info));
            call_info->caller_recv_audio.enable = 1;
            snprintf(call_info->caller_recv_audio.recv_ip, sizeof(call_info->caller_recv_audio.recv_ip),
                        "%s", "172.24.6.239");
            call_info->caller_recv_audio.recv_port = 61002;
            call_info->caller_recv_audio.format = 8;
            call_info->caller_recv_video.enable = 1;
            snprintf(call_info->caller_recv_video.recv_ip, sizeof(call_info->caller_recv_audio.recv_ip),
                        "%s", "172.24.6.239");
            call_info->caller_recv_video.recv_port = 61000;
            call_info->caller_recv_video.format = 96;
            break;
        }
    }
    return EGSIP_RET_SUCCESS;
}

// 流传输开始通知回调函数实现
EGSIP_RET_CODE doorphone_stream_started_cb(int handle, int sess_id, EGSIP_DEV_STREAM_TYPE stream_flag)
{
    egsip_log_debug("handle(%d) sess_id(%d) start stream_flag(%d) frame.\n", handle, sess_id,stream_flag);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            mydev_status[i].stream_flag |= stream_flag;
            break;
        }
    }

    return EGSIP_RET_SUCCESS;
}

// 呼叫被应答回调函数实现
EGSIP_RET_CODE doorphone_call_answered_cb(int handle, int sess_id)
{
    egsip_log_debug("handle(%d) sess_id(%d)\n", handle, sess_id);
    
    return EGSIP_RET_SUCCESS;
}

// 呼叫被停止回调函数实现
EGSIP_RET_CODE  doorphone_call_stopped_cb(int handle, int sess_id, int status)
{
    egsip_log_debug("handle(%d) sess_id(%d) status:%d\n", handle, sess_id, status);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            memset(&(mydev_status[i]), 0, sizeof(egsip_doorphone_dev_satus));

            char *url_pic = "jpge/test.jpg";
            char *invite_time = "2019-02-01 08:50:32";
            egsip_intercom_record_report_info open_record;
            memset(&open_record, 0, sizeof(open_record));

            memcpy(&open_record.call_info, &mydev_status[i].call_info.other_addr, sizeof(egsip_call_addr_info));
            open_record.invite_time = invite_time;
            open_record.talk_time = 30;
            open_record.answer = 1;
            open_record.lock = 1;
            open_record.url_pic = url_pic;

            //doorphone_call_report(mydev_status[i].handle, (mydev_status[i].sess_id+1), &open_record);
            break;
        }
    }

    return EGSIP_RET_SUCCESS;
}

// *** 门禁权限服务器下发通知 *** 
EGSIP_RET_CODE doorphone_load_certificate_cb(int handle, egsip_dev_cb_certificate_param *cert_param)
{
    if (cert_param == NULL)
    {
        egsip_log_debug("handle(%d) load fail.\n", handle);
        return EGSIP_RET_DATA_ERROR;
    }

    egsip_log_debug("handle(%d) user_id:%s user_type:%d start_time:%s end_time:%s id:%s \
                    credence_type:%d credence_no:%s op_time:%s\n", 
                    handle, cert_param->user_id, cert_param->user_type, cert_param->start_time, 
                    cert_param->end_time, cert_param->sub_dev[0].id, cert_param->credence_type,
                    cert_param->credence_no, cert_param->op_time);

    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));
    int i;
    for(i=0; i<MAX_CERT; i++)
    {
        if((mydev_cert_pa[i].using != 0) && 
             (memcmp(cert_param->credence_no, mydev_cert_pa[i].cert_param.credence_no, strlen(cert_param->credence_no)) == 0))
        {
            break;
        }

        if(mydev_cert_pa[i].using == 0)
        {
            mydev_cert_pa[i].using = 1;
            memcpy(&(mydev_cert_pa[i].cert_param), cert_param, sizeof(egsip_dev_cb_certificate_param));

            dev_para.cert_param_en[i] = 1;
            memcpy(&(dev_para.cert_pa[i]), &mydev_cert_pa[i], sizeof(egsip_doorphone_cert_param));
            user_file_store_doorphone_parameters(&dev_para);
            break;
        }
    }

    if(i < MAX_CERT)
    {
        egsip_log_debug("load this certificate success\n");
    }
    else
    {
        egsip_log_error("more than %d certificate\n", MAX_CERT);
    }

    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE doorphone_read_certificate_cb(int handle, int credence_type, char *credence_no,
                                          egsip_dev_cb_certificate_param *dev_param)
{
    if ((credence_no == NULL)||(dev_param == NULL))
    {
        egsip_log_debug("handle(%d) del fail.\n", handle);
        return EGSIP_RET_DATA_ERROR;
    }

    int i;
    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));
    for(i=0; i<MAX_CERT; i++)
    {
        if((mydev_cert_pa[i].using) && 
             (memcmp(credence_no,mydev_cert_pa[i].cert_param.credence_no,strlen(credence_no)) == 0))
        {
            memcpy(dev_param, &(mydev_cert_pa[i].cert_param), sizeof(egsip_dev_cb_certificate_param));
            break;
        }
        else
        {
            dev_para.cert_param_en[i] = 1;
            user_file_load_doorphone_parameters(&dev_para);
            memcpy(dev_param, &(dev_para.cert_pa[i].cert_param), sizeof(egsip_dev_cb_certificate_param));
        }
    }

    if(i < MAX_CERT)
    {
        egsip_log_debug("read this %s certificate success\n",credence_no);
        egsip_log_debug("handle(%d) user_id:%s user_type:%d start_time:%s start_time:%s id:%s credence_type:%d \
credence_no:%s op_time:%s\n", 
                handle, dev_param->user_id, dev_param->user_type, dev_param->start_time, dev_param->end_time,
                dev_param->sub_dev[0].id, dev_param->credence_type,
                dev_param->credence_no, dev_param->op_time);
    }
    else
    {
        egsip_log_error("not fand %s certificate\n",credence_no);
    }

    return EGSIP_RET_SUCCESS;
}

// *** 删除门禁权限服务器下发通知 *** 
EGSIP_RET_CODE doorphone_delete_certificate_cb(int handle, int credence_type, char *credence_no,
                                                     char *subdevice_id, char *user_id)
{
    if ((credence_no == NULL)||(subdevice_id == NULL)||(user_id == NULL))
    {
        egsip_log_debug("handle(%d) del fail.\n", handle);
        return EGSIP_RET_DATA_ERROR;
    }

    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));
    int i;
    for(i=0; i<MAX_CERT; i++)
    {
        if((mydev_cert_pa[i].using) && 
             (memcmp(credence_no,mydev_cert_pa[i].cert_param.credence_no,strlen(credence_no)) == 0))
        {
            dev_para.cert_param_en[i] = 1;
            user_file_del_doorphone_parameters(&dev_para);
            memset(&(mydev_cert_pa[i]), 0,  sizeof(egsip_doorphone_cert_param));
            break;
        }
    }

    if(i < MAX_CERT)
    {
        egsip_log_debug("del this %s certificate success\n",credence_no);
        egsip_log_debug("handle(%d) user_id:%s user_type:%d start_time:%s start_time:%s id:%s credence_type:%d \
credence_no:%s op_time:%s\n", 
                handle, mydev_cert_pa[i].cert_param.user_id, mydev_cert_pa[i].cert_param.user_type, 
                mydev_cert_pa[i].cert_param.start_time, mydev_cert_pa[i].cert_param.end_time,
                mydev_cert_pa[i].cert_param.sub_dev[0].id, mydev_cert_pa[i].cert_param.credence_type,
                mydev_cert_pa[i].cert_param.credence_no, mydev_cert_pa[i].cert_param.op_time);
    }
    else
    {
        egsip_log_error("not fand %s certificate\n",credence_no);
    }

    return EGSIP_RET_SUCCESS;
}

//开门
EGSIP_RET_CODE doorphone_acs_door_open_cb(int handle, int sess_id, egsip_door_open_type *door_open)
{
    if (door_open == NULL)
    {
        egsip_log_debug("handle(%d) sess_id(%d) opendoor info fail.\n", handle, sess_id);
        return EGSIP_RET_DATA_ERROR;
    }

    egsip_log_debug("handle(%d) sub_dev_id:%4d%12s%4d mode:%d user_id:%s user_type:%d\n", 
                    handle, door_open->sub_dev.subdev_type, door_open->sub_dev.mac,
                    door_open->sub_dev.subdev_num,door_open->mode, door_open->user_id,
                    door_open->user_type);

    //实现开门操作

    //开锁记录上报
    char *invite_time = "2019-01-11 14:41:32";
    egsip_intercom_unlock_record_report_info open_record;
    memset(&open_record, 0, sizeof(open_record));

    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].dev_call == 1)
        {
            memcpy(&open_record.call_info, &mydev_status[i].call_info, sizeof(egsip_call_addr_info));
            break;
        }
    }
    open_record.record_time = invite_time;
    open_record.mode = 1;
    open_record.result = 1;

    doorphone_lock_report(handle, (sess_id+1), &open_record);

#if 0
    //门禁记录上报
    egsip_acs_record_type record;

    record.type        = abs(rand())%20;
    record.user_id     = "1234567890";//用户id
    record.user_type   = 1;//用户类型
    record.sub_dev_id  ="34020000001320000004";//子设备id
    record.time        = "2018-01-15T19:04:33";//记录发生时间
    record.pic_url     = "test/1.jpg";
    record.pass_type   = abs(rand())%2;
    record.credence_no = "aaaaaaaaaabbbbbbbbbbbb";//凭证唯一标识
    
    doorphone_record_report(handle, sess_id, &record);
#endif

    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE doorphone_set_pic_storage_cb(int handle, int sess_id, char *http_url)
{
    if ((http_url == NULL) && (strlen(http_url) > 127))
    {
        egsip_log_debug("handle(%d) sess_id(%d) url_length(%d) pic info fail.\n", handle, sess_id, strlen(http_url));
        return EGSIP_RET_DATA_ERROR;
    }

    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            memcpy(mydev_status[i].pic_url, http_url, sizeof(mydev_status[i].pic_url));

            dev_para.pic_url_en = 1;
            memcpy(&(dev_para.pic_url), http_url, sizeof(dev_para.pic_url_en));
            user_file_store_doorphone_parameters(&dev_para);
            break;
        }
    }
    
    egsip_log_debug("handle:%d sess_id:%d http_url:%s\n", handle, sess_id, http_url);
    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE doorphone_get_pic_storage_cb(int handle, int sess_id, char *http_url)
{
    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].handle == handle)
        {
            memcpy(http_url, mydev_status[i].pic_url, sizeof(mydev_status[i].pic_url));
            break;
        }
        else
        {
            dev_para.pic_url_en = 1;
            user_file_load_doorphone_parameters(&dev_para);
            memcpy(http_url, &(dev_para.pic_url), sizeof(dev_para.pic_url));
        }
    }

    egsip_log_debug("handle:%d sess_id:%d http_url:%s\n", handle, sess_id, http_url);
    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE doorphone_get_door_param_cb(int handle, int sess_id, egsip_door_param_type *door)
{
    if (door == NULL)
    {
        egsip_log_debug("handle(%d) sess_id(%d) get door info fail.\n", handle, sess_id);
        return EGSIP_RET_DATA_ERROR;
    }

    //memcpy(door, &mydev_door_pa, sizeof(mydev_door_pa));
    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));

    dev_para.door_param_en = 1;
    user_file_load_doorphone_parameters(&dev_para);
    memcpy(door, &(dev_para.door_pa), sizeof(dev_para.door_pa));

    egsip_log_debug("handle:%d sess_id:%d open_durationl:%d alarm_timeout:%d ntp_server:%d\n",
                handle, sess_id, door->open_durationl, door->alarm_timeout, door->ntp_server);
    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE doorphone_set_door_param_cb(int handle, int sess_id, egsip_door_param_type *door)
{
    if (door == NULL)
    {
        egsip_log_debug("handle(%d) sess_id(%d) set door info fail.\n", handle, sess_id);
        return EGSIP_RET_DATA_ERROR;
    }

    egsip_log_debug("handle:%d sess_id:%d open_durationl:%d alarm_timeout:%d ntp_server:%d\n",
                handle, sess_id, door->open_durationl, door->alarm_timeout, door->ntp_server);

    //memcpy(&mydev_door_pa, door, sizeof(mydev_door_pa));
    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));

    dev_para.door_param_en = 1;
    memcpy(&(dev_para.door_pa), door, sizeof(dev_para.door_pa));
    user_file_store_doorphone_parameters(&dev_para);
    
    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE doorphone_device_upgrade_cb(int handle, int sess_id, char *file_url, char *ftp_addr)
{
    if ((file_url == NULL) || (ftp_addr == NULL))
    {
        egsip_log_debug("handle(%d) sess_id(%d) upgrade fail.\n", handle, sess_id);
        return EGSIP_RET_DATA_ERROR;
    }

    egsip_log_debug("handle:%d sess_id:%d file_url:%s ftp_addr:%s upgrade success\n", 
                     handle, sess_id, file_url, ftp_addr);

    return EGSIP_RET_SUCCESS;
}

void doorphone_alarm_report_res_cb(int handle, int msg_id, EGSIP_RET_CODE ret)
{
   egsip_log_debug("enter.\n");
   egsip_log_debug("handle(%d).\n", handle);
   egsip_log_debug("req_id(%d).\n", msg_id);
   egsip_log_debug("ret(%d).\n",ret);
   char tmp[1024]={0};
   sprintf(tmp,"handle[%d] msg_id[%d] ret[%d]",handle,msg_id,ret);
   //PutSendMQ(tmp);
   DevMsgAck(ret,tmp,USE_LONG_MSG);
}

int doorphone_alarm_report_by_id(char *arg)
{
    int loop = 0;
    doorphone_command_info  mydev_command_info;
    memset(&mydev_command_info, 0 ,sizeof(mydev_command_info));
    user_file_load_doorphone_command_config(&mydev_command_info);

    for(loop=0; loop<mydev_command_info.alarm_count; loop++)
    {
       egsip_log_info("report(%d) time =[%s].\n", loop,mydev_command_info.alarm_info[loop].event_time);
        g_doorphone_req_if_tbl.acs_alarm_report_if(mydev_status[0].handle, (mydev_status[0].sess_id+7+loop), 
                                            doorphone_alarm_report_res_cb, &(mydev_command_info.alarm_info[loop]));
    }
    return 0;
}
int doorphone_alarm_report(doorphone_command_info *mydev_command_info)
{
	int loop = 0;
	for(loop=0; loop<mydev_command_info->alarm_count; loop++)
    {
       egsip_log_info("report(%d) time =[%s].\n", loop,mydev_command_info->alarm_info[loop].event_time);
        g_doorphone_req_if_tbl.acs_alarm_report_if(mydev_status[0].handle, (mydev_status[0].sess_id+7+loop), 
                                            doorphone_alarm_report_res_cb, &(mydev_command_info->alarm_info[loop]));
		egsip_log_info("report(%d) complete.\n",loop);
    }
    return 0;
}

void doorphone_call_res_cb(int handle, int sess_id, EGSIP_RET_CODE ret, egsip_dev_call_info *call_info)
{
    if (call_info == NULL)
    {
        egsip_log_debug("handle(%d) sess_id(%d) call fail.\n", handle, sess_id);
        return;
    }
    egsip_log_debug("handle(%d) sess_id(%d) call sucess.\n", handle, sess_id);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            memcpy(&(mydev_status[i].call_info), call_info, sizeof(egsip_dev_call_info));
            break;
        }
    }
}

//主动呼叫
int doorphone_call_user(char *arg)
{
    int  sess_id = 7890;
    int  dev_num = 0;
    int  i = 0;
    int  ip_len = 0;
    char local_ip[16] = {0};

    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].using == 0)
        {
            dev_num = i;
            break;
        }
    }

    mydev_status[dev_num].handle = g_doorphone_dev_handle;
    mydev_status[dev_num].sess_id = sess_id;
    mydev_status[dev_num].using = 1;
    mydev_status[dev_num].dev_call = 1;
    if(0 == strncmp("89", arg, sizeof("89")))
    {
        //pc
        mydev_status[dev_num].call_info.other_addr.call_dev_type = EGSIP_CALL_DEV_MGMT_MACHINE;
        snprintf(mydev_status[dev_num].call_info.other_addr.addr_code, sizeof(mydev_status[dev_num].call_info.other_addr.addr_code), "%s", "99999999");
        mydev_status[dev_num].call_info.other_addr.dev_number = 1;
    }
    else
    {
        mydev_status[dev_num].call_info.other_addr.call_dev_type = EGSIP_CALL_DEV_INDOOR_MACHINE;
        snprintf(mydev_status[dev_num].call_info.other_addr.addr_code, sizeof(mydev_status[dev_num].call_info.other_addr.addr_code), "%s", arg);
        mydev_status[dev_num].call_info.other_addr.dev_number = 1;
    }

    ip_len = strstr(g_doorphone_dev_info.local_addr, ":") - g_doorphone_dev_info.local_addr;
    if(ip_len < 16)
    {
        strncpy(local_ip, g_doorphone_dev_info.local_addr, ip_len);
    }
    else
    {
        egsip_log_error("get local ip from (%s) failed\n", g_doorphone_dev_info.local_addr);
    }

    mydev_status[dev_num].call_info.caller_recv_audio.enable = 1;
    snprintf(mydev_status[dev_num].call_info.caller_recv_audio.recv_ip, sizeof(mydev_status[dev_num].call_info.caller_recv_audio.recv_ip), "%s", local_ip); 
    mydev_status[dev_num].call_info.caller_recv_audio.recv_port = UDP_BASE_PORT;
    mydev_status[dev_num].call_info.caller_recv_audio.format = 8;

    mydev_status[dev_num].call_info.caller_recv_video.enable = 1;
    snprintf(mydev_status[dev_num].call_info.caller_recv_video.recv_ip, sizeof(mydev_status[dev_num].call_info.caller_recv_audio.recv_ip), "%s", local_ip); 
    mydev_status[dev_num].call_info.caller_recv_video.recv_port = UDP_BASE_PORT+2;
    mydev_status[dev_num].call_info.caller_recv_video.format = video_format;

    g_doorphone_req_if_tbl.call_if(mydev_status[dev_num].handle, mydev_status[dev_num].sess_id, doorphone_call_res_cb, &(mydev_status[dev_num].call_info));

    return 0;
}

void doorphone_stop_call_res_cb(int handle, int sess_id, EGSIP_RET_CODE ret)
{
    egsip_log_debug("handle(%d) sess_id(%d) stop call\n", handle, sess_id);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            memset(&(mydev_status[i]), 0, sizeof(egsip_doorphone_dev_satus));
            break;
        }
    }
}

//主动挂断
int doorphone_stop_call_user(char *arg)
{
    int i = 0;

    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].dev_call == 1)
        {
            g_doorphone_req_if_tbl.stop_call_if(mydev_status[i].handle, mydev_status[i].sess_id, doorphone_stop_call_res_cb);

            char *url_pic = "jpge/test.jpg";
            char *invite_time = "2019-02-01 08:50:32";
            egsip_intercom_record_report_info open_record;
            memset(&open_record, 0, sizeof(open_record));

            memcpy(&open_record.call_info, &mydev_status[i].call_info.other_addr, sizeof(egsip_call_addr_info));
            open_record.invite_time = invite_time;
            open_record.talk_time = 30;
            open_record.answer = 1;
            open_record.lock = 1;
            open_record.url_pic = url_pic;

            doorphone_call_report(mydev_status[i].handle, (mydev_status[i].sess_id+1), &open_record);
        }
    }

    return 0;
}

void doorphone_call_lift_cb(int handle, int sess_id, EGSIP_RET_CODE ret)
{
    egsip_log_debug("handle(%d) sess_id(%d) calllift sucess.\n", handle, sess_id);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            memset(&(mydev_status[i]), 0, sizeof(egsip_doorphone_dev_satus));
            break;
        }
    }
}

//主动呼梯
int doorphone_call_lift(char *arg)
{
    int sess_id = 7891;
    int dev_num = 0;
    int i = 0;

    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].using == 0)
        {
            dev_num = i;
            break;
        }
    }

    int dir = atoi(arg);
    mydev_status[dev_num].handle = g_doorphone_dev_handle;
    mydev_status[dev_num].sess_id = sess_id;
    mydev_status[dev_num].using = 1;
    mydev_status[dev_num].dev_call = 1;

    g_doorphone_req_if_tbl.elev_control_if(mydev_status[dev_num].handle, mydev_status[dev_num].sess_id, doorphone_call_lift_cb, dir);

    return 0;
}

int doorphone_open_door(void *arg)
{
    if (arg == NULL)
    {
        egsip_log_error("opendoor info fail.\n");
        return EGSIP_RET_DATA_ERROR;
    }

    int sess_id = 111222;

    int i = 0;
    for(i=0;i<MAX_CERT;i++)
    {
        if(memcmp(arg, mydev_cert_pa[i].cert_param.credence_no, strlen(arg)) == 0)
        {
            //实现开门操作
            //门禁记录上报
            egsip_acs_record_type record;

#if 0
            record.type        = abs(rand())%20;
            record.user_id     = "1234567890";//用户id
            record.user_type   = 1;//用户类型
            record.sub_dev_id  ="34020000001320000004";//子设备id
            record.time        = "2018-01-15T19:04:33";//记录发生时间
            record.pic_url     = "test/1.jpg";
            record.pass_type   = abs(rand())%2;
            record.credence_no = "aaaaaaaaaabbbbbbbbbbbb";//凭证唯一标识
#endif
            record.type        = 2;
            memcpy(record.user_id, mydev_cert_pa[i].cert_param.user_id, sizeof(record.user_id));
            record.user_type   = mydev_cert_pa[i].cert_param.user_type;
            memcpy(record.sub_dev_id, mydev_cert_pa[i].cert_param.sub_dev[0].id, sizeof(record.sub_dev_id));
            memcpy(record.time, "2018-01-15T19:04:33", sizeof(record.time));
            memcpy(record.pic_url, "test/1.jpg", sizeof(record.pic_url));
            record.pass_type   = abs(rand())%2;
            memcpy(record.credence_no, arg, sizeof(record.credence_no));

            doorphone_record_report(g_doorphone_dev_handle, sess_id, &record);

            break;
        }
    }

    if(i < MAX_CERT)
    {
        egsip_log_debug("%s open door sucess.\n", arg);
    }
    else
    {
        egsip_log_debug("%s open door fail.\n", arg);
    }


    return EGSIP_RET_SUCCESS;
}


void *doorphone_input_test_task_fn(void *arg)
{
    int ret = 0;
    char *read_input = NULL;
    char input_req[1024];
    char input_req_cont[1024];

    while(mydev_test_start)
    {
        memset(input_req, 0, sizeof(input_req));
        read_input = fgets(input_req, sizeof(input_req), stdin);
        if(NULL == read_input)
        {
            egsip_log_info("input error,please retry\n");
            continue;
        }
        if('\n' == input_req[0])
        {
            continue;
        }

        if(strncmp(input_req, "help", strlen("help")) == 0)
        {
            egsip_log_info("support cmd list: \n");
            egsip_log_info("  help\n");
            egsip_log_info("  alarm\n");
            continue;
        }

        if(strncmp(input_req, "alarm", strlen("alarm")) == 0)
        {
            doorphone_alarm_report_by_id(input_req_cont);
        }
        else if(strncmp(input_req, "call", strlen("call")) == 0)
        {
            memset(input_req_cont, 0, sizeof(input_req_cont));
            ret = sscanf(input_req, "%*[a-z ]%[0-9]", input_req_cont);
            if(ret<0)
            {
                egsip_log_error("input format error %s,please retry\n", input_req_cont);
                continue;
            }
            egsip_log_info("get user req content(%s).\n", input_req_cont);
            doorphone_call_user(input_req_cont);
        }
        else if(strncmp(input_req, "stopcall", strlen("stopcall")) == 0)
        {
            memset(input_req_cont, 0, sizeof(input_req_cont));
            ret = sscanf(input_req, "%*[a-z ]%[0-9]", input_req_cont);
            if(ret<0)
            {
                egsip_log_error("input format error ,please retry\n", input_req_cont);
                continue;
            }
            egsip_log_info("get user req content(%s).\n", input_req_cont);
            doorphone_stop_call_user(input_req_cont);
        }
        else if(strncmp(input_req, "liftcall", strlen("liftcall")) == 0)
        {
            memset(input_req_cont, 0, sizeof(input_req_cont));
            ret = sscanf(input_req, "%*[a-z ]%[0-9]", input_req_cont);
            if(ret<0)
            {
                egsip_log_error("input format error ,please retry\n", input_req_cont);
                continue;
            }
            egsip_log_info("get user req content(%s).\n", input_req_cont);
            doorphone_call_lift(input_req_cont);
        }
        else if(strncmp(input_req, "alarm", strlen("alarm")) == 0)
        {
            doorphone_stop_call_user(input_req_cont);
        }
        else if(strncmp(input_req, "opendoor", strlen("opendoor")) == 0)
        {
            memset(input_req_cont, 0, sizeof(input_req_cont));
            ret = sscanf(input_req, "%*[a-z ]%[^\n]", input_req_cont);
            if(ret<0)
            {
                egsip_log_error("input format error ,please retry\n", input_req_cont);
                continue;
            }
            egsip_log_info("get user req content(%s).\n", input_req_cont);
            doorphone_open_door(input_req_cont);
        }
        else if(strncmp(input_req, "video", strlen("video")) == 0)
        {
            memset(input_req_cont, 0, sizeof(input_req_cont));
            if(sscanf(input_req, "%*[a-z ]%[0-9]", input_req_cont) < 0)
            {
                egsip_log_error("input format error %s ,please retry\n", input_req_cont);
                continue;
            }
            egsip_log_info("get user req content(%s).\n", input_req_cont);
            video_format = atoi(input_req_cont);
        }
        else
        {
            egsip_log_info("no found req(%s), skip.\n", input_req);
        }
        sleep(1);
    }

    return 0;
}

void *doorphone_send_video_task_fn(void *arg)
{
    struct pcap_pkthdr *ptk_header;
    FILE   *fp;
    int    i;
    int    start_send  = 0;
    int    pkt_offset  = 0;
    int    rtp_count   = 0;
    int    data_offset = 0;
    char   buf[BUFSIZE];
	size_t nowarning;

    if((fp = fopen(PCAP_VIDEO_FILE,"r")) == NULL)
    {
        egsip_log_error("error: can not open pcap file\n");
        return NULL;
    }

    struct stat statbuf;
    stat(PCAP_VIDEO_FILE,&statbuf);
    int file_size_cur=statbuf.st_size;
    int file_size=statbuf.st_size;

    int sock_fd = sock_udp_open(SOCK_DGRAM);
    if (sock_fd <= 0)
    {
        egsip_log_error("sock_open failed\n");
        return NULL;
    }

    ptk_header  = (struct pcap_pkthdr *)malloc(sizeof(struct pcap_pkthdr));

    //开始读数据包
    pkt_offset = 24; //pcap文件头结构 24个字节
    if(fseek(fp, pkt_offset, SEEK_SET))
    {
        egsip_log_error("error: can not open pcap file\n");
        return NULL;
    }

    while(mydev_test_start)
    {
        start_send = 0;
        for(i=0;i<MAX_USERS;i++)
        {
            if(mydev_status[i].stream_flag & EGSIP_DEV_STREAM_VIDEO_SEND)
            {
                start_send = 1;
                break;
            }
        }

        if(start_send)
        {
            if(file_size_cur<=1500)
            {
                file_size_cur = file_size;
                //开始读数据包
                pkt_offset = 24; //pcap文件头结构 24个字节
                if(fseek(fp, pkt_offset, SEEK_SET))
                {
                    egsip_log_error("error: can not open pcap file\n");
                    return NULL;
                }
                //break;
            }

            memset(buf, 0, sizeof(buf));
            //pcap_pkt_header 16 byte
            if(fread(ptk_header, 16, 1, fp) != 1) //读pcap数据包头结构
            {
                egsip_log_error("error: can not open pcap file\n");
                return NULL;
            }
            pkt_offset += 16 + ptk_header->caplen;   //下一个数据包的偏移值

            /*
            pcap文件组装解释 pcap文件头(24B,一个pcap文件只有一个);
                            pcap_pkt头(16B);
                            帧头,源MAC目的MAC(14B);
                            IP头(20B)；
                            UDP头(8B);之后就是RTP包
            */

            data_offset = 14 + 20 + 8;
            if(fseek(fp, data_offset, SEEK_CUR))
            {
                egsip_log_error("error: can not open pcap file\n");
                return NULL;
            }

            nowarning = fread(buf,(ptk_header->caplen - data_offset),1,fp);
			if(nowarning<0)
			{
				egsip_log_debug("nowarning = %u\n",nowarning);
			}

            StRtpFixedHdr *rtp_head = (StRtpFixedHdr *)buf;
            //egsip_log_info("rtp pay load type %d\n",rtp_head->u7Payload);

            if(rtp_head->u7Payload == 96)
            {
                for(i=0;i<MAX_USERS;i++)
                {
                    if((mydev_status[i].call_info.caller_send_video.enable == 1) &&
                        (mydev_status[i].stream_flag & EGSIP_DEV_STREAM_VIDEO_SEND))
                    {
                       sock_udp_send(sock_fd,
                                       mydev_status[i].call_info.caller_send_video.recv_ip, 
                                       mydev_status[i].call_info.caller_send_video.recv_port, 
                                       buf, (ptk_header->caplen - data_offset));
                    }
                }
                usleep(1000);
            }

            file_size_cur -= ((ptk_header->caplen - data_offset) + data_offset + 16);

            rtp_count++;
            if (rtp_count == 600)
            {
                egsip_log_info("rtp video len=%ld \n",(ptk_header->caplen - data_offset));
                rtp_count = 0;
            }
        }
        else
        {
            sleep(1);
        }
    }
    fclose(fp);
    free(ptk_header);
    sock_udp_close(sock_fd);
    return NULL;
}

void *doorphone_send_audio_task_fn(void *arg)
{
    struct pcap_pkthdr *ptk_header;
    FILE   *fp;
    int    i;
    int    start_send = 0;
    int    pkt_offset = 0;
    int    rtp_count = 0;
    int    data_offset = 0;
    char   buf[BUFSIZE];
	size_t nowarning;

    if((fp = fopen(PCAP_AUDIO_FILE,"r")) == NULL)
    {
        egsip_log_error("error: can not open pcap file\n");
        return NULL;
    }

    struct stat statbuf;
    stat(PCAP_AUDIO_FILE,&statbuf);
    int file_size_cur=statbuf.st_size;
    int file_size=statbuf.st_size;

    int sock_fd = sock_udp_open(SOCK_DGRAM);
    if (sock_fd <= 0)
    {
        egsip_log_error("sock_open failed\n");
        return NULL;
    }

    ptk_header  = (struct pcap_pkthdr *)malloc(sizeof(struct pcap_pkthdr));

    //开始读数据包
    pkt_offset = 24; //pcap文件头结构 24个字节
    if(fseek(fp, pkt_offset, SEEK_SET))
    {
        egsip_log_error("error: can not open pcap file\n");
        return NULL;
    }

    while(mydev_test_start)
    {
        start_send = 0;
        for(i=0;i<MAX_USERS;i++)
        {
            if(mydev_status[i].stream_flag & EGSIP_DEV_STREAM_VIDEO_SEND)
            {
                start_send = 1;
                break;
            }
        }

        if(start_send)
        {
            if(file_size_cur<=1500)
            {
                file_size_cur = file_size;
                //开始读数据包
                pkt_offset = 24; //pcap文件头结构 24个字节
                if(fseek(fp, pkt_offset, SEEK_SET))
                {
                    egsip_log_error("error: can not open pcap file\n");
                    return NULL;
                }
                //break;
            }

            memset(buf, 0, sizeof(buf));
            //pcap_pkt_header 16 byte
            if(fread(ptk_header, 16, 1, fp) != 1) //读pcap数据包头结构
            {
                egsip_log_error("error: can not open pcap file\n");
                return NULL;
            }
            pkt_offset += 16 + ptk_header->caplen;   //下一个数据包的偏移值

            /*
            pcap文件组装解释 pcap文件头(24B,一个pcap文件只有一个);
                            pcap_pkt头(16B);
                            帧头,源MAC目的MAC(14B);
                            IP头(20B)；
                            UDP头(8B);之后就是RTP包
            */

            data_offset = 14 + 20 + 8;
            if(fseek(fp, data_offset, SEEK_CUR))
            {
                egsip_log_error("error: can not open pcap file\n");
                return NULL;
            }

            nowarning = fread(buf,(ptk_header->caplen - data_offset),1,fp);
			if(nowarning<0)
			{
				egsip_log_debug("nowarning = %u\n",nowarning);
			}

            StRtpFixedHdr *rtp_head = (StRtpFixedHdr *)buf;
            //egsip_log_info("rtp pay load type %d\n",rtp_head->u7Payload);

            if(rtp_head->u7Payload != 96)
            {
                for(i=0;i<MAX_USERS;i++)
                {
                    if((mydev_status[i].call_info.caller_send_audio.enable == 1) &&
                        (mydev_status[i].stream_flag & EGSIP_DEV_STREAM_AUDIO_SEND))
                    {
                        sock_udp_send(sock_fd,
                                       mydev_status[i].call_info.caller_send_audio.recv_ip, 
                                       mydev_status[i].call_info.caller_send_audio.recv_port, 
                                       buf, (ptk_header->caplen - data_offset));
                    }
                }
                usleep(1000*350);
            }
            file_size_cur -= ((ptk_header->caplen - data_offset) + data_offset + 16);

            rtp_count++;
            if (rtp_count == 600)
            {
                egsip_log_info("rtp audio len=%ld \n",(ptk_header->caplen - data_offset));
                rtp_count = 0;
            }
        }
        else
        {
            sleep(1);
        }
    }
    fclose(fp);
    free(ptk_header);
    sock_udp_close(sock_fd);
    return NULL;
}

void *doorphone_recv_media_task_fn(void *arg)
{
    int fd;
    int ret;
    int i;
    int start_recv  = 0;
    int rtp_buf_len = MAX_RECV_BUF_LEN;
    STREAM_SESSION_PARAM session_param;
    fd_set read_fd_set;
    fd_set except_fd_set;
    struct timeval tmv_timeout = {2L, 1000L};

    init_signals();

    char * rtp_buf              = (char *)malloc(MAX_RECV_BUF_LEN);
    session_param.h264_pkt.buf  = (char *)malloc(H264_RTP_PKT_LEN);
    session_param.audio_pkt.buf = (char *)malloc(MAX_AUDIO_FRAME_LEN);
    session_param.ch_buf[0]     = (char *)malloc(CH1_BUF_LEN);

    if((rtp_buf == NULL) || (session_param.h264_pkt.buf == NULL) || (session_param.ch_buf[0] == NULL))
    {
        free(rtp_buf);
        free(session_param.h264_pkt.buf);
        free(session_param.ch_buf[0]);
        egsip_log_debug("malloc media buffer failed.\n");
        return 0;
    }
    memset(rtp_buf,0,(MAX_RECV_BUF_LEN));
    memset(session_param.h264_pkt.buf,0,(H264_RTP_PKT_LEN));
    memset(session_param.ch_buf[0],0,(CH1_BUF_LEN));

    fd = sock_udp_open(SOCK_DGRAM);
    sock_udp_bind(fd, UDP_BASE_PORT);

    while(mydev_test_start)
    {
        start_recv = 0;
        for(i=0;i<MAX_USERS;i++)
        {
            if(mydev_status[i].stream_flag & EGSIP_DEV_STREAM_AUDIO_RECV)
            {
                start_recv = 1;
                break;
            }
        }

        FD_ZERO(&read_fd_set);      // WRITE_EVENT
        FD_ZERO(&except_fd_set);    // EXCEPT_EVENT
        FD_SET(fd, &read_fd_set);
        FD_SET(fd, &except_fd_set);

        tmv_timeout.tv_sec  = 0;
        tmv_timeout.tv_usec = 100 * 1000;
        ret = select(fd + 1, &read_fd_set, NULL, &except_fd_set, &tmv_timeout);
        if ((ret > 0 ) && (start_recv == 1))
        {
            if (FD_ISSET(fd, &read_fd_set) != 0)
            {
                if ((ret = sock_udp_recv(fd, rtp_buf, rtp_buf_len)) > 0)
                {
                    udp_handle_data(&session_param, rtp_buf, ret);
                    FD_CLR(fd, &read_fd_set);
                    FD_CLR(fd, &except_fd_set);
                }
            }
            if (FD_ISSET(fd, &except_fd_set) != 0)
            {
                egsip_log_error("failed \n");
                FD_CLR(fd, &read_fd_set);
                FD_CLR(fd, &except_fd_set);
                break;
            }
        }
        else
        {
            //media_mgmt_info("select < 0,fd:%d media_index:%d\n",fd, media_index);
        }
        FD_CLR(fd, &read_fd_set);
        FD_CLR(fd, &except_fd_set);
    }

    sock_udp_close(fd);
    free(rtp_buf);
    free(session_param.h264_pkt.buf);
    free(session_param.ch_buf[0]);

    return 0;
}

int start_doorphone_test()
{
    int ret = -1;
    int arg = 0;
    pthread_t task_id = 0;

    ret = pthread_create( &task_id, NULL, doorphone_input_test_task_fn, (void *)(&arg));
    if((ret < 0) || (task_id < 0))
    {
        egsip_log_debug("mydev test task create failed.\n");
    }

    ret = pthread_create(&task_id, NULL,  doorphone_send_video_task_fn, (void *)(&arg));
    if((ret < 0) || (task_id < 0))
    {
        egsip_log_debug("mydev test task create failed.\n");
    }

    ret = pthread_create(&task_id, NULL,  doorphone_send_audio_task_fn, (void *)(&arg));
    if((ret < 0) || (task_id < 0))
    {
        egsip_log_debug("mydev test task create failed.\n");
    }

    ret = pthread_create(&task_id, NULL,  doorphone_recv_media_task_fn, (void *)(&arg));
    if((ret < 0) || (task_id < 0))
    {
        egsip_log_debug("mydev test task create failed.\n");
    }

    return 0;
}
void init_doorphone()
{
	// 设置服务器请求回调函数表
	g_doorphone_status_cb = doorphone_status_callback;
    memset(&g_doorphone_srv_req_cb_tbl, 0, sizeof(g_doorphone_srv_req_cb_tbl));
    g_doorphone_srv_req_cb_tbl.called_cb             = doorphone_called_cb;
    g_doorphone_srv_req_cb_tbl.call_answered_cb      = doorphone_call_answered_cb;
    g_doorphone_srv_req_cb_tbl.call_stopped_cb       = doorphone_call_stopped_cb;
    g_doorphone_srv_req_cb_tbl.stream_started_cb     = doorphone_stream_started_cb;
    g_doorphone_srv_req_cb_tbl.load_certificate_cb   = doorphone_load_certificate_cb;
    g_doorphone_srv_req_cb_tbl.read_certificate_cb   = doorphone_read_certificate_cb;
    g_doorphone_srv_req_cb_tbl.delete_certificate_cb = doorphone_delete_certificate_cb;
    g_doorphone_srv_req_cb_tbl.door_open_cb          = doorphone_acs_door_open_cb;
    g_doorphone_srv_req_cb_tbl.get_door_param_cb     = doorphone_get_door_param_cb;
    g_doorphone_srv_req_cb_tbl.set_door_param_cb     = doorphone_set_door_param_cb;
    g_doorphone_srv_req_cb_tbl.device_upgrade_cb     = doorphone_device_upgrade_cb;
    g_doorphone_srv_req_cb_tbl.set_pic_storage_cb    = doorphone_set_pic_storage_cb;
    //g_doorphone_srv_req_cb_tbl.get_pic_storage_cb    = doorphone_get_pic_storage_cb;

    memset(&mydev_door_pa, 0, sizeof(mydev_door_pa));
    memset(mydev_cert_pa, 0, sizeof(mydev_cert_pa));
    memset(mydev_status, 0, sizeof(egsip_doorphone_dev_satus)*MAX_CERT);

    int i = 0;
    doorphone_parameters_info dev_para;
    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));

    for(i = 0; i<MAX_CERT ; i++)
    {
        dev_para.cert_param_en[i] = 1;
    }
    dev_para.door_param_en = 1;
    dev_para.pic_url_en    = 1;
    user_file_load_doorphone_parameters(&dev_para);

    for(i = 0; i<5 ; i++)
    {
        memcpy(mydev_status[i].pic_url, &(dev_para.pic_url), sizeof(mydev_status[i].pic_url));
    }

    for(i = 0; i<MAX_CERT ; i++)
    {
        if(dev_para.cert_pa[i].using)
        {
            memcpy(&(mydev_cert_pa[i]), &(dev_para.cert_pa[i]), sizeof(egsip_doorphone_cert_param));
        }
    }

    memcpy(&mydev_door_pa, &(dev_para.door_pa), sizeof(mydev_door_pa));

    mydev_test_start = 1;
}
// 设备初始化函数
void camera_init_doorphone()
{
    egsip_log_debug("camera_init_doorphone\n");
#if 1
    // 设置设备信息
    int ret = 0;
    memset(&g_doorphone_dev_info, 0, sizeof(g_doorphone_dev_info));
    ret = user_file_load_doorphone_device_config(&g_doorphone_dev_info);
    if(ret < 0)
    {
        egsip_log_error("load dev conf file failed, please check.\n");
        return;
    }

    printf("file:%s function:%s, line: %d %s huzhe\n",__FILE__,__FUNCTION__,__LINE__, g_doorphone_dev_info.fw_version);
    
#else
    // 设置设备信息
    memset(&g_doorphone_dev_info, 0, sizeof(g_doorphone_dev_info));
    strcpy(g_doorphone_dev_info.srv_addr, "10.101.70.51:5060");
    strcpy(g_doorphone_dev_info.local_addr, "172.24.11.15:5060");
    g_doorphone_dev_info.dev_type = EGSIP_TYPE_ENTRA_MACHINE;
    g_doorphone_dev_info.vendor_num = EGSIP_VENDOR_NUM_HIKVISION;
    strcpy(g_doorphone_dev_info.mac, "000001055353");
    g_doorphone_dev_info.call_dev_type = EGSIP_CALL_DEV_ENTRA_MACHINE;
    strcpy(g_doorphone_dev_info.addr_code, "77550003");//楼栋号
    g_doorphone_dev_info.dev_number = 1;
    
    g_doorphone_dev_info.subdev_count = 6;
    g_doorphone_dev_info.subdev_info = g_subdev;

    int i;
    for (i = 0; i < 6; i++)
    {
        g_subdev[i].subdev_id.subdev_type = EGSIP_SUBTYPE_ENTRANCE_FACE_READER + i;
        strcpy(g_subdev[i].subdev_id.mac, "000001055353");
        g_subdev[i].subdev_id.subdev_num = 1;
        snprintf(g_subdev[i].name, sizeof(g_subdev[i].name), "door%d", i);
        snprintf(g_subdev[i].vendor_name, sizeof(g_subdev[i].vendor_name), "%s", "hk");
        snprintf(g_subdev[i].model, sizeof(g_subdev[i].model), "%s", "doorphone");
        snprintf(g_subdev[i].owner, sizeof(g_subdev[i].owner), "%s", "HD");
        snprintf(g_subdev[i].civilcode, sizeof(g_subdev[i].civilcode), "%s", "CHINA");
        snprintf(g_subdev[i].address, sizeof(g_subdev[i].address), "%s", "CHINA-1");
        g_subdev[i].safetyway = 0;
        g_subdev[i].registerway = 1;
        g_subdev[i].secrecy = 0;
        g_subdev[i].status = 1;
        g_subdev[i].extra_info = NULL;
    }
#endif

//    // 设置服务器请求回调函数表
//    memset(&g_doorphone_srv_req_cb_tbl, 0, sizeof(g_doorphone_srv_req_cb_tbl));
//    g_doorphone_srv_req_cb_tbl.called_cb             = doorphone_called_cb;
//    g_doorphone_srv_req_cb_tbl.call_answered_cb      = doorphone_call_answered_cb;
//    g_doorphone_srv_req_cb_tbl.call_stopped_cb       = doorphone_call_stopped_cb;
//    g_doorphone_srv_req_cb_tbl.stream_started_cb     = doorphone_stream_started_cb;
//    g_doorphone_srv_req_cb_tbl.load_certificate_cb   = doorphone_load_certificate_cb;
//    g_doorphone_srv_req_cb_tbl.read_certificate_cb   = doorphone_read_certificate_cb;
//    g_doorphone_srv_req_cb_tbl.delete_certificate_cb = doorphone_delete_certificate_cb;
//    g_doorphone_srv_req_cb_tbl.door_open_cb          = doorphone_acs_door_open_cb;
//    g_doorphone_srv_req_cb_tbl.get_door_param_cb     = doorphone_get_door_param_cb;
//    g_doorphone_srv_req_cb_tbl.set_door_param_cb     = doorphone_set_door_param_cb;
//    g_doorphone_srv_req_cb_tbl.device_upgrade_cb     = doorphone_device_upgrade_cb;
//    g_doorphone_srv_req_cb_tbl.set_pic_storage_cb    = doorphone_set_pic_storage_cb;
//    //g_doorphone_srv_req_cb_tbl.get_pic_storage_cb    = doorphone_get_pic_storage_cb;
//
//    memset(&mydev_door_pa, 0, sizeof(mydev_door_pa));
//    memset(mydev_cert_pa, 0, sizeof(mydev_cert_pa));
//    memset(mydev_status, 0, sizeof(egsip_doorphone_dev_satus)*MAX_CERT);
//
//    int i = 0;
//    doorphone_parameters_info dev_para;
//    memset(&dev_para, 0 , sizeof(doorphone_parameters_info));
//
//    for(i = 0; i<MAX_CERT ; i++)
//    {
//        dev_para.cert_param_en[i] = 1;
//    }
//    dev_para.door_param_en = 1;
//    dev_para.pic_url_en    = 1;
//    user_file_load_doorphone_parameters(&dev_para);
//
//    for(i = 0; i<5 ; i++)
//    {
//        memcpy(mydev_status[i].pic_url, &(dev_para.pic_url), sizeof(mydev_status[i].pic_url));
//    }
//
//    for(i = 0; i<MAX_CERT ; i++)
//    {
//        if(dev_para.cert_pa[i].using)
//        {
//            memcpy(&(mydev_cert_pa[i]), &(dev_para.cert_pa[i]), sizeof(egsip_doorphone_cert_param));
//        }
//    }
//
//    memcpy(&mydev_door_pa, &(dev_para.door_pa), sizeof(mydev_door_pa));
//
//    mydev_test_start = 1;
	init_doorphone();
    start_doorphone_test();
}

int doorphone_del_doorphone()
{
    mydev_test_start = 0;
    return 0;
}
