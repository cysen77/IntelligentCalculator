#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calculator.h"
#include <QDebug>
#include <QVector>
#include <QMessageBox>
#include <QStack>
#include <QSet>
#include <cmath>
#include <QTimer>
#include <QThread>
#include <QtMath>
#include <QComboBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <Q3DSurface>
#include <QRadioButton>
#include <QPainter>
#include <QPushButton>
#include <QtDataVisualization/QSurface3DSeries>
#include <QFileDialog>
#include <QProcessEnvironment>
#include <QLineEdit>




class MainWindow::variableObjectPointer
{
public:
    QComboBox* combobox;
    QLineEdit* lineedit;
    QSlider* slider;
    variableObjectPointer(QComboBox* cbb,QLineEdit* le,QSlider* sld)
    {
        combobox = cbb;
        lineedit = le;
        slider = sld;
    }
};

class MainWindow::functionObjectPointer
{
public:
    QLineEdit* functionedit;
    QPushButton* plotbutton;

    functionObjectPointer(QLineEdit*functionedit,QPushButton*plotbutton)
    {
        this->functionedit = functionedit;
        this->plotbutton = plotbutton;
    }
};

class MainWindow::expressionObjectPointer
{
public:
    QLineEdit* expressionedit;
    QLabel* answerlabel;

    expressionObjectPointer(QLineEdit*expressionedit,QLabel*answerlabel)
    {
        this->expressionedit = expressionedit;
        this->answerlabel = answerlabel;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(850,660);
    //设置滑块的范围
    value_variable = new QMap<QString,double>();
    functions = new QMap<QString,QString>();

    model1 = new QStandardItemModel();
    model1->setColumnCount(2);
    model1->setHeaderData(0,Qt::Horizontal,"变量名");
    model1->setHeaderData(1,Qt::Horizontal,"值");

    ui->tableView->setModel(model1);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);



    //给这个变量随机的初始的值

    value_variable->operator[]("a")=rand()%100000/double(5000);
    value_variable->operator[]("b")=(rand()%200000-100000)/double(5000);
    value_variable->operator[]("c")=(rand()%200000-100000)/double(5000);


    variableobjects.push_back(variableObjectPointer(ui->varcomboBox1,ui->varedit1,ui->varslider1));
    variableobjects.push_back(variableObjectPointer(ui->varcomboBox2,ui->varedit2,ui->varslider2));
    variableobjects.push_back(variableObjectPointer(ui->varcomboBox3,ui->varedit3,ui->varslider3));
    variableobjects.push_back(variableObjectPointer(ui->varcomboBox4,ui->varedit4,ui->varslider4));
    variableobjects.push_back(variableObjectPointer(ui->varcomboBox5,ui->varedit5,ui->varslider5));
    variableobjects.push_back(variableObjectPointer(ui->varcomboBox6,ui->varedit6,ui->varslider6));

    functionobjects.push_back(functionObjectPointer(ui->functionfedit,ui->plotfbtn));
    functionobjects.push_back(functionObjectPointer(ui->functiongedit,ui->plotgbtn));
    functionobjects.push_back(functionObjectPointer(ui->functionhedit,ui->plothbtn));

    expressionobjects.push_back(expressionObjectPointer(ui->m0edit,ui->m0anslabel));
    expressionobjects.push_back(expressionObjectPointer(ui->m1edit,ui->m1anslabel));
    expressionobjects.push_back(expressionObjectPointer(ui->m2edit,ui->m2anslabel));

    for(int i =0;i<6;i++)
    {
        variableobjects[i].slider->setRange(-100000,100000);
    }


    update();


    //连接滑块的滑动信号和槽
    //注意要用valuehanged,因为用moved或者press或者release会没反应
    //因为哪个行编辑也可以改变这个滑块的值
    for(int i =0;i<6;i++)
    {
        variableObjectPointer current = variableobjects[i];
        connect(current.slider,&QSlider::valueChanged ,this,[=]()
        {
            if(current.combobox->currentText()=="")return;
            //首先记录下这个滑块的内容
            double val = current.slider->value();
            //更新变量列表中的对应的值
            value_variable->operator[](current.combobox->currentText()) = val/5000;
            //修改对应行编辑的内容
            current.lineedit->setText(QVariant(val/5000).toString());

            //调用这个计算并且输出结果到窗口的函数
            update();

            for(int j =0;j<6;j++)
            {
                if(j==i)continue;
                if(current.combobox->currentText()==variableobjects[j].combobox->currentText())
                {
                    variableobjects[j].slider->setValue(val);
                }
            }
            tableviewlocate(current.combobox->currentText());

        });
    }

    for(int i = 0;i<6;i++)
    {

        variableObjectPointer current = variableobjects[i];
        //连接这个变量输入完成的信号到这个槽
        connect(current.lineedit,&QLineEdit::editingFinished,this,[=]()
        {
            if(current.combobox->currentText()=="")return;

            //记录这个行编辑的内容并且把它变成
            double val = current.lineedit->text().toDouble();
            //更新滑块的值
            current.slider->setValue(val*5000);
            //调用这个计算并且输出结果到窗口的函数
            update();

            for(int j =0;j<6;j++)
            {
                if(i==j)continue;
                if(current.combobox->currentText()==variableobjects[j].combobox->currentText())
                {
                    variableobjects[j].slider->setValue(val*5000);
                }
            }
            tableviewlocate(current.combobox->currentText());

        });
    }

    for(int i =0;i<3;i++)
    {
        functionObjectPointer current = functionobjects[i];

        connect(current.plotbutton,&QPushButton::clicked,this,[=]()
        {
            ui->stackedWidget->setCurrentIndex(1);
            ui->functioneditpage2->setText(current.functionedit->text());

        });
        connect(current.functionedit,&QLineEdit::textChanged,this,[=]()
        {
            QString w = current.functionedit->text().mid(0,1);
            functions->operator[](w)=current.functionedit->text();
            update();
        });
        functions->operator[](current.functionedit->text().mid(0,1))=current.functionedit->text();

    }


    for(int i =0; i<6; i++)
    {
        variableObjectPointer current = variableobjects[i];
        connect(current.combobox,&QComboBox::currentTextChanged,this,[=]()
        {
            QString varname = current.combobox->currentText();
            double val = value_variable->operator[](varname);
            current.slider->setValue(val*5000);
            tableviewlocate(current.combobox->currentText());

        });
    }



    //表达式和自定义函数每更改都要计算并且把结果输出到窗口
    for(int i =0;i<3;i++)
    {
        connect(expressionobjects[i].expressionedit,&QLineEdit::editingFinished,this,[=](){update();});
    }



    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    update();

    for(int i =0;i<6;i++)
    {
        double val =variableobjects[i].combobox->currentText().toDouble();
        variableobjects[i].lineedit->setText(QVariant(val).toString());
        variableobjects[i].slider->setValue(val);
    }

    for(int i =0;i<3;i++)
    {
        variableobjects[i].combobox->setCurrentIndex(i);
    }
    makepage2();
    update();
}

void MainWindow::update()
{
    QVector<float>answers = {};
    QSet<int>e={};


    //用来记录计算的结果的列表

    for(int i=0;i<3;i++)
    {

        Calculator* calculator = new Calculator(expressionobjects[i].expressionedit->text(),
                                                value_variable,functions);
        double answer = calculator->solve();
        if(calculator->status!=1)e.insert(i);

        QSet<QString>comboboxitems = {};
        for(int i =0;i<ui->varcomboBox1->count();i++)
        {
            comboboxitems.insert(ui->varcomboBox1->itemText(i));
        }

        for(QMap<QString,double>::iterator it = value_variable->begin();it!=value_variable->end();it++)
        {
            QString text = it.key();
            if(comboboxitems.find(text)!=comboboxitems.end())continue;
            for(int i =0;i<6;i++)
            {
                variableobjects[i].combobox->addItem(text);
            }
        }

        answers.push_back(answer);
        delete calculator;

    }

    //把结果显示到窗口
    for(int i =0;i<3;i++)
    {
        if(e.find(i)==e.end())expressionobjects[i].answerlabel->setText("= "+QVariant(answers[i]).toString());
        else if(expressionobjects[i].expressionedit->text()=="")expressionobjects[i].answerlabel->setText("");
        else expressionobjects[i].answerlabel->setText("计算失败");
    }



    model1->clear();
    model1 = new QStandardItemModel();
    model1->setColumnCount(2);
    model1->setHeaderData(0,Qt::Horizontal,QString::fromLocal8Bit("Name"));
    model1->setHeaderData(1,Qt::Horizontal,QString::fromLocal8Bit("Value"));


    for(QMap<QString,double>::iterator it = value_variable->begin();it!=value_variable->end();it++)
    {
        int row = model1->rowCount();
        model1->insertRow(model1->rowCount());
        model1->setData(model1->index(row,0),it.key());
        model1->setData(model1->index(row,1),it.value());

    }

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->setModel(model1);
}


void MainWindow::makepage2()
{
    ui->selectfunction->setCurrentIndex(0);
    connect(ui->selectfunction,&QComboBox::currentIndexChanged,this,[=]()
    {
        this->setWindowTitle("智能代数运算系统        正在计算，暂时动不了");



        if(ui->selectfunction->currentIndex()==1)ui->functioneditpage2->setText(ui->functionfedit->text());
        if(ui->selectfunction->currentIndex()==2)ui->functioneditpage2->setText(ui->functiongedit->text());
        if(ui->selectfunction->currentIndex()==3)ui->functioneditpage2->setText(ui->functionhedit->text());
        if(ui->selectfunction->currentIndex()==0)ui->functioneditpage2->setText("");
        QString expression = ui->functioneditpage2->text();
        graph->removeSeries(series);
        modifier = new SurfaceGraph(graph,value_variable,expression,series);
        this->setWindowTitle("智能代数运算系统");

    });
    connect(ui->functioneditpage2,&QLineEdit::textChanged,this,[=]()
    {
        QString text = ui->functioneditpage2->text();
        ui->selectfunction->setCurrentIndex(0);
        if(text=="")return;
        for(int i =0;i<3;i++)
        {
            if(text==functionobjects[i].functionedit->text())ui->selectfunction->setCurrentIndex(i);
        }


    });

    connect(ui->returntopage1btn,&QPushButton::clicked,this,[=]()
    {ui->stackedWidget->setCurrentIndex(0);});
    connect(ui->functioneditpage2,&QLineEdit::editingFinished,this,[=]()
    {

        this->setWindowTitle("智能代数运算系统        正在计算，暂时动不了");
        QString expression = ui->functioneditpage2->text();
        graph->removeSeries(series);
        modifier = new SurfaceGraph(graph,value_variable,expression,series);
        this->setWindowTitle("智能代数运算系统");
    });


    series = new QSurface3DSeries();
    graph = new Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(graph);
    //! [0]

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
    }

    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);


    QWidget *widget = MainWindow::ui->page2mainwidget;
    hLayout = new QHBoxLayout(widget);
    vLayout = new QVBoxLayout();
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);
    vLayout->setAlignment(Qt::AlignTop);


    QGroupBox *selectionGroupBox = new QGroupBox(QStringLiteral("Selection Mode"));

    QRadioButton *modeNoneRB = new QRadioButton(widget);
    modeNoneRB->setText(QStringLiteral("No selection"));
    modeNoneRB->setChecked(false);

    QRadioButton *modeItemRB = new QRadioButton(widget);
    modeItemRB->setText(QStringLiteral("Item"));
    modeItemRB->setChecked(false);

    QRadioButton *modeSliceRowRB = new QRadioButton(widget);
    modeSliceRowRB->setText(QStringLiteral("Row Slice"));
    modeSliceRowRB->setChecked(false);

    QRadioButton *modeSliceColumnRB = new QRadioButton(widget);
    modeSliceColumnRB->setText(QStringLiteral("Column Slice"));
    modeSliceColumnRB->setChecked(false);

    QVBoxLayout *selectionVBox = new QVBoxLayout;
    selectionVBox->addWidget(modeNoneRB);
    selectionVBox->addWidget(modeItemRB);
    selectionVBox->addWidget(modeSliceRowRB);
    selectionVBox->addWidget(modeSliceColumnRB);
    selectionGroupBox->setLayout(selectionVBox);

    QSlider *axisMinSliderX = new QSlider(Qt::Horizontal, widget);
    axisMinSliderX->setMinimum(0);
    axisMinSliderX->setTickInterval(1);
    axisMinSliderX->setEnabled(true);
    QSlider *axisMaxSliderX = new QSlider(Qt::Horizontal, widget);
    axisMaxSliderX->setMinimum(1);
    axisMaxSliderX->setTickInterval(1);
    axisMaxSliderX->setEnabled(true);
    QSlider *axisMinSliderZ = new QSlider(Qt::Horizontal, widget);
    axisMinSliderZ->setMinimum(0);
    axisMinSliderZ->setTickInterval(1);
    axisMinSliderZ->setEnabled(true);
    QSlider *axisMaxSliderZ = new QSlider(Qt::Horizontal, widget);
    axisMaxSliderZ->setMinimum(1);
    axisMaxSliderZ->setTickInterval(1);
    axisMaxSliderZ->setEnabled(true);

    QComboBox *themeList = new QComboBox(widget);
    themeList->addItem(QStringLiteral("Qt"));
    themeList->addItem(QStringLiteral("Primary Colors"));
    themeList->addItem(QStringLiteral("Digia"));
    themeList->addItem(QStringLiteral("Stone Moss"));
    themeList->addItem(QStringLiteral("Army Blue"));
    themeList->addItem(QStringLiteral("Retro"));
    themeList->addItem(QStringLiteral("Ebony"));
    themeList->addItem(QStringLiteral("Isabelle"));

    QGroupBox *colorGroupBox = new QGroupBox();

    QLinearGradient grBtoY(0, 0, 1, 100);
    grBtoY.setColorAt(1.0, Qt::black);
    grBtoY.setColorAt(0.67, Qt::blue);
    grBtoY.setColorAt(0.33, Qt::red);
    grBtoY.setColorAt(0.0, Qt::yellow);
    QPixmap pm(24, 100);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(grBtoY));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 24, 100);
    QPushButton *gradientBtoYPB = new QPushButton(widget);
    gradientBtoYPB->setIcon(QIcon(pm));
    gradientBtoYPB->setIconSize(QSize(24, 100));

    QLinearGradient grGtoR(0, 0, 1, 100);
    grGtoR.setColorAt(1.0, Qt::darkGreen);
    grGtoR.setColorAt(0.5, Qt::yellow);
    grGtoR.setColorAt(0.2, Qt::red);
    grGtoR.setColorAt(0.0, Qt::darkRed);
    pmp.setBrush(QBrush(grGtoR));
    pmp.drawRect(0, 0, 24, 100);
    QPushButton *gradientGtoRPB = new QPushButton(widget);
    gradientGtoRPB->setIcon(QIcon(pm));
    gradientGtoRPB->setIconSize(QSize(24, 100));

    QHBoxLayout *colorHBox = new QHBoxLayout;
    colorHBox->addWidget(gradientBtoYPB);
    colorHBox->addWidget(gradientGtoRPB);
    colorGroupBox->setLayout(colorHBox);

    QPushButton *saveButton = new QPushButton(widget);
    saveButton->setText(QStringLiteral("Save as image"));

    vLayout->addWidget(selectionGroupBox);
    vLayout->addWidget(new QLabel(QStringLiteral("Column range")));
    vLayout->addWidget(axisMinSliderX);
    vLayout->addWidget(axisMaxSliderX);
    vLayout->addWidget(new QLabel(QStringLiteral("Row range")));
    vLayout->addWidget(axisMinSliderZ);
    vLayout->addWidget(axisMaxSliderZ);
    vLayout->addWidget(new QLabel(QStringLiteral("Theme")));
    vLayout->addWidget(themeList);
    vLayout->addWidget(colorGroupBox);
    vLayout->addWidget(saveButton);



    QString expression = ui->functioneditpage2->text();
    modifier = new SurfaceGraph(graph,value_variable,expression,series);



    QObject::connect(modeItemRB,  &QRadioButton::toggled,
                     modifier, &SurfaceGraph::toggleModeItem);
    QObject::connect(modeSliceRowRB,  &QRadioButton::toggled,
                     modifier, &SurfaceGraph::toggleModeSliceRow);
    QObject::connect(modeSliceColumnRB,  &QRadioButton::toggled,
                     modifier, &SurfaceGraph::toggleModeSliceColumn);
    QObject::connect(axisMinSliderX, &QSlider::valueChanged,
                     modifier, &SurfaceGraph::adjustXMin);
    QObject::connect(axisMaxSliderX, &QSlider::valueChanged,
                     modifier, &SurfaceGraph::adjustXMax);
    QObject::connect(axisMinSliderZ, &QSlider::valueChanged,
                     modifier, &SurfaceGraph::adjustZMin);
    QObject::connect(axisMaxSliderZ, &QSlider::valueChanged,
                     modifier, &SurfaceGraph::adjustZMax);
    QObject::connect(themeList, SIGNAL(currentIndexChanged(int)),
                     modifier, SLOT(changeTheme(int)));
    QObject::connect(gradientBtoYPB, &QPushButton::pressed,
                     modifier, &SurfaceGraph::setBlackToYellowGradient);
    QObject::connect(gradientGtoRPB, &QPushButton::pressed,
                     modifier, &SurfaceGraph::setGreenToRedGradient);

    connect(saveButton,&QPushButton::clicked,this,[=]()
    {
        QImage image = graph->renderToImage();
        QString defaultname = ui->functioneditpage2->text()+".png";
        for(int i =0;i<defaultname.size();i++)
        {
            if(defaultname[i]=='/')
            {
                defaultname = "functionimage.png";
                break;
            }
        }
        QString defaultpath = QProcessEnvironment::systemEnvironment().value("USERPROFILE")+"\\Desktop\\"+defaultname;
        QString path = QFileDialog::getSaveFileName(nullptr,"",defaultpath);
        image.save(path);
    });

    modifier->setAxisMinSliderX(axisMinSliderX);
    modifier->setAxisMaxSliderX(axisMaxSliderX);
    modifier->setAxisMinSliderZ(axisMinSliderZ);
    modifier->setAxisMaxSliderZ(axisMaxSliderZ);

    modeItemRB->setChecked(true);
    themeList->setCurrentIndex(2);
}

void MainWindow::tableviewlocate(QString target)
{
    int tablesize = model1->rowCount();
    int locate = 0;
    for(;locate<tablesize&&model1->data(model1->index(locate,0)).toString()!=target;locate++);
    QModelIndex locateindex = model1->index(locate,1);
    ui->tableView->setCurrentIndex(locateindex);
}
MainWindow::~MainWindow()
{
    delete ui;
}
