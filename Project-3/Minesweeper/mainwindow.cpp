#include "mainwindow.h"
#include "flagbutton.h"
#include <QRandomGenerator>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QMouseEvent>
#include <vector>  // Include necessary headers for hint mechanism

// Main window constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , rows(10)  // Initialize default rows
    , cols(10)  // Initialize default columns
    , mines(10)  // Initialize default mines
    , revealedCells(0)  // Initialize revealed cells count
    , lastHint{-1, -1}  // Initialize lastHint to invalid state
{
    resize(150, 150); // Initial size

    QWidget *centralWidget = new QWidget(this); // Central widget for the main window
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget); // Main vertical layout

    // Add inputs for rows, columns, and mines
    rowsInput = new QSpinBox(this);
    rowsInput->setRange(5, 50);  // Set range for rows input
    rowsInput->setValue(rows);  // Set initial value for rows input
    rowsInput->setFixedSize(40, 25);

    colsInput = new QSpinBox(this);
    colsInput->setRange(5, 50);  // Set range for columns input
    colsInput->setValue(cols);  // Set initial value for columns input
    colsInput->setFixedSize(40, 25);

    minesInput = new QSpinBox(this);
    minesInput->setRange(1, rows * cols);  // Ensure mines do not exceed cells
    minesInput->setValue(mines);  // Set initial value for mines input
    minesInput->setFixedSize(40, 25);

    QLabel *rowsLabel = new QLabel("Rows:", this);
    rowsLabel->setFixedSize(55, 25);

    QLabel *colsLabel = new QLabel("Columns:", this);
    colsLabel->setFixedSize(55, 25);

    QLabel *minesLabel = new QLabel("Mines:", this);
    minesLabel->setFixedSize(55, 25);

    QPushButton *applyButton = new QPushButton("Apply", this); // Button to apply settings
    applyButton->setFixedSize(60, 25);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::applySettings);

    QVBoxLayout *inputLayout = new QVBoxLayout(); // Vertical layout for inputs

    inputLayout->setAlignment(Qt::AlignLeft);  // Align the whole input layout to the left

    QHBoxLayout *rowsLayout = new QHBoxLayout(); // Horizontal layout for rows input
    rowsLayout->addWidget(rowsLabel);
    rowsLayout->addWidget(rowsInput);
    rowsLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *colsLayout = new QHBoxLayout(); // Horizontal layout for columns input
    colsLayout->addWidget(colsLabel);
    colsLayout->addWidget(colsInput);
    colsLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *minesLayout = new QHBoxLayout(); // Horizontal layout for mines input
    minesLayout->addWidget(minesLabel);
    minesLayout->addWidget(minesInput);
    minesLayout->setAlignment(Qt::AlignLeft);

    inputLayout->addLayout(rowsLayout);
    inputLayout->addLayout(colsLayout);
    inputLayout->addLayout(minesLayout);
    inputLayout->addWidget(applyButton);

    mainLayout->addLayout(inputLayout);

    // Create a grid layout for the cells
    gridLayout = new QGridLayout;
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // Create a widget to hold the grid layout
    QWidget *gridWidget = new QWidget(this);
    gridWidget->setLayout(gridLayout);

    // Add spacers around the grid layout to center it
    QHBoxLayout *hLayout = new QHBoxLayout();
    QSpacerItem *hSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *hSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addItem(hSpacer1);
    hLayout->addWidget(gridWidget);
    hLayout->addItem(hSpacer2);

    QVBoxLayout *vLayout = new QVBoxLayout();
    QSpacerItem *vSpacer1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSpacerItem *vSpacer2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vLayout->addItem(vSpacer1);
    vLayout->addLayout(hLayout);
    vLayout->addItem(vSpacer2);

    mainLayout->addLayout(vLayout);

    setCentralWidget(centralWidget);

    // Add restart button
    QPushButton *restartButton = new QPushButton("Restart", this);
    connect(restartButton, &QPushButton::clicked, this, &MainWindow::restartGame);
    mainLayout->addWidget(restartButton);

    // Add score label
    scoreLabel = new QLabel("Revealed Cells: 0", this);
    mainLayout->addWidget(scoreLabel);

    // Add hint button
    QPushButton *hintButton = new QPushButton("Hint", this);
    connect(hintButton, &QPushButton::clicked, this, &MainWindow::showHint);
    mainLayout->addWidget(hintButton);

    initializeGame();  // Initialize the game
}

// Destructor
MainWindow::~MainWindow()
{
}

// Initialize the game state
void MainWindow::initializeGame()
{
    revealedCells = 0; // Reset revealed cells count
    scoreLabel->setText("Revealed Cells: 0");

    cells.clear();  // Clear previous cells
    gameState.clear();  // Clear previous game state
    certainMines.clear();  // Clear certain mines

    cells.resize(rows);  // Resize cells vector
    gameState.resize(rows);  // Resize game state vector

    // Clear previous layout items if any
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // Initialize each cell
    for (int row = 0; row < rows; ++row) {
        cells[row].resize(cols);
        gameState[row].resize(cols);
        for (int col = 0; col < cols; ++col) {
            cells[row][col] = new FlagButton(this);  // Create new flag button
            cells[row][col]->setIcon(QIcon(":/assets/empty.png"));  // Set initial icon
            cells[row][col]->setFixedSize(15, 15);  // Set size
            cells[row][col]->setIconSize(QSize(15, 15)); // Ensure the icon size is set correctly
            cells[row][col]->setStyleSheet("QPushButton { border: none; margin: 0px; padding: 0px; }"); // Ensure no margins or padding
            gridLayout->addWidget(cells[row][col], row, col);
            connect(cells[row][col], &QPushButton::clicked, this, &MainWindow::handleCellClick);
            connect(qobject_cast<FlagButton*>(cells[row][col]), &FlagButton::rightClicked, this, &MainWindow::handleCellRightClick);
        }
    }

    placeMines();  // Place mines on the grid
    calculateNumbers();  // Calculate numbers for each cell
    findMinesRevealed();  // Update certain mines
}

// Place mines randomly on the grid
void MainWindow::placeMines()
{
    int placedMines = 0;
    while (placedMines < mines) {
        int row = QRandomGenerator::global()->bounded(rows);
        int col = QRandomGenerator::global()->bounded(cols);
        if (gameState[row][col] == -1) continue;  // Skip if already a mine
        gameState[row][col] = -1;  // Mark as mine
        placedMines++;
    }
}

// Calculate the number of mines surrounding each cell
void MainWindow::calculateNumbers()
{
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (gameState[row][col] == -1) continue;  // Skip mines
            int count = 0;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int r = row + i;
                    int c = col + j;
                    if (r >= 0 && r < rows && c >= 0 && c < cols && gameState[r][c] == -1) {
                        count++;
                    }
                }
            }
            gameState[row][col] = count;  // Set mine count
        }
    }
}

// Handle left-click on a cell
void MainWindow::handleCellClick()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    int row, col;
    for (row = 0; row < rows; ++row) {
        for (col = 0; col < cols; ++col) {
            if (cells[row][col] == button) {
                revealCell(row, col);  // Reveal the clicked cell
                return;
            }
        }
    }
}

// Handle right-click on a cell
void MainWindow::handleCellRightClick()
{
    FlagButton *button = qobject_cast<FlagButton *>(sender());
    int row, col;
    for (row = 0; row < rows; ++row) {
        for (col = 0; col < cols; ++col) {
            if (cells[row][col] == button) {
                if (!button->isFlagged) {
                    button->setIcon(QIcon(":/assets/flag.png"));  // Set flag icon
                } else {
                    button->setIcon(QIcon(":/assets/empty.png"));  // Remove flag icon
                }
                button->isFlagged = !button->isFlagged;  // Toggle flag state
                return;
            }
        }
    }
}

// Reveal a cell and handle game logic
void MainWindow::revealCell(int row, int col)
{
    if (gameState[row][col] == -1) {
        cells[row][col]->setIcon(QIcon(":/assets/mine.png"));
        endGame(false);  // End game if mine is revealed
        return;
    }

    QString iconPath = QString(":/assets/%1.png").arg(gameState[row][col]); // Get icon path for cell number
    cells[row][col]->setIcon(QIcon(iconPath));
    cells[row][col]->setFixedSize(15, 15);
    cells[row][col]->setIconSize(QSize(15, 15)); // Ensure the icon size is set correctly
    cells[row][col]->setStyleSheet("QPushButton { border: none; margin: 0px; padding: 0px; }"); // Ensure no margins or padding

    cells[row][col]->setEnabled(false);  // Disable the revealed cell
    revealedCells++;
    scoreLabel->setText("Revealed Cells: " + QString::number(revealedCells));

    if (gameState[row][col] == 0) {
        revealAdjacentCells(row, col);  // Reveal adjacent cells if empty
    }

    checkWinCondition();  // Check if the game is won
}

// Reveal adjacent cells if they are empty
void MainWindow::revealAdjacentCells(int row, int col)
{
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int r = row + i;
            int c = col + j;
            if (r >= 0 && r < rows && c >= 0 && c < cols && cells[r][c]->isEnabled()) {
                revealCell(r, c);  // Reveal adjacent cell
            }
        }
    }
}

// Check if the game is won
void MainWindow::checkWinCondition()
{
    if (revealedCells == rows * cols - mines) {
        endGame(true);  // End game with win condition
    }
}

// End the game and show message
void MainWindow::endGame(bool won)
{
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            cells[row][col]->setEnabled(false);  // Disable all cells
            if (gameState[row][col] == -1) {
                cells[row][col]->setIcon(QIcon(":/assets/mine.png"));
                cells[row][col]->setIconSize(QSize(15, 15)); // Ensure the icon size is set correctly
                cells[row][col]->setStyleSheet("QPushButton { border: none; margin: 0px; padding: 0px; }"); // Ensure no margins or padding
            }
        }
    }

    QString message = won ? "You won!" : "You lost!";
    QMessageBox::information(this, "Game Over", message);  // Show game over message
}

// Restart the game
void MainWindow::restartGame()
{
    initializeGame();  // Reinitialize the game
}

// Apply new settings for rows, columns, and mines
void MainWindow::applySettings()
{
    rows = rowsInput->value();
    cols = colsInput->value();
    mines = minesInput->value();

    if (mines > rows * cols) {
        QMessageBox::warning(this, "Invalid Input", "Number of mines cannot exceed total number of cells.");
        return;
    }

    minesInput->setMaximum(rows * cols);

    // Clear the old grid layout
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    cells.clear();  // Clear previous cells
    gameState.clear();  // Clear previous game state
    cells.resize(rows);  // Resize cells vector
    gameState.resize(rows);  // Resize game state vector
    for (int row = 0; row < rows; ++row) {
        cells[row].resize(cols);
        gameState[row].resize(cols);
        for (int col = 0; col < cols; ++col) {
            cells[row][col] = new FlagButton(this);
            cells[row][col]->setFixedSize(40, 40);
            cells[row][col]->setIconSize(QSize(40, 40)); // Ensure the icon size is set correctly
            cells[row][col]->setStyleSheet("QPushButton { border: none; margin: 0px; padding: 0px; }"); // Ensure no margins or padding
            gridLayout->addWidget(cells[row][col], row, col);
            connect(cells[row][col], &QPushButton::clicked, this, &MainWindow::handleCellClick);
            connect(qobject_cast<FlagButton*>(cells[row][col]), &FlagButton::rightClicked, this, &MainWindow::handleCellRightClick);
        }
    }

    initializeGame();  // Initialize the game with new settings
}

// Find certain mines based on revealed cells
void MainWindow::findMinesRevealed() {
    certainMines.clear();  // Clear previous certain mines

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (gameState[row][col] == -1 || gameState[row][col] == 0) {
                continue;
            }
            std::vector<std::pair<int, int>> unrevealedSquares;
            int val = gameState[row][col];
            int count = 0;

            // Check surrounding cells
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int newRow = row + i;
                    int newCol = col + j;
                    if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                        if (cells[newRow][newCol]->isEnabled()) {
                            count++;
                            unrevealedSquares.push_back({newRow, newCol});
                        }
                    }
                }
            }
            // If the number of unrevealed surrounding cells equals the cell's value, mark them as certain mines
            if (val == count) {
                for (const auto &cell : unrevealedSquares) {
                    certainMines.push_back(cell);
                }
            }
        }
    }
}

// Find a safe cell for the hint mechanism
std::pair<int, int> MainWindow::findHint() {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (gameState[row][col] > 0 && gameState[row][col] <= 8) {  // Check only numbered cells
                auto safeCells = findSafeCells(row, col);
                if (!safeCells.empty()) {
                    return safeCells.front();  // Return the first safe cell found
                }
            }
        }
    }
    return {-1, -1};  // No safe cell found
}

// Find safe cells around a given cell
std::vector<std::pair<int, int>> MainWindow::findSafeCells(int row, int col) {
    std::vector<std::pair<int, int>> safeCells;
    int surroundingMines = 0;
    std::vector<std::pair<int, int>> unrevealedCells;
    // Check surrounding cells
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int newRow = row + i;
            int newCol = col + j;
            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                if (std::find(certainMines.begin(), certainMines.end(), std::make_pair(newRow, newCol)) != certainMines.end()) {
                    surroundingMines++;
                } else if (gameState[newRow][newCol] != -1 && cells[newRow][newCol]->isEnabled()) {
                    unrevealedCells.push_back({newRow, newCol});
                }
            }
        }
    }
    // If the number of surrounding mines equals the cell's number, all other unrevealed cells are safe
    if (surroundingMines == gameState[row][col]) {
        safeCells = unrevealedCells;
    }
    return safeCells;
}

// Show a hint to the player
void MainWindow::showHint() {
    findMinesRevealed();  // Update certain mines before providing a hint

    if (lastHint.first != -1 && lastHint.second != -1) {
        // Reveal the last hint cell if the hint button is pressed again
        revealCell(lastHint.first, lastHint.second);
        lastHint = {-1, -1};  // Reset lastHint
        return;
    }

    auto hint = findHint();
    if (hint.first != -1 && hint.second != -1) {
        // Highlight the hint cell using hint.png asset
        cells[hint.first][hint.second]->setIcon(QIcon(":/assets/hint.png"));
        lastHint = hint;  // Store the hint cell
    } else {
        // Show a message that no safe hint is available
        QMessageBox::information(this, "Hint", "No safe move available.");
        lastHint = {-1, -1};  // Reset lastHint
    }
}
