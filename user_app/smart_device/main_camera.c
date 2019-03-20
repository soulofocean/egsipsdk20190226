#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "camera_dev.h"

void init_signals(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    /* 忽略socket写入错误导致的SIGPIPE信号 */
    sigaddset(&sa.sa_mask, SIGPIPE);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
}

int main_camera(int argc, char *argv[])
{
    EGSIP_RET_CODE ret = EGSIP_RET_ERROR;

    init_signals();

    // 初始化SDK
    ret = egsip_sdk_init(printf);
    egsip_log_info("main enter\n");
    
    if(ret != EGSIP_RET_SUCCESS)
    {
        egsip_log_error("egsip_sdk_init failed\n");
        return -1;
    }

    // 初始化设备信息
    camera_init();

    // 创建设备
    int mydev_handle = -1;
    ret = egsip_dev_create(&g_camera_dev_info, camera_status_callback, &mydev_handle);
    if(ret != EGSIP_RET_SUCCESS)
    {
        egsip_log_error("egsip_dev_create failed\n");
        egsip_sdk_uninit();
        return -1;
    }

    // 注册设备功能
    int get_len = 0;
    ret = egsip_dev_func_register(mydev_handle, &g_camera_srv_req_cb_tbl, sizeof(g_camera_srv_req_cb_tbl), 
                                    &g_camera_req_if_tbl, sizeof(g_camera_req_if_tbl), &get_len);
    if(ret != EGSIP_RET_SUCCESS || get_len != sizeof(g_camera_req_if_tbl))
    {
        egsip_log_error("egsip_dev_func_register failed, ret(%d) get_len(%d).\n", ret, get_len);
        egsip_dev_delete(mydev_handle);
        egsip_sdk_uninit();
        return -1;
    }

    // 启动设备
    ret = egsip_dev_start(mydev_handle);
    if(ret != EGSIP_RET_SUCCESS)
    {
        egsip_log_error("egsip_dev_start failed\n");
        egsip_dev_delete(mydev_handle);
        egsip_sdk_uninit();
        return -1;
    }

    int i=30;
    while(i>=0)
    {
        //i--;
        //if(i%10 == 0)
            //camera_report_alarm(mydev_handle); // 上报事件
        usleep(1000*1000);
    }

    camera_del();

    // 停止设备
    egsip_dev_stop(mydev_handle);

    // 删除设备
    egsip_dev_delete(mydev_handle);
    mydev_handle = -1;

    //释放SDK
    egsip_sdk_uninit();

    return 0;
}

