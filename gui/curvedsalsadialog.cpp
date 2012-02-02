#include <inttypes.h>

#include <QtGui>
#include <QDir>
#include <QtConcurrentRun>

#include "curvedsalsadialog.h"
#include "progressbardialog.h"
#include "curvedsalsa.h"

CurvedSalsaDialog::CurvedSalsaDialog( QWidget *parent ) : QDialog(parent)
{
	setupUi( this );

//	layout()->setSizeConstraint( QLayout::SetFixedSize );

	connect( inFileButton,   SIGNAL(clicked()), this, SLOT(slot_browseInputFile()) );
	connect( outFileButton,  SIGNAL(clicked()), this, SLOT(slot_browseOutputFile()) );
	connect( okButton,       SIGNAL(clicked()), this, SLOT(slot_doit()) );
	connect( quitButton,     SIGNAL(clicked()), this, SLOT(close()) );
	connect( encrypt, 	 SIGNAL(clicked()), this, SLOT(slot_radiobuttonChanged()) );
	connect( decrypt, 	 SIGNAL(clicked()), this, SLOT(slot_radiobuttonChanged()) );

	connect( secKeyLine, SIGNAL(textChanged(const QString&)),
		 this, SLOT(slot_generateKey(const QString&)) );
}


void CurvedSalsaDialog::slot_radiobuttonChanged()
{
	if( encrypt->isChecked() )
		okButton->setText( "&Encrypt" );
	else if( decrypt->isChecked() )
		okButton->setText( "&Decrypt" );
}


void CurvedSalsaDialog::slot_generateKey( const QString &key )
{
	uint8_t pubKey[32];
	generate_key( pubKey, key.toStdString()  );
	pubKeyLine->setText( b64encode(pubKey).c_str() );
}


void CurvedSalsaDialog::slot_browseInputFile()
{
	m_inFilename = QFileDialog::getOpenFileName( this, tr("Open File"), "", "All (*)" );
	inFileLine->setText( m_inFilename );	
}


void CurvedSalsaDialog::slot_browseOutputFile()
{
	m_outFilename = QFileDialog::getSaveFileName( this, tr("Output File"), "", "All (*)", 0, QFileDialog::DontConfirmOverwrite );
	outFileLine->setText( m_outFilename );	
}


void CurvedSalsaDialog::slot_doit()
{
	if( inFileLine->text() == "" )
	{
		QMessageBox::warning( this, "CurvedSalsa", "Please specify input files!" );
		return;
	}

	if( outFileLine->text() == "" )
	{
		QMessageBox::warning( this, "CurvedSalsa", "Please specify output files!" );
		return;
	}

	m_inFilename  = inFileLine->text();
	m_outFilename = outFileLine->text();

	if( ! QFile::exists( m_inFilename ) )
	{
		QMessageBox::warning( this, "CurvedSalsa", "Input file does not exist!" );
		return;
	}

	if( QFile::exists( m_outFilename ) )
	{
		QMessageBox msgBox;
		msgBox.setText( "CurvedSalsa" );
		msgBox.setInformativeText( "Output file exists - overwrite?" );
		msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
		msgBox.setDefaultButton( QMessageBox::No );

		switch( msgBox.exec() )
		{
		case QMessageBox::Yes:
			break; // and keep going ;)

		case QMessageBox::No:
			return;
			break;
		}
	}

	if( encrypt->isChecked() )
	{
		if( pubKeyLine->text() == "" )
		{
			QMessageBox::warning( this, "CurvedSalsa", "You need to enter the recipients public key" );
			return;
		}

		QtConcurrent::run(
			encryptFile,
			m_outFilename.toStdString(),
			m_inFilename.toStdString(),
			secKeyLine->text().toStdString(),
			pubKeyLine->text().toStdString(),
			false
		);

		ProgressBarDialog *progressbar = new ProgressBarDialog();
		progressbar->setSource( m_inFilename );
		progressbar->setDestination( m_outFilename );
		progressbar->show();
	}
	else if( decrypt->isChecked() )
	{
		if( secKeyLine->text() == "" )
		{
			QMessageBox::warning( this, "CurvedSalsa", "You need to enter your private passphrase" );
			return;
		}

		QtConcurrent::run(
			decryptFile,
			m_outFilename.toStdString(),
			m_inFilename.toStdString(),
			secKeyLine->text().toStdString(),
			false	
		);

		ProgressBarDialog *progressbar = new ProgressBarDialog();
		progressbar->setSource( m_inFilename );
		progressbar->setDestination( m_outFilename );
		progressbar->show();
	}
	else
	{
		QMessageBox::warning( this, "CurvedSalsa", "Please select encryption or decryption!" );
		return;
	}
}
