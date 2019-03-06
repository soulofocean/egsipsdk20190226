#ifndef _EGSIP_DEF_H_
#define _EGSIP_DEF_H_

// 设备状态编码
typedef enum _EGSIP_DEV_STATUS_CODE
{
    EGSIP_DEV_STATUS_STOPED          = 0,    // 设备未初始化，功能完全不可用 (ip未配置，功能模块未初始化等)
//    EGSC_DEV_STATUS_TO_CONNECT      = 1,
    EGSIP_DEV_STATUS_TO_REGISTER     = 2,    // 设备已初始化，未注册到服务器 (只有本地功能可用)
    EGSIP_DEV_STATUS_WORKING         = 3,    // 设备已注册到服务器 (功能完全可用)
}EGSIP_DEV_STATUS_CODE;

typedef enum _EGSIP_DEV_STREAM_TYPE
{
    EGSIP_DEV_STREAM_VIDEO_SEND = 1,
    EGSIP_DEV_STREAM_AUDIO_SEND = 2,
    EGSIP_DEV_STREAM_VIDEO_RECV = 4,
    EGSIP_DEV_STREAM_AUDIO_RECV = 8,
}EGSIP_DEV_STREAM_TYPE;

// 返回值定义
typedef enum _EGSIP_RET_CODE
{
    EGSIP_RET_ERROR                      = -1, // 一般错误，没有明确定义的错误
    EGSIP_RET_SUCCESS                    =  0, // 返回成功
    EGSIP_RET_NO_SURPORT                 =  1, // 设备不支持
    EGSIP_RET_NOT_FOUND_DEV              =  2, // 未找到设备
    EGSIP_RET_NOT_START_DEV              =  3, // 设备未开始工作
    EGSIP_RET_DATA_ERROR                 =  4, // 数据错误
    EGSIP_RET_DEV_INTERNAL_PROC_FAIL     =  5, // 设备内部处理失败
    EGSIP_RET_SUBDEVICE_TIMEOUT          =  6, // 子设备超时未返回
    EGSIP_RET_CHILD_DEV_OFFLINE          =  7, // 子设备不在线
    EGSIP_RET_DEV_NOT_SUPPORT_THE_INST   =  8, // 设备不支持该指令
}EGSIP_RET_CODE;


// 设备类型
typedef enum _EGSIP_DEV_TYPE
{
    EGSIP_TYPE_CAMERA        = 2001, // IPC枪机
    EGSIP_TYPE_EAGLEEYE_CAM  = 2002, // 鹰眼摄像机
    EGSIP_TYPE_BALLHEAD_CAM  = 2003, // 球机
    EGSIP_TYPE_FACE_CAP_CAM  = 2004, // 人脸抓拍机
    EGSIP_TYPE_PARKING_CTRL  = 2005, // 停车场控制器
    EGSIP_TYPE_PARK_LPN_ID   = 2006, // 停车场车牌识别仪
    EGSIP_TYPE_DOOR_CTRL     = 2009, // 门禁控制器
    EGSIP_TYPE_GATE_CTRL     = 2010, // 人行通道控制器
    EGSIP_TYPE_ENTRA_MACHINE = 2011, // 门口机
    EGSIP_TYPE_FENCE_MACHINE = 2012, // 围墙机
    EGSIP_TYPE_INDOOR_MACHINE= 2013, // 室内机
    EGSIP_TYPE_MGMT_MACHINE  = 2014, // 管理机
    EGSIP_TYPE_ELE_LINK_CTRL = 2016, // 电梯联动控制器
    EGSIP_TYPE_PATROL_DEV    = 2017, // 巡更设备
    EGSIP_TYPE_SCREEN_CTRL   = 2018, // 信息发布屏控制器
    EGSIP_TYPE_BROADCAST_CTRL= 2019, // 广播控制器
    EGSIP_TYPE_ELEVATOR_CTRL = 2020, // 电梯厂商控制器
    EGSIP_TYPE_SMART_CTRL_KB = 2021, // 智能控制终端
    EGSIP_TYPE_CARPARK_CAM   = 2022, // 车位检测相机
    EGSIP_TYPE_ELEC_LPN_CTRL = 2023, // 电子车位控制器
}EGSIP_DEV_TYPE;

// 子设备类型
typedef enum _EGSIP_SUBDEV_TYPE
{
    //EGSIP_TYPE_CAMERA        = 2001, // IPC枪机
    EGSIP_SUBTYPE_CAMERA_VIDEO_CHANNEL           = 3001,     // IPC枪机通道
    EGSIP_SUBTYPE_CAMERA_ALERT_IN_CHANNEL        = 3002,     // IPC枪机告警输入通道
    EGSIP_SUBTYPE_CAMERA_ALERT_OUT_CHANNEL       = 3003,     // IPC枪机告警输出通道

    //EGSIP_TYPE_EAGLEEYE_CAM  = 2002, // 鹰眼摄像机
    EGSIP_SUBTYPE_EAGLEEYE_VIDEO_CHANNEL         = 3004,     // 鹰眼摄像机通道
    EGSIP_SUBTYPE_EAGLEEYE_ALERT_IN_CHANNEL      = 3005,     // 鹰眼摄像机告警输入通道
    EGSIP_SUBTYPE_EAGLEEYE_ALERT_OUT_CHANNEL     = 3006,     // 鹰眼摄像机告警输出通道

    //EGSIP_TYPE_BALLHEAD_CAM  = 2003, // 球机
    EGSIP_SUBTYPE_BALLHEAD_CHANNEL               = 3007,     //  球机通道
    EGSIP_SUBTYPE_BALLHEAD_ALERT_IN_CHANNEL      = 3008,     //  球机告警输入通道
    EGSIP_SUBTYPE_BALLHEAD_ALERT_OUT_CHANNEL     = 3009,     //  球机告警输出通道

    //EGSIP_TYPE_FACE_CAP_CAM  = 2004, // 人脸抓拍机
    EGSIP_SUBTYPE_FACE_CAPTURE_CHANNEL           = 3041,     // 人脸抓拍机通道
    EGSIP_SUBTYPE_FACE_CAPTURE_ALERT_IN_CHANNEL  = 3042,     // 人脸抓拍机告警输入通道
    EGSIP_SUBTYPE_FACE_CAPTURE_ALERT_OUT_CHANNEL = 3043,     // 人脸抓拍机告警输出通道

    //EGSIP_TYPE_PARKING_CTRL  = 2005, // 停车场控制器
    EGSIP_SUBTYPE_PARKING_SPACE_DISPLAY          = 3023,     // 门口显示剩余车位数大屏
    EGSIP_SUBTYPE_PARKING_BARRIER_GATE           = 3024,     // 停车场道闸
    EGSIP_SUBTYPE_PARKING_FLOOR_SENSOR           = 3025,     // 停车场地感
    EGSIP_SUBTYPE_PARKING_CPU_CARD_READER        = 3026,     // CPU读头
    EGSIP_SUBTYPE_PARKING_QR_READER              = 3027,     // 二维码读头
    EGSIP_SUBTYPE_PARKING_RFID_READER            = 3028,     // RFID读头
    EGSIP_SUBTYPE_PARKING_IC_CARD_READER         = 3029,     // 停车场IC读头
    EGSIP_SUBTYPE_PARKING_ID_CARD_READER         = 3030,     // 停车场ID读头
    EGSIP_SUBTYPE_PARKING_TICKET_BOX             = 3031,     // 停车场票箱
    EGSIP_SUBTYPE_PARKING_CARD_BOX               = 3032,     // 停车场卡箱
    EGSIP_SUBTYPE_PARKING_LCD_DISPLAY            = 3033,     // 停车场LCD显示屏
    EGSIP_SUBTYPE_PARKING_LED_DISPLAY            = 3034,     // 停车场LED显示屏
    EGSIP_SUBTYPE_PARKING_INTERCOM               = 3035,     // 停车场对讲
    EGSIP_SUBTYPE_PARKING_SPEAKER                = 3036,     // 停车场语音喇叭

    //EGSIP_TYPE_DOOR_CTRL     = 2009, // 门禁控制器
    EGSIP_SUBTYPE_DOOR_READER                    = 3010,     // 门禁读头
    EGSIP_SUBTYPE_DOOR_FACE_READER               = 3011,     // 门禁人脸读卡器
    EGSIP_SUBTYPE_DOOR_FINGER_READER             = 3012,     // 门禁指纹识别读卡器
    EGSIP_SUBTYPE_DOOR_QR_READER                 = 3013,     // 门禁二维码读卡器
    EGSIP_SUBTYPE_DOOR_BLUETOOTH_READER          = 3014,     // 门禁蓝牙读卡器
    EGSIP_SUBTYPE_DOOR_PASSWORD_KEYBOARD         = 3015,     // 门禁密码输入键盘
    EGSIP_SUBTYPE_DOOR_IC_CARD_READER            = 3016,     // 门禁IC读头
    EGSIP_SUBTYPE_DOOR_CPU_CARD_READER           = 3017,     // 门禁CPU读头

    //EGSIP_TYPE_GATE_CTRL     = 2010, // 人行通道控制器
    EGSIP_SUBTYPE_GATE_MACHINE                   = 3018,     // 人证读卡器（闸机）
    EGSIP_SUBTYPE_GATE_IQR_READER                = 3039,     // 二维码读卡器（闸机）
    EGSIP_SUBTYPE_GATE_IC_CARD_READER            = 3040,     // IC卡读卡器（闸机）

    //EGSIP_TYPE_ENTRA_MACHINE = 2011, // 门口机
    EGSIP_SUBTYPE_ENTRANCE_READER                = 3044,     // 门口机读头
    EGSIP_SUBTYPE_ENTRANCE_FACE_READER           = 3045,     // 门口机人脸读卡器
    EGSIP_SUBTYPE_ENTRANCE_FINGER_READER         = 3046,     // 门口机指纹识别读卡器
    EGSIP_SUBTYPE_ENTRANCE_QR_READER             = 3047,     // 门口机二维码读卡器
    EGSIP_SUBTYPE_ENTRANCE_BLUETOOTH_READER      = 3048,     // 门口机蓝牙读卡器
    EGSIP_SUBTYPE_ENTRANCE_PASSWORD_KEYBOARD     = 3049,     // 门口机密码输入键盘
    EGSIP_SUBTYPE_ENTRANCE_IC_CARD_READER        = 3050,     // 门口机IC读头
    EGSIP_SUBTYPE_ENTRANCE_CPU_CARD_READER       = 3051,     // 门口机CPU读头

    //EGSIP_TYPE_FENCE_MACHINE = 2012, // 围墙机
    EGSIP_SUBTYPE_FENCE_READER                   = 3052,     // 围墙机读头
    EGSIP_SUBTYPE_FENCE_FACE_READER              = 3053,     // 围墙机人脸读卡器
    EGSIP_SUBTYPE_FENCE_FINGER_READER            = 3054,     // 围墙机指纹识别读卡器
    EGSIP_SUBTYPE_FENCE_QR_READER                = 3055,     // 围墙机二维码读卡器
    EGSIP_SUBTYPE_FENCE_BLUETOOTH_READER         = 3056,     // 围墙机蓝牙读卡器
    EGSIP_SUBTYPE_FENCE_PASSWORD_KEYBOAR         = 3057,     // 围墙机密码输入键盘
    EGSIP_SUBTYPE_FENCE_IC_CARD_READER           = 3058,     // 围墙机IC读头
    EGSIP_SUBTYPE_FENCE_CPU_CARD_READER          = 3059,     // 围墙机CPU读头

    //EGSIP_TYPE_ELE_LINK_CTRL = 2016, // 电梯联动控制器
    EGSIP_SUBTYPE_ELEVATOR_SUB_CONTROLLER        = 3037,    // 电梯联动控制子设备（电梯联动控制器里的虚拟子设备）
    
    //EGSIP_TYPE_SCREEN_CTRL   = 2018, // 信息发布屏控制器
    EGSIP_SUBTYPE_INFORMATION_SCREEN             = 3019,    // 信息发布屏
    EGSIP_SUBTYPE_INFORMATION_LED_SCREEN         = 3020,    // 信息LED大屏
    EGSIP_SUBTYPE_INFORMATION_LCD_SCREEN         = 3021,    // 信息LCD大屏

    //EGSIP_TYPE_BROADCAST_CTRL= 2019, // 广播控制器
    EGSIP_SUBTYPE_BROADCAST_GROUP                = 3022,    // 广播分区

    //EGSIP_TYPE_ELEVATOR_CTRL = 2020, // 电梯厂商控制器
    EGSIP_SUBTYPE_ELEVATOR_IC_CARD_READER        = 3038,     // 电梯IC卡读头

    //EGSIP_TYPE_SMART_CTRL_KB = 2021, // 智能控制终端
    EGSIP_SUBTYPE_ELECTRIC_FENCE                 = 3060,     // 电子围栏

    //EGSIP_TYPE_CARPARK_CAM   = 2022, // 车位检测相机
    EGSIP_SUBTYPE_CARPORT_CAMERA_VIDEO_CHANNEL   = 3061,     // 车位检测相机通道

    //EGSIP_TYPE_ELEC_LPN_CTRL = 2023, // 电子车位控制器
    EGSIP_SUBTYPE_ELECTRIC_LPN_DISPLAY           = 3062,     // 电子车位显示屏
}EGSIP_SUBDEV_TYPE;

// 厂商编号
typedef enum _EGSIP_VENDOR_NUMBER
{
    EGSIP_VENDOR_NUM_HIKVISION   = 1001, // 海康
    EGSIP_VENDOR_NUM_DAHUA       = 1002, // 大华
    EGSIP_VENDOR_NUM_JIESHUN     = 1003, // 捷顺
    EGSIP_VENDOR_NUM_ANJUBAO     = 1004, // 安居宝
    EGSIP_VENDOR_NUM_LEELEN      = 1005, // 立林
    EGSIP_VENDOR_NUM_IBM         = 1006, // IBM
    EGSIP_VENDOR_NUM_HONEYWELL   = 1007, // 霍尼韦尔
    EGSIP_VENDOR_NUM_EVERGRANDE  = 1008, // 恒大
    EGSIP_VENDOR_NUM_HITACHI     = 1009, // 日立电梯
    EGSIP_VENDOR_NUM_QLDT        = 1010, // 三菱电梯
    EGSIP_VENDOR_NUM_OTIS        = 1011, // 奥的斯电梯
}EGSIP_VENDOR_NUMBER;

// 呼叫设备类型
typedef enum _EGSIP_CALL_DEV_TYPE
{
    EGSIP_CALL_UNKNOW_DEV_MACHINE  = 0, //非呼叫类设备
    EGSIP_CALL_DEV_MGMT_MACHINE    = 1, // 管理机
    EGSIP_CALL_DEV_FENCE_MACHINE   = 2, // 围墙机
    EGSIP_CALL_DEV_ENTRA_MACHINE   = 3, // 门口机
    EGSIP_CALL_DEV_INDOOR_MACHINE  = 4, // 室内机
    EGSIP_CALL_DEV_APP             = 5, // APP
    EGSIP_CALL_DEV_PARKING         = 6, // 停车场
    EGSIP_CALL_DEV_ELEVATOR        = 7, // 电梯
    EGSIP_CALL_DEV_PATROL_APP      = 8, // 巡更APP
}EGSIP_CALL_DEV_TYPE;

#endif
