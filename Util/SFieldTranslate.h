#ifndef SFIELDTRANSLATE_H_
#define SFIELDTRANSLATE_H_

#include<QString>
#include<QMap>

class SFieldTrandlate
{
	SFieldTrandlate() = default;
public:
	static SFieldTrandlate* instance();
	void addTrans(const QString& field, const QString& trans);
	QString trans(const QString& field);
	QString trans(const QString& group,const QString& field);
private:
	QMap<QString, QString>m_map{};
};


#endif // !SFIELDTRANSLATE_H_
