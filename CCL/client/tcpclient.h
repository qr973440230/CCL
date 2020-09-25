#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QtGlobal>

#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include "../queue/abstractqueue.h"
#include "messagebuffer.h"

#define DEfAULT_TCP_RECONNECT_TIME 2000

class Q_DECL_EXPORT TcpClient: public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpClient(AbstractQueue<MessageBuffer> * queue,
                       QObject * parent = nullptr);

    explicit TcpClient(const QHostAddress &host,
          quint16 port,
          AbstractQueue<MessageBuffer> * queue,
          QObject * parent = nullptr);

    virtual ~TcpClient() override;

    void writeBuffer(const MessageBuffer &buffer);

    void start();

    void stop();

signals:
    void startSignal();
    void stopSignal();

    void writeBufferSignal(const MessageBuffer &buffer);
    void tcpBufferQueueUdpateSignal();

    void disconnectedSignal(const QHostAddress &addr,quint16 port);
    void connectingSignal(const QHostAddress &addr,quint16 port);
    void connectedSignal(const QHostAddress &addr,quint16 port);
    void closingSignal(const QHostAddress &addr,quint16 port);

private slots:
    void startSlot();
    void stopSlot();

    void writeBufferSlot(const MessageBuffer &buffer);

    void readyReadSlot();
    void stateChangedSlot(QTcpSocket::SocketState state);

    void timeoutSlot();

private:
    void init();

public:
    QHostAddress host() const;
    void setHost(const QHostAddress &host);

    quint16 port() const;
    void setPort(const quint16 &port);

private:
    QHostAddress m_host;
    quint16 m_port;

    AbstractQueue<MessageBuffer> *m_queue;

    QTimer * m_timer;
    int m_interval;
};

#endif // TCPCLIENT_H
