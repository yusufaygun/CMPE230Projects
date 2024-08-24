#ifndef FLAGBUTTON_H
#define FLAGBUTTON_H

#include <QPushButton>
#include <QWidget>
#include <QMouseEvent>

class FlagButton : public QPushButton {
    Q_OBJECT
public:
    FlagButton(QWidget *parent = nullptr);
    bool isFlagged;

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void rightClicked();
};

#endif // FLAGBUTTON_H
