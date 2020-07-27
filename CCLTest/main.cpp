#include <QCoreApplication>
#include <QThread>

#include "include/queue/dropqueue.h"
#include "include/client/tcpclient.h"
#include "include/server/tcpserver.h"

class TestThread:public QThread
{
public:
    TestThread(AbstractQueue<TCPBuffer> * queue);

protected:
    void run() override;

private:
    AbstractQueue<TCPBuffer> * m_queue;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QThread thread;
    thread.start();


    DropQueue<TCPBuffer> dropQueue(200,1000);

    TestThread testThread(&dropQueue);
    testThread.start();


    TcpServer server(&dropQueue);
    server.setHost(QHostAddress::Any);
    server.setPort(9898);
    server.moveToThread(&thread);
    server.start();

    return a.exec();
}

TestThread::TestThread(AbstractQueue<TCPBuffer> *queue):m_queue(queue)
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

        qDebug()<<QByteArray(buffer->buffer,buffer->len).toHex();
        m_queue->next(buffer);
    }
}
