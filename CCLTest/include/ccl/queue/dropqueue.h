#ifndef DROPQUEUE_H
#define DROPQUEUE_H

#include "abstractqueue.h"
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#define DROP_DEFAULT_QUEUE_MAX_SIZE 20
#define DROP_DEFAULT_TIME_OUT 500

/**
 * if buffer overflow, will drop oldest data.
 */
template <typename T>
class DropQueue: public AbstractQueue<T>{
    typedef typename AbstractQueue<T>::QueueNode Node;

public:
    explicit DropQueue();
    explicit DropQueue(unsigned int maxSize,unsigned long dropTimeout);
    ~DropQueue();

    virtual T * peekReadable(unsigned long timeout) override;
    virtual void next(T * data) override;
    virtual QList<T*> peekAllReadable(unsigned long timeout) override;
    virtual void nextAll(const QList<T*> &list) override;

    virtual T * peekWriteable() override;
    virtual void push(T * data) override;
    virtual QList<T*> peekAllWriteable() override;
    virtual void pushAll(const QList<T *> &list) override;

    virtual void abort() override;
    virtual bool isAbort() override;

private:
    Node * m_wIdx;
    Node * m_rIdx;

    unsigned int m_maxSize;
    unsigned long m_dropTimeout;
    bool m_abort;


    QMap<T*,Node *> m_map;
    QMutex m_mutex;
    QWaitCondition m_cond;
};

template<typename T>
DropQueue<T>::DropQueue()
    :m_wIdx(nullptr),
      m_rIdx(nullptr),
      m_maxSize(DROP_DEFAULT_QUEUE_MAX_SIZE),
      m_dropTimeout(DROP_DEFAULT_TIME_OUT),
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
DropQueue<T>::DropQueue(unsigned int maxSize, unsigned long dropTimeout)
    :m_wIdx(nullptr),
      m_rIdx(nullptr),
      m_maxSize(maxSize),
      m_dropTimeout(dropTimeout),
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
DropQueue<T>::~DropQueue()
{
    while(m_rIdx->next != m_wIdx){
        Node *readNode = m_rIdx->next;
        readNode->pre->next = readNode->next;
        readNode->next->pre = readNode->pre;
        readNode->next = nullptr;
        readNode->pre = nullptr;
        delete  readNode;
    }

    while(m_wIdx->next != m_rIdx){
        Node * writeNode = m_wIdx->next;
        writeNode->pre->next = writeNode->next;
        writeNode->next->pre = writeNode->pre;
        writeNode->pre = nullptr;
        writeNode->next = nullptr;
        delete  writeNode;
    }

    delete m_rIdx;
    delete m_wIdx;
}

template<typename T>
T *DropQueue<T>::peekReadable(unsigned long timeout)
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
void DropQueue<T>::next(T *data)
{
    if(!m_map.contains(data)){
        return;
    }

    m_mutex.lock();

    // insert read node
    Node *readNode = m_map[data];
    readNode->pre = m_rIdx->pre;
    readNode->next = m_rIdx;
    readNode->pre->next = readNode;
    readNode->next->pre = readNode;
    readNode = nullptr;

    m_cond.wakeOne();
    m_mutex.unlock();
}

template<typename T>
QList<T *> DropQueue<T>::peekAllReadable(unsigned long timeout)
{
    QList<T*> list;
    QMutexLocker locker(&m_mutex);
    while(m_rIdx->next == m_wIdx && !m_abort){
        if(!m_cond.wait(&m_mutex,timeout)){
            // timeout
            return list;
        }
    }

    if(m_abort){
        return list;
    }

    while(m_rIdx->next != m_wIdx){
        // detach read node
        Node *readNode = m_rIdx->next;
        readNode->pre->next = readNode->next;
        readNode->next->pre = readNode->pre;
        readNode->next = nullptr;
        readNode->pre = nullptr;
        list.append(&readNode->data);
    }
    return list;
}

template<typename T>
void DropQueue<T>::nextAll(const QList<T *> &list)
{
    m_mutex.lock();

    for(auto data : list){
        if(!m_map.contains(data)){
            continue;
        }

        // insert read node
        Node *readNode = m_map[data];
        readNode->pre = m_rIdx->pre;
        readNode->next = m_rIdx;
        readNode->pre->next = readNode;
        readNode->next->pre = readNode;
        readNode = nullptr;
    }

    m_cond.wakeOne();
    m_mutex.unlock();

}

template<typename T>
T *DropQueue<T>::peekWriteable()
{
    QMutexLocker locker(&m_mutex);
    while(m_wIdx->next == m_rIdx && !m_abort){
        if(!m_cond.wait(&m_mutex,m_dropTimeout)){
            // timeout
            m_rIdx = m_rIdx->next;
        }
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
void DropQueue<T>::push(T *data)
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
QList<T *> DropQueue<T>::peekAllWriteable()
{
    QList<T*> list;
    QMutexLocker locker(&m_mutex);
    while(m_wIdx->next == m_rIdx && !m_abort){
        if(!m_cond.wait(&m_mutex,m_dropTimeout)){
            // timeout
            m_rIdx = m_rIdx->next;
        }
    }

    if(m_abort){
        return list;
    }

    while(m_wIdx->next != m_rIdx){
        // detach write node
        Node * writeNode = m_wIdx->next;
        writeNode->pre->next = writeNode->next;
        writeNode->next->pre = writeNode->pre;
        writeNode->pre = nullptr;
        writeNode->next = nullptr;
        list.append(&writeNode->data);
    }

    return list;
}

template<typename T>
void DropQueue<T>::pushAll(const QList<T*> &list)
{
    m_mutex.lock();

    for(auto data : list){
        // insert write node
        Node * writeNode = m_map[data];
        writeNode->pre = m_wIdx->pre;
        writeNode->next = m_wIdx;
        writeNode->pre->next = writeNode;
        writeNode->next->pre = writeNode;
        writeNode = nullptr;
    }

    m_cond.wakeOne();
    m_mutex.unlock();

}

template<typename T>
void DropQueue<T>::abort()
{
    m_mutex.lock();
    m_abort = true;
    m_cond.wakeAll();
    m_mutex.unlock();
}

template<typename T>
bool DropQueue<T>::isAbort()
{
    QMutexLocker locker(&m_mutex);
    return m_abort;
}


#endif // DROPQUEUE_H
