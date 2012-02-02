#ifndef PROGRESSBARDIALOG_H
#define PROGRESSBARDIALOG_H

#include <QDialog>
#include "ui_progressbardialog.h"

class ProgressBarDialog
	: public QDialog
	, public Ui::ProgressBarDialog
{
	Q_OBJECT

public:
	ProgressBarDialog( QWidget *parent = 0 );
	void setSource( const QString &str );
	void setDestination( const QString &str );


private slots:
	void slot_update();
	void slot_cancel();

private:
	QTimer *timer;
};

#endif
