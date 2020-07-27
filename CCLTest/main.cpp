#include <QCoreApplication>
#include <QThread>

#include "include/queue/dropqueue.h"
#include "include/client/tcpclient.h"
#include "include/server/tcpserver.h"

class TestThread:public QThread
{
public:
    TestThread(AbstractQueue<TCPBuffer> * queue,TcpServer * server);

protected:
    void run() override;

private:
    AbstractQueue<TCPBuffer> * m_queue;
    TcpServer * m_server;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QThread thread;
    thread.start();

    DropQueue<TCPBuffer> dropQueue(200,1000);

    TcpServer server(&dropQueue);
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

    TcpClient client(QHostAddress("127.0.0.1"),8888,&dropQueue);
    client.start();
    client.moveToThread(&thread);

    TestThread testThread(&dropQueue,&server);
    testThread.start();



    return a.exec();
}

TestThread::TestThread(AbstractQueue<TCPBuffer> *queue, TcpServer *server):m_queue(queue),m_server(server)
{

}

void TestThread::run()
{
    while(1){
        TCPBuffer * buffer = m_queue->peekReadable(1000);
        if(!buffer){
            // 超时
            continue;
        }

        QByteArray ba(buffer->buffer,buffer->len);
        qDebug()<<ba.toHex();

        buffer->port = 0;

        m_server->writeBuffer(*buffer);

        m_queue->next(buffer);
    }
}
