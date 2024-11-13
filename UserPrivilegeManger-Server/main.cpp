#include <QCoreApplication>
#include"SSqlConnectionPool.h"
#include"server.h"



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    SSqlConnectionPool::instance()->setDatabaseName("user_privilege_manager_system");
    SSqlConnectionPool::instance()->setHostName("localhost");
    SSqlConnectionPool::instance()->setPort(3306);
    SSqlConnectionPool::instance()->setUserName("root");
    SSqlConnectionPool::instance()->setPassword("200600xx");

    Server server;

   
    return a.exec();
}
