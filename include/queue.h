#ifndef _QUEUE_H_
#define _QUEUE_H_
#include "w32thread.h"
extern "C"
{
#define __STDC_CONSTANT_MACROS
#include "libavutil\avutil.h"
#include "libavformat\avformat.h"
#include "libavcodec\avcodec.h"
#include "libswscale\swscale.h"
};

typedef struct PacketListNode{
    AVPacket pkt;
    PacketListNode* next;
}PacketListNode;

typedef struct PacketQueue{
    PacketListNode* firstNode, *lastNode;
    int size;    // current packet queue size
    int maxsize; // maximum packet queue size
    int abort;
    pthread_mutex_t* mtx;
    pthread_cond_t*  cond;
}PacketQueue;

void packet_queue_init(PacketQueue* pq);
void packet_queue_destory(PacketQueue* pq);
void packet_queue_put(PacketQueue* pq, AVPacket* pkt);
void packet_queue_get(PacketQueue* pq, AVPacket *rpkt);
void packet_queue_abort(PacketQueue* pq);

typedef struct Frame{
    AVFrame* yuvframe;
    AVFrame* rgbframe;
    double pts;
}Frame;

#define FRAME_QUEUE_SIZE 20
typedef struct FrameQueue{
    Frame fqueue[FRAME_QUEUE_SIZE];
    int ridx;
    int widx;
    int size;  // frame queue size
    int max_size;
    int abort;
    pthread_mutex_t* mtx;
    pthread_cond_t*  cond;
}FrameQueue;

int    picture_queue_init(FrameQueue* fq);
int    picture_queue_alloc_rgbframe(FrameQueue* fq, int width, int height);
void   picture_queue_destory(FrameQueue* fq);
Frame* picture_queue_get_write_picture(FrameQueue* fq);
void   picture_queue_write(FrameQueue* fq);
Frame* picture_queue_get_read_picture(FrameQueue* fq);
void   picture_queue_finish_read(FrameQueue* fq);
void   picture_queue_abort(FrameQueue* fq);

#endif