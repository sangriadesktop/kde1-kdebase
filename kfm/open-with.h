#ifndef __open_with_h__
#define __open_with_h__

#include <qwidget.h>
#include <qfileinf.h>
#include <kapp.h>
#include <ktreelist.h>

class KAppTreeListItem : public KTreeListItem
{	
public:
  KAppTreeListItem( const char *name, QPixmap *pixmap, bool parse, bool dir, 
		    QString p, QString c );

  bool parsed;
  bool directory;
  QString path;
  QString exec;
};

class KApplicationTree : public QWidget
{
  Q_OBJECT
public:
  KApplicationTree( QWidget *parent );

  bool isKdelnkFile( const char *filename );
  void parseKdelnkFile( QFileInfo *fi, KTreeList *tree, KAppTreeListItem *item );
  short parseKdelnkDir( QDir d, KTreeList *tree, KAppTreeListItem *item = 0 );
  
  KTreeList *tree;
  KAppTreeListItem *it, *it2, *dummy;
  
protected:
  virtual void resizeEvent( QResizeEvent *_ev );
  
public slots:
  void expanded(int index);
  void selected(int index);
  // void highlighted(int index);

signals:
  void selected( const char *_name );
};

#endif
