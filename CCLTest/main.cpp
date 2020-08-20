#include <QCoreApplication>
#include <QThread>

#include "include/queue/dropqueue.h"
#include "include/client/tcpclient.h"
#include "include/server/tcpserver.h"

class TestServerThead:public QThread
{
public:
    TestServerThead(AbstractQueue<TCPBuffer> * queue,TcpServer * tcpServer);

protected:
    void run() override;

private:
    AbstractQueue<TCPBuffer> * m_queue;
    TcpServer * m_tcpServer;
};

class TestClientThread: public QThread
{
public:
    TestClientThread(AbstractQueue<TCPBuffer> * queue,TcpClient * client);

protected:
    void run() override;

private:
    AbstractQueue<TCPBuffer> * m_queue;
    TcpClient * m_tcpClient;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QThread thread;
    thread.start();

    DropQueue<TCPBuffer> serverDropQueue(200,1000);

    TcpServer server(&serverDropQueue);
    server.setHost(QHostAddress::Any);
    server.setPort(9898);
    server.moveToThread(&thread);
    server.start();

    QObject::connect(&server,&TcpServer::clientConnected,&server,[](const QHostAddress &addr,quint16 port){
        qDebug()<<"Client Connected: "<< addr<<
                  " Port: "<<port;
    });
    QObject::connect(&server,&TcpServer::clientDisconnected,&server,[](const QHostAddress &addr,quint16 port){
        qDebug()<<"Client Disconnected: "<<addr<<
                  " Port: "<<port;
    });

    DropQueue<TCPBuffer> clientDropQueue(200,1000);
    TcpClient client(QHostAddress("127.0.0.1"),9898,&clientDropQueue);
    client.start();
    client.moveToThread(&thread);

    TestServerThead testServerThread(&serverDropQueue,&server);
    testServerThread.start();

    TestClientThread testClientThread(&clientDropQueue,&client);
    testClientThread.start();

    return a.exec();
}

TestServerThead::TestServerThead(AbstractQueue<TCPBuffer> *queue, TcpServer *tcpServer):m_queue(queue),m_tcpServer(tcpServer)
{
    // 每200ms发送数据
    QTimer * timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this](){
        TCPBuffer buffer;
        buffer.len = TCP_DEFAULT_BUF_SIZE;
        m_tcpServer->writeBuffer(buffer);
    });
    timer->start(20);
}

void TestServerThead::run()
{
    while(1){
        TCPBuffer * buffer = m_queue->peekReadable(1000);
        if(!buffer){
            // 超时
            continue;
        }

        QByteArray ba(buffer->buffer,buffer->len);
//        qDebug()<<"Server: "<<ba.length();

        m_queue->next(buffer);
    }
}

TestClientThread::TestClientThread(AbstractQueue<TCPBuffer> *queue, TcpClient *client)
    :m_queue(queue),m_tcpClient(client)
{
    QTimer * timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this](){
        static int i = 0;
        if(i++ % 2 == 0){
            m_tcpClient->stop();
        }else{
            m_tcpClient->start();
        }
    });
//    timer->start(4000);

    QTimer * timer2 = new QTimer(this);
    connect(timer2,&QTimer::timeout,this,[this](){
        TCPBuffer buffer;
        buffer.len = TCP_DEFAULT_BUF_SIZE;
        m_tcpClient->writeBuffer(buffer);
    });
    timer2->start(200);
}

void TestClientThread::run()
{
    while(1){
//        TCPBuffer * buffer = m_queue->peekReadable(1000);
//        if(!buffer){
//            // 超时
//            continue;
//        }

//        QByteArray ba(buffer->buffer,buffer->len);
//        qDebug()<<"Client: "<< ba.length();

//        m_queue->next(buffer);

        QList<TCPBuffer *> list = m_queue->peekAllReadable(1000);
        qDebug()<<"Client: "<< list.size();
        m_queue->nextAll(list);

        QThread::msleep(2000);
    }
}
