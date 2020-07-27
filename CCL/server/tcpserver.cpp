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
        for(auto socket : m_clients.values()){
            socket->writeBuffer(buffer);
        }
    }else if(buffer.port == 0){
        // send point at addr
        for(auto const & key : m_clients.keys()){
            QStringList strList = key.split("_",QString::SkipEmptyParts);
            if(strList.size() != 2){
                continue;
            }

            if(QHostAddress(strList.at(0)) == buffer.addr){
                m_clients.value(key)->writeBuffer(buffer);
            }
        }
    }else{
        // send point at addr and port
        for(auto key : m_clients.keys()){
            QStringList strList = key.split("_",QString::SkipEmptyParts);
            if(strList.size() != 2){
                continue;
            }

            if(QHostAddress(strList.at(0)) == buffer.addr &&
                    strList.at(1).toUShort() == buffer.port){
                m_clients.value(key)->writeBuffer(buffer);
            }
        }
    }
}

void TcpServer::clientDisconnectedSlot(const QHostAddress &addr, quint16 port)
{
    QString key = QString("%1_%2").arg(addr.toString()).arg(port);
    TcpClient * client = m_clients.value(key,nullptr);
    if(client){
        qDebug()<<"Client has Closed!"<<
                  " Addr: "<<addr<<
                  " Port: "<<port;

        client->close();
        client->deleteLater();
        m_clients.remove(key);
    }else{
        qDebug()<< "Unkown Client!!! Addr: "<<addr<<
                   " Port: "<<port;
    }
}

void TcpServer::incomingConnection(qintptr handle)
{
    TcpClient * client = new TcpClient(m_queue);
    connect(client,&TcpClient::unconnected,this,&TcpServer::clientDisconnected);
    connect(client,&TcpClient::unconnected,this,&TcpServer::clientDisconnectedSlot);

    client->setSocketDescriptor(handle);
    QHostAddress peerAddress = client->peerAddress();
    quint16 peerPort = client->peerPort();
    client->setHost(peerAddress);
    client->setPort(peerPort);

    client->moveToThread(m_thread);

    QString key = QString("%1_%2").arg(peerAddress.toString()).arg(peerPort);
    m_clients.insert(key,client);
    emit clientConnected(peerAddress,peerPort);
}

void TcpServer::init()
{
    m_thread = new QThread();
    m_thread->start();

    qRegisterMetaType<TCPBuffer>("TCPBuffer");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    connect(this,&TcpServer::startSignal,this,&TcpServer::startSlot);
    connect(this,&TcpServer::stopSignal,this,&TcpServer::stopSlot);
    connect(this,&TcpServer::writeBufferSignal,this,&TcpServer::writeBufferSlot);
}

void TcpServer::closeSockets()
{
    for(auto socket : m_clients.values()) {
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
