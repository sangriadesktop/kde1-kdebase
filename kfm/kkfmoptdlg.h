#ifndef __KOPTTAB_H
#define __KOPTTAB_H

#include <qdialog.h>
#include <qtabdlg.h>
#include <ktabctl.h>
#include "kproxydlg.h"
#include "useragentdlg.h"
#include "htmlopts.h"

class KKFMOptDlg : public QDialog
{
  Q_OBJECT
  public:
    KKFMOptDlg(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~KKFMOptDlg();

    // methods to access data
    void setUsrAgentData(QStrList *strList);
    QStrList dataUsrAgent() const;
    void setProxyData(QStrList *strList);
    QStrList dataProxy() const;
    void fontData(struct fontoptions& fontopts);
    void colorData(struct coloroptions& coloropts);
    
  private:
    QPushButton *help;
    QPushButton *ok;
    QPushButton *cancel;
    KTabCtl     *tabDlg;
    KProxyDlg   *prxDlg;
    KFontOptions   *fontDlg;
    KColorOptions   *colorDlg;
    UserAgentDialog *usrDlg;
    
  public slots:
    void helpShow();

};

#endif // __KOPTTAB_H
