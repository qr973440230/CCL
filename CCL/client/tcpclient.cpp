#include "tcpclient.h"
#include <QDebug>
#include <QThread>
#include <QTimer>

TcpClient::TcpClient(AbstractQueue<TCPBuffer> *queue,
                     QObject *parent)
    :QTcpSocket(parent),
      m_host(QHostAddress::Null),
      m_port(0),
      m_queue(queue),
      m_interval(TCP_DEfAULT_RECONNECT_TIME),
      m_reconnecting(false),
      m_reconnectCount(0)

{
    init();
}

TcpClient::TcpClient(const QHostAddress &host,
             quint16 port,
             AbstractQueue<TCPBuffer> *queue,
             QObject *parent)
    :QTcpSocket(parent),
      m_host(host),
      m_port(port),
      m_queue(queue),
      m_interval(TCP_DEfAULT_RECONNECT_TIME),
      m_reconnecting(false),
      m_reconnectCount(0)
{
    init();
}

TcpClient::~TcpClient()
{

}

void TcpClient::writeBuffer(const TCPBuffer &buffer)
{
    emit writeBufferSignal(buffer);
}

void TcpClient::start()
{
    emit startSignal();
}

void TcpClient::stop()
{
    emit stopSignal();
}

void TcpClient::startSlot()
{
    if(state() == QAbstractSocket::ConnectedState){
        close();
    }
    if(m_timer->isActive()){
        m_timer->stop();
    }

    m_timer->start(m_interval);
    connectToHost(m_host,m_port);
}

void TcpClient::stopSlot()
{
    close();

    if(m_timer->isActive()){
        m_timer->stop();
    }
}

void TcpClient::writeBufferSlot(const TCPBuffer &buffer)
{
    qint64 len = 0;

    while(len < buffer.len){
        qint64 lenTmp = write(buffer.buffer + len,buffer.len - len);
        if(lenTmp < 0){
            qDebug()<<"Write buffer failure! Error:"<<errorString()<<
                      " Buffer: "<<QByteArray(buffer.buffer + len,static_cast<int>(buffer.len - len)).toHex() <<
                      " Addr: "<<peerAddress() <<
                      " Port: "<<peerPort();
            break;
        }
        len += lenTmp;
    }
}

void TcpClient::readyReadSlot()
{
    TCPBuffer *buffer = m_queue->peekWriteable();
    if(!buffer){
        qDebug()<<"Peek write buffer failure! Please check queue is abort!" <<
                  " Addr: " << m_host <<
                  " Port: " << m_port;
        return;
    }

    buffer->len = read(buffer->buffer,TCP_DEFAULT_BUF_SIZE);
    if(buffer->len < 0){
        qDebug()<<"Socket read failure! Error: "<< errorString() <<
                  " Addr: "<<m_host<<
                  " Port: "<<m_port;

        m_queue->next(buffer);
        return;
    }
    buffer->addr = m_host;
    buffer->port = m_port;

    m_queue->push(buffer);
}

void TcpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QTcpSocket::UnconnectedState:
        emit unconnected(m_host,m_port);
        break;
    case QTcpSocket::ConnectingState:
        emit connecting(m_host,m_port);
        break;
    case QTcpSocket::ConnectedState:
        emit connected(m_host,m_port);
        break;
    case QTcpSocket::ClosingState:
        emit closing(m_host,m_port);
        break;

    default:
        break;
    }
}

void TcpClient::timeoutSlot()
{
    if(state() == QTcpSocket::UnconnectedState){
        if(m_reconnecting){
            qDebug()<< "reconnect failure!" <<
                       " Host: "<< m_host<<
                       " Port: "<< m_port<<
                       " Reconnect Count: "<<
                       m_reconnectCount++;
        }else{
            m_reconnectCount = 1;
        }

        connectToHost(m_host,m_port);
        m_reconnecting = true;
    }else if(state() == QTcpSocket::ConnectingState){
        qDebug()<< "connecting......"<<
                   " Host: "<<m_host<<
                   " Port: "<<m_port;
    }else if(state() == QTcpSocket::ConnectedState){
        if(m_reconnecting){
            m_reconnecting = false;
            qDebug()<< "connect success!"<<
                       " Host: "<<m_host<<
                       " Port: "<<m_port;
        }
    }
}

void TcpClient::init()
{
    m_timer = new QTimer(this);

    // socket
    qRegisterMetaType<TCPBuffer>("TCPBuffer");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    connect(this,&QTcpSocket::stateChanged,this,&TcpClient::stateChangedSlot);
    connect(this,&QTcpSocket::readyRead,this,&TcpClient::readyReadSlot);

    // timer
    connect(m_timer,&QTimer::timeout,this,&TcpClient::timeoutSlot);

    // self
    connect(this,&TcpClient::startSignal,this,&TcpClient::startSlot);
    connect(this,&TcpClient::stopSignal,this,&TcpClient::stopSlot);

    // self write
    connect(this,&TcpClient::writeBufferSignal,this,&TcpClient::writeBufferSlot);
}

QHostAddress TcpClient::host() const
{
    return m_host;
}

void TcpClient::setHost(const QHostAddress &host)
{
    m_host = host;
}

quint16 TcpClient::port() const
{
    return m_port;
}

void TcpClient::setPort(const quint16 &port)
{
    m_port = port;
}

