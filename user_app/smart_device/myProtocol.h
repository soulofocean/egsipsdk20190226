#ifndef _MYPROTOCOL_H
#define _MYPROTOCOL_H
#include "myMQ.h"
#include "egsip_def.h"

#define MQ_SEND_BUFF sizeof(msg_struct)-sizeof(long) //ENQueue_MQ消息的大小
#define MQ_RSV_BUFF sizeof(msg_struct)				 //DeQueue_MQ消息的大小 必须比ENQueue_MQ的大否则报错
#define MQ_SEND_BUFF_SHORT sizeof(msg_short_struct) - sizeof(long)
#define MQ_RSV_BUFF_SHORT sizeof(msg_short_struct)
#define SOCKET_RSV_MQ_KEY 8887	//接收到的SOCKET消息缓存的MQKEY ftok(".",8) 
#define SOCKET_SEND_MQ_KEY 9998 //将要通过SOCKET发送出去的MSG的缓存MQKEY ftok(".",9) 
#define SOCKET_SEND_SHORT_MQ_KEY 6665 //将要通过SOCKET发送出去的SHORT_MSG的缓存MQKEY ftok(".",6) 
#define SOCKET_RSV_MSG_TYPE 1		//Socket接收到的数据放入MQ默认类型
#define SOCKET_SEND_MSG_TYPE 2		//Socket发送出去的数据放入MQ默认类型
#define DEV_INDEX_OFFSET 16			//预留16bit放设备序号，DEVTYPE<<16
#define DEV_OFFSET_OP 0xFFFF		//和上面的16bits对应
#define DEV_FORK_LIST_MAX_SIZE 16	//最多支持16种设备
#define DEV_MAX_COUNT 1000			//每种设备最大数目
extern unsigned int global_fork_us;
typedef enum _Rsv_Msg_Process_Result{
	No_Need_Rsp = 0,
	DSP_MSG = 1,
	SEND_MSG = 2
}RsvMsgProcResultEnum;
typedef enum _dev_msg_ack_enum{
	NO_ACK = 0,
	SHORT_ACK = 1,
	LONG_ACK = 2
}DEV_MSG_ACK_ENUM;
extern DEV_MSG_ACK_ENUM global_ack_type;
extern long global_msg_type;
int Update_Dev_Fork_List(unsigned       int arr[], int arrIndex, EGSIP_DEV_TYPE devType, int devCount);
unsigned int GetMQMsgType(int dev_type,int dev_offset);
unsigned int GetDevType(unsigned int msg_type);
unsigned int GetDevCount(unsigned int msg_type);
int PutRsvMQ(msg_struct msgs);
int PutSendMQ(int code,const char* func_name,char * info);
int PutSendShortMQ(int status_code);
int PutDispatchMQ(int dev_type,int dev_index,char* info);
int PutDispatchNMQ(msg_struct msgs,int put_count);
int GetRsvMQ(msg_struct *msgbuff);
int GetSendMQ(msg_struct *msgbuff);
int GetSendShortMQ(msg_short_struct *msgbuff);
int GetDispatchMQ(long msgType,msg_struct *msgbuff);
int DelDispatchMQ(long msgType);
void DevMsgAck(int code,const char* func_name,char* msg);
int ForkMulDev(unsigned int dev_arr[],msgQueenDataType *myarg);
#endif
