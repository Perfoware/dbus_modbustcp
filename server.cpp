#include "server.h"
#include "backend.h"

//#define QS_LOG_DISABLE
#include "QsLog.h"

Server::Server(int tcpPort, QObject *parent) :
	QObject(parent)
{
	mServer = new QTcpServer(this);
	if (mServer->listen(QHostAddress::Any, tcpPort)) {
		QLOG_INFO() << QString("[Server] Server listening at: %1:%2").
					   arg(mServer->serverAddress().toString()).
					   arg(mServer->serverPort());
		connect(mServer, SIGNAL(newConnection()), SLOT(newConnection()));
	}
	else QLOG_ERROR() << "[Server] QTcpServer error: " + mServer->errorString();
}

void Server::newConnection()
{
	QTcpSocket * newConnection = mServer->nextPendingConnection();

	newConnection->socketOption(QAbstractSocket::LowDelayOption);
	mClients.append(newConnection);
	QLOG_TRACE() << "[Server] New connecion: " << newConnection->peerAddress().toString()+":"+QString::number(newConnection->peerPort());
	connect(newConnection, SIGNAL(readyRead()), SLOT(readyRead()));
	connect(newConnection, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(newConnection, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten(qint64)));
}

void Server::readyRead()
{
	QTcpSocket * socket = (QTcpSocket *) sender();
	if (socket == 0) return;

	QByteArray tcpReq = socket->readAll();
	QLOG_TRACE() << "[Server] request from " << socket->peerAddress().toString()+":"+QString::number(socket->peerPort());
	QLOG_TRACE() << "[Server] request data " << tcpReq.toHex().toUpper();
	ADU * request = new ADU(socket, tcpReq);
	mRequests.insert(socket,request);
	QLOG_DEBUG() << "[Server] Request:" << request->aduToString();
	emit modbusRequest(request);
}

void Server::disconnected()
{
	QTcpSocket *socket = (QTcpSocket *) sender();
	if (socket == 0) return;

	QLOG_TRACE() << "[Server] Disconnected: " << socket->peerAddress().toString()+":"+QString::number(socket->peerPort());
	mClients.removeAll(socket);
	delete mRequests.value(socket);
	socket->deleteLater();
}

void Server::modbusReply(ADU * const modbusReply)
{
	QLOG_DEBUG() << "[Server] Reply:" << modbusReply->aduToString();
	modbusReply->getSocket()->write(modbusReply->toQByteArray());
}

void Server::bytesWritten(qint64 bytes)
{
	QTcpSocket * socket = (QTcpSocket *) sender();

	QLOG_DEBUG() << "[Server] bytesWritten:" << bytes << socket->peerAddress().toString()+":"+QString::number(socket->peerPort());
	delete mRequests.value(socket);
	mRequests.remove(socket);
}
