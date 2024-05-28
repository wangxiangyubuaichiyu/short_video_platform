#ifndef MYLABEL_H
#define MYLABEL_H

#include <QObject>
#include <QLabel>

class Mylabel : public QLabel
{
    Q_OBJECT
public:
    Mylabel(QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // MYLABEL_H
