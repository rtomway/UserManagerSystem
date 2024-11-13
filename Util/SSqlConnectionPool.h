#ifndef SSQLCONNECTIONPOOL_H_
#define SSQLCONNECTIONPOOL_H_

#include<QSqlDataBase>
#include<QSqlQuery>
#include<QSqlRecord>
#include<QSqlError>
#include<QQueue>

#include<QMutex>
#include<QMutexLocker>
#include<QWaitCondition>

class SSqlConnectionPool
{
	SSqlConnectionPool();
public:
	static SSqlConnectionPool* instance();
	~SSqlConnectionPool();

	QSqlDatabase openConnection();
	void closeConnection(QSqlDatabase db);


	void setHostName(const QString& host) { m_hostName = host; }
	void setPort(quint16 port) { m_port = port; }
	void setDatabaseName(const QString& databaseName) { m_databaseName = databaseName; }
	void setUserName(const QString& username) { m_userName = username; }
	void setPassword(const QString& password) { m_password = password; }
	void setDatabaseType(const QString& type) { m_databaseType = type; }
	void setMaxConnectionCount(quint32 count) { m_maxConnectionCount = count; }
private:
	QSqlDatabase CreateConnection(const QString& con_name);
	void  releaseConnection(QSqlDatabase db);

	quint16 m_port;
	QString m_hostName;
	QString m_databaseName;
	QString m_userName;
	QString m_password;
	QString m_databaseType;

	QQueue<QString> m_useConnections;
	QQueue<QString>m_unUseConnections;
	quint32 m_maxConnectionCount;

	inline static QMutex m_mutex;
	QWaitCondition m_waitCond;
	quint32 m_waitInterval;
	quint32 m_waitTime;

	friend class SConnectionWrap;
};

class SConnectionWrap
{
public:
	SConnectionWrap();
	~SConnectionWrap();
	QSqlDatabase openConnection();
private:
	QSqlDatabase m_db;
};

#endif // !SSQLCONNECTIONPOOL_H_
