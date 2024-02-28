/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   XQThread.h
 * Author: xiafeng
 *
 * Created on January 1, 2022, 12:11 PM
 */
#include "pthread.h"
#ifndef XQTHREAD_H
#define XQTHREAD_H

class XQThread
{

public:
    XQThread()
    {
        tid_ = 0;
        m_bRun = false;
    }
    virtual ~XQThread()
    {
        stop();
    }
    
    void start()
    {
        if (m_bRun)
            return;
        m_bRun = true;
        pthread_create(&tid_, NULL, run_, this);
        // todo
    }

    void stop()
    {
        if (m_bRun)
        {
            m_bRun = false;
            join();
        }
    }

    void join()
    {

        if (tid_)
        {
            pthread_join(tid_, NULL);
            tid_ = 0;
        }
    }
    virtual void run() = 0;

protected:
    static void *run_(void *param)
    {
        XQThread *thiz = (XQThread *)param;
        thiz->run();
        return NULL;
    }
public:
    volatile bool m_bRun;
protected:
    pthread_t tid_;

};

#endif /* XQTHREAD_H */
