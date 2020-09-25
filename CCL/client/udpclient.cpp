#include "udpclient.h"

UdpClient::UdpClient(AbstractQueue<MessageBuffer> *queue, QObject *parent)
    :QUdpSocket(parent),
      m_host(QHostAddress::LocalHost),
      m_port(0),
      m_queue(queue)
{
    init();
}

UdpClient::UdpClient(quint16 port,
             AbstractQueue<MessageBuffer> *queue,
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
             AbstractQueue<MessageBuffer> *queue,
             QObject *parent)
    :QUdpSocket(parent),
      m_host(host),
      m_port(port),
      m_queue(queue)
{
    init();
}

void UdpClient::writeBuffer(const MessageBuffer &buffer)
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
    bool success = bind(m_host,m_port);
    if(!success){
        qWarning()<<"Binding failure!"
                    " Host: "<<m_host<<
                    " Port: "<<m_port<<
                    " Error: "<<errorString();
    }
}

void UdpClient::stopSlot()
{
    if(state() == QAbstractSocket::BoundState){
        close();
    }
}

void UdpClient::writeBufferSlot(const MessageBuffer &buffer)
{
    qint64 len = writeDatagram(buffer.buffer,buffer.len,
                         buffer.addr,buffer.port);
    if(len < 0){
        qWarning()<<"Write buffer failure! buffer: "<<
              QByteArray(buffer.buffer + len,
                     static_cast<int>(buffer.len - len)).toHex()<<
              " Host: "<<buffer.addr<<
              " Port: "<<buffer.port;
    }
}

void UdpClient::readyReadSlot()
{
    if(hasPendingDatagrams()){
        MessageBuffer * buffer = m_queue->peekWriteable();
        if(!buffer){
            return;
        }

        buffer->len = readDatagram(buffer->buffer,DEFAULT_MESSAGE_BUF_SIZE,
                                   &buffer->addr,&buffer->port);
        if(buffer->len < 0){
            m_queue->next(buffer);
            return;
        }
        m_queue->push(buffer);

        emit udpBufferQueueUpdateSignal();
    }
}

void UdpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    qInfo()<<"UDPClient state changed! Current state: " << state<<
              " Host: "<<m_host <<
              " Port: "<<m_port;
}

void UdpClient::init()
{
    qRegisterMetaType<MessageBuffer>("UDPBuffer");
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
