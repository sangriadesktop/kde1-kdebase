// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de


#include "kfmserver.h"
#include "kfmdlg.h"
#include "kfmgui.h"
#include "root.h"
#include "kioserver.h"
#include "kfmprops.h"
#include "kiojob.h"
#include "kfmpaths.h"
#include "kfmexec.h"
#include <config-kfm.h>

#include <qmsgbox.h>

#include <stdlib.h>
#include <time.h>

QString KFMClient::password;

KFMServer::KFMServer() : KfmIpcServer()
{
    // Create the password file if it does not exist
    QString fn = getenv( "HOME" );
    fn += "/.kfm/magic";
    FILE *f = fopen( fn.data(), "rb" );
    if ( f == 0L )
    {
	FILE *f = fopen( fn.data(), "wb" );
	if ( f == 0L )
	{
	    QMessageBox::message("KFM Error", "Could not create ~/.kfm/magic" );
	    return;
	}
	
	QString pass;
	pass.sprintf("%i",time(0L));
	fwrite( pass.data(), 1, pass.length(), f );
	fclose( f );

	QMessageBox::message("KFM Warning", "Please change the password in\n\r~/.kfm/magic" );
    }
    else
	fclose( f );
}

void KFMServer::slotAccept( KSocket * _sock )
{
    KfmIpc * i = new KFMClient( _sock, this );
    emit newClient( i );
}

#define root KRootWidget::getKRootWidget()

void KFMServer::slotSelectRootIcons( int _x, int _y, int _w, int _h, bool _add )
{
  if ( !_add )
    root->unselectAllIcons();
  QRect r( _x, _y, _w, _h );
  root->selectIcons( r );
}

void KFMServer::slotSortDesktop()
{
    KRootWidget::getKRootWidget()->sortIcons();
}

void KFMServer::slotRefreshDesktop()
{
    KRootWidget::getKRootWidget()->update();
}

void KFMServer::slotMoveClients( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
	dest = "file:" + KFMPaths::TrashPath();

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    job->move( urlList, dest.data() );
}

void KFMServer::slotCopyClients( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
        dest = "file:" + KFMPaths::TrashPath();

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    job->copy( urlList, dest.data() );
}

void KFMServer::slotOpenURL( const char* _url )
{
    debugT("KFMServer::Opening URL '%s'\n", _url );

    if ( _url[0] != 0 )
    {
	debugT("There is an URL\n");
	
	QString url = _url;
	KURL u( _url );
	if ( u.isMalformed() )
	{
	    QMessageBox::message( "KFM Error", "Malformed URL\n" + url );
	    return;
	}
	debugT("OK\n");
	
	url = u.url().copy();
	
	if ( url == "trash:/" )
	    url = "file:" + KFMPaths::TrashPath();
	
	debugT("Seaerching window\n");
	KfmGui *w = KfmGui::findWindow( url.data() );
	if ( w != 0L )
	{
	    debugT("Window found\n");
	    w->show();
	    return;
	}
	
	debugT("Opening new window\n");
	KfmGui *f = new KfmGui( 0L, 0L, url.data() );
	f->show();
	return;
    }
    
    QString home = "file:";
    home += QDir::homeDirPath().data();
    DlgLineEntry l( "Open Location:", home.data(), KRootWidget::getKRootWidget(), true );
    if ( l.exec() )
    {
	QString url = l.getText();
	// Exit if the user did not enter an URL
	if ( url.data()[0] == 0 )
	    return;
	// Root directory?
	if ( url.data()[0] == '/' )
	{
	    url = "file:";
	    url += l.getText();
	}
	// Home directory?
	else if ( url.data()[0] == '~' )
	{
	    url = "file:";
	    url += QDir::homeDirPath().data();
	    url += l.getText() + 1;
	}
	
	KURL u( url.data() );
	if ( u.isMalformed() )
	{
	    debugT("ERROR: Malformed URL\n");
	    return;
	}

	KfmGui *f = new KfmGui( 0L, 0L, url.data() );
	f->show();
    }
}

void KFMServer::slotRefreshDirectory( const char* _url )
{
    debugT("CMD: refeshDirectory '%s'\n",_url );
    
    QString tmp = _url;
    
    KURL u( _url );
    if ( tmp.right(1) == "/" )
	KIOServer::sendNotify( u.url() );
    else
    {
	debugT("Sending something to '%s'\n",u.directoryURL());
	KIOServer::sendNotify( u.directoryURL() );
    }
}

void KFMServer::slotOpenProperties( const char* _url )
{
    new Properties( _url );
}

void KFMServer::slotExec( const char* _url, const char * _binding )
{
    if ( _binding == 0L && _url != 0L )
    {
	KFMExec *e = new KFMExec;
	e->openURL( _url );
	return;
    }
    
    // Attention this is a little hack by me (Matthias)
    /* QStrList sl;
    sl.append(_binding);   

    if ( _binding == 0L )
	KMimeBind::runBinding( _url );
    else
	KMimeBind::runBinding( _url, _binding, &sl ); */
}

KFMClient::KFMClient( KSocket *_sock, KFMServer *_server ) : KfmIpc( _sock )
{
    bAuth = FALSE;
    server = _server;
    
    connect( this, SIGNAL( auth( const char* ) ), this, SLOT( slotAuth( const char* ) ) );
}

void KFMClient::slotAuth( const char *_password )
{
    if ( KFMClient::password.isNull() )
    {
	QString fn = getenv( "HOME" );
	fn += "/.kfm/magic";
	FILE *f = fopen( fn.data(), "rb" );
	if ( f == 0L )
	{
	    QMessageBox::message( "KFM Error", "You dont have the file ~/.kfm/magic\n\rAuthorization failed" );
	    return;
	}
	char buffer[ 1024 ];
	char *p = fgets( buffer, 1023, f );
	fclose( f );
	if ( p == 0L )
	{
	    QMessageBox::message( "KFM Error", "The file ~/.kfm/magic is corrupted\n\rAuthorization failed" );
	    return;
	}
	KFMClient::password = buffer;
    }
    if ( KFMClient::password != _password )
    {
	QMessageBox::message( "KFM Error", "Someone tried to authorize himself\nusing a wrong password" );
	bAuth = false;
	return;
    }

    bAuth = true;
    connect( this, SIGNAL( list( const char* ) ),
	     this, SLOT( slotList( const char* ) ) );
    connect( this, SIGNAL( copy( const char*, const char* ) ),
	     this, SLOT( slotCopy( const char*, const char* ) ) );
    connect( this, SIGNAL( move( const char*, const char* ) ), 
	     this, SLOT( slotMove( const char*, const char* ) ) );
    connect( this, SIGNAL( copyClient( const char*, const char* ) ),
	     server, SLOT( slotCopyClients( const char*, const char* ) ) );
    connect( this, SIGNAL( moveClient( const char*, const char* ) ), 
	     server, SLOT( slotMoveClients( const char*, const char* ) ) );
    connect( this, SIGNAL( refreshDesktop() ), server, SLOT( slotRefreshDesktop() ) );
    connect( this, SIGNAL( openURL( const char* ) ), server, SLOT( slotOpenURL( const char *) ) );    
    connect( this, SIGNAL( refreshDirectory( const char* ) ), server, SLOT( slotRefreshDirectory( const char *) ) );    
    connect( this, SIGNAL( openProperties( const char* ) ), server, SLOT( slotOpenProperties( const char *) ) );    
    connect( this, SIGNAL( exec( const char*, const char* ) ),
	     server, SLOT( slotExec( const char *, const char*) ) );    
    connect( this, SIGNAL( sortDesktop() ), server, SLOT( slotSortDesktop() ) );
    connect( this, SIGNAL( selectRootIcons( int, int, int, int, bool ) ),
	     server, SLOT( slotSelectRootIcons( int, int, int, int, bool ) ) );
}

void KFMClient::slotCopy( const char *_src_urls, const char * _dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
	dest = "file:" + KFMPaths::TrashPath();

    debugT("Moving to '%s'\n",dest.data());
    
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	debugT("Appened '%s'\n",t.data());
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    debugT("Appened '%s'\n",s.data());

    KIOJob *job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
    job->copy( urlList, dest.data() );
}

void KFMClient::slotMove( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
	dest = "file:" + KFMPaths::TrashPath();

    debugT("Moving to '%s'\n",dest.data());
    
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	debugT("Appened '%s'\n",t.data());
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    debugT("Appened '%s'\n",s.data());

    KIOJob *job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
    job->move( urlList, dest.data() );
}

void KFMClient::slotList( const char *_url )
{
  KIOJob *job = new KIOJob();
  
  connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
	   this, SLOT( newDirEntry( int, KIODirectoryEntry* ) ) );
  connect( job, SIGNAL( error( int, const char* ) ),
	   this, SLOT( slotError( int, const char* ) ) );
  connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
  job->list( _url );
}

void KFMClient::slotError( int _kioerror, const char *_text )
{
  KfmIpc::error( _kioerror, _text );
}

void KFMClient::newDirEntry( int, KIODirectoryEntry * _entry )
{
  KfmIpc::dirEntry( _entry->getName(), _entry->getAccess(), _entry->getOwner(),
		    _entry->getGroup(), _entry->getCreationDate(), _entry->getSize() );
}

void KFMClient::finished( int )
{
  KfmIpc::finished();
}

#include "kfmserver.moc"
