/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef __FreeBSD__
#include <sys/vmmeter.h>
#include <kvm.h>
#include <nlist.h>
#endif

/*=============================================================================
  CLASSes
 =============================================================================*/
//-----------------------------------------------------------------------------
// class  : MemMon
//-----------------------------------------------------------------------------
class MemMon : public QWidget
{
    Q_OBJECT

public:     
     MemMon (QWidget *parent = 0, const char *name = 0, QWidget *child = 0);
    ~MemMon ();

#ifdef __FreeBSD__
     void setTargetLabels(QLabel **);
     int  swapInfo(int *, int *);
#endif
     void updateLabel(QLabel *, int);
protected:

    virtual void paintEvent(QPaintEvent *);
    virtual void timerEvent(QTimerEvent *);

    int      intervals,
             mem_size, 
             physsize,
            *mem_values;
    QWidget *my_child;
    QBrush   brush_0, 
             brush_1;
#ifdef __FreeBSD__
    u_long physmem, usermem;
    int    sw_avail, sw_free;
    int    mib_usermem[2], bufspace;
    QLabel **memstat_labels;
    struct vmmeter vmstat;
    u_long   cnt_offset, buf_offset;
    struct nlist nlst[] = {
#define X_CNT          0
        { "_cnt" },                /* 0 */
#define X_VMTOTAL      1
        { "_vmtotal"},
#define X_BUFSPACE     2
        { "_bufspace"},
#define VM_SWAPLIST     3
        { "_swaplist"},
#define VM_SWDEVT       4
        { "_swdevt"},
#define VM_NSWAP        5
        { "_nswap"},
#define VM_NSWDEV       6
        { "_nswdev"},
#define VM_DMMAX        7
        { "_dmmax"},
        { NULL }
    };
    struct _ivm {
        int active, inactive, wired, cache, buffers;
    };
    struct _ivm ivm, *iVm;
#endif
};

#define MEM_ACTIVE 0
#define MEM_INACTIVE 1
#define MEM_WIRED 2
#define MEM_CACHE 3
#define MEM_BUFFERS 4
#define MEM_PHYS 5
#define MEM_KERN 6
#define MEM_SWAPAVAIL 7
#define MEM_SWAPFREE  8
#define MEM_TOTAL 9
#define MEM_END 10
