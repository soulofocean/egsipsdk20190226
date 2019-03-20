#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "camera_dev.h"
#include "media_mgmt.h"
#include "mydev_json.h"
#include "camera_file_parse.h"
#include "myMQ.h"

#define  MAX_USERS 5   //每个IPC最多5路流

typedef struct _egsip_dev_satus
{
    int using;
    int start_send;
    int handle;
    int sess_id;
    char pic_url[128];
    egsip_dev_call_info call_info;
}egsip_dev_satus;


egsip_dev_satus mydev_status[5];
egsip_subdev_info g_subdev = {0};
static int        mydev_test_start;
static int        video_format = 96;


// 设备状态回调函数实现
void camera_status_callback(int handle, EGSIP_DEV_STATUS_CODE status,char *desc_info)
{
    egsip_log_debug("enter\n", handle);
	char msgTmp[1024] = {0}; 
	sprintf(msgTmp,"handle(%d) status=[%d] desc=[%s]\n", handle,status,desc_info);
    egsip_log_debug("%s\n", msgTmp);
	DevMsgAck(status,msgTmp,USE_LONG_MSG);
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
                    memset(&(mydev_status[i]), 0, sizeof(egsip_dev_satus));
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
EGSIP_RET_CODE camera_called_cb(int handle, int sess_id, egsip_dev_call_info *call_info)
{
    egsip_log_debug("handle(%d) sess_id(%d) be invited.\n", handle, sess_id);
    int i = 0;
    int  ip_len = 0;
    char local_ip[16] = {0};
    ip_len = strstr(g_camera_dev_info.local_addr, ":") - g_camera_dev_info.local_addr;
    if(ip_len < 16)
    {
        strncpy(local_ip, g_camera_dev_info.local_addr, ip_len);
    }
            else
    {
        egsip_log_error("get local ip from (%s) failed\n", g_camera_dev_info.local_addr);
    }

    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].using == 0)
        {
            mydev_status[i].using = 1;
            //strncpy(mydev_status[i].call_id, call_id, sizeof(mydev_status[i].call_id));
            mydev_status[i].sess_id = sess_id;
            mydev_status[i].handle = handle;

            call_info->caller_recv_audio.enable = 1;
            snprintf(call_info->caller_recv_audio.recv_ip, sizeof(call_info->caller_recv_audio.recv_ip), "%s", local_ip); 
            call_info->caller_recv_audio.recv_port = UDP_BASE_PORT;
            call_info->caller_recv_audio.format = 8;
            
            call_info->caller_recv_video.enable = 1;
            snprintf(call_info->caller_recv_video.recv_ip, sizeof(call_info->caller_recv_audio.recv_ip), "%s", local_ip); 
            call_info->caller_recv_video.recv_port = UDP_BASE_PORT+2;
            call_info->caller_recv_video.format = video_format;

            memcpy(&(mydev_status[i].call_info), call_info, sizeof(egsip_dev_call_info));
            break;
        }
    }
    return EGSIP_RET_SUCCESS;
}

// 流传输开始通知回调函数实现
EGSIP_RET_CODE camera_stream_started_cb(int handle, int sess_id, EGSIP_DEV_STREAM_TYPE stream_flag)
{
    egsip_log_debug("handle(%d) sess_id(%d) start send audio and video frame.\n", handle, sess_id);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            mydev_status[i].start_send = 1;
            break;
        }
    }

    return EGSIP_RET_SUCCESS;
}

// 呼叫被应答回调函数实现
EGSIP_RET_CODE camera_call_answered_cb(int handle, int sess_id)
{
    egsip_log_debug("handle(%d) sess_id(%d)\n", handle, sess_id);
    
    return EGSIP_RET_SUCCESS;
}

// 呼叫被停止回调函数实现
EGSIP_RET_CODE  camra_call_stopped_cb(int handle, int sess_id, int status)
{
    egsip_log_debug("handle(%d) sess_id(%d)\n", handle, sess_id);
    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].sess_id == sess_id))
        {
            memset(&(mydev_status[i]), 0, sizeof(egsip_dev_satus));
            break;
        }
    }

    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE camera_device_upgrade_cb(int handle, int sess_id, char *file_url, char *ftp_addr)
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

EGSIP_RET_CODE camera_set_pic_storage_cb(int handle, int sess_id, char *http_url)
{
    if ((http_url == NULL) && (strlen(http_url) > 127))
    {
        egsip_log_debug("handle(%d) sess_id(%d) url_length(%d) pic info fail.\n", handle, sess_id, strlen(http_url));
        return EGSIP_RET_DATA_ERROR;
    }

    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if(mydev_status[i].handle == handle)
        {
            memcpy(mydev_status[i].pic_url, http_url, sizeof(mydev_status[i].pic_url));
            user_file_store_camera_parameters(http_url);
            break;
        }
    }
    
    egsip_log_debug("handle:%d sess_id:%d http_url:%s\n", handle, sess_id, http_url);
    return EGSIP_RET_SUCCESS;
}

EGSIP_RET_CODE camera_get_pic_storage_cb(int handle, int sess_id, char *http_url)
{

    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        if((mydev_status[i].handle == handle)&&(mydev_status[i].pic_url != NULL))
        {
            memcpy(mydev_status[i].pic_url, mydev_status[i].pic_url, sizeof(mydev_status[i].pic_url));
            break;
        }
        else
        {
            user_file_load_camera_parameters(http_url);
        }
    }

    egsip_log_debug("handle:%d sess_id:%d http_url:%s\n", handle, sess_id, http_url);
    return EGSIP_RET_SUCCESS;
}

void camera_alarm_report_res_cb(int handle, int msg_id, EGSIP_RET_CODE ret)
{
   egsip_log_debug("enter.\n");
   egsip_log_debug("handle(%d).\n", handle);
   egsip_log_debug("req_id(%d).\n", msg_id);
   egsip_log_debug("ret(%d).\n",ret);
}

int camera_alarm_report(char *arg)
    {
        int loop = 0;
        int i = 0;
        command_info  mydev_command_info;
        memset(&mydev_command_info, 0 ,sizeof(mydev_command_info));

#if 0
        alarm_info.priority    = 2;
        alarm_info.time        = "2018-12-13T17:03:32";
        alarm_info.method      = 5;
        alarm_info.desc        = "hd test";
        alarm_info.longitude   = 17.17;
        alarm_info.latitude    = 18.18;
        alarm_info.alarm_type  = 12;
        alarm_info.subdev_id   = NULL;
        alarm_info.alarm_param = NULL;
#endif
    
        for(loop=0; loop<MAX_COMMAND; loop++)
        {
            mydev_command_info.alarm_info[loop].subdev_id = (egsip_subdev_id *)malloc(sizeof(egsip_subdev_id));
            if(mydev_command_info.alarm_info[loop].subdev_id == NULL)
            {
                for(i=0; i<loop; i++)
                {
                    free(mydev_command_info.alarm_info[i].subdev_id);
                }
    
                return -1;
            }
        }
    
        user_file_load_camera_command_config(&mydev_command_info);
    
        for(loop=0; loop<mydev_command_info.alarm_count; loop++)
        {
           egsip_log_info("report(%d).\n", loop);
            g_camera_req_if_tbl.dev_alarm_report_if(mydev_status[0].handle, (mydev_status[0].sess_id+7+loop), 
                                                camera_alarm_report_res_cb, &(mydev_command_info.alarm_info[loop]));
        }
    
        for(loop=0; loop<MAX_COMMAND; loop++)
        {
            free(mydev_command_info.alarm_info[loop].subdev_id);
        }

        return 0;
    }


void *camera_input_test_task_fn(void *arg)
{
    char *read_input = NULL;
    char input_req[1024];
    char input_req_cmd[64];
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
            egsip_log_info("  video 96\n");
            continue;
        }

        if(strncmp(input_req, "alarm", strlen("alarm")) == 0)
        {
            camera_alarm_report(input_req_cont);
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
            egsip_log_info("no found req(%s), skip.\n", input_req_cmd);
        }
        sleep(1);
    }
    return NULL;
}

void *camera_send_media_task_fn(void *arg)
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

    if((fp = fopen(PCAP_FILE,"r")) == NULL)
    {
        printf("error: can not open pcap file\n");
        return NULL;
    }

    struct stat statbuf;
    stat(PCAP_FILE,&statbuf);
    int file_size_cur=statbuf.st_size;
    int file_size=statbuf.st_size;

    int sock_fd = sock_udp_open(SOCK_DGRAM);
    if (sock_fd <= 0)
    {
        printf("sock_open failed\n");
        return NULL;
    }

    ptk_header  = (struct pcap_pkthdr *)malloc(sizeof(struct pcap_pkthdr));

    //开始读数据包
    pkt_offset = 24; //pcap文件头结构 24个字节
    if(fseek(fp, pkt_offset, SEEK_SET))
    {
        printf("error: can not open pcap file\n");
        return NULL;
    }

    while(mydev_test_start)
    {
        start_send = 0;
        for(i=0;i<MAX_USERS;i++)
        {
            if(mydev_status[i].start_send == 1)
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
                    printf("error: can not open pcap file\n");
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
			egsip_log_debug("nowarning = [%u]\n",nowarning);

            for(i=0;i<MAX_USERS;i++)
            {
                if(mydev_status[i].call_info.caller_send_video.enable == 1)
                {
                    sock_udp_send(sock_fd,
                                   mydev_status[i].call_info.caller_send_video.recv_ip, 
                                   mydev_status[i].call_info.caller_send_video.recv_port, 
                                   buf, (ptk_header->caplen - data_offset));
                }
            }

             file_size_cur -= ((ptk_header->caplen - data_offset) + data_offset + 16);

             rtp_count++;
             if (rtp_count == 600)
             {
                 egsip_log_info("rtp len=%ld \n",(ptk_header->caplen - data_offset));
                 rtp_count = 0;
             }

             usleep(1000);
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

int start_camera_test()
{
    int ret = -1;
    int arg = 0;
    pthread_t task_id = 0;
    ret = pthread_create( &task_id, NULL, camera_input_test_task_fn, (void *)(&arg));
    if((ret < 0) || (task_id < 0))
    {
        egsip_log_debug("mydev test task create failed.\n");
    }

    ret = pthread_create(&task_id, NULL,  camera_send_media_task_fn, (void *)(&arg));
    if((ret < 0) || (task_id < 0))
    {
        egsip_log_debug("mydev test task create failed.\n");
    }

    return 0;
}
void init_camera()
{
	
	egsip_log_debug("enter\n");

    // 设置服务器请求回调函数表
    g_camera_status_cb = camera_status_callback;
    memset(&g_camera_srv_req_cb_tbl, 0, sizeof(g_camera_srv_req_cb_tbl));
    g_camera_srv_req_cb_tbl.called_cb         = camera_called_cb;
    g_camera_srv_req_cb_tbl.stream_started_cb = camera_stream_started_cb;
    g_camera_srv_req_cb_tbl.call_answered_cb  = camera_call_answered_cb;
    g_camera_srv_req_cb_tbl.call_stopped_cb   = camra_call_stopped_cb;
    g_camera_srv_req_cb_tbl.device_upgrade_cb = camera_device_upgrade_cb;
    g_camera_srv_req_cb_tbl.set_pic_storage_cb= camera_set_pic_storage_cb;
    g_camera_srv_req_cb_tbl.get_pic_storage_cb= camera_get_pic_storage_cb;

    mydev_test_start = 1;

    memset(mydev_status, 0, sizeof(egsip_dev_satus)*MAX_USERS);

    int i = 0;
    for(i=0;i<MAX_USERS;i++)
    {
        user_file_load_camera_parameters(mydev_status[i].pic_url);
        break;
    }
}

// 设备初始化函数
void camera_init()
{
    egsip_log_debug("camera_init\n");
    int ret = 0;
#if 1
    // 设置设备信息
    memset(&g_camera_dev_info, 0, sizeof(g_camera_dev_info));
    ret = user_file_load_camera_device_config(&g_camera_dev_info);
    if(ret < 0)
    {
        egsip_log_info("load dev conf file failed, please check.\n");
        return;
    }
#else
    memset(&g_camera_dev_info, 0, sizeof(g_camera_dev_info));
    strcpy(g_camera_dev_info.srv_addr, "10.101.70.51:5060");
    strcpy(g_camera_dev_info.local_addr, "172.26.87.20:5060");
    g_camera_dev_info.dev_type = EGSIP_TYPE_CAMERA;
    g_camera_dev_info.vendor_num = EGSIP_VENDOR_NUM_HIKVISION;
    strcpy(g_camera_dev_info.id, "000001055353");
    g_camera_dev_info.call_dev_type = EGSIP_CALL_DEV_ENTRA_MACHINE;
    strcpy(g_camera_dev_info.addr_code, "77550022");
    g_camera_dev_info.dev_number = 1;
    g_camera_dev_info.subdev_count = 0;
    g_camera_dev_info.subdev_info = NULL;

    g_subdev.subdev_id.subdev_type = EGSIP_SUBTYPE_CAMERA_VIDEO_CHANNEL;
    strcpy(g_subdev.subdev_id.mac, "000001055353");
    g_subdev.subdev_id.subdev_num = 1;
    snprintf(g_subdev.name, sizeof(g_subdev.name), "%s", "SDK-IPCAM2");
    snprintf(g_subdev.vendor_name, sizeof(g_subdev.vendor_name), "%s", "Hikvision");
    snprintf(g_subdev.model, sizeof(g_subdev.model), "%s", "IP Camera");
    snprintf(g_subdev.owner, sizeof(g_subdev.owner), "%s", "Owner");
    snprintf(g_subdev.civilcode, sizeof(g_subdev.civilcode), "%s", "Civilcode");
    snprintf(g_subdev.address, sizeof(g_subdev.address), "%s", "Address");
    g_subdev.safetyway = 0;
    g_subdev.registerway = 1;
    g_subdev.secrecy = 0;
    g_subdev.status = 1;
    g_subdev.extra_info = NULL;

    g_camera_dev_info.subdev_info = &g_subdev;
#endif

    g_camera_dev_info.extra_info              = NULL;
    g_camera_dev_info.subdev_info->extra_info = NULL;
//
//    // 设置服务器请求回调函数表
//    memset(&g_camera_srv_req_cb_tbl, 0, sizeof(g_camera_srv_req_cb_tbl));
//    g_camera_srv_req_cb_tbl.called_cb         = camera_called_cb;
//    g_camera_srv_req_cb_tbl.stream_started_cb = camera_stream_started_cb;
//    g_camera_srv_req_cb_tbl.call_answered_cb  = camera_call_answered_cb;
//    g_camera_srv_req_cb_tbl.call_stopped_cb   = camra_call_stopped_cb;
//    g_camera_srv_req_cb_tbl.device_upgrade_cb = camera_device_upgrade_cb;
//    g_camera_srv_req_cb_tbl.set_pic_storage_cb= camera_set_pic_storage_cb;
//    g_camera_srv_req_cb_tbl.get_pic_storage_cb= camera_get_pic_storage_cb;
//
//    mydev_test_start = 1;
//
//    memset(mydev_status, 0, sizeof(egsip_dev_satus)*MAX_USERS);
//
//    int i = 0;
//    for(i=0;i<MAX_USERS;i++)
//    {
//        user_file_load_camera_parameters(mydev_status[i].pic_url);
//        break;
//    }
	init_camera();
    start_camera_test();
}

int camera_del()
{
    if (g_camera_dev_info.subdev_info != NULL)
    {
        free(g_camera_dev_info.subdev_info);
    }
    mydev_test_start = 0;
    return 0;
}

int g_msg_id = 0;

// 设备报警上报函数
void camera_report_alarm(int handle)
{
    if(g_camera_req_if_tbl.dev_alarm_report_if)
    {
        egsip_dev_alarm_info alarm_info={0};
        (*g_camera_req_if_tbl.dev_alarm_report_if)(handle, g_msg_id, camera_alarm_report_res_cb, &alarm_info);
    }
}

