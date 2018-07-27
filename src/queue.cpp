#include "StdAfx.h"
#include "queue.h"
#include<stdio.h>

void packet_queue_init(PacketQueue* pq)
{
    pq->size    = 0;
    pq->maxsize = 30;
    pq->abort   = 0;
    pq->firstNode = pq->lastNode = NULL;
    pq->mtx  = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pq->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(pq->mtx, NULL);
    pthread_cond_init(pq->cond, NULL);
}

void packet_queue_destory(PacketQueue* pq)
{
    PacketListNode* nd = pq->firstNode;
    while(nd)
    {
        pq->firstNode = pq->firstNode->next; // move to next
        free(nd);
        nd = pq->firstNode;
    }
    pthread_mutex_destroy(pq->mtx);
    pthread_cond_destroy(pq->cond);
    free(pq->mtx);
    free(pq->cond);
}

void packet_queue_put(PacketQueue* pq, AVPacket* pkt)
{
    pthread_mutex_lock(pq->mtx);
    if(pq->firstNode == NULL) // queue empty
    {
        pq->firstNode = (PacketListNode *)malloc(sizeof(PacketListNode));
        pq->firstNode->pkt  = *pkt;
        pq->firstNode->next = NULL;
        pq->lastNode = pq->firstNode;
        pq->size++;
    }
    else
    {
        while(pq->size >= pq->maxsize && !pq->abort) // packet queue full
            pthread_cond_wait(pq->cond, pq->mtx);

        PacketListNode* nnode = (PacketListNode *)malloc(sizeof(PacketListNode));
        nnode->pkt  = *pkt;
        nnode->next = NULL;
        pq->lastNode->next = nnode;
        pq->lastNode = pq->lastNode->next;
        pq->size++;
    }
    pthread_cond_signal(pq->cond);
    pthread_mutex_unlock(pq->mtx);
}

void packet_queue_get(PacketQueue* pq, AVPacket *rpkt)
{
    PacketListNode* hdNode;
    pthread_mutex_lock(pq->mtx);
    while(!pq->abort && (pq->size == 0 || pq->firstNode == NULL)) // packet queue empty, wait
        pthread_cond_wait(pq->cond, pq->mtx);
    hdNode = pq->firstNode;
    *rpkt = pq->firstNode->pkt;
    pq->firstNode = pq->firstNode->next;
    free(hdNode);
    pq->size--;
    pthread_cond_signal(pq->cond); // inform 
    pthread_mutex_unlock(pq->mtx);
}

void packet_queue_abort(PacketQueue* pq)
{
    pthread_mutex_lock(pq->mtx);
    pq->abort = 1;
    pthread_cond_signal(pq->cond);
    pthread_mutex_unlock(pq->mtx);
}

int picture_queue_init(FrameQueue* fq)
{
    fq->size = fq->ridx = fq->widx = 0;
    fq->abort = 0;
    fq->max_size = FRAME_QUEUE_SIZE;
    fq->mtx  = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    fq->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    for(int i = 0; i < fq->max_size; i++)
    {
        fq->fqueue[i].yuvframe = av_frame_alloc();
        fq->fqueue[i].rgbframe = NULL;
        if(fq->fqueue[i].yuvframe == NULL)
            return AVERROR(ENOMEM);
    }
    pthread_mutex_init(fq->mtx, NULL);
    pthread_cond_init(fq->cond, NULL);
    return 0;
}

int picture_queue_alloc_rgbframe(FrameQueue* fq, int width, int height)
{
    int ret;
    for(int i = 0; i < fq->max_size; i++)
    {
        av_frame_free(&(fq->fqueue[i].rgbframe)); // first free, then alloc

        fq->fqueue[i].rgbframe = av_frame_alloc();
        if(fq->fqueue[i].rgbframe == NULL)
            return AVERROR(ENOMEM);
        fq->fqueue[i].rgbframe->format = AV_PIX_FMT_BGR24;
        fq->fqueue[i].rgbframe->width  = width;
        fq->fqueue[i].rgbframe->height = height;

        ret = av_frame_get_buffer(fq->fqueue[i].rgbframe, 32);
        if(ret < 0)
            return -1;
    }
    return 0;
}

void picture_queue_destory(FrameQueue* fq)
{
    for(int i = 0; i < fq->max_size; i++)
    {
        Frame* vp = &fq->fqueue[i];
        av_frame_free(&vp->yuvframe);
        av_frame_free(&vp->rgbframe);
    }
    pthread_mutex_destroy(fq->mtx);
    pthread_cond_destroy(fq->cond);
    free(fq->mtx);
    free(fq->cond);
}

Frame* picture_queue_get_write_picture(FrameQueue* fq)
{
    pthread_mutex_lock(fq->mtx);
    while(fq->size >= FRAME_QUEUE_SIZE && !fq->abort)  // frame queue full
        pthread_cond_wait(fq->cond, fq->mtx);
    pthread_mutex_unlock(fq->mtx);
    if(fq->abort)
        return NULL;
    return &(fq->fqueue[fq->widx]);
}

void picture_queue_write(FrameQueue* fq)
{
    pthread_mutex_lock(fq->mtx);
    fq->widx = ((fq->widx + 1) == FRAME_QUEUE_SIZE)? 0: fq->widx + 1; // update write index
    fq->size++;
    pthread_cond_signal(fq->cond);
    pthread_mutex_unlock(fq->mtx);
}

Frame* picture_queue_read(FrameQueue* fq)
{
    Frame* rf;
    pthread_mutex_lock(fq->mtx);
    while(fq->size == 0 && !fq->abort)
        pthread_cond_wait(fq->cond, fq->mtx);  // frame queue empty
    rf = &(fq->fqueue[fq->ridx]);
    fq->size--;
    fq->ridx = ((fq->ridx + 1) == FRAME_QUEUE_SIZE)? 0: fq->ridx + 1; //
    pthread_cond_signal(fq->cond);
    pthread_mutex_unlock(fq->mtx);
    return rf;
}

void picture_queue_abort(FrameQueue* fq)
{
    pthread_mutex_lock(fq->mtx);
    fq->abort = 1;
    pthread_cond_signal(fq->cond);
    pthread_mutex_unlock(fq->mtx);
}