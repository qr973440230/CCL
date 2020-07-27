#include "tcpserver.h"

#include <QThread>

TcpServer::TcpServer(AbstractQueue<TCPBuffer> *queue, QObject *parent)
    :QTcpServer(parent),
      m_queue(queue)
{
    init();
}

TcpServer::TcpServer(const QHostAddress &host, quint16 port,
                     AbstractQueue<TCPBuffer> *queue, QObject *parent)
    :QTcpServer(parent),
      m_host(host),
      m_port(port),
      m_queue(queue)
{
    init();
}

TcpServer::~TcpServer()
{
    m_thread->terminate();
    m_thread->wait();
}

void TcpServer::start()
{
    emit startSignal();
}

void TcpServer::stop()
{
    emit stopSignal();
}

void TcpServer::writeBuffer(const TCPBuffer &buffer)
{
    emit writeBufferSignal(buffer);
}

void TcpServer::startSlot()
{
    if(isListening()){
        // close and delete socket
        closeSockets();
        close();
    }

    if(!listen(QHostAddress(m_host),m_port)){
        emit listenError(errorString());
    }
}

void TcpServer::stopSlot()
{
    if(isListening()){
        closeSockets();
        close();
    }
}

void TcpServer::writeBufferSlot(const TCPBuffer &buffer)
{
    if(buffer.addr.isNull()){
        // send all
        for(auto socket : m_clients){
            socket->write(buffer.buffer,buffer.len);
        }
    }else if(buffer.port == 0){
        // send point at addr
        for(auto socket : m_clients){
            if(socket->peerAddress() == buffer.addr){
                socket->write(buffer.buffer,buffer.len);
            }
        }
    }else{
        // send point at addr and port
        for(auto socket : m_clients){
            if(socket->peerAddress() == buffer.addr &&
                    socket->peerPort() == buffer.port){
                socket->write(buffer.buffer,buffer.len);
            }
        }
    }
}

void TcpServer::acceptErrorSlot(QAbstractSocket::SocketError socketError)
{
    qDebug()<<"Accept Error: "<< socketError;
}

void TcpServer::incomingConnection(qintptr handle)
{
    TcpClient * client = new TcpClient(m_queue);
    client->setSocketDescriptor(handle);
    client->moveToThread(m_thread);

    m_clients.append(client);
}

void TcpServer::init()
{
    m_thread = new QThread();
    m_thread->start();

    connect(this,&TcpServer::startSignal,this,&TcpServer::startSlot);
    connect(this,&TcpServer::stopSignal,this,&TcpServer::stopSlot);
    connect(this,&TcpServer::writeBufferSignal,this,&TcpServer::writeBufferSlot);
    connect(this,&QTcpServer::acceptError,this,&TcpServer::acceptErrorSlot);
}

void TcpServer::closeSockets()
{
    for(auto socket : m_clients) {
        socket->stop();
        socket->deleteLater();
    }
    m_clients.clear();
}

QHostAddress TcpServer::host() const
{
    return m_host;
}

void TcpServer::setHost(const QHostAddress &host)
{
    m_host = host;
}

quint16 TcpServer::port() const
{
    return m_port;
}

void TcpServer::setPort(const quint16 &port)
{
    m_port = port;
}
