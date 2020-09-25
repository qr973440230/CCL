#include "udpclientthread.h"
#include <QDebug>

UdpClientThread::UdpClientThread(const QHostAddress &host, quint16 port,
                                 AbstractQueue<MessageBuffer> *readQueue,
                                 AbstractQueue<MessageBuffer> *writeQueue,
                                 QObject *parent)
    :QThread(parent),
      m_host(host),
      m_port(port),
      m_readQueue(readQueue),
      m_writeQueue(writeQueue)
{

}

void UdpClientThread::run()
{
    m_socket = new QUdpSocket();

    while(!isInterruptionRequested()){
        if(!bind()){
            continue;
        }

        readMessage();
        writeMessage();
    }

    delete m_socket;

    qWarning()<<"Udp client exit success."<<
                " Host: "<<m_host<<
                " Port: "<<m_port;
}

bool UdpClientThread::bind()
{
    if(m_socket->state() == QUdpSocket::BoundState){
        if(m_socketLastState != QUdpSocket::BoundState){
            qInfo()<<"Udp client bind success."<<
                     " Host: "<<m_host<<
                     " Port: "<<m_port;

            m_socketLastState = QUdpSocket::BoundState;
            emit bindSuccess(m_host,m_port);
        }

        return true;
    }
    else{
        if(m_socket->state() != m_socketLastState){
            qWarning()<<"Udp client bind error."<<
                        " Host: "<<m_host<<
                        " Port: "<<m_port<<
                        " Error: "<<m_socket->errorString();

            m_socketLastState = m_socket->state();
            emit bindError(m_host,m_port,m_socket->errorString());
        }

        m_socket->bind(m_host,m_port);
        return false;
    }
}

void UdpClientThread::readMessage()
{
    if(!m_socket->hasPendingDatagrams()){
        if(!m_socket->waitForReadyRead(100)){
            return;
        }
    }

    MessageBuffer *buffer = m_readQueue->peekWriteable();
    if(!buffer){
        return;
    }

    buffer->len = m_socket->read(buffer->buffer,DEFAULT_MESSAGE_BUF_SIZE);
    if(buffer->len < 0){
        qWarning()<<"Tcp client read error!"<<
                    " Host: "<<m_host<<
                    " Port: "<<m_port<<
                    " Error: "<<m_socket->errorString();

        m_readQueue->next(buffer);
        return;
    }

    if(buffer->len == 0){
        m_readQueue->next(buffer);
        return;
    }

    buffer->addr = m_host;
    buffer->port = m_port;
    m_readQueue->push(buffer);
}

void UdpClientThread::writeMessage()
{
    QList<MessageBuffer *> list = m_writeQueue->peekAllReadable(100);
    for(auto buffer : list){
        qint64 index = 0;
        while(index < buffer->len){
            qint64 len = m_socket->write(buffer->buffer + index,buffer->len - index);
            if(len < 0){
                qWarning()<<"Tcp client write error!"<<
                            " Host: "<<m_host<<
                            " Port: "<<m_port<<
                            " Error: "<<m_socket->errorString();

                m_writeQueue->nextAll(list);
                return;
            }

            index += len;
        }
    }

    m_writeQueue->nextAll(list);
}
