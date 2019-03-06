#ifndef _EGSIP_SDK_H_
#define _EGSIP_SDK_H_

#include "egsip_def.h"

/*
    设备状态回调函数定义
*/
typedef void (*egsip_dev_status_callback)(int handle, EGSIP_DEV_STATUS_CODE status, char *desc_info);

/*
    调试打印回调函数
*/
typedef int (*egsip_print_callback)(const char * format, ...);


/*
    子设备ID结构体
*/
typedef struct _egsip_subdev_id
{
    EGSIP_SUBDEV_TYPE subdev_type;       // 子设备类型
    char mac[16];                        // 子设备唯一标识（使用MAC地址,112233AABBCC）
    int subdev_num;                     // 子设备序号（1~9999）
}egsip_subdev_id;

/*
    子设备信息
*/
typedef struct _egsip_subdev_info
{
    egsip_subdev_id subdev_id;  // 子设备ID信息
    char name[64];              //子设备名字
    char vendor_name[64];       // 厂商名称
    char model[64];             // 设备型号
    char owner[64];             // 设备归属
    char civilcode[64];         // 行政区域
    char address[64];           // 安装地址
    int safetyway;              // 信令安全模式，0:不采用;2:S/MIME 签名方式;3:S/MIME加密签名同时采用方式;4:数字摘要方式
    int registerway;            // 注册方式; 1: 符合IETFRFC3261标准的认证注册模式;2: 基于口令的双向认证注册模式;3: 基于数字证书的双向认证注册模式
    int secrecy;                // 保密属性;0:不涉密,1:涉密
    int status;                 // 设备状态, 0:OFF, 1: ON
    void * extra_info;          // 可选,子设备额外参数，不同类型的子设备使用不同的结构体定义
}egsip_subdev_info;

/*
    设备信息结构体
*/
typedef struct _egsip_dev_info
{
    char srv_addr[64];                  // 服务器地址，格式示例："192.168.1.1:12000" 或 "egsip.evergrande.com:12000"
    char local_addr[64];                // 本地IP和端口
    int encrpyt_enable;                 // 服务器通信是否为加密方式

    EGSIP_DEV_TYPE dev_type;            // 设备类型
    EGSIP_VENDOR_NUMBER vendor_num;     // 厂商编号
    char mac[16];                       // 设备唯一标识（使用MAC地址,112233AABBCC）
    
    EGSIP_CALL_DEV_TYPE call_dev_type;  // 呼叫设备类型
    char addr_code[16];                 // 设备安装地址编码，8位字符串格式 
    int dev_number;                     // 同一地址下的设备序号 

    char dev_type_desc[64];             // 设备类型描述
    char vendor_name[64];               // 厂商名称
    char model[64];                     // 设备型号
    char fw_version[64];                // 固件版本号

    int subdev_count;                   // 子设备数量
    egsip_subdev_info * subdev_info;    // 子设备ID

    void * extra_info;                  // extra_info 可选,不同类型设备参数使用不同的结构体定义
}egsip_dev_info;

/*
    SDK初始化
*/
EGSIP_RET_CODE egsip_sdk_init(egsip_print_callback print_cb);

/*
    释放SDK
*/
void egsip_sdk_uninit();

/*
    创建设备
参数:
    info -- 设备信息
    status_cb -- 设备状态回调函数
    handle -- 创建的设备句柄
*/
EGSIP_RET_CODE egsip_dev_create(egsip_dev_info *info,        egsip_dev_status_callback status_cb, int *handle);

/*
    设备功能注册
参数:
    handle -- 设备句柄
    srv_req_cb_tbl -- 服务器请求回调函数表指针，不同类型设备                有不同函数表, 由用户层传给SDK
    srv_req_cb_tbl_len -- 服务器请求回调函数表大小（字节数）
    dev_req_intface_buf -- 保存设备请求接口函数表的缓冲区地址，不同类型设备      有不同函数表, 由SDK返回给用户层
    buf_len -- 缓冲区大小（字节数）
    get_len -- 返回的函数表大小（字节数）
*/
EGSIP_RET_CODE egsip_dev_func_register(int handle, void *srv_req_cb_tbl, int srv_req_cb_tbl_len,
                                                void * dev_req_if_tbl_buf, int buf_len, int *get_len);

/*
    启动设备
参数:
    handle -- 设备句柄
*/
EGSIP_RET_CODE egsip_dev_start(int handle);


/*
    停止设备
参数:
    handle -- 设备句柄
*/
void egsip_dev_stop(int handle);

/*
    删除设备
参数:
    handle -- 设备句柄
*/
void egsip_dev_delete(int handle);

#endif
