#include "tcpclientthread.h"
#include <QDebug>

TcpClientThread::TcpClientThread(const QHostAddress &host,
                                 quint16 port,
                                 AbstractQueue<MessageBuffer> *readQueue, AbstractQueue<MessageBuffer> *writeQueue,
                                 QObject *parent)
    :QThread(parent),
      m_host(host),
      m_port(port),
      m_readQueue(readQueue),
      m_writeQueue(writeQueue)
{

}

void TcpClientThread::run()
{
    m_socket = new QTcpSocket();

    while(!isInterruptionRequested()){
        if(!connectToHost()){
            continue;
        }

        readMessage();

        writeMessage();
    }

    delete m_socket;
    qInfo()<<"Tcp client thread exit success!"<<
             " Host: "<<m_host<<
             " Port: "<<m_port;
}

bool TcpClientThread::connectToHost()
{
    if(m_socket->state() == QTcpSocket::ConnectedState){
        if(m_socketLastState != QTcpSocket::ConnectedState){
            qInfo()<<"Tcp client connect to host success."<<
                     " Host: "<<m_host<<
                     " Port: "<<m_port;
            m_socketLastState = QTcpSocket::ConnectedState;

            emit connectedHostSignal(m_host,m_port);
        }

        return true;
    }
    else if(m_socket->state() == QTcpSocket::ConnectingState){
        if(m_socketLastState != QTcpSocket::ConnectingState){
            qInfo()<<"Tcp client conecting to host......"<<
                     " Host: "<<m_host<<
                     " Port: "<<m_port;
            m_socketLastState = QTcpSocket::ConnectingState;

            emit connectingHostSignal(m_host,m_port);
        }
        m_socket->waitForConnected(100);

        return false;
    }
    else{
        if(m_socket->state() != m_socketLastState){
            qInfo()<<"Tcp client disconnected."<<
                     " Host: "<<m_host<<
                     " Port: "<<m_port<<
                     " Error: "<<m_socket->errorString();
            m_socketLastState = m_socket->state();

            emit disconnectedHostSignal(m_host,m_port,m_socket->errorString());
        }
        m_socket->connectToHost(m_host,m_port);
        m_socket->waitForConnected(100);

        return false;
    }
}

void TcpClientThread::readMessage()
{
    if(!m_socket->waitForReadyRead(100)){
        return;
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

void TcpClientThread::writeMessage()
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
