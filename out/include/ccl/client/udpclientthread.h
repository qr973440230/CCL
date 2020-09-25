#ifndef UDPCLIENTTHREAD_H
#define UDPCLIENTTHREAD_H

#include <QThread>
#include <QUdpSocket>
#include "../queue/abstractqueue.h"
#include "messagebuffer.h"

class Q_DECL_EXPORT UdpClientThread: public QThread
{
    Q_OBJECT

public:
    UdpClientThread(const QHostAddress &host,quint16 port,
                    AbstractQueue<MessageBuffer> * readQueue,
                    AbstractQueue<MessageBuffer> * writeQueue,
                    QObject * parent = nullptr);
signals:
    void bindSuccess(const QHostAddress &host,quint16 port);
    void bindError(const QHostAddress &host,quint16 port,const QString &errorStr);

protected:
    virtual void run() override;

private:
    bool bind();
    void readMessage();
    void writeMessage();

private:
    QHostAddress m_host;
    quint16 m_port;
    AbstractQueue<MessageBuffer> * m_readQueue;
    AbstractQueue<MessageBuffer> * m_writeQueue;

    QUdpSocket * m_socket = nullptr;
    QUdpSocket::SocketState m_socketLastState = QUdpSocket::SocketState::ClosingState;
};

#endif // UDPCLIENTTHREAD_H
