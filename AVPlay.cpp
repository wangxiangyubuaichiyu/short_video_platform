#include "AVPlay.h"
#include <qDebug>

double synchronize_video(VideoState *is, AVFrame *src_frame, double pts);
void audio_callback(void *userdata, Uint8 *stream, int len);
Uint32 timer_callback(Uint32 interval, void *param);
int find_stream_index(AVFormatContext *pformat_ctx, int* video_stream, int* audio_stream);
int audio_decode_frame(VideoState *is, uint8_t *audio_buf);
int video_thread(void *arg);

AVPlay::AVPlay(QObject *parent)
    : QThread{parent}
{
    m_fileName=" ";
    m_playerState=AVPlay::PlayerState::Stop;     //���ó�ʼ״̬
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
        //������Ƶ�߳�->��ֻ����Ƶû����Ƶ���������
        //m_videoState.video_tid = SDL_CreateThread( video_thread ,"video_thread" ,&m_videoState );
        if( m_videoState.audioStream != -1 )
        {
            //������Ƶ�߳�
            m_videoState.video_tid = SDL_CreateThread( video_thread , "video_thread" , &m_videoState );
        }
        else
        {
            double fps = av_q2d(m_videoState.video_st->r_frame_rate);
            double pts_diff = 1/ fps ;
            //��ȡ����ļ��ʱ��
            m_videoState.timer_id = SDL_AddTimer( pts_diff*1000, timer_callback, &m_videoState);
            if (m_videoState.timer_id == 0)
            {
                fprintf(stderr, "SDL_AddTimer Error: %s\n", SDL_GetError());
                return;
            }
        }
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
        m_videoState.out_frame.channel_layout =av_get_default_channel_layout(pAudioCodecCtx->channels);
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
    int DelayCount = 0;
    emit SIG_TotalTime(getTotalTime());
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
        //��ת
        if( m_videoState.seek_req )
        {
            // ��ת��־λ seek_req --> 1 ���������Ļ��� 3s --> 3min 3s ��������� ���� ���кͽ�����
            // 3s �ڽ�������������ݺ� 3min �Ļ����һ�� ������ --> ������� �������������AV_flush_...
            //ʲôʱ������ -->Ҫ������ , ����Ҫ����־�� FLUSH_DATA "FLUSH"
            //�ؼ�֡--���� 10 �� --> 15 �� ��ת�ؼ�֡ ֻ���� 10 �� 15 , �����Ҫ���� 13 , ����������10 Ȼ�� 10-13 �İ�ȫ�ӵ�
            int stream_index = -1;
            int64_t seek_target = m_videoState.seek_pos;                                 //΢��
            if (m_videoState.videoStream >= 0)
                stream_index = m_videoState.videoStream;
            else if (m_videoState.audioStream >= 0)
                stream_index = m_videoState.audioStream;
            AVRational aVRational = {1, AV_TIME_BASE};
            if (stream_index >= 0)
            {
                seek_target = av_rescale_q(seek_target, aVRational,
                                           pFormatCtx->streams[stream_index]->time_base); //��ת����λ��
            }
            if (av_seek_frame(m_videoState.pFormatCtx, stream_index, seek_target,AVSEEK_FLAG_BACKWARD) < 0)
            {
                fprintf(stderr, "%s: error while seeking\n",m_videoState.pFormatCtx->filename);
            }
            else
            {
                if (m_videoState.audioStream >= 0)
                {
                    AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));             //����һ�� packet
                    av_new_packet(packet, 10);
                    strcpy((char*)packet->data,FLUSH_DATA);
                    packet_queue_flush(m_videoState.audioq);                              //�������
                    packet_queue_put(m_videoState.audioq, packet);                        //�������д�����������İ�
                }
                if (m_videoState.videoStream >= 0)
                {
                    AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));             //����һ�� packet
                    av_new_packet(packet, 10);
                    strcpy((char*)packet->data,FLUSH_DATA);
                    packet_queue_flush(m_videoState.videoq);                              //�������
                    packet_queue_put(m_videoState.videoq, packet);                        //�������д�����������İ�
                    m_videoState.video_clock = 0;                                         //���ǵ�������� ���⿨��
                    //��Ƶ�����������Ƶ ѭ�� SDL_Delay ��ѭ�������� ��Ƶʱ�ӻ�ı� , ���� ��Ƶʱ�ӱ�С
                }
            }
            m_videoState.seek_req = 0;
            m_videoState.seek_time = m_videoState.seek_pos ; //��ȷ��΢�� seek_time ����������Ƶ��Ƶ��ʱ�ӵ��� --�ؼ�֡
            m_videoState.seek_flag_audio = 1;                //����Ƶ��Ƶѭ���� , �ж�, AVPacket �� FLUSH_DATA��ս���������
            m_videoState.seek_flag_video = 1;
        }
        //���Կ��� av_read_frame ��ȡ����һ֡��Ƶ��������һ�� AVPacket �Ľṹ��
        //ÿ������ȴ�3�룬���3����Ϊ��Ƶ����
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            DelayCount++;
            if( DelayCount>= 300)
            {
                m_videoState.readFinished = true;
                DelayCount = 0 ;
            }
            if( m_videoState.quit) break;     //�����߳�ִ���� �˳�
            SDL_Delay(10);
            continue;
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
    //��������
    while( !m_videoState.quit)
    {
        SDL_Delay(100);
    }
    if( m_videoState.videoStream != -1)
        packet_queue_flush( m_videoState.videoq);//���л���
    if( m_videoState.audioStream != -1)
        packet_queue_flush( m_videoState.audioq); //���л���

    while( m_videoState.videoStream != -1 && !m_videoState.videoThreadFinished )
    {
        SDL_Delay(10);
    }

    //���տռ�
    if( audioStream != -1)
    {
        avcodec_close(pAudioCodecCtx);
    }
    //9.��������
    if( videoStream != -1 )
    {
        avcodec_close(pCodecCtx);
    }
    avformat_close_input(&pFormatCtx);
    //������Դ֮��,�������Ӷ�ȡ�ļ��߳��˳���־.->ȫ���˳��ı�־
    m_videoState.readThreadFinished = true;
    //��Ƶ�Զ����� �ñ�־λ
    m_playerState = PlayerState::Stop;
}

void AVPlay::SetFilePath(QString path)
{

    if( m_playerState != PlayerState::Stop ) return;
    m_fileName=path;
    m_playerState = PlayerState::Playing;
    this->start();
}

void AVPlay::play()
{
    m_videoState.isPause = false;
    if( m_playerState != Pause) return;
    m_playerState = Playing;
}

void AVPlay::pause()
{
    m_videoState.isPause = true;
    if( m_playerState != Playing ) return;
    m_playerState = Pause;
}

double AVPlay::getCurrentTime()
{
    return m_videoState.audio_clock;
}

int64_t AVPlay::getTotalTime()
{
    if( m_videoState.pFormatCtx )
        return m_videoState.pFormatCtx->duration;
    return -1;
}

void AVPlay::stop(bool isWait)
{
    m_videoState .quit = 1;
    if( isWait ) //������־
    {
        while(m_videoState.readThreadFinished==-1&&(!m_videoState.readThreadFinished) )//�ȴ���ȡ�߳��˳�
        {
            if(m_videoState.audioStream==-1)
            {
                SDL_RemoveTimer(m_videoState.timer_id);
                m_videoState.videoThreadFinished=true;
            }
            SDL_Delay(10);
        }
    }
    //�ر� SDL ��Ƶ�豸
    if (m_videoState.audioID != 0)
    {
        SDL_LockAudio();
        SDL_PauseAudioDevice(m_videoState.audioID,1);//ֹͣ����,��ֹͣ��Ƶ�ص�����
        SDL_CloseAudioDevice( m_videoState.audioID );
        SDL_UnlockAudio();
        m_videoState.audioID = 0;
    }
    m_playerState = PlayerState::Stop;
    Q_EMIT SIG_PlayerStateChanged(PlayerState::Stop);
}

void AVPlay::SendGetOneImage(QImage img)
{
    emit SIG_GetOneImage(img); //�����ź�
}

void AVPlay::seek(int64_t pos)
{
    //��ȷ��΢��
    if(!m_videoState.seek_req)
    {
        m_videoState.seek_pos = pos;
        m_videoState.seek_req = 1;
    }
}

AVPlay::PlayerState AVPlay::playerState() const
{
    return m_playerState;
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
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,pCodecCtx->width, pCodecCtx->height);
    while(1)
    {
        if(is->quit)break;
        if(is->isPause)
        {
            SDL_Delay(5);//Ҫ��ͣ������һֱ������ cpuռ��ֱ�������ˡ�
            continue;
        }

        if (packet_queue_get(is->videoq, packet, 0) <= 0)
        {
            if( is->readFinished && is->audioq->nb_packets == 0)//û�������ͻ����
            {
                //���߳����
                break;
            }
            else
            {
                SDL_Delay(1);                                  //ֻ�Ƕ���������ʱû�����ݶ���->����û����Ƶ����������
                continue;
            }
        }
        if(strcmp((char*)packet->data,FLUSH_DATA) == 0)
        {
            avcodec_flush_buffers(is->video_st->codec);
            av_free_packet(packet);
            is->video_clock = 0;                               //�ܹؼ� , ����� ������ת, ��Ƶ֡��ȴ���Ƶ֡
            continue;
        }
        //��ȡ���
        while(1)
        {
            if(is->quit||is ->audioq->size == 0)break;        //��ֹû�������޷��˳�ͬ��
            audio_pts = is->audio_clock;
            video_pts = is->video_clock;                      //ͬ��ʱ��תʹ�õ�
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
        if (is->seek_flag_video)
        {
            //��������ת �������ؼ�֡��Ŀ��ʱ����⼸֡
            if (video_pts < is->seek_time)
            {
                av_free_packet(packet);
                continue;
            }
            else
            {
                is->seek_flag_video = 0;
            }
        }
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
    if( !is->quit)
    {
        is->quit = true;
    }
    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    is->videoThreadFinished = true;
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

    memset( stream , 0 , len);     //��ֹ��������
    if(is->isPause ) return;       //������ͣ

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

Uint32 timer_callback(Uint32 interval, void *param)
{
    VideoState *is = (VideoState *)param;
    AVPacket pkt1, *packet = &pkt1;
    int ret, got_picture, numBytes;
    double video_pts = 0;                       //��ǰ��Ƶ�� pts
    double audio_pts = 0;                       //��Ƶ pts
    //������Ƶ���
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb;                    //������ rgb ����
    struct SwsContext *img_convert_ctx;         //���ڽ�������Ƶ��ʽת��
    AVCodecContext *pCodecCtx = is->pCodecCtx;  //��Ƶ������
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    //�������Ǹĳ��� �������� YUV ����ת���� RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,
                                  pCodecCtx->width,pCodecCtx->height);
    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);
    do
    {
        if(is->quit)break;
        if (packet_queue_get(is->videoq, packet, 0) <= 0) break;        //��������û��������
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Error decoding video frame\n");
            break;
        }
        //��ȡ��ʾʱ�� pts
        video_pts = pFrame->pts = pFrame->best_effort_timestamp;
        video_pts *= 1000000 *av_q2d(is->video_st->time_base);
        video_pts = synchronize_video(is, pFrame, video_pts);           //��Ƶʱ�Ӳ���
        if (got_picture)
        {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //����� RGB ���� �� QImage ����
            QImage tmpImg((uchar*)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy();                               //��ͼ����һ�� ���ݸ�������ʾ
            is->m_player->SendGetOneImage(image);                       //���ü����źŵĺ���
        }
        av_free_packet(packet);                                         //�°濼��ʹ�� av_packet_unref() ����������
    }while(0);
    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    return interval;
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
        //��״̬������ͣ��ʱ�˳�����
        if( is->quit||is->isPause||!audioq )  return -1;
        if(packet_queue_get(audioq, &pkt, 0) <= 0) //һ��ע��
        {
            if( is->readFinished && is->audioq->nb_packets == 0 )
                is->quit = true;
            return -1;
        }
        if(strcmp((char*)pkt.data,FLUSH_DATA) == 0)
        {
            avcodec_flush_buffers(is->audio_st->codec);
            av_free_packet(&pkt);
            continue;
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
            //��ת���ؼ�֡,����һЩ֡
            if( is->seek_flag_audio)
            {
                if( is ->audio_clock < is->seek_time) //û�е�Ŀ��ʱ��
                {
                    break;
                }
                else
                {
                    is->seek_flag_audio = 0 ;
                }
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

//���л��պ���
void AVPlay::packet_queue_flush(PacketQueue *q)
{
    AVPacketList *pkt, *pkt1;
    SDL_LockMutex(q->mutex);
    for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1)
    {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    SDL_UnlockMutex(q->mutex);
}
