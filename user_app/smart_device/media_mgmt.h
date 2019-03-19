#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

//rtp frame
#define H264_FILE "huz.h264"

#define MAX_PACK_LEN (1448)
#define MAX_FILE_LEN (200*1024)
#define RTP_HEAD_LEN (12)
#define UDP_BASE_PORT   (60000+66)
#define H264_RTP_PKT_LEN    (4*1024)
#define AUDIO_RTP_PKT_LEN   (2*1024)

#define CH1_BUF_LEN         (200*1024)  /* 视频通道buf长度 */
#define MAX_VIDEO_FRAME_LEN (200*1024)
#define MAX_AUDIO_FRAME_LEN (1024)
#define MAX_RECV_BUF_LEN    (4*1024)

//rtp头结构
typedef struct
{
    /**//* byte 0 */
    unsigned char u4CSrcLen:4;      /**//* expect 0 */
    unsigned char u1Externsion:1;   /**//* expect 1, see RTP_OP below */
    unsigned char u1Padding:1;      /**//* expect 0 */
    unsigned char u2Version:2;      /**//* expect 2 */
    /**//* byte 1 */
    unsigned char u7Payload:7;      /**//* RTP_PAYLOAD_RTSP */
    unsigned char u1Marker:1;       /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short u16SeqNum;
    /**//* bytes 4-7 */
    unsigned int u32TimeStamp;
    /**//* bytes 8-11 */
    unsigned int u32SSrc;          /**//* stream number is used here. */
} StRtpFixedHdr;

struct rtp_pack_head
{
    unsigned short sernum;
    unsigned int timtamp;
    unsigned int ssrc;
};

//rtp数据结构
struct rtp_data
{
    void *buff;
    char rtp_fui;
    char rtp_fuh;
    unsigned int offset;
    unsigned int datalen;
    unsigned int bufrelen;
    unsigned int rtpdatakcount;
};
//rtp 包结构
struct rtp_pack
{
    void *databuff;
    unsigned int packlen;
    unsigned int packtype;
};

typedef enum
{
    FRAME_I     = 1,
    FRAME_P     = 2,
    FRAME_B     = 3,
    FRAME_IDR   = 4,
    FRAME_AAC   = 5,  /* aac */
    FRAME_G711  = 6,  /* g.711 */
    FRAME_G726  = 6,  /* g.726 */
    FRAME_G729  = 7,  /* g.729 */
    FRAME_MJPEG = 8,
    UNDEFINED_FRAME,
} FRAME_TYPE;

typedef enum _PAYLOAD
{
    RTP_PT_ULAW             = 0,        /* mu-law */
    RTP_PT_GSM              = 3,        /* GSM */
    RTP_PT_G723             = 4,        /* G.723 */
    RTP_PT_ALAW             = 8,        /* a-law */
    RTP_PT_G722             = 9,        /* G.722 */
    RTP_PT_S16BE_STEREO     = 10,       /* linear 16, 44.1khz, 2 channel */
    RTP_PT_S16BE_MONO       = 11,       /* linear 16, 44.1khz, 1 channel */
    RTP_PT_MPEGAUDIO        = 14,       /* mpeg audio */
    RTP_PT_JPEG             = 26,       /* jpeg */
    RTP_PT_H261             = 31,       /* h.261 */
    RTP_PT_MPEGVIDEO        = 32,       /* mpeg video */
    RTP_PT_MPEG2TS          = 33,       /* mpeg2 TS stream */
    RTP_PT_H263             = 34,       /* old H263 encapsulation */
    RTP_PT_H264             = 96,       /* define as h.264 */
    RTP_PT_G726             = 97,       /* define as G.726 */
    RTP_PT_ADPCM            = 98,       /* define as ADPCM */
    RTP_PT_MPEG4            = 99,
    RTP_PT_HK_AAC           = 104,      /* added by hehao */
    RTP_PT_AAC              = 108,
    RTP_PT_CTRL             = 109,
    RTP_PT_INVALID          = 127
} PAYLOAD;

typedef struct  _MEDIA_DEC_PKT
{
    /* AAC 需要用 */
    int  sample_rate;       /* 采样率 48K 44.1K */
    int  channel;           /* 通道数1 2 */
    unsigned int finish;

    char * buf;             /* 存放数据  buf, 调用者必须申请 */
    unsigned int len;       /* out:解码后包的长度 */

    unsigned short is_start;/* 是否是开始包 1表示是 0表示否 */
    unsigned short is_end;  /* 是否是结束包 1表示是 0表示否 */

    unsigned short frame_type;  /* 5: I帧 1: P帧 6: SEI 7: SPS 8: PPS */
    unsigned char  pt;          /* 负载类型 96 为 H264数据类型 */
    unsigned short seq;         /* 序列号 */
    unsigned int   ts;          /* 时戳 */
    unsigned int   ssrc;        /* sync source */
} MEDIA_DEC_PKT;


typedef struct _STREAM_SESSION_PARAM
{
    MEDIA_DEC_PKT  h264_pkt;
    MEDIA_DEC_PKT  audio_pkt;

    char * ch_buf[1];    /* 一个用来存储视频数据 另外一个用来存储音频数据 */
    int data_len[1];
    int finish_flag[1];
    int v_frame_count;                  /* 接收到的视频帧数 */
} STREAM_SESSION_PARAM;

#if 1
#define PCAP_VIDEO_FILE "doorphone.pcap"
#define PCAP_AUDIO_FILE "doorphone.pcap"
#define PCAP_FILE "hk.pcap"
#define BUFSIZE 4096

typedef int             bpf_int32;
typedef unsigned int    bpf_u_int32;
typedef unsigned short  u_short;
typedef unsigned int    u_int32;
typedef unsigned short  u_int16;
typedef unsigned char   u_int8;

//pcap数据包头结构体
struct pcap_pkthdr
{
    bpf_u_int32 tv_sec;                  /* seconds */
    bpf_u_int32 tv_usec;                 /* and microseconds */
    bpf_u_int32 caplen; /* length of portion present */
    bpf_u_int32 len;    /* length this packet (off wire) */
};
#endif
void init_signals(void);

int sock_udp_open(int sock_type);
void sock_udp_close(int sock_fd);
int sock_udp_bind(int sock_fd, unsigned short port);
int sock_udp_recv(int fd, char* buf, int len);
int sock_udp_send(int sock_fd, char * ip, int port, char * buffer, int len);

int get_data(unsigned char *buf, struct rtp_data *pdata);
int creat_rtp_pack(struct rtp_data *pdata,struct rtp_pack_head *head, struct rtp_pack *pack);
void udp_handle_data(STREAM_SESSION_PARAM* session_param, const char * data, int len);


