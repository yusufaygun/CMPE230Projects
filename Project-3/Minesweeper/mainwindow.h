#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include "flagbutton.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleCellClick();
    void handleCellRightClick();
    void applySettings();
    void restartGame();

private:
    void initializeGame();
    void placeMines();
    void calculateNumbers();
    void revealCell(int row, int col);
    void revealAdjacentCells(int row, int col);
    void checkWinCondition();
    void endGame(bool won);
    std::pair<int, int> findHint();
    std::vector<std::pair<int, int>> findSafeCells(int row, int col);
    void showHint();
    void findMinesRevealed();


    int rows;
    int cols;
    int mines;
    int revealedCells;
    QVector<QVector<FlagButton*>> cells;
    QVector<QVector<int>> gameState; // -1 for mines, 0-8 for numbers, -2 for flagged cells
    QGridLayout *gridLayout;
    QLabel *scoreLabel;
    QSpinBox *rowsInput;
    QSpinBox *colsInput;
    QSpinBox *minesInput;

    // Add this member variable
    std::pair<int, int> lastHint;
    std::vector<std::pair<int, int>> certainMines;
};

#endif // MAINWINDOW_H
