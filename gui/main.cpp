#include <QApplication>
#include <QDialog>

#include "curvedsalsadialog.h"

int main( int argc, char *argv[] )
{
	QApplication app( argc, argv );
	
	CurvedSalsaDialog dialog;
	dialog.show();

	return app.exec();
}
