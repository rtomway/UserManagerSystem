#include"SApp.h"
#include"LoginRegisterDlg.h"
#include"Mainwindow.h"
#include <QStackedWidget>


int main(int argc, char* argv[])
{
	SApp a(argc,argv);

	LoginRegisterDlg login;
	
	if (login.exec() == QDialog::Rejected)
	{
		return 0;
	}

	Mainwindow w;
	
	QObject::connect(&w, &Mainwindow::reLogin,[&]
		{
			LoginRegisterDlg relogin;
			if (relogin.exec() == QDialog::Rejected)
			{
				return 0;
			}
			w.updateReloginlater();
		});

	w.show();
	
	return a.exec();
}