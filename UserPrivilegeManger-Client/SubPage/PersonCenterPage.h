#ifndef PERSONCENTERPAGE_H_
#define PERSONCENTERPAGE_H_

#include <QWidget>
#include "PersonMessage.h"

class PersonCenterPage :public PersonMessage
{
	Q_OBJECT
public:
	PersonCenterPage(QWidget* parent = nullptr);
	void init();

};


#endif // !PERSONCENTERPAGE_H_
