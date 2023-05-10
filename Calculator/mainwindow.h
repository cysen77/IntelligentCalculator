#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QVector>
#include <QSet>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "qstandarditemmodel.h"
#include "qtableview.h"
#include "surfacegraph.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    class variableObjectPointer;
    class functionObjectPointer;
    class expressionObjectPointer;
    QMap<QString,double>*value_variable;
    QMap<QString,QString>*functions;

    QVector<variableObjectPointer>variableobjects;
    QVector<functionObjectPointer>functionobjects;
    QVector<expressionObjectPointer>expressionobjects;

    void update();
    void tableviewlocate(QString target);

    //page2object
    void makepage2();
    SurfaceGraph *modifier;
    QHBoxLayout *hLayout;
    QVBoxLayout *vLayout;
    Q3DSurface *graph;
    QStandardItemModel* model1;
    QSurface3DSeries *series;
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
