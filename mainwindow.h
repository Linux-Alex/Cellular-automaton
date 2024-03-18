#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QTimer>
#include <QRandomGenerator>
#include <algorithm>
#include <QRandomGenerator>
#include <QMessageBox>
#include "cell_2d.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_toolBox_currentChanged(int index);

    void on_btnStart_clicked();

    void on_sliderGenerationSpeed_valueChanged(int value);

    void on_btnStop_clicked();

    void on_txt2dAutomatonViewWidth_textChanged(const QString &arg1);

    void on_txt2dAutomatonViewHeight_textChanged(const QString &arg1);

    void on_btnAutoResize_clicked();

    void on_txt2dAutomatonSurviveRules_textChanged(const QString &arg1);

    void on_txt2dAutomatonBornRules_textChanged(const QString &arg1);

    void on_slider2dAutomatonGenerationValue_valueChanged(int value);

    void on_btn2dAutomatonGenerate_clicked();

    void on_txt1dAutomatonSurviveRules_textChanged(const QString &arg1);

    void on_txt1dAutomatonBornRules_textChanged(const QString &arg1);

    void on_txt1dAutomatonViewWidth_textChanged(const QString &arg1);

    void on_slider1dAutomatonGenerationValue_valueChanged(int value);

    void on_btn1dAutomatonGenerate_clicked();

    void on_btn2dAutomatonSample1_clicked();

    void on_btn2dAutomatonSample2_clicked();

    void on_btnMaterialsGenerateNewCave_clicked();

    void on_listMaterialsSelect_currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    QLabel* statusMessage;
    QLabel* statusIcon;
    bool enableStatus = false;

    CellType gameMode;
    int selectedMode;
    int generationSpeed;

    QGraphicsScene *scene;
    QVector<QVector<Cell_2D*>> grid;
    QTimer *timer;
    int cellRows = 64;
    int cellColumns = 64;

    QList<int> surviveRules;
    QList<int> bornRules;
    int generationRatio;

    void readFormData();
    QList<int> readRuleList(QString ruleString);
    void updateGrid();
    void updateCaveGrid();
    void initializeGrid();
    int countNeighbors(int x, int y);
    void adjustCellSize();
    QVector<int> generateUniqueRandomNumbers(int n, int min, int max);
    void generateRandomGridValues();
    void generateRandomCave();
    QString gridToString();
    QList<Material> getCaveNeighbours(int x, int y);
};
#endif // MAINWINDOW_H
