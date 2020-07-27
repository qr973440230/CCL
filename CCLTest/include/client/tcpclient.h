#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QtGlobal>

#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include "../queue/abstractqueue.h"

#define TCP_DEFAULT_BUF_SIZE 1024
#define TCP_DEfAULT_RECONNECT_TIME 2000

struct TCPBuffer{
    char buffer[TCP_DEFAULT_BUF_SIZE];
    qint64 len;
    QHostAddress addr;
    quint16 port;
};

class Q_DECL_EXPORT TcpClient: public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpClient(AbstractQueue<TCPBuffer> * queue,
                       QObject * parent = nullptr);

    explicit TcpClient(const QString &host,
          quint16 port,
          AbstractQueue<TCPBuffer> * queue,
          QObject * parent = nullptr);

    virtual ~TcpClient() override;

    void writeBuffer(const TCPBuffer &buffer);

    void start();

    void stop();

    QString host() const;
    void setHost(const QString &host);

    quint16 port() const;
    void setPort(const quint16 &port);

signals:
    void startSignal();
    void stopSignal();

    void writeBufferSignal(const TCPBuffer &buffer);

    void unconnected();
    void connecting();
    void connected();
    void closing();

    void error(QAbstractSocket::SocketError sockeError);

private slots:
    void startSlot();
    void stopSlot();

    void writeBufferSlot(const TCPBuffer &buffer);

    void readyReadSlot();
    void stateChangedSlot(QTcpSocket::SocketState state);
    void errorSlot(QAbstractSocket::SocketError socketError);

    void timeoutSlot();

private:
    void init();

private:
    QString m_host;
    quint16 m_port;

    AbstractQueue<TCPBuffer> *m_queue;

    QTimer * m_timer;

    int m_interval;
};

#endif // TCPCLIENT_H
