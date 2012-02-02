#ifndef CURVEDSALSADIALOG_H 
#define CURVEDSALSADIALOG_H

#include <QDialog>
#include "ui_curvedsalsagui.h"


class CurvedSalsaDialog
	: public QDialog
	, public Ui::CurvedSalsaDialog
{
	Q_OBJECT

public:
	CurvedSalsaDialog( QWidget *parent = 0 );

public slots:
	void slot_generateKey ( const QString & );
	void slot_browseInputFile ();
	void slot_browseOutputFile ();
	void slot_doit ();
	void slot_radiobuttonChanged ();

private:
	QString m_inFilename, m_outFilename;
};


#endif
