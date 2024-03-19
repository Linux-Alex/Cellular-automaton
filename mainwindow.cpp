#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Customized status display
    qApp->setProperty("mainWindow", QVariant::fromValue<QWidget *>(this));
    if(this->enableStatus)
        qInstallMessageHandler(this->messageHandler);

    // Statusbar
    this->statusIcon = new QLabel(this);
    this->statusIcon->setObjectName("lblStatusIcon");
    this->statusIcon->setFixedHeight(16);
    this->statusIcon->setFixedWidth(16);
    this->statusIcon->setScaledContents(true);
    ui->statusbar->addWidget(this->statusIcon);

    this->statusMessage = new QLabel(this);
    this->statusMessage->setObjectName("lblStatusMessage");
    ui->statusbar->addWidget(this->statusMessage);

    // Timer
    timer = new QTimer(this);

    // Default values
    this->selectedMode = 1;

    this->generationRatio = 50;
    ui->slider2dAutomatonGenerationValue->setValue(this->generationRatio);

    // Set the scene
    this->scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(this->scene);

    this->initializeGrid(); // Initialize the grid
    this->updateGrid();

    ui->toolBox->setCurrentIndex(this->selectedMode);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateGrid);


    qDebug() << "[INFO] MainWindow initialized.";
}

MainWindow::~MainWindow()
{
    delete ui;

    delete this->timer;
    delete this->scene;
    delete this->statusMessage;
    delete this->statusIcon;
}

void MainWindow::on_toolBox_currentChanged(int index)
{
    this->selectedMode = index;
    qDebug() << "[INFO] Selected mode changed to:" << index;

    this->readFormData();
    qDebug() << "Tukaj dela.";

    this->timer->stop();
    this->initializeGrid();
    this->updateGrid();

    if(this->selectedMode == 3) {
        this->generateRandomCave();
        this->gameMode = CellType::CaveAutomaton;
    }
    else {
        this->gameMode = CellType::GameOfLife;
    }
}

// Method to update the grid based on Game of Life rules
void MainWindow::updateGrid() {
    if(selectedMode == 0 || selectedMode == 1 || (selectedMode == 3 && this->gameMode == CellType::GameOfLife)) {
        QVector<QVector<bool>> nextGen(grid.size(), QVector<bool>(grid[0].size(), false));

        for (int i = 0; i < grid.size(); ++i) {
            for (int j = 0; j < grid[i].size(); ++j) {
                int neighbors = countNeighbors(i, j);

                if (grid[i][j]->isAlive) {
                    // nextGen[i][j] = (neighbors == 2 || neighbors == 3);
                    nextGen[i][j] = this->surviveRules.contains(neighbors);
                } else {
                    // nextGen[i][j] = (neighbors == 3);
                    nextGen[i][j] = this->bornRules.contains(neighbors);
                }
            }
        }

        this->adjustCellSize();

        for (int i = 0; i < grid.size(); ++i) {
            for (int j = 0; j < grid[i].size(); ++j) {
                grid[i][j]->setState(nextGen[i][j]);

                // set mode
                this->grid[i][j]->cellType = this->gameMode;
            }
        }
    }
    else if(selectedMode == 2) {

    }
    else if(selectedMode == 3 && this->gameMode == CellType::CaveAutomaton) {
        this->updateCaveGrid();
    }

}

void MainWindow::updateCaveGrid()
{
    if(this->grid[0][0]->cellType == CellType::CaveAutomaton) {
        QVector<QVector<Cell_2D*>> nextGen(grid.size(), QVector<Cell_2D*>(grid[0].size(), nullptr));

        for(int i = 0; i < grid.size(); ++i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                // Copying
                nextGen[i][j] = new Cell_2D(nullptr);
                nextGen[i][j]->setMaterial(this->grid[i][j]->material, this->grid[i][j]->waterDensity);
                nextGen[i][j]->setAge(this->grid[i][j]->age);
            }
        }

        // All that is going down
        for(int i = grid.size() -1; i >= 0; --i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                QList<Material> neighbours = getCaveNeighbours(j, i);

                // Sand
                if(grid[i][j]->material == Material::sand) {
                    if(neighbours[2] == Material::air){
                        nextGen[i][j]->setMaterial(Material::air);
                        this->grid[i][j]->setMaterial(Material::air);
                        nextGen[i+1][j]->setMaterial(Material::sand);
                        // qDebug() << neighbours;
                    }
                    else if(neighbours[2] == Material::water) {
                        nextGen[i][j]->setMaterial(this->grid[i+1][j]->material, this->grid[i+1][j]->waterDensity);
                        nextGen[i+1][j]->setMaterial(this->grid[i][j]->material, this->grid[i][j]->waterDensity);
                        this->grid[i+1][j]->setMaterial(this->grid[i][j]->material, this->grid[i][j]->waterDensity);
                    }
                    else {
                        int randomNumber = QRandomGenerator::global()->bounded(0, 1+1);
                        if(randomNumber == 0 && (neighbours[1] == Material::air /*|| neighbours[i] == Material::water*/)) {
                            nextGen[i][j]->setMaterial(Material::air);
                            this->grid[i][j]->setMaterial(Material::air);
                            nextGen[i+1][j+1]->setMaterial(Material::sand);
                            // nextGen[i][j]->setMaterial(this->grid[i+1][j+1]->material, this->grid[i+1][j+1]->waterDensity);
                            // nextGen[i+1][j+1]->setMaterial(this->grid[i][j]->material, this->grid[i][j]->waterDensity);
                            // this->grid[i+1][j+1]->setMaterial(this->grid[i][j]->material, this->grid[i][j]->waterDensity);
                        }
                        else if(neighbours[3] == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            this->grid[i][j]->setMaterial(Material::air);
                            nextGen[i+1][j-1]->setMaterial(Material::sand);
                        }
                        else if(neighbours[1] == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            this->grid[i][j]->setMaterial(Material::air);
                            nextGen[i+1][j+1]->setMaterial(Material::sand);
                        }
                    }
                }

                // Wood
                if(grid[i][j]->material == Material::wood) {
                    // if fire is near
                    if(neighbours.contains(Material::fire)) {
                        nextGen[i][j]->setMaterial(Material::darkSmoke);
                    }
                    else if(neighbours[2] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        this->grid[i][j]->setMaterial(Material::air);
                        nextGen[i+1][j]->setMaterial(Material::wood);
                    }
                }

                // Fire
                if(grid[i][j]->material == Material::fire) {
                    if(neighbours[2] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        this->grid[i][j]->setMaterial(Material::air);
                        nextGen[i+1][j]->setMaterial(Material::fire);
                    }
                    else {
                        this->grid[i][j]->setMaterial(Material::lightSmoke);
                        nextGen[i][j]->setMaterial(Material::lightSmoke);
                    }
                }

                //Water
                if(grid[i][j]->material == Material::water) {
                    if(neighbours[2] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        this->grid[i][j]->setMaterial(Material::air);
                        nextGen[i+1][j]->setMaterial(Material::water, nextGen[i][j]->waterDensity);
                    }
                    else if(neighbours[2] == Material::water && nextGen[i+1][j]->waterDensity+1 < Cell_2D::waterColors.count()) {
                        nextGen[i][j]->setMaterial(Material::water, nextGen[i][j]->waterDensity - 1);
                        nextGen[i+1][j]->setMaterial(Material::water, nextGen[i+1][j]->waterDensity + 1 );
                    }
                    else if(neighbours[2] == Material::wood) {
                        nextGen[i+1][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity);
                        nextGen[i][j]->setMaterial(grid[i+1][j]->material);
                    }
                    else if(neighbours[1] == Material::wood && neighbours[0] == Material::air) {
                        nextGen[i][j+1]->setMaterial(grid[i+1][j+1]->material);
                        nextGen[i+1][j+1]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity);
                        nextGen[i][j]->setMaterial(Material::air);
                    }
                    else if(neighbours[3] == Material::wood && neighbours[4] == Material::air) {
                        nextGen[i][j-1]->setMaterial(grid[i+1][j-1]->material);
                        nextGen[i+1][j-1]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity);
                        nextGen[i][j]->setMaterial(Material::air);
                    }
                    else if(neighbours[1] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        this->grid[i][j]->setMaterial(Material::air);
                        nextGen[i+1][j+1]->setMaterial(Material::water, nextGen[i][j]->waterDensity);
                    }
                    else if(neighbours[3] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        this->grid[i][j]->setMaterial(Material::air);
                        nextGen[i+1][j-1]->setMaterial(Material::water, nextGen[i][j]->waterDensity);
                    }
                    else if(neighbours[1] == Material::water && nextGen[i+1][j+1]->waterDensity+1 < Cell_2D::waterColors.count()) {
                        nextGen[i][j]->setMaterial(Material::water, nextGen[i][j]->waterDensity - 1);
                        nextGen[i+1][j+1]->setMaterial(Material::water, nextGen[i+1][j+1]->waterDensity + 1);
                    }
                    else if(neighbours[3] == Material::water && nextGen[i+1][j-1]->waterDensity+1 < Cell_2D::waterColors.count()) {
                        nextGen[i][j]->setMaterial(Material::water, nextGen[i][j]->waterDensity - 1);
                        nextGen[i+1][j-1]->setMaterial(Material::water, nextGen[i+1][j-1]->waterDensity + 1);
                    }
                }
            }
        }

        for(int i = 0; i < grid.size(); ++i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                this->grid[i][j]->setMaterial(nextGen[i][j]->material, nextGen[i][j]->waterDensity);
                this->grid[i][j]->setAge(nextGen[i][j]->age);
            }
        }

        // All that is going up
        for(int i = 0; i < grid.size(); ++i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                QList<Material> neighbours = getCaveNeighbours(j, i);

                // Dark smoke
                if(grid[i][j]->material == Material::darkSmoke) {
                    if(neighbours[6] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        nextGen[i-1][j]->setMaterial(Material::darkSmoke);
                        nextGen[i-1][j]->setAge(this->grid[i][j]->age);
                        this->grid[i][j]->setMaterial(Material::air);
                    }
                    else {
                        int randomNumber = QRandomGenerator::global()->bounded(0, 1+1);
                        if(randomNumber == 0 && neighbours[0] == Material::air && nextGen[i-1][j]->material == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            nextGen[i][j+1]->setMaterial(Material::darkSmoke);
                            nextGen[i][j+1]->setAge(this->grid[i][j]->age);
                            this->grid[i][j]->setMaterial(Material::air);
                        }
                        else if(neighbours[4] == Material::air && nextGen[i][j-1]->material == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            nextGen[i][j-1]->setMaterial(Material::darkSmoke);
                            nextGen[i][j-1]->setAge(this->grid[i][j]->age);
                            this->grid[i][j]->setMaterial(Material::air);
                        }
                        else if(neighbours[0] == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            nextGen[i][j+1]->setAge(this->grid[i][j]->age);
                            nextGen[i][j+1]->setMaterial(Material::darkSmoke);
                            this->grid[i][j]->setMaterial(Material::air);
                        }
                    }
                }

                // Light smoke
                if(grid[i][j]->material == Material::lightSmoke) {
                    if(neighbours[6] == Material::air) {
                        nextGen[i][j]->setMaterial(Material::air);
                        nextGen[i-1][j]->setMaterial(Material::lightSmoke);
                        nextGen[i-1][j]->setAge(this->grid[i][j]->age);
                        this->grid[i][j]->setMaterial(Material::air);
                    }
                    else {
                        int randomNumber = QRandomGenerator::global()->bounded(0, 1+1);
                        if(randomNumber == 0 && neighbours[0] == Material::air && nextGen[i-1][j]->material == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            nextGen[i][j+1]->setMaterial(Material::lightSmoke);
                            nextGen[i][j+1]->setAge(this->grid[i][j]->age);
                            this->grid[i][j]->setMaterial(Material::air);
                        }
                        else if(neighbours[4] == Material::air && nextGen[i][j-1]->material == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            nextGen[i][j-1]->setMaterial(Material::lightSmoke);
                            nextGen[i][j-1]->setAge(this->grid[i][j]->age);
                            this->grid[i][j]->setMaterial(Material::air);
                        }
                        else if(neighbours[0] == Material::air) {
                            nextGen[i][j]->setMaterial(Material::air);
                            nextGen[i][j+1]->setMaterial(Material::lightSmoke);
                            nextGen[i][j+1]->setAge(this->grid[i][j]->age);
                            this->grid[i][j]->setMaterial(Material::air);
                        }
                    }
                }
            }
        }

        for(int i = 0; i < grid.size(); ++i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                this->grid[i][j]->setMaterial(nextGen[i][j]->material, nextGen[i][j]->waterDensity);
                this->grid[i][j]->setAge(nextGen[i][j]->age);
            }
        }

        // Water spilling
        for(int i = 0; i < grid.size(); ++i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                QList<Material> neighbours = getCaveNeighbours(j, i);

                if(grid[i][j]->material == Material::water) {
                    if(grid[i][j]->waterDensity == 0) {
                        continue;
                    }

                    // qDebug() << neighbours;

                    // random between 0 and 1
                    int randomNumber = QRandomGenerator::global()->bounded(0, 1+1);
                    if(randomNumber == 0 && (neighbours[0] == Material::air || (neighbours[0] == Material::water && grid[i][j+1]->waterDensity < grid[i][j]->waterDensity))) {
                        nextGen[i][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity - 1);
                        nextGen[i][j+1]->setMaterial(grid[i][j]->material, (grid[i][j+1]->material == Material::water) ? grid[i][j+1]->waterDensity + 1 : 0);

                        // grid[i][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity - 1);
                    }
                    else if(neighbours[4] == Material::air || (neighbours[4] == Material::water && nextGen[i][j-1]->waterDensity < grid[i][j]->waterDensity)) {
                        nextGen[i][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity - 1);
                        nextGen[i][j-1]->setMaterial(grid[i][j]->material, (nextGen[i][j-1]->material == Material::water) ? nextGen[i][j-1]->waterDensity + 1 : 0);

                        // grid[i][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity - 1);
                    }
                    else if(neighbours[0] == Material::air || (neighbours[0] == Material::water && grid[i][j+1]->waterDensity < grid[i][j]->waterDensity)) {
                        nextGen[i][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity - 1);
                        nextGen[i][j+1]->setMaterial(grid[i][j]->material, (grid[i][j+1]->material == Material::water) ? grid[i][j+1]->waterDensity + 1 : 0);

                        // grid[i][j]->setMaterial(grid[i][j]->material, grid[i][j]->waterDensity - 1);
                    }
                }
            }
        }

        // update material
        for(int i = 0; i < grid.size(); ++i) {
            for(int j = 0; j < grid[i].size(); ++j) {
                this->grid[i][j]->setMaterial(nextGen[i][j]->material, nextGen[i][j]->waterDensity);
                this->grid[i][j]->setAge(nextGen[i][j]->age);

                if(this->grid[i][j]->material == Material::darkSmoke || this->grid[i][j]->material == Material::lightSmoke) {
                    this->grid[i][j]->ageMaterial();
                }
                else {
                    this->grid[i][j]->setAge(0);
                }
            }
        }
    }
}

// Method to initialize the grid
void MainWindow::initializeGrid() {
    this->scene->clear();
    grid.clear();
    this->scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(this->scene);

    if(this->selectedMode == 0) {
        qDebug() << ui->graphicsView->width() << this->cellColumns;
        int cellSize = qMax(ui->graphicsView->width(), this->cellColumns) / cellColumns;

        cellSize = qMax(10, cellSize);


        grid.resize(1);
        for(int i = 0; i < 1; ++i) {
            grid[i].resize(cellColumns);
            for(int j = 0; j < cellColumns; ++j) {
                Cell_2D *cell = new Cell_2D(nullptr);
                cell->setRect(0, 0, cellSize, cellSize);
                cell->setPos(j * cellSize, 0);
                scene->addItem(cell);
                grid[i][j] = cell;
            }
        }
    }
    if(this->selectedMode == 1) {
        int cellSize = qMin(
            qMax(ui->graphicsView->width(), this->cellColumns) / cellColumns,
            qMax(ui->graphicsView->height(), this->cellRows) / cellRows
        );

        cellSize = qMax(10, cellSize);

        grid.resize(cellRows);
        for (int i = 0; i < cellRows; ++i) {
            grid[i].resize(cellColumns);
            for (int j = 0; j < cellColumns; ++j) {
                Cell_2D *cell = new Cell_2D(nullptr);
                cell->setRect(0, 0, cellSize, cellSize);
                cell->setPos(j * cellSize, i * cellSize); // Set cell position
                scene->addItem(cell);
                grid[i][j] = cell;
            }
        }
    }
    if(this->selectedMode == 2) {
        QMessageBox::information(this, "Informacije", "Ta metoda je združena z funkcionalnostimi peska, lesa, ognja in dimov.");
        ui->toolBox->setCurrentIndex(3);
    }
    if(this->selectedMode == 3) {
        int cellSize = qMin(
            qMax(ui->graphicsView->width(), this->cellColumns) / cellColumns,
            qMax(ui->graphicsView->height(), this->cellRows) / cellRows
            );

        cellSize = qMax(10, cellSize);

        grid.resize(cellRows);
        for (int i = 0; i < cellRows; ++i) {
            grid[i].resize(cellColumns);
            for (int j = 0; j < cellColumns; ++j) {
                Cell_2D *cell = new Cell_2D(nullptr);
                cell->setRect(0, 0, cellSize, cellSize);
                cell->setPos(j * cellSize, i * cellSize); // Set cell position
                scene->addItem(cell);
                grid[i][j] = cell;
            }
        }

        this->gameMode = CellType::CaveAutomaton;
    }
}

int MainWindow::countNeighbors(int x, int y) {
    int count = 0;
    int rows = grid.size();
    int cols = grid[0].size();

    // Define offsets for neighboring cells
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 8; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && nx < rows && ny >= 0 && ny < cols && grid[nx][ny]->isAlive) {
            count++;
        }
    }

    return count;
}


void MainWindow::on_btnStart_clicked()
{
    // Calculate speed based on the value of the slider
    this->on_sliderGenerationSpeed_valueChanged(ui->sliderGenerationSpeed->value());

    this->timer->start();

    // Get values from input boxes
    // this->on_txt2dAutomatonBornRules_textChanged(ui->txt2dAutomatonBornRules->text());
    // this->on_txt2dAutomatonSurviveRules_textChanged(ui->txt2dAutomatonSurviveRules->text());

    qDebug() << "[INFO] Grid sizes:" << this->cellRows << "," << this->cellColumns;
}



void MainWindow::on_sliderGenerationSpeed_valueChanged(int value)
{
    this->generationSpeed = 1000 - value;
    this->timer->setInterval(this->generationSpeed);
}


void MainWindow::on_btnStop_clicked()
{
    this->timer->stop();
}


void MainWindow::adjustCellSize() {
    try {
        ui->graphicsView->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
    } catch (const std::exception& e){
        qWarning() << "[ERROR] Can't adjust cell size: " << e.what();
    }
}


void MainWindow::on_txt2dAutomatonViewWidth_textChanged(const QString &arg1)
{
    try {
        this->cellColumns = arg1.toInt();
        if(arg1.toInt() < 1)
            this->cellColumns = 1;
        if(arg1 == "")
            this->cellColumns = 64;
        this->scene->clear();
        this->scene = new QGraphicsScene(this);
        ui->graphicsView->setScene(this->scene);
        this->initializeGrid();

        this->updateGrid();
    } catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}


void MainWindow::on_txt2dAutomatonViewHeight_textChanged(const QString &arg1)
{
    try {
        this->cellRows = arg1.toInt();
        if(this->cellRows < 1)
            this->cellRows = 1;
        if(arg1 == "")
            this->cellRows = 64;
        qDebug() << "Scene rect:" << this->scene->sceneRect();
        this->scene->clear();
        this->scene = new QGraphicsScene(this);
        ui->graphicsView->setScene(this->scene);
        this->initializeGrid();

        ui->graphicsView->update();

        this->updateGrid();
        qDebug() << "Scene rect:" << this->scene->sceneRect();
    } catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}


void MainWindow::on_btnAutoResize_clicked()
{
    this->adjustCellSize();
}



void MainWindow::on_txt2dAutomatonSurviveRules_textChanged(const QString &arg1)
{
    try {
        this->surviveRules = this->readRuleList(arg1);
    }
    catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}


void MainWindow::on_txt2dAutomatonBornRules_textChanged(const QString &arg1)
{
    try {
        this->bornRules = this->readRuleList(arg1);
    }
    catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}


void MainWindow::on_slider2dAutomatonGenerationValue_valueChanged(int value)
{
    this->generationRatio = value;
}


void MainWindow::on_btn2dAutomatonGenerate_clicked()
{
    this->generateRandomGridValues();
}


QVector<int> MainWindow::generateUniqueRandomNumbers(int n, int min, int max) {
    QVector<int> numbers;
    for (int i = min; i <= max; ++i) {
        numbers.push_back(i);
    }
    std::random_shuffle(numbers.begin(), numbers.end());

    QVector<int> result;
    for (int i = 0; i < n; ++i) {
        result.push_back(numbers[i]);
    }

    return result;
}

void MainWindow::generateRandomGridValues()
{
    // Set all values to black
    for(int i = 0; i < this->cellRows; i++) {
        for(int j = 0; j < this->cellColumns; j++) {
            this->grid[i][j]->setState(false);
        }
    }

    int numberOfAllCells = this->cellRows * this->cellColumns;
    int numberOfAliveCells = (this->generationRatio * numberOfAllCells) / 100;

    // Generate a list of unique random numbers from 0 to numberOfAllCells
    QVector<int> randomNumbers = generateUniqueRandomNumbers(numberOfAliveCells, 0, numberOfAllCells - 1);

    // Set the cells to alive based on the random numbers
    for(int i = 0; i < randomNumbers.size(); i++) {
        int row = randomNumbers[i] / this->cellColumns;
        int col = randomNumbers[i] % this->cellColumns;
        this->grid[row][col]->setState(true);
    }

    qDebug() << "[INFO] Successfully generated a new grid with " << numberOfAliveCells << " alive cells.";
}

void MainWindow::generateRandomCave()
{
    this->bornRules = {6, 7, 8};
    this->surviveRules = {2, 3, 4, 5, 6, 7, 8};

    this->generationRatio = QRandomGenerator::global()->bounded(40, 50+1);
    qDebug() << "Randomly generated generation ratio:" << this->generationRatio;
    this->generateRandomGridValues();

    int i = 0;

    QString oldCave;
    this->gameMode = CellType::GameOfLife;
    do {
        oldCave = this->gridToString();
        this->updateGrid();
        i++;
    } while (oldCave != this->gridToString());
    this->gameMode = CellType::CaveAutomaton;

    // transform the grid into a cave
    for(int i = 0; i < this->cellRows; i++) {
        for(int j = 0; j < this->cellColumns; j++) {
            if(this->grid[i][j]->isAlive) {
                this->grid[i][j]->setMaterial(Material::air);
            }
            else {
                this->grid[i][j]->setMaterial(Material::wall);
            }
        }
    }

    qDebug() << "[INFO] Successfully generated a new cave with " << i << " generations.";
}

QString MainWindow::gridToString()
{
    QString result = "";
    for(int i = 0; i < this->cellRows; i++) {
        for(int j = 0; j < this->cellColumns; j++) {
            result += this->grid[i][j]->isAlive ? "1" : "0";
        }
        result += "\n";
    }
    return result;
}

QList<Material> MainWindow::getCaveNeighbours(int x, int y)
{
    QList<Material> neighbours;
    // get all 8 neighbours from this->grid
    QList<int> dx = QList<int>{ 1, 1, 0, -1, -1, -1, 0, 1 };
    QList<int> dy = QList<int>{ 0, 1, 1, 1, 0, -1, -1, -1 };

    for(int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if(nx >= 0 && nx < this->cellColumns && ny >= 0 && ny < this->cellRows) {
            neighbours.append(this->grid[ny][nx]->material);
        }
        else {
            neighbours.append(Material::border);
        }
    }

    return neighbours;

}

void MainWindow::readFormData() {
    if(this->selectedMode == 0) {
        this->cellRows = 1;
        try {
            this->on_txt1dAutomatonBornRules_textChanged(ui->txt1dAutomatonBornRules->text());
            this->on_txt1dAutomatonSurviveRules_textChanged(ui->txt1dAutomatonSurviveRules->text());
            this->on_txt1dAutomatonViewWidth_textChanged(ui->txt1dAutomatonViewWidth->text());
            this->on_slider1dAutomatonGenerationValue_valueChanged(ui->slider1dAutomatonGenerationValue->value());
        } catch (const std::exception& e) {
            qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
        }
    }
    else if(this->selectedMode == 1) {
        try {
            this->on_txt2dAutomatonBornRules_textChanged(ui->txt2dAutomatonBornRules->text());
            this->on_txt2dAutomatonSurviveRules_textChanged(ui->txt2dAutomatonSurviveRules->text());
            this->on_txt2dAutomatonViewWidth_textChanged(ui->txt2dAutomatonViewWidth->text());
            this->on_txt2dAutomatonViewHeight_textChanged(ui->txt2dAutomatonViewHeight->text());
            this->on_slider2dAutomatonGenerationValue_valueChanged(ui->slider2dAutomatonGenerationValue->value());
        } catch (const std::exception& e) {
            qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
        }
    }
    else if(this->selectedMode == 2) {

    }
    else if(this->selectedMode == 3) {
        try {
            this->on_listMaterialsSelect_currentRowChanged(ui->listMaterialsSelect->currentRow());
        } catch (const std::exception& e) {
            qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
        }
    }
}

QList<int> MainWindow::readRuleList(QString ruleString)
{
    QList<int> rules;
    QStringList rulesString = ruleString.split("");
    try {
        bool status;
        for(int i = 0; i < rulesString.size(); ++i) {
            if(rulesString[i] != "") {
                int dec = rulesString[i].toInt(&status);
                if(status == false)
                    throw std::invalid_argument("Invalid argument");
                rules.append(dec);
            }
        }
    }
    catch(const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }

    return rules;
}



void MainWindow::on_txt1dAutomatonSurviveRules_textChanged(const QString &arg1)
{
    try {
        this->surviveRules = this->readRuleList(arg1);
    }
    catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}


void MainWindow::on_txt1dAutomatonBornRules_textChanged(const QString &arg1)
{
    try {
        this->bornRules = this->readRuleList(arg1);
    }
    catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}

void MainWindow::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Update the label with the latest message
    QLabel *label = qobject_cast<QMainWindow *>(qApp->property("mainWindow").value<QWidget *>())->findChild<QLabel *>("lblStatusMessage");
    QLabel *icon = qobject_cast<QMainWindow *>(qApp->property("mainWindow").value<QWidget *>())->findChild<QLabel *>("lblStatusIcon");
    if (label && icon) {
        QString logMessage;
        if(type == QtDebugMsg) {
            label->setText(msg);
            icon->setPixmap(QPixmap(":/icons/Resources/checkbox.svg"));
        }
        else if(type == QtWarningMsg) {
            label->setText(msg);
            icon->setPixmap(QPixmap(":/icons/Resources/error.svg"));
        }
        else {
            label->setText(msg);
            icon->clear();
        }
    }
}



void MainWindow::on_txt1dAutomatonViewWidth_textChanged(const QString &arg1)
{
    try {
        if(arg1 == "") {
            this->cellColumns = 64;
        }
        else {

        bool status;
        int val = arg1.toInt(&status);

        if(!status)
            throw std::invalid_argument("Invalid argument, string can't convert to numebr");

        this->cellColumns = qMax(1, val);
        }

    }
    catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }

    this->initializeGrid();
    ui->graphicsView->update();
    this->updateGrid();
}


void MainWindow::on_slider1dAutomatonGenerationValue_valueChanged(int value)
{
    this->generationRatio = value;
}


void MainWindow::on_btn1dAutomatonGenerate_clicked()
{
    this->generateRandomGridValues();
}


void MainWindow::on_btn2dAutomatonSample1_clicked()
{
    ui->txt2dAutomatonSurviveRules->setText("23");
    ui->txt2dAutomatonBornRules->setText("3");

    ui->slider2dAutomatonGenerationValue->setValue(50);
}


void MainWindow::on_btn2dAutomatonSample2_clicked()
{
    ui->txt2dAutomatonSurviveRules->setText("2345678");
    ui->txt2dAutomatonBornRules->setText("678");

    // random number between 40 and 50
    int randomNumber = QRandomGenerator::global()->bounded(40, 50+1);
    ui->slider2dAutomatonGenerationValue->setValue(randomNumber);
}


void MainWindow::on_btnMaterialsGenerateNewCave_clicked()
{
    this->generateRandomCave();
}


void MainWindow::on_listMaterialsSelect_currentRowChanged(int currentRow)
{
    try {
        if(currentRow == -1) {
            // Exception
            throw std::runtime_error("Invalid material selected.");
        }
        // get text value
        QString material = ui->listMaterialsSelect->item(currentRow)->text();
        if(material == "Pesek") {
            Cell_2D::selectedMaterial = Material::sand;
        }
        else if(material == "Les") {
            Cell_2D::selectedMaterial = Material::wood;
        }
        else if(material == "Ogenj") {
            Cell_2D::selectedMaterial = Material::fire;
        }
        else if(material == "Temen dim") {
            Cell_2D::selectedMaterial = Material::darkSmoke;
        }
        else if(material == "Svetel dim") {
            Cell_2D::selectedMaterial = Material::lightSmoke;
        }
        else if(material == "Voda") {
            Cell_2D::selectedMaterial = Material::water;
        }
    }
    catch (const std::exception& e) {
        qWarning() << "[ERROR] Can't transform the value to an integer: " << e.what();
    }
}

