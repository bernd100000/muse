#ifndef MUSEMDIAREA_H
#define MUSEMDIAREA_H

#include <QMdiArea>


class MuseMdiArea : public QMdiArea
{
  Q_OBJECT


public:
    MuseMdiArea(QWidget *);
    virtual void keyPressEvent(QKeyEvent *);
};

#endif // MUSEMDIAREA_H
