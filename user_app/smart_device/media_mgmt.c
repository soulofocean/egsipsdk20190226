#include "media_mgmt.h"

//创建rtp包
void *creat_rtp_data(struct rtp_data *data)
{
    unsigned char *buf;
    //数据长度检测，数据长度大于最大包长，则分包。
    if(data->datalen>MAX_PACK_LEN&&data->datalen>(data->bufrelen+data->rtpdatakcount))
    {
        //分包
        unsigned int templen=(data->datalen-data->bufrelen-1);
        data->rtpdatakcount+=1;
        if(templen>MAX_PACK_LEN-1)
        {
            buf=(unsigned char *)malloc(MAX_PACK_LEN);
            memset(buf,0,MAX_PACK_LEN);
            if(data->bufrelen==0)
            {
                //第一个分包
                memcpy((buf+RTP_HEAD_LEN+1),(data->buff+data->offset),(MAX_PACK_LEN-RTP_HEAD_LEN-1));
                data->rtp_fui=((*(buf+RTP_HEAD_LEN+1)&0xE0));
                data->rtp_fuh=(*(buf+RTP_HEAD_LEN+1)&0x1f);
                *(buf+RTP_HEAD_LEN+1)=(data->rtp_fuh|(0x80));
                data->bufrelen+=MAX_PACK_LEN;
                data->offset+=MAX_PACK_LEN-RTP_HEAD_LEN-1;
            }
            else
            {
            //中间分包
                 memcpy((buf+RTP_HEAD_LEN+2),(data->buff+data->offset),(MAX_PACK_LEN-RTP_HEAD_LEN-2));
                *(buf+RTP_HEAD_LEN+1)=(data->rtp_fuh|(0x00));
                data->bufrelen+=MAX_PACK_LEN;
                data->offset+=MAX_PACK_LEN-RTP_HEAD_LEN-2;
            }
            *(buf+RTP_HEAD_LEN)=(data->rtp_fui|(0x1c));
        }
        else
        {
            //最后一个分包
            templen=data->datalen-data->offset;
            buf=(unsigned char *)malloc(templen+RTP_HEAD_LEN+2);
            memset(buf,0,templen+RTP_HEAD_LEN+2);
            memcpy((buf+RTP_HEAD_LEN+2),(data->buff+data->offset),templen);
            *(buf+RTP_HEAD_LEN)=(0x1c|data->rtp_fui);
            *(buf+RTP_HEAD_LEN+1)=(data->rtp_fuh|(0x40));
            data->bufrelen+=templen+RTP_HEAD_LEN+2;
            data->offset+=templen-1;
        }
    }
    else if(data->datalen>data->bufrelen)
    {
        //数据长度小于包长，则不分包
        buf=(unsigned char *)malloc(data->datalen+RTP_HEAD_LEN);
        memset(buf,0,data->datalen+RTP_HEAD_LEN);
 
        memcpy((buf+RTP_HEAD_LEN),data->buff,data->datalen);
        data->bufrelen+=data->datalen+RTP_HEAD_LEN;
    }
    else
    {
        return NULL;
    }

    return buf;
}

int creat_rtp_pack(struct rtp_data *pdata, struct rtp_pack_head *head, struct rtp_pack *pack)
{
    char *rtp_buff;
    int len=pdata->bufrelen;
    StRtpFixedHdr  *pRtpFixedHdr;
 
    //获取封包后的rtp数据包
    rtp_buff=creat_rtp_data(pdata);

    if(rtp_buff!=NULL)
    {
        //固定头部填充
        pRtpFixedHdr = (StRtpFixedHdr *)rtp_buff;

        pRtpFixedHdr->u7Payload   = 96;
        pRtpFixedHdr->u2Version   = 2;
        pRtpFixedHdr->u1Marker    = 0;
        pRtpFixedHdr->u32SSrc     = head->ssrc;
        pRtpFixedHdr->u16SeqNum   = htons(head->sernum);
        pRtpFixedHdr->u32TimeStamp = htonl(head->timtamp);

        //创建rtp_pack结构
        pack->databuff=rtp_buff;
        pack->packlen=pdata->bufrelen-len;
    }
    else
    {
        free(pdata->buff);
        pdata->buff=NULL;
        return -1;
    }
    return 0;
}
 
//H264 开始码检测
char checkend(unsigned char *p)
{
    if((*(p+0)==0x00)&&(*(p+1)==0x00)&&(*(p+2)==0x00)&&(*(p+3)==0x01))
        return 1;
    else
        return 0;
}
 
//压入新读取的字节
void put_to_char(unsigned char *tempbuff,unsigned char c)
{
    *(tempbuff+0)=*(tempbuff+1);
    *(tempbuff+1)=*(tempbuff+2);
    *(tempbuff+2)=*(tempbuff+3);
    *(tempbuff+3)=c;
}
 
//获取H264 数据
int get_data(unsigned char *buf, struct rtp_data *pdata)
{
    unsigned int len=0;
    unsigned char tempbuff[4];
    unsigned char c;
    unsigned int i=0;

    //跳过文件头的开始码
    tempbuff[0] = buf[4];
    tempbuff[1] = buf[5];
    tempbuff[2] = buf[6];
    tempbuff[3] = buf[7];

    for(i = 0; i < (MAX_FILE_LEN - 4); i ++)
    {
        if(checkend(tempbuff))
        {
            len = i - 4;
            break;
        }
        else
        {
            //将下一个字节压入缓冲区。
            c = buf[4 + i];
            put_to_char(tempbuff,c);
        }
    }

    unsigned char *databuf=(unsigned char *)malloc(len);
    memcpy(databuf,(buf+4),len);

    pdata->buff=databuf;
    pdata->datalen=len;
    pdata->bufrelen=0;
    return 0;
}

static const char start_sequence[4] = { 0x00, 0x00, 0x00, 0x01 };

int rtp_g711_dec_packet(MEDIA_DEC_PKT *pkt, const char *buf, int len)
{
    int ret = -1;
    if ((len <= (int)sizeof( StRtpFixedHdr)) || (NULL == pkt))
    {
        printf("G711 RTP packet len %d invalid \n",  len);
        return -1;
    }
    if (NULL == buf)
    {
        printf("Invalid param !\n");
        return -1;
    }

    StRtpFixedHdr * rtp_hdr = (StRtpFixedHdr *)buf;

    len -= sizeof(StRtpFixedHdr);

    if ((RTP_PT_ULAW != rtp_hdr->u7Payload) &&
        (RTP_PT_G726 != rtp_hdr->u7Payload) &&
        (RTP_PT_ALAW != rtp_hdr->u7Payload) &&
        (RTP_PT_HK_AAC != rtp_hdr->u7Payload)) /* 检查是否是音频数据包 */
    {
        // media_mgmt_err("Not A G.711 RTP packet pt is %d \n", rtp_hdr->pt);
        return -1;
    }

    pkt->pt   = rtp_hdr->u7Payload;
    pkt->seq  = rtp_hdr->u16SeqNum;
    pkt->ts   = rtp_hdr->u32TimeStamp;
    pkt->ssrc = rtp_hdr->u32SSrc;

    /* 数据长度小于buffer长度 */
    if ((len <= (int)pkt->len) && (len > 0))
    {
        /* 前面已经减去了RTP头的长度 */
        memcpy(pkt->buf, buf+sizeof( StRtpFixedHdr), len);
        pkt->len = len;

        ret = 0;
    }

    return ret;
}



int rtp_h264_dec_packet(MEDIA_DEC_PKT *pkt, const char *buf, int len)
{
    if ((NULL == pkt) || (NULL == buf))
    {
        printf("Invalid param !\n");
        return -1;
    }

    int ret = -1;
    unsigned char nal;
    unsigned char type;
    unsigned char * nal_buf = (unsigned char *)&buf[12];
    StRtpFixedHdr * rtp_hdr = (StRtpFixedHdr *)buf;
    //uint16_t        cur_seq = 0;

    len -= sizeof(StRtpFixedHdr);

    if (96 != rtp_hdr->u7Payload) /* 检查是否是H.264数据包 */
    {
        printf("Not A H.264 RTP packet type %d \n", rtp_hdr->u7Payload);
        return -1;
    }

    pkt->pt   = rtp_hdr->u7Payload;
    pkt->seq  = rtp_hdr->u16SeqNum;
    pkt->ts   = rtp_hdr->u32TimeStamp;
    pkt->ssrc = rtp_hdr->u32SSrc;

    nal  = nal_buf[0]&0xFF;
    type = nal&0x1f;

    /* Simplify the case these are all the NAL types used internally by */
    /* the H.264 codec. */
    if ((type >= 1) && (type <= 23))
    {
        type = 1; /* 单个NAL单元包 */
    }

    switch (type)
    {
        case 0: /* undefined, but pass them through */
        case 1:
        {
            if (pkt->len >= ((unsigned int)len + sizeof(start_sequence)))
            {
                memcpy(pkt->buf, start_sequence, sizeof(start_sequence));
                memcpy(pkt->buf + sizeof(start_sequence), nal_buf, len);

                pkt->is_start = 1;
                pkt->is_end   = 1;

                pkt->frame_type = (pkt->buf[4]&0x1F);

                if (pkt->buf[5] == 0x88)
                {
                    // pkt->frame_type = 0x05;
                }

                pkt->len = len + sizeof(start_sequence);

                ret = 0;
                break;
            }
            else
            {
                printf("invalid buffer len:%d < %d\n", pkt->len, len);
                break;
            }
        }

        case 28:    /* FU-A (fragmented nal) */
        {
            unsigned char fu_indicator, fu_header, start_bit, nal_type, nal, end_bit;
            unsigned short pos = 0;
            int data_len = 0;
            if (len < 3)          /* 因为len已经减去了 RTPHEAD的长度 */
            {
                printf("Too short data for FU-A H.264 RTP packet\n");
                break;
            }

            fu_indicator = nal_buf[0]&0xFF;
            fu_header    = nal_buf[1]&0xFF;
            start_bit    = (fu_header >> 7);
            end_bit      = (fu_header >> 6);

            // printf("start 0x%x end 0x%x 0x%x 0x%x \n", start_bit, end_bit, fu_header, nal_buf[1]);

            nal_type     = fu_header & 0x1f;
            nal          = ((fu_indicator & 0xe0) | nal_type); /* indicator 的前三位和 FU header 的后5位 */

            /* skip the fu_indicator and fu_header */
            nal_buf += 2;
            len     -= 2;

            /*
            输入缓存区的长度必须足够放下所有数据
            如果是第一包 则必须加上00 00 00 01 并且将开始两字节的数据 整理成 1 字节
            如果不是第一包 则直接去掉前面两字节把数据保存下来就行
            */
            data_len = (0x01 == start_bit) ? (len + (int)sizeof(start_sequence) + 1) : (len);
            if (pkt->len >= (unsigned int)data_len)
            {
                if (0x01 == start_bit)
                {
                    memcpy(pkt->buf + pos, start_sequence, sizeof(start_sequence));
                    /* FU indicator的前三位和FU Header的后五位，即0110 0101（0x65）为NAL类型 */
                    pos += sizeof(start_sequence);
                    memcpy(pkt->buf + pos, &nal, 1); /* 一个字节 */
                    pos += 1;
                }

                memcpy(pkt->buf + pos, nal_buf, len);

                if (0x01 == start_bit)
                {
                    pkt->frame_type = (nal&0x1F);
                    if (pkt->buf[5] == 0x88)
                    {
                        // pkt->frame_type = 0x05;
                    }
                }

                pkt->is_start = (start_bit & 0x01);
                pkt->is_end   = (end_bit & 0x01);
                pkt->len      = (len + pos);
                ret = 0;
            }
            else
            {
                printf("invalid buffer len:%d < %d\n", pkt->len, len);
                break;
            }
            break;
        }

        case 24:    /* STAP-A (one packet, multiple nals) */
        case 25:    /* STAP-B */
        case 26:    /* MTAP-16 */
        case 27:    /* MTAP-24 */
        case 29:    /* FU-B */
        case 30:    /* undefined */
        case 31:    /* undefined */
        default:
            ret = -1;
            printf("H.264 unsupportted packet type %d \n", type);
            break;
    }


    return ret;
}

void udp_handle_data(STREAM_SESSION_PARAM* session_param, const char * data, int len)
{
    int track = 0;

    if (NULL == data)   /* 表示线程已经退出 */
    {
        printf("thread exit ?\n");
        return;
    }

    /* 视频目前支持H.264 */
    StRtpFixedHdr * rtp_hdr = (StRtpFixedHdr *)data;
    if (96 == rtp_hdr->u7Payload)
    {
        track = 0;
    }
    else
    {
        track = 1;
    }

    if (0 == track)
    {
        session_param->h264_pkt.len = H264_RTP_PKT_LEN;
        if (0 == rtp_h264_dec_packet(&(session_param->h264_pkt), data, len))
        {
            if (96 == session_param->h264_pkt.pt)
            {
                if ((1 == session_param->h264_pkt.is_start) &&
                    (1 == session_param->h264_pkt.is_end))
                {
                    /* 一包对应一帧 */
                    if (session_param->h264_pkt.len < CH1_BUF_LEN)
                    {
                        memcpy(session_param->ch_buf[0], session_param->h264_pkt.buf, session_param->h264_pkt.len);
                        session_param->data_len[0] = session_param->h264_pkt.len;
                        session_param->finish_flag[0] = 1;
                    }
                }
                else
                {
                    if (0 == session_param->h264_pkt.is_end) /* 分包时 第一包 或者 中间包 */
                    {
                        /* 分多包时 开始包 或 中间包 */
                        if ((session_param->data_len[0] + session_param->h264_pkt.len) < CH1_BUF_LEN)
                        {
                            memcpy(session_param->ch_buf[0] + session_param->data_len[0], session_param->h264_pkt.buf, session_param->h264_pkt.len);
                            session_param->finish_flag[0] = 0;
                            session_param->data_len[0]    += session_param->h264_pkt.len;
                        }
                    }
                    else
                    {
                        /* 分多包时 结束包 */
                        if ((session_param->data_len[0] + session_param->h264_pkt.len) < CH1_BUF_LEN)
                        {
                            memcpy(session_param->ch_buf[0] + session_param->data_len[0], session_param->h264_pkt.buf, session_param->h264_pkt.len);
                            session_param->finish_flag[0] = 1;
                            session_param->data_len[0]    += session_param->h264_pkt.len;
                        }
                    }
                }
            }

            if ((session_param->finish_flag[0] == 1) &&
                (session_param->data_len[0] > 0) &&
                (session_param->data_len[0] < MAX_VIDEO_FRAME_LEN))
            {
                session_param->data_len[0]    = 0;
                session_param->finish_flag[0] = 0;
            }
        }
        else
        {
            /*如果返回-1清除当前的session*/
            session_param->data_len[0]    = 0;
            session_param->finish_flag[0] = 0;
        }
    }
    else if (1 == track) /* 音频 */
    {
        /* mpeg4-generic/48000/1 */
        session_param->audio_pkt.len         = AUDIO_RTP_PKT_LEN;
        session_param->audio_pkt.channel     = 1;
        session_param->audio_pkt.sample_rate = 8000; // 48000;

        if (0 == rtp_g711_dec_packet(&(session_param->audio_pkt), data, len))
        {
            if (session_param->audio_pkt.len < MAX_AUDIO_FRAME_LEN)
            {
#if 1
                  static long count = 0;
                  static FILE * fp = NULL;
                
                  if (NULL == fp)
                  {
                      fp = fopen("huz_recv.g711", "wb");
                  }
                
                  if (NULL != fp)
                  {
                      printf("write %d bytes \n",session_param->audio_pkt.len);
                      fwrite(session_param->audio_pkt.buf, 1, session_param->audio_pkt.len, fp);
                  }
                  count ++;
                  if(count>0xfffffff)
                  {
                      printf("write success \n");
                      //fclose(fp);
                      //fp = NULL;
                  }
#endif

            }
        }
    }
}



int sock_udp_open(int sock_type)
{
    int fd = socket(AF_INET, sock_type, 0);
    if (fd <= 0)
    {
        printf("sock_open failed errno: 0x%x \n", errno);
        return -1;
    }

    return fd;
}

void sock_udp_close(int sock_fd)
{
    if (sock_fd > 0)
    {
        close(sock_fd);
    }
}

int sock_udp_bind(int sock_fd, unsigned short port)
{
    struct  sockaddr_in svr_addr;
    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sin_family       = AF_INET;
    svr_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    svr_addr.sin_port         = htons(port);
    if (-1 == bind(sock_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_in)))
    {
        printf("udp bind failed \n");
        return -1;
    }

    return 0;
}

int sock_udp_recv(int fd, char* buf, int len)
{
    if ((buf == NULL ) || (len <= 0))
    {
        printf("Invalid param !\n");
        return -1;
    }

    int ret = -1;
    struct sockaddr_in tmp_addr;
    socklen_t addr_len = sizeof(tmp_addr);
    ret = recvfrom(fd, buf, len, 0, (struct sockaddr *)&tmp_addr, &addr_len);
    if (ret < 0)
    {
        if ((errno == EWOULDBLOCK) || (errno == EINTR))
        {
            return -1;
        }
        return 0;
    }

    return ret;
}

int sock_udp_send(int sock_fd, char * ip, int port, char * buffer, int len)
{
    if ((NULL == ip) || (NULL == buffer) || (len <= 0))
    {
        printf("Invalid param !\n");
        return 0;
    }
    struct sockaddr_in addr;
    addr.sin_family        = AF_INET;
    addr.sin_port          = htons(port);
    addr.sin_addr.s_addr   = inet_addr(ip);
    //addr.sin_addr.s_addr   = htonl(INADDR_ANY);
    memset(&addr.sin_zero, 0, 8);

    int ret = sendto(sock_fd, buffer, len, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        if ((errno == EWOULDBLOCK) || (errno == EINTR))
        {
            printf("udp send failed \n");
            return -1; /* would block */
        }

        return 0;
    }

    return ret;
}

#if 0

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


int main(void)
{
    //打开H264视频文件
    FILE *file=fopen(H264_FILE,"r");
    struct stat statbuf;

    stat(H264_FILE,&statbuf);
    int file_size_cur=statbuf.st_size;
    int file_size=statbuf.st_size;

    init_signals();

    //struct rtp_pack rtp;
    //rtp_data结构创建，填充
    struct rtp_data pdata;
    memset(&pdata,0,sizeof(struct rtp_data));

    struct rtp_pack pack;
    memset(&pack,0,sizeof(struct rtp_pack));

    int sock_fd = sock_udp_open(SOCK_DGRAM);

    char ip[16];
    unsigned char *buf=(unsigned char *)malloc(MAX_FILE_LEN);
    if(buf == NULL)
    {
        return 0;
    }
    memset(buf,0,(MAX_FILE_LEN));

    //创建rtp头
    struct rtp_pack_head head;
    head.sernum=0x5372;
    head.ssrc=0x69257765;
    head.timtamp=0x9afcaf27;

    pdata.datalen = 0;
    unsigned int file_index = 0;

    while(1)
    {
        if(file_size_cur<=0)
        {
            file_size_cur = file_size;
            file_index = 0;
            break;
        }
        fseek(file,file_index,0);
        memset(buf,0,(MAX_FILE_LEN));
        fread(buf,sizeof(char),MAX_FILE_LEN,file);
        memset(&pdata,0,sizeof(struct rtp_data));

        if(0 == get_data(buf, &pdata))
        {
            //数据封包
            while(0 == creat_rtp_pack(&pdata,&head, &pack))
            {
                //数据发送
                udp_sock_send(sock_fd, ip, UDP_BASE_PORT, pack.databuff, pack.packlen);
                printf("file:%s function:%s, line: %d len=%d huz\n",__FILE__,__FUNCTION__,__LINE__,pack.packlen);
#if 0
                if (handle->cb_fun != NULL)
                {
                    /* handle rtp, +4 是要跳过 RTP_TCP_HDR 头 */
                    handle->cb_fun(handle->user_info, 0, pack.databuff, pack.packlen);
                }
#endif
                //序列号增加
                free(pack.databuff);
                head.sernum++;
                usleep(10*1000);
            }
            //时间戳增加
            head.timtamp+=3000;
        }
        file_size_cur -= (pdata.datalen+4);
        file_index += (pdata.datalen+4);
    }

    free(buf);
    fclose(file);
    return 0;

}
#endif


