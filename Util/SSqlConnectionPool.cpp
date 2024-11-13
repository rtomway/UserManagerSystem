#include "SSqlConnectionPool.h"
#include<QThread>

SSqlConnectionPool::SSqlConnectionPool()
	:m_databaseType("QMYSQL")
	,m_maxConnectionCount(5)
	,m_waitInterval(100)
	,m_waitTime(5)
{

}

SSqlConnectionPool* SSqlConnectionPool::instance()
{
	static SSqlConnectionPool* ins = nullptr;
	if (!ins)
	{
		ins = new SSqlConnectionPool;
	}
	return ins;
}

SSqlConnectionPool::~SSqlConnectionPool()
{
	QMutexLocker locker(&m_mutex);

	for (const auto& con_name : m_useConnections)
	{
		QSqlDatabase::removeDatabase(con_name);
	}

	for (const auto& con_name : m_unUseConnections)
	{
		QSqlDatabase::removeDatabase(con_name);
	}

}

QSqlDatabase SSqlConnectionPool::openConnection()
{
	QMutexLocker<QMutex> locker(&m_mutex);

	
	//当前总连接数
	int count = m_useConnections.count()+m_unUseConnections.count();
	//连接全在使用中
	for (size_t i = 0;i<m_waitTime&&m_unUseConnections.isEmpty()&&m_maxConnectionCount==count; i++)
	{
		//等待其他连接关闭
		m_waitCond.wait(&m_mutex, m_waitInterval);
		//重新计算总连接数
		count = m_useConnections.count() + m_unUseConnections.count();
	}

	QString con_name;
	//判断是否重新创建新连接
	if (m_unUseConnections.count() > 0)
		con_name = m_unUseConnections.dequeue();
	else if (count < m_maxConnectionCount)
		con_name = QString("connection-%1").arg(count+1);
	else
	{
		qWarning() << "All connection use";
		return QSqlDatabase();
	}

	auto db = CreateConnection(con_name);
	if (db.isOpen())
		m_useConnections.enqueue(con_name);

	return db;
}

void SSqlConnectionPool::closeConnection(QSqlDatabase db)
{
	QMutexLocker locker(&m_mutex);
	auto con_name = db.connectionName();
	if (m_useConnections.contains(con_name))
	{
		m_useConnections.removeOne(con_name);
		m_unUseConnections.enqueue(con_name);
		//唤醒一个正在等待的线程
		m_waitCond.wakeOne();
	}
}

QSqlDatabase SSqlConnectionPool::CreateConnection(const QString& con_name)
{
	if (QSqlDatabase::contains(con_name))
	{
		qWarning() << con_name << "connection already exists!";
		auto db= QSqlDatabase::database(con_name);
		if (db.isOpen())
			return db;
	}
	QSqlDatabase db = QSqlDatabase::addDatabase( m_databaseType,con_name);
	db.setHostName(m_hostName);
	db.setDatabaseName(m_databaseName);
	db.setPort(m_port);
	db.setUserName(m_userName);
	db.setPassword(m_password);
	if (!db.open())
	{
		qWarning() << "createConnection error" << db.lastError().text();
		return QSqlDatabase();
	}
	return db;
}

void SSqlConnectionPool::releaseConnection(QSqlDatabase db)
{
	
	auto con_name = db.connectionName();
	QSqlDatabase::removeDatabase(con_name);
}

SConnectionWrap::SConnectionWrap()
{
}

SConnectionWrap::~SConnectionWrap()
{
	auto pool = SSqlConnectionPool::instance();
	pool->closeConnection(m_db);

	/*	printf(R"(
	---max connections:%d
	---total connections:%d
	---use connections:%d
	---unUse connections:%d)",
		pool->maxConnectionCount(),
		pool->m_useConnections.count() + pool->m_unUseConnections.count(),
		pool->useConnectionCount(),
		pool->m_unUseConnections.count());
		*/
}

QSqlDatabase SConnectionWrap::openConnection()
{
	m_db = SSqlConnectionPool::instance()->openConnection();
	return m_db;
}
