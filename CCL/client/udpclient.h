#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QtGlobal>
#include <QUdpSocket>
#include "../queue/abstractqueue.h"
#include "messagebuffer.h"


class Q_DECL_EXPORT UdpClient:public QUdpSocket
{
    Q_OBJECT
public:
    explicit UdpClient(AbstractQueue<MessageBuffer> * queue,
                       QObject * parent = nullptr);

    explicit UdpClient(quint16 port,
               AbstractQueue<MessageBuffer> *queue,
               QObject * parent = nullptr);

    explicit UdpClient(const QHostAddress &host,
               quint16 port,
               AbstractQueue<MessageBuffer> *queue,
               QObject * parent = nullptr);

    void writeBuffer(const MessageBuffer &buffer);

    void start();
    void stop();

signals:
    void startSignal();
    void stopSignal();

    void udpBufferQueueUpdateSignal();
    void writeBufferSignal(const MessageBuffer &buffer);

private slots:
    void startSlot();
    void stopSlot();

    void writeBufferSlot(const MessageBuffer &buffer);

    void readyReadSlot();
    void stateChangedSlot(QUdpSocket::SocketState state);

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
};

#endif // UDPCLIENT_H
