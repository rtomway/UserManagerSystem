﻿#ifndef SAPP_H_
#define SAPP_H_

#include<QApplication>
#include"SConfigFile.h"

#define sApp static_cast<SApp*>(qApp)
#define URL(route) QUrl("http://" + sApp->globalConfig()->value("host/host").toString() +route)

class SApp :public QApplication
{
public:
	SApp(int argc, char* argv[]);
	inline SConfigFile* globalConfig();

	void setUserData(const QString& key,const QVariant& data);
	QVariant userData(const QString& key);
private:
	SConfigFile* m_globalConfig{};
	void initGlobalConfig();

	QVariantMap m_userData;
};

#endif // !SAPP_H_