#include<stdio.h>
#include<unistd.h> 
#include <errno.h> 
#include<string.h>

#include "myProtocol.h"

#include "myMQ.h"
#include <signal.h>
#include <sys/wait.h>

#include<mySocket.h>
#include "mymuldev.h"
#include<pthread.h>


pthread_t socket_rcv_tid = -1; //socket接收线程ID
pthread_t socket_send_tid = -1;//socket发送线程ID
pthread_t socket_send_short_tid = -1;//socket发送线程ID

void* socket_rcv_fn(void* arg)
{
	egsip_log_debug("enter.\n");
	char buff[SOCKET_RCV_BUFF] = {0};
	int socket_id = -1;
	socketServerInit(SOCKET_SERVER_PORT,SOCKET_SERVER_LISNUM, buff, &socket_id);
	socketServerLoopRsv(socket_id);
	return 0;
}
void* socket_send_fn(void* arg)
{
	egsip_log_debug("enter.\n");
	socketServerLoopSend();
	return 0;
}
void* socket_send_short_fn(void* arg)
{
	egsip_log_debug("enter.\n");
	socketServerLoopSendShort();
	return 0;
}

int main(int argc, char * argv [ ])
{
	EGSIP_RET_CODE ret = EGSIP_RET_ERROR;
	egsip_log_level = EGSIP_LOG_DEBUG;
	signal(SIGCHLD, SIG_IGN);//和主进程wait(NULL)选着搭配用防止僵尸进程,去掉后进程会提示为僵尸进程
	signal(SIGPIPE, SIG_IGN);//Socket的异常断开处理忽略
	int arg = 0;
	//初始化设备配置文件
	ret = muldev_init();
	if(ret<0)
		return ret;
	ret = pthread_create(&socket_rcv_tid, NULL, socket_rcv_fn, (void *)(&arg));
	ret = pthread_create(&socket_send_tid, NULL, socket_send_fn, (void  *)(&arg));
	ret = pthread_create(&socket_send_short_tid, NULL, socket_send_short_fn, (void  *)(&arg));
	ret = process_loop_msg();
	return ret;
}
