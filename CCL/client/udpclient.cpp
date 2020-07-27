#include "udpclient.h"

UdpClient::UdpClient(AbstractQueue<UDPBuffer> *queue, QObject *parent)
    :QUdpSocket(parent),
      m_host(QHostAddress::LocalHost),
      m_port(0),
      m_queue(queue)
{
    init();
}

UdpClient::UdpClient(quint16 port,
             AbstractQueue<UDPBuffer> *queue,
             QObject *parent)
    :QUdpSocket(parent),
      m_host(QHostAddress::LocalHost),
      m_port(port),
      m_queue(queue)
{
    init();
}

UdpClient::UdpClient(const QHostAddress &host,
             quint16 port,
             AbstractQueue<UDPBuffer> *queue,
             QObject *parent)
    :QUdpSocket(parent),
      m_host(host),
      m_port(port),
      m_queue(queue)
{
    init();
}

void UdpClient::writeBuffer(const UDPBuffer &buffer)
{
    emit writeBufferSignal(buffer);
}

void UdpClient::start()
{
    emit startSignal();
}

void UdpClient::stop()
{
    emit stopSignal();
}

void UdpClient::startSlot()
{
    if(state() == QAbstractSocket::BoundState){
        close();
    }
    bind(m_host,m_port);
}

void UdpClient::stopSlot()
{
    if(state() == QAbstractSocket::BoundState){
        close();
    }
}

void UdpClient::writeBufferSlot(const UDPBuffer &buffer)
{
    qint64 len = writeDatagram(buffer.buffer,buffer.len,
                         buffer.addres,buffer.port);
    if(len < 0){
        qDebug()<<"Write buffer failure! buffer: "<<
              QByteArray(buffer.buffer + len,
                     static_cast<int>(buffer.len - len)).toHex()<<
              " Host: "<<buffer.addres<<
              " Port: "<<buffer.port;
    }
}

void UdpClient::readyReadSlot()
{
    if(hasPendingDatagrams()){
        UDPBuffer * buffer = m_queue->peekWriteable();
        if(!buffer){
            qDebug()<<"Peek write buffer failure! Please check queue is abort!";
            return;
        }
        buffer->len = readDatagram(buffer->buffer,UDP_DEFAULT_BUF_SIZE,&buffer->addres,&buffer->port);
        if(buffer->len < 0){
            qDebug()<<"Socket read failure! Error: "<< errorString()<<
                      " Host: "<<m_host <<
                      " Port: "<<m_port;
            m_queue->next(buffer);
            return;
        }
        m_queue->push(buffer);
    }
}

void UdpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    qDebug()<<"UDPClient state changed! Current state: " << state<<
              " Host: "<<m_host <<
              " Port: "<<m_port;
}

void UdpClient::init()
{
    qRegisterMetaType<UDPBuffer>("UDPBuffer");
    connect(this,&UdpClient::startSignal,this,&UdpClient::startSlot);
    connect(this,&UdpClient::stopSignal,this,&UdpClient::stopSlot);

    connect(this,&UdpClient::writeBufferSignal,this,&UdpClient::writeBufferSlot);

    connect(this,&QUdpSocket::readyRead,this,&UdpClient::readyReadSlot);
    connect(this,&QUdpSocket::stateChanged,this,&UdpClient::stateChangedSlot);
}

QHostAddress UdpClient::host() const
{
    return m_host;
}

void UdpClient::setHost(const QHostAddress &host)
{
    m_host = host;
}

quint16 UdpClient::port() const
{
    return m_port;
}

void UdpClient::setPort(const quint16 &port)
{
    m_port = port;
}
