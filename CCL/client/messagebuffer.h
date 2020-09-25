#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include <QHostAddress>

#define DEFAULT_MESSAGE_BUF_SIZE 1024

struct MessageBuffer{
    char buffer[DEFAULT_MESSAGE_BUF_SIZE];
    qint64 len;
    QHostAddress addr;
    quint16 port;
};

#endif // MESSAGEBUFFER_H
