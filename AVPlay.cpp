#include "AVPlay.h"
#include <qDebug>

double synchronize_video(VideoState *is, AVFrame *src_frame, double pts);
void audio_callback(void *userdata, Uint8 *stream, int len);
int find_stream_index(AVFormatContext *pformat_ctx, int* video_stream, int* audio_stream);
int audio_decode_frame(VideoState *is, uint8_t *audio_buf);
int video_thread(void *arg);

AVPlay::AVPlay(QObject *parent)
    : QThread{parent}
{
    m_fileName=" ";
}
void AVPlay::run()
{
    //开始解码获取图片
    //添加音频需要的变量
    int audioStream = -1;                 //音频解码器需要的流的索引
    AVCodecContext *pAudioCodecCtx = NULL;//音频解码器信息指针
    AVCodec *pAudioCodec = NULL;          //音频解码器

    //SDL
    SDL_AudioSpec wanted_spec;            //SDL 音频设置
    SDL_AudioSpec spec ;                  //SDL 音频设置

    //视频
    AVCodecContext *pCodecCtx ;           //视频的解码器信息指针
    AVCodec *pCodec ;                     //视频解码器
    AVPacket *packet;                     //读取解码前的包

    //1.初始化 FFMPEG 调用了这个才能正常适用编码器和解码器 注册所用函数
    av_register_all();
    //SDL 初始化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Couldn't init SDL:%s\n", SDL_GetError());
        return;
    }
    memset ( &m_videoState , 0 , sizeof(VideoState) );
    //2.需要分配一个 AVFormatContext，FFMPEG 所有的操作都要通过这个 AVFormatContext 来进行 可以理解为视频文件指针
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //中文兼容
    std::string path = m_fileName.toStdString();
    const char* file_path = path.c_str();
    //3. 打开视频文件
    if( avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0 )
    {
        qDebug()<<"can't open file";
        return;
    }
    //3.1 获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug()<<"Could't find stream infomation.";
        return;
    }
    //4.读取视频流
    int videoStream = -1;
    //查找音频视频流索引
    if (find_stream_index(pFormatCtx, &videoStream, &audioStream) == -1)
    {
        printf("Couldn't find stream index\n");
        return;
    }
    m_videoState.pFormatCtx = pFormatCtx;
    m_videoState.videoStream = videoStream;
    m_videoState.audioStream = audioStream;
    m_videoState.m_player = this;
    //视频播放
    if(videoStream != -1)
    {
        //5.查找解码器
        pCodecCtx = pFormatCtx->streams[ videoStream ]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL)
        {
            printf("Codec not found.");
            return;
        }
        //打开解码器
        if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            printf("Could not open codec.");
            return;
        }
        //视频流
        m_videoState.video_st = pFormatCtx->streams[ videoStream ];
        m_videoState.pCodecCtx = pCodecCtx;
        //视频同步队列
        m_videoState.videoq = new PacketQueue;
        packet_queue_init( m_videoState.videoq);
        //创建视频线程
        m_videoState.video_tid = SDL_CreateThread( video_thread ,"video_thread" ,&m_videoState );
    }
    //音频播放
    if(audioStream != -1 )
    {
        //5.找到对应的音频解码器
        pAudioCodecCtx = pFormatCtx->streams[audioStream]->codec;
        pAudioCodec = avcodec_find_decoder(pAudioCodecCtx ->codec_id);
        if (!pAudioCodec)
        {
            printf( "Couldn't find decoder\n");
            return;
        }
        //打卡音频解码器
        avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);
        m_videoState.audio_st = pFormatCtx->streams[audioStream];
        m_videoState.pAudioCodecCtx = pAudioCodecCtx;
        //6.设置音频信息, 用来打开音频设备。
        SDL_LockAudio();
        wanted_spec.freq = pAudioCodecCtx->sample_rate;
        switch (pFormatCtx->streams[audioStream]->codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
            wanted_spec.format = AUDIO_S8;
            break;
        case AV_SAMPLE_FMT_S16:
            wanted_spec.format = AUDIO_S16SYS;
            break;
        default:
            wanted_spec.format = AUDIO_S16SYS;
            break;
        };
        wanted_spec.channels = pAudioCodecCtx->channels;       //通道数
        wanted_spec.silence = 0;                               //设置静音值
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;           //读取第一帧后调整
        wanted_spec.callback = audio_callback;                 //回调函数
        wanted_spec.userdata = &m_videoState;                  //回调函数参数
        //7.打开音频设备
        m_videoState.audioID = SDL_OpenAudioDevice( NULL ,0,&wanted_spec, &spec,0);
        if( m_videoState.audioID < 0 ) //第二次打开 audio 会返回-1
        {
            printf( "Couldn't open Audio: %s\n", SDL_GetError());
            return;
        }
        //设置参数，供解码时候用, swr_alloc_set_opts 的 in 部分参数
        switch (pFormatCtx->streams[audioStream]->codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
            break;
        default:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
            break;
        };
        m_videoState.out_frame.sample_rate = pAudioCodecCtx->sample_rate;
        m_videoState.out_frame.channel_layout =
            av_get_default_channel_layout(pAudioCodecCtx->channels);
        m_videoState.out_frame.channels = pAudioCodecCtx->channels;
        m_videoState.audioq = new PacketQueue;
        //初始化队列
        packet_queue_init(m_videoState.audioq);
        m_videoState.audioFrame = av_frame_alloc();
        SDL_UnlockAudio();
        // SDL 播放声音 0 播放
        SDL_PauseAudioDevice(m_videoState.audioID,0);
    }
    packet = (AVPacket *) malloc(sizeof(AVPacket));  //分配一个 packet
    //8.循环读取视频帧, 转换为 RGB 格式, 抛出信号去控件显示
    while(1)
    {
        if( m_videoState.quit ) break;
        //这里做了个限制 当队列里面的数据超过某个大小的时候 就暂停读取 防止一下子就把视频读完了，导致的空间分配不足
        //这里 audioq.size 是指队列中的所有数据包带的音频数据的总量或者视频数据总量，并不是包的数量
        //这个值可以稍微写大一些
        if( m_videoState.audioStream != -1 && m_videoState.audioq->size >MAX_AUDIO_SIZE )
        {
            SDL_Delay(10);
            continue;
        }
        if ( m_videoState.videoStream != -1 &&m_videoState.videoq->size >MAX_VIDEO_SIZE)
        {
            SDL_Delay(10);
            continue;
        }
        //可以看出 av_read_frame 读取的是一帧视频，并存入一个 AVPacket 的结构中
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            if( m_videoState.quit ) break;
            break; //这里认为视频读取完了
        }
        //生成图片
        if (packet->stream_index == m_videoState.videoStream)
        {
            packet_queue_put(m_videoState.videoq, packet);
        }
        else if ( packet->stream_index == m_videoState.audioStream)
        {
            packet_queue_put(m_videoState.audioq, packet);
        }
        else
        {
            av_free_packet(packet);
        }
    }
    //9.回收数据
    while( m_videoState.videoStream != -1&&m_videoState.videoq->nb_packets != 0)
    {
        if( m_videoState .quit ) break;
        SDL_Delay(100);
    }
    SDL_Delay(100);
    while( m_videoState.audioStream != -1 && m_videoState.audioq->nb_packets != 0)
    {
        if( m_videoState .quit ) break;
        SDL_Delay(100);
    }
    SDL_Delay(100);
    //回收空间
    if( audioStream != -1 )
        avcodec_close(pAudioCodecCtx);
    if( videoStream != -1 )
        avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    av_free( m_videoState.audioFrame );
    m_videoState.audioFrame = NULL;
}

void AVPlay::SetFilePath(QString path)
{
    m_fileName=path;
}
void AVPlay::SendGetOneImage(QImage img)
{
    emit SIG_GetOneImage(img); //发送信号
}

//视频解码线程函数
int video_thread(void *arg)
{
    VideoState *is = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;
    int ret, got_picture, numBytes;
    double video_pts = 0;                 //当前视频的 pts
    double audio_pts = 0;                 //音频 pts

    //解码视频相关
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb;              //解码后的 rgb 数据
    struct SwsContext *img_convert_ctx;   //用于解码后的视频格式转换
    AVCodecContext *pCodecCtx = is->pCodecCtx; //视频解码器
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //这里我们改成了 将解码后的 YUV 数据转换成 RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height);
    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);
    while(1)
    {
        if (packet_queue_get(is->videoq, packet, 1) <= 0) break;//队列里面没有数据了 读取完毕了

        while(1)
        {
            audio_pts = is->audio_clock;
            if (video_pts <= audio_pts) break;
            SDL_Delay(5);
        }
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Error decoding video frame\n");
            break;
        }

        //获取显示时间 pts
        video_pts = pFrame->pts = pFrame->best_effort_timestamp;
        video_pts *= 1000000 *av_q2d(is->video_st->time_base);
        video_pts = synchronize_video(is, pFrame, video_pts);//视频时钟补偿
        if (got_picture)
        {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //把这个 RGB 数据 用 QImage 加载
            QImage tmpImg((uchar*)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy();               //把图像复制一份 传递给界面显示
            is->m_player->SendGetOneImage(image);       //调用激发信号的函数
        }
        av_free_packet(packet);
    }
    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    return 0;
}

//时间补偿函数--视频延时 官方文档给出的
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts)
{
    double frame_delay;                  // 缓存帧和帧之间的延迟
    if (pts != 0)
    {
        // 如果当前帧有 PTS 时间戳，那么使用它来更新视频时钟
        is->video_clock = pts;
    }
    else
    {
        // 如果没有 PTS 时间戳，则采用视频时钟作为当前时间
        pts = is->video_clock;
    }

    // 计算当前帧和上一帧之间的时钟差
    frame_delay = av_q2d(is->video_st->codec->time_base);
    // 如果当前帧是重复帧，需要根据重复数调整帧之间的时间差
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    // 更新视频时钟
    is->video_clock += frame_delay;
    // 返回当前帧的 PTS 时间戳
    return pts;
}

//13.回调函数中将从队列中取数据, 解码后填充到播放缓冲区.
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    VideoState * is = (VideoState *) userdata;
    int len1, audio_data_size;
    /* len 是由 SDL 传入的 SDL 缓冲区的大小，如果这个缓冲未满，我们就一直往里填充数据
       audio_buf_index 和 audio_buf_size 标示我们自己用来放置解码出来的数据的缓冲区，
       这些数据待 copy 到 SDL 缓冲区， 当 audio_buf_index >= audio_buf_size 的时候意味着我
       们的缓冲为空，没有数据可供 copy，这时候需要调用 audio_decode_frame 来解码出更多的桢数据 */

    while (len > 0)
    {
        if (is->audio_buf_index >= is->audio_buf_size)
        {
            audio_data_size = audio_decode_frame( is ,is->audio_buf);

            //标示没能解码出数据，我们默认播放静音
            if (audio_data_size < 0)
            {
                //清零，静音
                is->audio_buf_size = 1024;
                memset(is->audio_buf, 0, is->audio_buf_size);
            }
            else
            {
                is->audio_buf_size = audio_data_size;
            }
            is->audio_buf_index = 0;
        }
        // 查看 stream 可用空间，决定一次 copy 多少数据，剩下的下次继续 copy
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
        {
            len1 = len;
        }
        memset( stream , 0 , len1);
        //混音函数 sdl 2.0 版本使用该函数 替换 SDL_MixAudio
        SDL_MixAudioFormat(stream, (uint8_t *) is->audio_buf + is->audio_buf_index,AUDIO_S16SYS,len1,100);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}

//解码音频帧函数  对于音频来说，一个 packet 里面，可能含有多帧(frame)数据。
int audio_decode_frame(VideoState *is, uint8_t *audio_buf)
{
    AVPacket pkt;
    int audio_pkt_size = 0;
    int data_size;
    AVCodecContext *aCodecCtx = is->pAudioCodecCtx;
    AVFrame *audioFrame = is->audioFrame;
    PacketQueue *audioq = is->audioq;
    AVFrame wanted_frame = is->out_frame;
    if( !aCodecCtx|| !audioFrame ||!audioq) return -1;
    struct SwrContext *swr_ctx = NULL;
    while(1)
    {
        if( is->quit ) return -1;
        if( !audioq ) return -1;
        if(packet_queue_get(audioq, &pkt, 0) <= 0) //一定注意
        {
            return -1;
        }
        audio_pkt_size = pkt.size;
        while(audio_pkt_size > 0)
        {
            if( is->quit ) return -1;
            int got_picture;
            memset(audioFrame, 0, sizeof(AVFrame));
            int ret =avcodec_decode_audio4( aCodecCtx, audioFrame, &got_picture, &pkt);
            if( ret < 0 )
            {
                printf("Error in decoding audio frame.\n");
                exit(0);
            }
            //一帧一个声道读取数据字节数是 nb_samples , channels 为声道数 2 表示 16 位2 个字节
            switch( is->out_frame.format )
            {
            case AV_SAMPLE_FMT_U8:
                data_size = audioFrame->nb_samples * is->out_frame.channels * 1;
                break;
            case AV_SAMPLE_FMT_S16:
                data_size = audioFrame->nb_samples * is->out_frame.channels * 2;
                break;
            default:
                data_size = audioFrame->nb_samples * is->out_frame.channels * 2;
                break;
            }
            //计算音频时钟
            if( pkt.pts != AV_NOPTS_VALUE)
            {
                is->audio_clock = pkt.pts * av_q2d( is->audio_st->time_base )*1000000 ;

            }
            else//取音频时钟
            {
                is->audio_clock = (*(uint64_t *)audioFrame->opaque)*av_q2d( is->audio_st->time_base )*1000000 ;
            }

            if( got_picture )
            {
                swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout,
                                             (AVSampleFormat)wanted_frame.format,wanted_frame.sample_rate,
                                             audioFrame->channel_layout,(AVSampleFormat)audioFrame->format,
                                             audioFrame->sample_rate, 0, NULL);
                //初始化
                if (swr_ctx == NULL || swr_init(swr_ctx) < 0)
                {
                    printf("swr_init error\n");
                    break;
                }
                swr_convert(swr_ctx, &audio_buf,AVCODEC_MAX_AUDIO_FRAME_SIZE,(const uint8_t **)audioFrame->data,audioFrame->nb_samples);
                swr_free( &swr_ctx );
            }
            audio_pkt_size -= ret;
            if (audioFrame->nb_samples <= 0)
            {
                continue;
            }
            av_free_packet(&pkt);
            return data_size ;
        }
        av_free_packet(&pkt);
    }
}

//查找数据流函数
int find_stream_index(AVFormatContext *pformat_ctx, int* video_stream, int* audio_stream)
{
    assert(video_stream != NULL || audio_stream != NULL);
    int i = 0;
    int audio_index = -1;
    int video_index = -1;
    for (i = 0; i < pformat_ctx->nb_streams; i++)
    {
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_index = i;
        }
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_index = i;
        }
    }
    //注意以下两个判断有可能返回-1.
    if (video_stream == NULL)
    {
        *audio_stream = audio_index;
        return *audio_stream;
    }
    if (audio_stream == NULL)
    {
        *video_stream = video_index;
        return *video_stream;
    }
    *video_stream = video_index;
    *audio_stream = audio_index;
    return 0;
}
