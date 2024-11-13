#include"SApp.h"
#include"LoginRegisterDlg.h"
#include"Mainwindow.h"


int main(int argc, char* argv[])
{
	SApp a(argc,argv);

	LoginRegisterDlg login;
	if (login.exec() == QDialog::Rejected)
	{
		return 0;
	}

	Mainwindow w;
	w.show();

	
	return a.exec();
}