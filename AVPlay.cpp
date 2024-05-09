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
    //��ʼ�����ȡͼƬ
    //�����Ƶ��Ҫ�ı���
    int audioStream = -1;                 //��Ƶ��������Ҫ����������
    AVCodecContext *pAudioCodecCtx = NULL;//��Ƶ��������Ϣָ��
    AVCodec *pAudioCodec = NULL;          //��Ƶ������

    //SDL
    SDL_AudioSpec wanted_spec;            //SDL ��Ƶ����
    SDL_AudioSpec spec ;                  //SDL ��Ƶ����

    //��Ƶ
    AVCodecContext *pCodecCtx ;           //��Ƶ�Ľ�������Ϣָ��
    AVCodec *pCodec ;                     //��Ƶ������
    AVPacket *packet;                     //��ȡ����ǰ�İ�

    //1.��ʼ�� FFMPEG ��������������������ñ������ͽ����� ע�����ú���
    av_register_all();
    //SDL ��ʼ��
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Couldn't init SDL:%s\n", SDL_GetError());
        return;
    }
    memset ( &m_videoState , 0 , sizeof(VideoState) );
    //2.��Ҫ����һ�� AVFormatContext��FFMPEG ���еĲ�����Ҫͨ����� AVFormatContext ������ �������Ϊ��Ƶ�ļ�ָ��
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //���ļ���
    std::string path = m_fileName.toStdString();
    const char* file_path = path.c_str();
    //3. ����Ƶ�ļ�
    if( avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0 )
    {
        qDebug()<<"can't open file";
        return;
    }
    //3.1 ��ȡ��Ƶ�ļ���Ϣ
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug()<<"Could't find stream infomation.";
        return;
    }
    //4.��ȡ��Ƶ��
    int videoStream = -1;
    //������Ƶ��Ƶ������
    if (find_stream_index(pFormatCtx, &videoStream, &audioStream) == -1)
    {
        printf("Couldn't find stream index\n");
        return;
    }
    m_videoState.pFormatCtx = pFormatCtx;
    m_videoState.videoStream = videoStream;
    m_videoState.audioStream = audioStream;
    m_videoState.m_player = this;
    //��Ƶ����
    if(videoStream != -1)
    {
        //5.���ҽ�����
        pCodecCtx = pFormatCtx->streams[ videoStream ]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL)
        {
            printf("Codec not found.");
            return;
        }
        //�򿪽�����
        if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            printf("Could not open codec.");
            return;
        }
        //��Ƶ��
        m_videoState.video_st = pFormatCtx->streams[ videoStream ];
        m_videoState.pCodecCtx = pCodecCtx;
        //��Ƶͬ������
        m_videoState.videoq = new PacketQueue;
        packet_queue_init( m_videoState.videoq);
        //������Ƶ�߳�
        m_videoState.video_tid = SDL_CreateThread( video_thread ,"video_thread" ,&m_videoState );
    }
    //��Ƶ����
    if(audioStream != -1 )
    {
        //5.�ҵ���Ӧ����Ƶ������
        pAudioCodecCtx = pFormatCtx->streams[audioStream]->codec;
        pAudioCodec = avcodec_find_decoder(pAudioCodecCtx ->codec_id);
        if (!pAudioCodec)
        {
            printf( "Couldn't find decoder\n");
            return;
        }
        //����Ƶ������
        avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);
        m_videoState.audio_st = pFormatCtx->streams[audioStream];
        m_videoState.pAudioCodecCtx = pAudioCodecCtx;
        //6.������Ƶ��Ϣ, ��������Ƶ�豸��
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
        wanted_spec.channels = pAudioCodecCtx->channels;       //ͨ����
        wanted_spec.silence = 0;                               //���þ���ֵ
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;           //��ȡ��һ֡�����
        wanted_spec.callback = audio_callback;                 //�ص�����
        wanted_spec.userdata = &m_videoState;                  //�ص���������
        //7.����Ƶ�豸
        m_videoState.audioID = SDL_OpenAudioDevice( NULL ,0,&wanted_spec, &spec,0);
        if( m_videoState.audioID < 0 ) //�ڶ��δ� audio �᷵��-1
        {
            printf( "Couldn't open Audio: %s\n", SDL_GetError());
            return;
        }
        //���ò�����������ʱ����, swr_alloc_set_opts �� in ���ֲ���
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
        //��ʼ������
        packet_queue_init(m_videoState.audioq);
        m_videoState.audioFrame = av_frame_alloc();
        SDL_UnlockAudio();
        // SDL �������� 0 ����
        SDL_PauseAudioDevice(m_videoState.audioID,0);
    }
    packet = (AVPacket *) malloc(sizeof(AVPacket));  //����һ�� packet
    //8.ѭ����ȡ��Ƶ֡, ת��Ϊ RGB ��ʽ, �׳��ź�ȥ�ؼ���ʾ
    while(1)
    {
        if( m_videoState.quit ) break;
        //�������˸����� ��������������ݳ���ĳ����С��ʱ�� ����ͣ��ȡ ��ֹһ���ӾͰ���Ƶ�����ˣ����µĿռ���䲻��
        //���� audioq.size ��ָ�����е��������ݰ�������Ƶ���ݵ�����������Ƶ���������������ǰ�������
        //���ֵ������΢д��һЩ
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
        //���Կ��� av_read_frame ��ȡ����һ֡��Ƶ��������һ�� AVPacket �Ľṹ��
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            if( m_videoState.quit ) break;
            break; //������Ϊ��Ƶ��ȡ����
        }
        //����ͼƬ
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
    //9.��������
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
    //���տռ�
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
    emit SIG_GetOneImage(img); //�����ź�
}

//��Ƶ�����̺߳���
int video_thread(void *arg)
{
    VideoState *is = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;
    int ret, got_picture, numBytes;
    double video_pts = 0;                 //��ǰ��Ƶ�� pts
    double audio_pts = 0;                 //��Ƶ pts

    //������Ƶ���
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb;              //������ rgb ����
    struct SwsContext *img_convert_ctx;   //���ڽ�������Ƶ��ʽת��
    AVCodecContext *pCodecCtx = is->pCodecCtx; //��Ƶ������
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //�������Ǹĳ��� �������� YUV ����ת���� RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height);
    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);
    while(1)
    {
        if (packet_queue_get(is->videoq, packet, 1) <= 0) break;//��������û�������� ��ȡ�����

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

        //��ȡ��ʾʱ�� pts
        video_pts = pFrame->pts = pFrame->best_effort_timestamp;
        video_pts *= 1000000 *av_q2d(is->video_st->time_base);
        video_pts = synchronize_video(is, pFrame, video_pts);//��Ƶʱ�Ӳ���
        if (got_picture)
        {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //����� RGB ���� �� QImage ����
            QImage tmpImg((uchar*)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy();               //��ͼ����һ�� ���ݸ�������ʾ
            is->m_player->SendGetOneImage(image);       //���ü����źŵĺ���
        }
        av_free_packet(packet);
    }
    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    return 0;
}

//ʱ�䲹������--��Ƶ��ʱ �ٷ��ĵ�������
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts)
{
    double frame_delay;                  // ����֡��֮֡����ӳ�
    if (pts != 0)
    {
        // �����ǰ֡�� PTS ʱ�������ôʹ������������Ƶʱ��
        is->video_clock = pts;
    }
    else
    {
        // ���û�� PTS ʱ������������Ƶʱ����Ϊ��ǰʱ��
        pts = is->video_clock;
    }

    // ���㵱ǰ֡����һ֮֡���ʱ�Ӳ�
    frame_delay = av_q2d(is->video_st->codec->time_base);
    // �����ǰ֡���ظ�֡����Ҫ�����ظ�������֮֡���ʱ���
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    // ������Ƶʱ��
    is->video_clock += frame_delay;
    // ���ص�ǰ֡�� PTS ʱ���
    return pts;
}

//13.�ص������н��Ӷ�����ȡ����, �������䵽���Ż�����.
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    VideoState * is = (VideoState *) userdata;
    int len1, audio_data_size;
    /* len ���� SDL ����� SDL �������Ĵ�С������������δ�������Ǿ�һֱ�����������
       audio_buf_index �� audio_buf_size ��ʾ�����Լ��������ý�����������ݵĻ�������
       ��Щ���ݴ� copy �� SDL �������� �� audio_buf_index >= audio_buf_size ��ʱ����ζ����
       �ǵĻ���Ϊ�գ�û�����ݿɹ� copy����ʱ����Ҫ���� audio_decode_frame ������������������ */

    while (len > 0)
    {
        if (is->audio_buf_index >= is->audio_buf_size)
        {
            audio_data_size = audio_decode_frame( is ,is->audio_buf);

            //��ʾû�ܽ�������ݣ�����Ĭ�ϲ��ž���
            if (audio_data_size < 0)
            {
                //���㣬����
                is->audio_buf_size = 1024;
                memset(is->audio_buf, 0, is->audio_buf_size);
            }
            else
            {
                is->audio_buf_size = audio_data_size;
            }
            is->audio_buf_index = 0;
        }
        // �鿴 stream ���ÿռ䣬����һ�� copy �������ݣ�ʣ�µ��´μ��� copy
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
        {
            len1 = len;
        }
        memset( stream , 0 , len1);
        //�������� sdl 2.0 �汾ʹ�øú��� �滻 SDL_MixAudio
        SDL_MixAudioFormat(stream, (uint8_t *) is->audio_buf + is->audio_buf_index,AUDIO_S16SYS,len1,100);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}

//������Ƶ֡����  ������Ƶ��˵��һ�� packet ���棬���ܺ��ж�֡(frame)���ݡ�
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
        if(packet_queue_get(audioq, &pkt, 0) <= 0) //һ��ע��
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
            //һ֡һ��������ȡ�����ֽ����� nb_samples , channels Ϊ������ 2 ��ʾ 16 λ2 ���ֽ�
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
            //������Ƶʱ��
            if( pkt.pts != AV_NOPTS_VALUE)
            {
                is->audio_clock = pkt.pts * av_q2d( is->audio_st->time_base )*1000000 ;

            }
            else//ȡ��Ƶʱ��
            {
                is->audio_clock = (*(uint64_t *)audioFrame->opaque)*av_q2d( is->audio_st->time_base )*1000000 ;
            }

            if( got_picture )
            {
                swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout,
                                             (AVSampleFormat)wanted_frame.format,wanted_frame.sample_rate,
                                             audioFrame->channel_layout,(AVSampleFormat)audioFrame->format,
                                             audioFrame->sample_rate, 0, NULL);
                //��ʼ��
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

//��������������
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
    //ע�����������ж��п��ܷ���-1.
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
