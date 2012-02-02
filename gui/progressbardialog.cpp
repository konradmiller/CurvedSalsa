#include <QtGui>
#include "progressbardialog.h"

extern long long progress;
extern long long progress_max;
extern bool 	 progress_cancel;

void ProgressBarDialog::setSource( const QString &str )
{
	sourceLine->setText( str );
}

void ProgressBarDialog::setDestination( const QString &str )
{
	destLine->setText( str );
}

ProgressBarDialog::ProgressBarDialog( QWidget *parent ) : QDialog(parent)
{
	setupUi( this );
//	layout()->setSizeConstraint( QLayout::SetFixedSize );

	timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(slot_update()) );
	timer->start();

	progressbar->setMinimum( 0 );
	progressbar->setMaximum( 100 );
	progressbar->setValue( 0 );

	connect( cancelButton, SIGNAL(clicked()), this, SLOT(slot_cancel()) );
}

void ProgressBarDialog::slot_update()
{
	if( progress_max == -1 )
	{
		timer->stop();
		disconnect( cancelButton, SIGNAL(clicked()), 0, 0 );
		connect( cancelButton, SIGNAL(clicked()), this, SLOT(close()) );
		cancelButton->setText( "Wrong Password" );
	}

	if( progress == progress_max  || progress_max == 0 )
	{
		timer->stop();

		progressbar->setMinimum( 0 );
		progressbar->setMaximum( 100 );
		progressbar->setValue( 100 );

		disconnect( cancelButton, SIGNAL(clicked()), 0, 0 );
		connect( cancelButton, SIGNAL(clicked()), this, SLOT(close()) );
		cancelButton->setText( "Done" );

		return;
	}

	progressbar->setMaximum( progress_max );
	progressbar->setValue( progress );
}

void ProgressBarDialog::slot_cancel()
{
	progress_cancel = true;	
	disconnect( cancelButton, SIGNAL(clicked()), 0, 0 );
	connect( cancelButton, SIGNAL(clicked()), this, SLOT(close()) );
	cancelButton->setText( "Close" );
}
