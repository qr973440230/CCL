#include "tcpclient.h"
#include <QDebug>
#include <QThread>
#include <QTimer>

TcpClient::TcpClient(AbstractQueue<MessageBuffer> *queue,
                     QObject *parent)
    :QTcpSocket(parent),
      m_host(QHostAddress::Null),
      m_port(0),
      m_queue(queue),
      m_interval(DEfAULT_TCP_RECONNECT_TIME)
{
    init();
}

TcpClient::TcpClient(const QHostAddress &host,
             quint16 port,
             AbstractQueue<MessageBuffer> *queue,
             QObject *parent)
    :QTcpSocket(parent),
      m_host(host),
      m_port(port),
      m_queue(queue),
      m_interval(DEfAULT_TCP_RECONNECT_TIME)
{
    init();
}

TcpClient::~TcpClient()
{

}

void TcpClient::writeBuffer(const MessageBuffer &buffer)
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

void TcpClient::writeBufferSlot(const MessageBuffer &buffer)
{
    qint64 len = 0;

    while(len < buffer.len){
        qint64 lenTmp = write(buffer.buffer + len,buffer.len - len);
        if(lenTmp < 0){
            qWarning()<<"Write buffer failure! Error:"<<errorString()<<
                      " Buffer: "<<QByteArray(buffer.buffer + len,static_cast<int>(buffer.len - len)).length() <<
                      " Addr: "<<m_host<<
                      " Port: "<<m_port;
            break;
        }
        len += lenTmp;
    }
}

void TcpClient::readyReadSlot()
{
    MessageBuffer *buffer = m_queue->peekWriteable();
    if(!buffer){
        return;
    }

    buffer->len = read(buffer->buffer,DEFAULT_MESSAGE_BUF_SIZE);
    if(buffer->len < 0){
        m_queue->next(buffer);
        return;
    }

    buffer->addr = m_host;
    buffer->port = m_port;
    m_queue->push(buffer);

    emit tcpBufferQueueUdpateSignal();
}

void TcpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QTcpSocket::UnconnectedState:
        emit disconnectedSignal(m_host,m_port);
        break;
    case QTcpSocket::ConnectingState:
        emit connectingSignal(m_host,m_port);
        break;
    case QTcpSocket::ConnectedState:
        emit connectedSignal(m_host,m_port);
        break;
    case QTcpSocket::ClosingState:
        emit closingSignal(m_host,m_port);
        break;

    default:
        break;
    }
}

void TcpClient::timeoutSlot()
{
    if(state() == QTcpSocket::UnconnectedState){
        connectToHost(m_host,m_port);
    }
}

void TcpClient::init()
{
    m_timer = new QTimer(this);

    // socket
    qRegisterMetaType<MessageBuffer>("TCPBuffer");
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

