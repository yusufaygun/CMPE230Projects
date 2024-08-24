#include "flagbutton.h"

// Constructor for FlagButton, initializes isFlagged to false
FlagButton::FlagButton(QWidget *parent)
    : QPushButton(parent), isFlagged(false) {}

// Custom mouse press event to handle right-clicks
void FlagButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        emit rightClicked();  // Emit rightClicked signal when right button is pressed
    } else {
        QPushButton::mousePressEvent(event);  // Call base class implementation for other buttons
    }
}
