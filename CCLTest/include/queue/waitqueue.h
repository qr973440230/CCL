#ifndef WAITQUEUE_H
#define WAITQUEUE_H

#include "abstractqueue.h"
#include <QMap>
#include <QMutex>
#include <QWaitCondition>

#define WAIT_DEFAULT_QUEUE_MAX_SIZE 20

/**
 * if buffer overflow, will wait.
 */
template <typename T>
class WaitQueue: public AbstractQueue<T>{
    typedef typename AbstractQueue<T>::QueueNode Node;

public:
    explicit WaitQueue();
    explicit WaitQueue(unsigned long maxSize);
    ~WaitQueue();

    virtual T * peekReadable(unsigned long timeout) override;
    virtual void next(T * data) override;

    virtual T * peekWriteable() override;
    virtual void push(T * data) override;

    virtual void abort() override;
    virtual bool isAbort() override;

private:
    Node * m_wIdx;
    Node * m_rIdx;

    unsigned int m_maxSize;
    bool m_abort;

    QMap<T*,Node*> m_map;
    QMutex m_mutex;
    QWaitCondition m_cond;
};


template<typename T>
WaitQueue<T>::WaitQueue()
    :m_wIdx(nullptr),
      m_rIdx(nullptr),
      m_maxSize(WAIT_DEFAULT_QUEUE_MAX_SIZE),
      m_abort(false)
{
    m_wIdx = new Node();
    m_rIdx = new Node();
    m_wIdx->pre = m_rIdx;
    m_wIdx->next = m_rIdx;
    m_rIdx->next = m_wIdx;
    m_rIdx->pre = m_wIdx;

    m_map.insert(&m_wIdx->data,m_wIdx);
    m_map.insert(&m_rIdx->data,m_rIdx);

    Node * node = nullptr;
    for(unsigned int i = 0;i< m_maxSize;i++){
        node = new Node();
        node->pre = m_wIdx;
        node->next = m_wIdx->next;
        node->pre->next = node;
        node->next->pre = node;

        m_map.insert(&node->data,node);
    }
}

template<typename T>
WaitQueue<T>::WaitQueue(unsigned long maxSize)
    :m_wIdx(nullptr),
      m_rIdx(nullptr),
      m_maxSize(maxSize),
      m_abort(false)
{
    m_wIdx = new Node();
    m_rIdx = new Node();
    m_wIdx->pre = m_rIdx;
    m_wIdx->next = m_rIdx;
    m_rIdx->next = m_wIdx;
    m_rIdx->pre = m_wIdx;

    m_map.insert(&m_wIdx->data,m_wIdx);
    m_map.insert(&m_rIdx->data,m_rIdx);

    Node * node = nullptr;
    for(unsigned int i = 0;i< m_maxSize;i++){
        node = new Node();
        node->pre = m_wIdx;
        node->next = m_wIdx->next;
        node->pre->next = node;
        node->next->pre = node;

        m_map.insert(&node->data,node);
    }
}

template<typename T>
WaitQueue<T>::~WaitQueue()
{

}

template<typename T>
T *WaitQueue<T>::peekReadable(unsigned long timeout)
{
    QMutexLocker locker(&m_mutex);
    while(m_rIdx->next == m_wIdx && !m_abort){
        if(!m_cond.wait(&m_mutex,timeout)){
            // timeout
            return nullptr;
        }
    }

    if(m_abort){
        return nullptr;
    }

    // detach read node
    Node *readNode = m_rIdx->next;
    readNode->pre->next = readNode->next;
    readNode->next->pre = readNode->pre;
    readNode->next = nullptr;
    readNode->pre = nullptr;

    return &readNode->data;
}

template<typename T>
void WaitQueue<T>::next(T *data)
{
    if(!m_map.contains(data)){
        return;
    }

    m_mutex.lock();

    // insert read node
    Node *readNode = m_map.value(data);
    readNode->pre = m_rIdx->pre;
    readNode->next = m_rIdx;
    readNode->pre->next = readNode;
    readNode->next->pre = readNode;
    readNode = nullptr;

    m_cond.wakeOne();
    m_mutex.unlock();
}

template<typename T>
T *WaitQueue<T>::peekWriteable()
{
    QMutexLocker locker(&m_mutex);
    while(m_wIdx->next == m_rIdx && !m_abort){
        m_cond.wait(&m_mutex);
    }

    if(m_abort){
        return nullptr;
    }

    // detach write node
    Node * writeNode = m_wIdx->next;
    writeNode->pre->next = writeNode->next;
    writeNode->next->pre = writeNode->pre;
    writeNode->pre = nullptr;
    writeNode->next = nullptr;

    return &writeNode->data;
}

template<typename T>
void WaitQueue<T>::push(T *data)
{
    if(!m_map.contains(data)){
        return;
    }

    m_mutex.lock();

    // insert write node
    Node * writeNode = m_map[data];
    writeNode->pre = m_wIdx->pre;
    writeNode->next = m_wIdx;
    writeNode->pre->next = writeNode;
    writeNode->next->pre = writeNode;
    writeNode = nullptr;

    m_cond.wakeOne();
    m_mutex.unlock();
}

template<typename T>
void WaitQueue<T>::abort()
{
    m_mutex.lock();
    m_abort = true;
    m_cond.wakeAll();
    m_mutex.unlock();
}

template<typename T>
bool WaitQueue<T>::isAbort()
{
    QMutexLocker locker(&m_mutex);
    return m_abort;
}

#endif // WAITQUEUE_H
