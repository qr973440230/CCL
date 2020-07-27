#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpSocket>
#include <QThread>
#include <QtGlobal>

#include <QTcpServer>
#include "../queue/abstractqueue.h"
#include "../client/tcpclient.h"

class Q_DECL_EXPORT TcpServer:public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(AbstractQueue<TCPBuffer> * queue,
                       QObject * parent = nullptr);
    explicit TcpServer(const QHostAddress &host,quint16 port,
                       AbstractQueue<TCPBuffer> * queue,
                       QObject * parent = nullptr);
    ~TcpServer();

    void start();
    void stop();

    void writeBuffer(const TCPBuffer &buffer);

signals:
    void startSignal();
    void stopSignal();

    void writeBufferSignal(const TCPBuffer &buffer);

    void listenError(const QString &error);

private slots:
    void startSlot();
    void stopSlot();

    void writeBufferSlot(const TCPBuffer &buffer);
    void acceptErrorSlot(QAbstractSocket::SocketError socketError);

protected:
    void incomingConnection(qintptr handle) override;

private:
    void init();
    void closeSockets();

public:
    QHostAddress host() const;
    void setHost(const QHostAddress &host);

    quint16 port() const;
    void setPort(const quint16 &port);

private:
    QHostAddress m_host;
    quint16 m_port;

    AbstractQueue<TCPBuffer> * m_queue;

    QThread * m_thread;
    QList<TcpClient*> m_clients;
};

#endif // TCPSERVER_H
