#include "tcpclient.h"
#include <QDebug>
#include <QThread>
#include <QTimer>

TcpClient::TcpClient(AbstractQueue<TCPBuffer> *queue,
                     QObject *parent)
    :QTcpSocket(parent),
      m_queue(queue),
      m_interval(TCP_DEfAULT_RECONNECT_TIME)
{
    init();
}

TcpClient::TcpClient(const QString &host,
             quint16 port,
             AbstractQueue<TCPBuffer> *queue,
             QObject *parent)
    :QTcpSocket(parent),
      m_host(host),
      m_port(port),
      m_queue(queue),
      m_interval(TCP_DEfAULT_RECONNECT_TIME)
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
    m_timer->start(m_interval);
    connectToHost(m_host,m_port);
}

void TcpClient::stopSlot()
{
    m_timer->stop();
}

void TcpClient::writeBufferSlot(const TCPBuffer &buffer)
{
    qint64 len = 0;

    while(len < buffer.len){
        qint64 lenTmp = write(buffer.buffer + len,buffer.len - len);
        if(lenTmp < 0){
            qDebug()<<"Write buffer failure! Error:"<<errorString()<<
                  " buffer: "<<QByteArray(buffer.buffer + len,
                              static_cast<int>(buffer.len - len)).toHex();
            break;
        }
        len += lenTmp;
    }
}

void TcpClient::readyReadSlot()
{
    TCPBuffer *buffer = m_queue->peekWriteable();
    if(!buffer){
        qDebug()<<"Peek write buffer failure! Please check queue is abort!";
        return;
    }

    buffer->len = read(buffer->buffer,TCP_DEFAULT_BUF_SIZE);
    if(buffer->len < 0){
        qDebug()<<"Socket read failure! Error: "<< errorString() <<
                  " Addr: "<<peerAddress()<<
                  " Port: "<<peerPort();

        m_queue->next(buffer);
        return;
    }
    buffer->addr = peerAddress();
    buffer->port = peerPort();

    m_queue->push(buffer);
}

void TcpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    qDebug()<<"TcpClient state changed! Current state: " << state <<
              " Addr: " << peerAddress() <<
              " Port: " << peerPort();

    switch (state) {
    case QTcpSocket::UnconnectedState:
        emit unconnected();
        break;
    case QTcpSocket::ConnectingState:
        emit connecting();
        break;
    case QTcpSocket::ConnectedState:
        emit connected();
        break;
    case QTcpSocket::ClosingState:
        emit closing();
        break;

    default:
        break;
    }
}

void TcpClient::errorSlot(QAbstractSocket::SocketError socketError)
{
    qDebug()<<"Tcp Socket Error: "<< socketError<<
              " Addr: "<<peerAddress() <<
              " Port: "<<peerPort();

    emit error(socketError);
}

void TcpClient::timeoutSlot()
{
    if(state() == QTcpSocket::UnconnectedState){
        connectToHost(m_host,m_port);
        if(!waitForConnected(m_interval/2)){
            qDebug()<<"Reconnect server failure!"<<
                  " Host: "<<m_host<<
                  " Port: "<<m_port;
        }else{
            qDebug()<<"Reconnect server success!"<<
                  " Host: "<<m_host<<
                  " Port: "<<m_port;
        }
    }
}

void TcpClient::init()
{
    m_timer = new QTimer(this);

    // socket
    qRegisterMetaType<QTcpSocket::SocketState>("QTcpSocket::SocketState");
    connect(this,&QTcpSocket::stateChanged,this,&TcpClient::stateChangedSlot);
    connect(this,&QTcpSocket::readyRead,this,&TcpClient::readyReadSlot);
    connect(this,QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
        this,&TcpClient::errorSlot);

    // timer
    connect(m_timer,&QTimer::timeout,this,&TcpClient::timeoutSlot);

    // self
    connect(this,&TcpClient::startSignal,this,&TcpClient::startSlot);
    connect(this,&TcpClient::stopSignal,this,&TcpClient::stopSlot);

    // self write
    connect(this,&TcpClient::writeBufferSignal,this,&TcpClient::writeBufferSlot);

}

quint16 TcpClient::port() const
{
    return m_port;
}

void TcpClient::setPort(const quint16 &port)
{
    m_port = port;
}

QString TcpClient::host() const
{
    return m_host;
}

void TcpClient::setHost(const QString &host)
{
    m_host = host;
}
