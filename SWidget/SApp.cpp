#include"SApp.h"
#include<QFile>

SApp::SApp(int argc, char* argv[])
	:QApplication(argc,argv)
{
	initGlobalConfig();

}
inline SConfigFile* SApp::globalConfig()
{
	if (!m_globalConfig)
	{
		qWarning() << "Global config is nullptr";
	}
	return m_globalConfig;
}
void SApp::initGlobalConfig()
{
	const char* configFileName = "config.ini";
	//判断配置文件是否存在
	bool isExists = false;
	if (QFile::exists(configFileName))
	{
		isExists = true;
	}
	//打开配置文件
	m_globalConfig = new SConfigFile("config.ini");
	Q_ASSERT_X(m_globalConfig != nullptr, __FUNCTION__, "Global Config file Create failed~");
	if (!m_globalConfig->IsOpen())
	{
		qWarning() << "global config init failed";
		return;
	}
	//不存在，添加默认配置文件
	if (!isExists)
	{
		globalConfig()->setValue("host/host", "127.0.0.1:8888");
	}


}
void SApp::setUserData(const QString& key, const QVariant& data)
{
	auto it = m_userData.find(key);
	if (it != m_userData.end()) {
		qWarning() << key << "of value exists";
	}
	m_userData.insert(key, data);
}

QVariant SApp::userData(const QString& key)
{
	auto it = m_userData.find(key);
	if (it == m_userData.end())
	{
		qWarning() << key << "of value not exists";
		return QVariant();
	}
	return m_userData.value(key);
}
