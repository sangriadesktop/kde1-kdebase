#ifndef _MAINWIDGET_H
#define _MAINWIDGET_H

#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qfont.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qregexp.h>
#include <qpixmap.h>
#include <kapp.h>
#include <qimage.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>


class mainWidget : public QWidget
{  
  Q_OBJECT    
  
  public:   
  
  mainWidget(QWidget *parent, const char *name=0);	
  
  private:   
  
  QLabel *heading;
  QImage image;
  bool loadOK;
  struct utsname info;
  
protected:   

  virtual void paintEvent(QPaintEvent *);	  
  virtual void resizeEvent(QResizeEvent *);
    
signals:
    
  void resized();
    
};


#endif
