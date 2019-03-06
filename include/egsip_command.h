#ifndef _EGSIP_DEV_COMMON_H_
#define _EGSIP_DEV_COMMON_H_

#include "egsip_def.h"
#include "egsip_sdk.h"


/*
    呼叫地址信息
*/
typedef struct _egsip_call_addr_info
{
    EGSIP_DEV_TYPE dev_type;            // 设备类型
    EGSIP_VENDOR_NUMBER vendor_num;     // 厂商编号
    char mac[16];                       // 设备唯一标识（使用MAC地址,112233AABBCC）
    
    EGSIP_CALL_DEV_TYPE call_dev_type;  // 呼叫设备类型
    char addr_code[16];                 // 设备安装地址编码，8位字符串格式 
    int dev_number;                     // 同一地址下的设备序号 

    char offline_call_ip[32];           // 呼叫IP,只在离线时使用
    unsigned short offline_call_port;   // 呼叫端口,只在离线时使用
}egsip_call_addr_info;

/*
    媒体流信息
*/
typedef enum EGSIP_VIDEO_IPC_FORMAT_
{
    EGSIP_VIDEO_IPC_PS = 96,
    EGSIP_VIDEO_IPC_MPEG4 = 97,
    EGSIP_VIDEO_IPC_H264 = 98,
}EGSIP_VIDEO_IPC_FORMAT;
typedef struct _egsip_media_stream_info
{
    int enable;                 // 启用/禁用流传输，由呼叫者填充，禁用时，以下字段不需填充，不传相应的流
    char recv_ip[32];           // 接收者IP，由流接收者填充
    unsigned short recv_port;   // 接收者端口，由流接收者填充
    int format;                 // media payload，由流发送者填充
}egsip_media_stream_info;


/*
    呼叫信息
*/
typedef struct _egsip_dev_call_info
{
    egsip_call_addr_info  other_addr;           // 对端地址
    egsip_media_stream_info caller_send_video;    // 视频流：呼叫者-->被呼叫者
    egsip_media_stream_info caller_recv_video;    // 视频流：呼叫者<--被呼叫者
    egsip_media_stream_info caller_send_audio;    // 音频流：呼叫者-->被呼叫者
    egsip_media_stream_info caller_recv_audio;    // 音频流：呼叫者<--被呼叫者
}egsip_dev_call_info;

/************************************
**************门禁功能***************
*************************************/

/*
    凭证下发信息
*/
#define MAX_ACS_SUB_DEV_NUM 10

/**子设备信息**/
typedef struct _egsip_sub_dev_info
{
    char id[128];
}egsip_sub_dev_info;

/**凭证下发信息**/
typedef struct _egsip_dev_cb_certificate_param
{
    char start_time[128];       //必选，有效期开始时间yyyy-MM-dd hh:mm:ss
    char end_time[128];         //必选，有效期结束时间yyyy-MM-dd hh:mm:ss
    int credence_type;          //必选，凭证类型
    char credence_no[128];      //必选，凭证编号（车牌号、IC卡号、人脸图片ID(即人脸ID)、指纹特征码、二维码编号、NFC卡、密码号等）;其中卡号按读卡器读出来的卡片原始卡号数据的16进制表示。
    //读卡器列表 可最多下发到512个设备；如果没有子设备就填主设备；如果填主设备则表示该凭证对该主设备下属所有子设备都有效。
    egsip_sub_dev_info sub_dev[MAX_ACS_SUB_DEV_NUM];
    int user_type;              //下发必选，用户类别
    char user_name[128];        //可选，用户名称
    char user_id[128];          //下发可选，用户编号，电梯厂商控制器的该字段是必须下载的
    char op_time[128];          //可选，操作时间
    void *dev_special_param;    //可选，设备专用参数结构指针，与具体设备有关
}egsip_dev_cb_certificate_param;

/*
    门禁记录上报参数
*/
typedef struct _egsip_acs_record_type
{
    /*
    0:人脸开门上报;
    1:远程开门上报;
    2:CPU卡开门上报;
    3:二维码开门上报;
    4:指纹开门上报;
    5:密码开门上报;
    6:按钮开门上报;
    7:黑名单开门上报;
    8:人脸验证失败上报;
    9:远程验证失败上报;
    10:CPU卡验证失败上报;
    11:二维码验证失败上报;
    12:指纹验证失败上报;
    13:密码验证失败上报;
    14:人行道闸出园记录上报;
    15:动态密码验证成功;
    16:动态密码失败上报;
    17:NFC开门成功;
    18:NFC开门失败;
    19:蓝牙开门成功;
    20:蓝牙开门失败;
    *注：蓝牙开门失败指认证设备对凭证鉴权失败（凭证不存在或当前时间不在权限有效期内）
    */
    int type;
    char user_id[32];//用户id
    int user_type;//用户类型
    char sub_dev_id[32];//子设备id
    char time[64];//记录发生时间
    char pic_url[64];//图片地址
    int pass_type;//0：入；1出；失败默认为0
    char credence_no[2048];//凭证唯一标识
}egsip_acs_record_type;

/*
    门参数信息
*/
typedef struct _egsip_door_param_type
{
    int open_durationl;
    int alarm_timeout;
    char file_server[128];
    char ntp_server[128];
}egsip_door_param_type;

/**开门消息**/
typedef struct _egsip_door_open_type
{
    egsip_subdev_id sub_dev;//没有子设备填主设备ID
    int mode;//0- 关闭（对于梯控，表示受控），1- 打开，2- 常开，3- 常关，4- 恢复
    char user_id[32];
    int user_type;//0xf0,表示对接设备开门，对应的用户ID为对接设备ID；其他按正常用户类型来定义
}egsip_door_open_type;


// 服务器请求回调函数定义 -----------------------------------------------------------------------

// *** 设备被呼叫 *** 
/*
    sess_id 会话ID
    call_info 呼叫信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_called_cb)(int handle, int sess_id, egsip_dev_call_info *call_info);

// *** 呼叫被接听通知 *** 
// handle 设备句柄
typedef EGSIP_RET_CODE (*egsip_dev_call_answered_cb)(int handle, int sess_id);

// *** 流传输开始通知  *** 
// handle 设备句柄
typedef EGSIP_RET_CODE (*egsip_dev_stream_started_cb)(int handle, int sess_id, EGSIP_DEV_STREAM_TYPE stream_flag);


// *** 呼叫被停止通知 *** 
// handle 设备句柄
//status 会话结束原因，0:正常关闭 1:用户忙 2:拒接 3:超时(CALL_STOP_STATUS_E)
typedef EGSIP_RET_CODE (*egsip_dev_call_stopped_cb)(int handle, int sess_id, int status);


// *** 门禁权限服务器下发通知 *** 
// 下发固定凭证信息 COM_LOAD_CERTIFICATE
typedef EGSIP_RET_CODE (*egsip_dev_load_certificate_cb)(int handle, egsip_dev_cb_certificate_param *cert_param);

// 读取固定凭证信息 COM_READ_CERTIFICATE
// credence_type 必选，凭证类型
// credence_no 必选，凭证编号（车牌号、IC卡号、人脸图片ID(即人脸ID)、指纹特征码、二维码编号、NFC卡、密码号等）;其中卡号按读卡器读出来的卡片原始卡号数据的16进制表示。
typedef EGSIP_RET_CODE (*egsip_dev_read_certificate_cb)(int handle, int credence_type, char *credence_no,
                                                                egsip_dev_cb_certificate_param *dev_param);

// 删除固定凭证信息 COM_DELETE_CERTIFICATE
// credence_type 必选，凭证类型
// credence_no 必选，凭证编号（车牌号、IC卡号、人脸图片ID(即人脸ID)、指纹特征码、二维码编号、NFC卡、密码号等）;其中卡号按读卡器读出来的卡片原始卡号数据的16进制表示。
// subdevice_id 可选，子设备ID，没有子设备填主设备ID
// user_id 可选，用户编号，电梯厂商控制器的该字段是必须下载的
typedef EGSIP_RET_CODE (*egsip_dev_delete_certificate_cb)(int handle, int credence_type, char *credence_no,
                                                                char *subdevice_id, char *user_id);

// *** 远程开门服务器下发命令 *** 
// handle 设备句柄
//door_open 开门信息
typedef EGSIP_RET_CODE (*egsip_dev_acs_door_open_cb)(int handle, int sess_id, egsip_door_open_type *door_open);



// *** 设备查询图片参数 *** 
// handle 设备句柄
//http_url http地址
typedef EGSIP_RET_CODE (*egsip_dev_get_pic_storage_cb)(int handle, int sess_id, char *http_url);

// *** 设备配置图片参数 *** 
// handle 设备句柄
//http_url http地址
typedef EGSIP_RET_CODE (*egsip_dev_set_pic_storage_cb)(int handle, int sess_id, char *http_url);

// *** 查询门参数配置 *** 
// handle 设备句柄
//door 门参数信息
typedef EGSIP_RET_CODE (*egsip_dev_get_door_param_cb)(int handle, int sess_id, egsip_door_param_type *door);

// *** 设置门参数配置 *** 
// handle 设备句柄
//door 门参数信息
typedef EGSIP_RET_CODE (*egsip_dev_set_door_param_cb)(int handle, int sess_id, egsip_door_param_type *door);


// *** 固件升级消息 *** 
// handle 设备句柄
//device_id 设备id
//firmware 固件版本号
//file_url http文件url
//ftp_addr ftp地址
typedef EGSIP_RET_CODE (*egsip_dev_device_upgrade_cb)(int handle, int sess_id, char *file_url, char *ftp_addr);


// 设备请求接口函数定义 --------------------------------------------------------------------------

// *** 设备呼叫 *** 
/*
    sess_id 会话ID
    call_info 呼叫信息
*/
typedef void (*egsip_dev_call_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret,
                                                    egsip_dev_call_info *call_info);
/*
    sess_id 会话ID由用户指定且唯一
    call_info 呼叫信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_call_if)(int handle, int sess_id, egsip_dev_call_res_cb res_cb,
                                                        egsip_dev_call_info *call_info);


// *** 接听呼叫(音频) *** 
typedef void (*egsip_dev_answer_call_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    handle 设备句柄
    res_cb 回复回调函数
*/
typedef EGSIP_RET_CODE (*egsip_dev_answer_call_if)(int handle, int sess_id, egsip_dev_answer_call_res_cb res_cb);



// *** 停止呼叫 *** 
typedef void (*egsip_dev_stop_call_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    handle 设备句柄
    res_cb 回复回调函数
    url_pic 抓拍图片地址
*/
typedef EGSIP_RET_CODE (*egsip_dev_stop_call_if)(int handle, int sess_id, egsip_dev_stop_call_res_cb res_cb);


// *** 设备报警上报 *** 
/*
 设备报警信息(用于IPC)
*/
typedef struct _egsip_dev_alarm_info
{
    int priority;       // 必选,报警级别
    char * time;        // 必选,报警时间
    int method;         // 必选,报警方式
    char * desc;        // 可选,报警描述
    double longitude;   // 可选,经度
    double latitude;    // 可选,纬度
    egsip_subdev_id * subdev_id; // 可选，NULL表示主设备
    int alarm_type;           // 必选,报警类型 
    void * alarm_param; // 可选,扩展报警参数，不同的设备事件参数使用不同的结构体定义
}egsip_dev_alarm_info;
typedef void (*egsip_dev_alarm_report_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于IPC类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    alarm_info 报警信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_alarm_report_if)(int handle, int sess_id, egsip_dev_alarm_report_res_cb res_cb,
                                                        egsip_dev_alarm_info *alarm_info);


typedef void (*egsip_dev_acs_record_report_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于可视对讲类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    acs_record 事件记录信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_acs_record_report_if)(int handle, int sess_id, egsip_dev_acs_record_report_res_cb res_cb,
                                                        egsip_acs_record_type *acs_record);
/*
    报警消息参数
*/
typedef struct _egsip_acs_alarm_info
{
    int event_type;//必选 事件类型
    egsip_subdev_id sub_dev;//必选 子设备id
    char event_time[64];//必选 发生时间
    char desc[128];//可选 描述信息
    char pic_url[128];//可选 图片地址
    char user_id[32];//可选 用户id
    int user_type;//可选 用户类型
}egsip_acs_alarm_info;

typedef void (*egsip_dev_acs_alarm_report_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于可视对讲类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    alarm_info 报警信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_acs_alarm_report_if)(int handle, int sess_id, egsip_dev_acs_alarm_report_res_cb res_cb,
                                                        egsip_acs_alarm_info *alarm_info);

typedef void (*egsip_dev_open_door_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于可视对讲类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    door_open 开门信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_open_door_if)(int handle, int sess_id, egsip_dev_open_door_res_cb res_cb,
                                                        egsip_door_open_type *door_open);

/*
    电梯控制
*/
typedef void (*egsip_dev_elevator_control_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于可视对讲类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    dir 方向 方向:1 上 2下
*/
typedef EGSIP_RET_CODE (*egsip_dev_elevator_control_if)(int handle, int sess_id, egsip_dev_elevator_control_res_cb res_cb,
                                                        int dir);

/*
    对讲记录上报
*/
typedef struct _egsip_intercom_record_report_info
{
    egsip_call_addr_info call_info;
    char *invite_time;
    int talk_time;
    int answer;
    int lock;
    char *url_pic;
}egsip_intercom_record_report_info;

typedef void (*egsip_dev_intercom_record_report_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于可视对讲类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    record 对讲记录上报信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_intercom_record_report_if)(int handle, int sess_id, egsip_dev_intercom_record_report_res_cb res_cb,
                                                        egsip_intercom_record_report_info *record);

/*
    开锁记录上报
*/
typedef struct _egsip_intercom_unlock_record_report_info
{
    egsip_call_addr_info call_info;//呼叫信息
    int mode;//0- 关闭（对于梯控，表示受控），1- 打开，2- 常开，3- 常关，4- 恢复
    int result;//0-成功，1-失败
    char *record_time;//记录发生时间
}egsip_intercom_unlock_record_report_info;

typedef void (*egsip_dev_intercom_unlock_record_report_res_cb)(int handle, int sess_id, EGSIP_RET_CODE ret);
/*
    (主要用于可视对讲类设备)
    handle 设备句柄
    res_cb 回复回调函数
    sess_id 会话ID由用户指定且唯一
    record 开锁记录上报信息
*/
typedef EGSIP_RET_CODE (*egsip_dev_intercom_unlock_record_report_if)(int handle, int sess_id, egsip_dev_intercom_unlock_record_report_res_cb res_cb,
                                                        egsip_intercom_unlock_record_report_info *record);

// ---------------------------------------------------------------------------------------------------------

#endif
