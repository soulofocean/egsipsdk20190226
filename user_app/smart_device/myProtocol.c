#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
#include "myProtocol.h"
#include "myMQ.h"
#include "egsip_util.h"
#include <time.h>
unsigned int global_fork_us = 1000;
DEV_MSG_ACK_ENUM global_ack_type = NO_ACK;
long global_msg_type = SOCKET_SEND_MSG_TYPE;
int Update_Dev_Fork_List(unsigned int arr[], int arrIndex, EGSIP_DEV_TYPE devType, int devCount)
{
	if(arrIndex > DEV_FORK_LIST_MAX_SIZE - 1){
		egsip_log_error("arrIndex out of range.\n");
		return EGSIP_RET_ERROR;
	}
	arr[arrIndex] = GetMQMsgType(devType,devCount);
	return EGSIP_RET_SUCCESS;
}
unsigned int GetMQMsgType(int dev_type,int dev_offset)
{
	return (dev_type << DEV_INDEX_OFFSET) + dev_offset;
}
unsigned int GetDevType(unsigned int msg_type)
{
	return msg_type>>DEV_INDEX_OFFSET;
}
unsigned int GetDevCount(unsigned int msg_type)
{
	return msg_type & DEV_OFFSET_OP;
}
int PutRsvMQ(msg_struct msgs)
{
	return Enqueue_MQ(SOCKET_RSV_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutSendMQ(int code,const char* func_name,char * info)
{
	msg_struct msgs;
	memset(&msgs,0,sizeof(msg_struct));
	msgs.msgType = global_msg_type;
	char tmp[MQ_INFO_BUFF] = {0};
	#if 0 
	//这些操作放在Client端执行替换就可以了，此处不做处理
	char jsonmsg[MQ_INFO_BUFF] = {0};
	if(strstr(info,"{")==NULL)
	{
		snprintf(tmp,MQ_INFO_BUFF-1,"{\"pid\":\%u,\"code\":%d,\"func\":\"\%s\",\"info\":\"%s\"}",getpid(),code,func_name,info);
	}
	else
	{
		snprintf(tmp,MQ_INFO_BUFF-1,"{\"pid\":\%u,\"code\":%d,\"func\":\"\%s\",\"info\":%s}",getpid(),code,func_name,info);
	}
	replace_string(jsonmsg, tmp, "\"{", "{");
	strncpy(tmp,jsonmsg,MQ_INFO_BUFF-1);
	replace_string(jsonmsg, tmp, "}\"", "}");
	strncpy(tmp,jsonmsg,MQ_INFO_BUFF-1);
	#else
	snprintf(tmp,MQ_INFO_BUFF-1,"{\"pid\":\%u,\"code\":%d,\"func\":\"\%s\",\"info\":\"%s\"}",getpid(),code,func_name,info);
	#endif
	strncpy(msgs.msgData.info,tmp,sizeof(msgs.msgData.info)-1);
	return Enqueue_MQ(SOCKET_SEND_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutSendShortMQ(int status_code)
{
	msg_short_struct msgs;
	memset(&msgs,0,sizeof(msg_short_struct));
	msgs.msgType = global_msg_type;
	msgs.msgData.statusCode = status_code;
	return Enqueue_MQ_Short(SOCKET_SEND_SHORT_MQ_KEY, msgs, MQ_SEND_BUFF_SHORT, ipc_no_wait);
}

int PutDispatchMQ(int dev_type,int dev_index,char* info)
{
	egsip_log_debug("Enter PutDispatchMQ dev_type=[%d] dev_index=[%d] info=[%s]\n",dev_type,dev_index,info);
	msg_struct msgs;
	msgs.msgType = GetMQMsgType(dev_type, dev_index);
	msgs.msgData.devType = dev_type;
	msgs.msgData.offset = dev_index;
	strncpy(msgs.msgData.info,info,sizeof(msgs.msgData.info));
	return Enqueue_MQ(GetDispatchMQKey(msgs.msgType), msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutDispatchNMQ(msg_struct msgs,int put_count)
{
	int index = 0;
	int ret = 0;
	for(;index<put_count;++index){
		ret = Enqueue_MQ(GetDispatchMQKey(msgs.msgType), msgs, MQ_SEND_BUFF, ipc_no_wait);
		if(ret < 0)
			return ret;
		if(index<put_count){
			msgs.msgType++;
			msgs.msgData.offset++;
		}
	}
	return ret;
}

int GetRsvMQ(msg_struct *msgbuff)
{
	return Dequeue_MQ(SOCKET_RSV_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int GetSendMQ(msg_struct *msgbuff)
{
	return Dequeue_MQ(SOCKET_SEND_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int GetSendShortMQ(msg_short_struct *msgbuff)
{
	return Dequeue_MQ_Short(SOCKET_SEND_SHORT_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF_SHORT, ipc_need_wait);
}

int GetDispatchMQ(long msgType,msg_struct *msgbuff)
{
	return Dequeue_MQ(GetDispatchMQKey(msgType), 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int DelDispatchMQ(long msgType)
{
	return Delete_MQ(GetDispatchMQKey(msgType));
}
void DevMsgAck(int code,const char* func_name,char* msg)
{
	egsip_log_debug("enter.\n");
	egsip_log_debug("pid=[%u] code=[%d] msg=[%s]\n",getpid(),code,msg);
	EGSIP_RET_CODE ret = EGSIP_RET_ERROR;
	//后续useLongMsg可能扩展成枚举，这里先直接判断
	switch (global_ack_type)
	{
		case LONG_ACK:
		{
			ret = PutSendMQ(code,func_name,msg);
			break;
		}
		case SHORT_ACK:
		{
			ret = PutSendShortMQ(code);
			break;
		}
		case NO_ACK:
		{
			egsip_log_debug("PID[%d] is set NO_ACK\n",getpid());
			break;
		}
		default:
		{
			egsip_log_error("Invalid Type:[%d]\n",global_ack_type);
			break;
		}
	}
	egsip_log_debug("pid:[%d] global_ack_type=[%d] ret=[%d]\n",getpid(),global_ack_type,ret);
}
int ForkMulDev(unsigned int dev_arr[],msgQueenDataType *myarg)
{
	int index = 0;
	EGSIP_DEV_TYPE dev_type;
	int dev_count = 0;
	pid_t fpid;
	int child_break = 0;
	for(index=0;index<DEV_FORK_LIST_MAX_SIZE;++index){
		if(!dev_arr[index]){
			egsip_log_debug("Quit Fork!\n");
			break;
		}
		dev_type = GetDevType(dev_arr[index]);
		dev_count = GetDevCount(dev_arr[index]);
		egsip_log_debug("dev_type=%d dev_count=%d\n",dev_type,dev_count);
		myarg->devType = dev_type;
		int dev_index = 0;//不在外面定义有的编译器会有warning
		for(dev_index = 0;dev_index<dev_count;++dev_index){
			myarg->offset = dev_index;
			fpid = fork();
			if(fpid<0){
				egsip_log_error("fork error,errno=%d[%s]\n",errno,strerror(errno));
				return -1;
			}
			else if(fpid == 0)
			{
				//I'm child
				global_msg_type = GetMQMsgType(dev_type, dev_index);
				child_break = 1;
				break;
			}
			usleep(global_fork_us);
		}
		if(child_break)
			break;
	}
	return 0;
}
