#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

#include "adu.h"
#include "backend.h"
#include "dbus_services.h"

class Server : public QObject
{
	Q_OBJECT
public:
	Server(int tcpPort, QObject *parent = 0);

public slots:
	void modbusReply(ADU * const reply);

private slots:
	void newConnection();
	void readyRead();
	void disconnected();
	void bytesWritten(qint64 bytes);

signals:
	void modbusRequest(ADU * const request);

private:
	QTcpServer * mServer;
	QList<QTcpSocket *> mClients;
	QMap<QTcpSocket *, ADU *> mRequests;
};

#endif // SERVER_H
