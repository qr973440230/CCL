#ifndef TCPCLIENTTHREAD_H
#define TCPCLIENTTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include "../queue/abstractqueue.h"
#include "messagebuffer.h"

class Q_DECL_EXPORT TcpClientThread: public QThread
{
    Q_OBJECT

public:
    TcpClientThread(const QHostAddress &host,
                    quint16 port,
                    AbstractQueue<MessageBuffer> * readQueue,
                    AbstractQueue<MessageBuffer> * writeQueue,
                    QObject * parent = nullptr);
signals:
    void connectingHostSignal(const QHostAddress &host,quint16 port);
    void connectedHostSignal(const QHostAddress &host,quint16 port);
    void disconnectedHostSignal(const QHostAddress &host,quint16 port,
                                const QString &errorStr);

protected:
    virtual void run() override;

private:
    bool connectToHost();
    void readMessage();
    void writeMessage();

private:
    QHostAddress m_host;
    quint16 m_port;
    AbstractQueue<MessageBuffer> * m_readQueue;
    AbstractQueue<MessageBuffer> * m_writeQueue;

    QTcpSocket * m_socket = nullptr;
    QTcpSocket::SocketState m_socketLastState = QTcpSocket::UnconnectedState;
};

#endif // TCPCLIENTTHREAD_H
