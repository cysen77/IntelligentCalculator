// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "surfacegraph.h"
#include <QMessageBox>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include "mainwindow.h"
#include "calculator.h"


const int sampleCountX = 100;
const int sampleCountZ = 100;

const float sampleMin = -20.0f;
const float sampleMax = 20.0f;



SurfaceGraph::SurfaceGraph(Q3DSurface *surface,QMap<QString,double>*value_variable,
                           QString expression, QSurface3DSeries *&series)
    : m_graph(surface)
{
    this->value_variable = value_variable;
    this->expression = expression;
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    m_Proxy = new QSurfaceDataProxy();
    m_Series = new QSurface3DSeries(m_Proxy);
    fillExpressionProxy();
    m_graph->axisX()->setLabelFormat("%.1f N");
    m_graph->axisZ()->setLabelFormat("%.1f E");
    m_graph->axisX()->setRange(-40.0f, 40.0f);
    m_graph->axisY()->setAutoAdjustRange(true);
    m_graph->axisZ()->setRange(-24.0f, 24.0f);


    m_graph->removeSeries(m_Series);
    m_Series->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    m_Series->setFlatShadingEnabled(true);

    m_graph->axisX()->setLabelFormat("%.2f");
    m_graph->axisZ()->setLabelFormat("%.2f");
    m_graph->axisX()->setRange(sampleMin, sampleMax);
    m_graph->axisY()->setRange(-20.0f, 20.0f);
    m_graph->axisZ()->setRange(sampleMin, sampleMax);
    m_graph->axisX()->setLabelAutoRotation(30);
    m_graph->axisY()->setLabelAutoRotation(90);
    m_graph->axisZ()->setLabelAutoRotation(30);

    m_graph->removeSeries(m_Series);

    m_graph->addSeries(m_Series);

    m_rangeMinX = sampleMin;
    m_rangeMinZ = sampleMin;
    m_stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    m_stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);
    series =  m_Series;
}

SurfaceGraph::~SurfaceGraph()
{
    delete m_graph;
}

void SurfaceGraph::fillExpressionProxy()
{
    float stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    float stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;

    //如果ui中那个行编辑内容是空，就什么都不做
    if(expression.size()==0)
    {
        m_Proxy->resetArray(dataArray);
        return;
    }

    Calculator calculator1(expression,value_variable,nullptr);
    calculator1.split(calculator1.expression,false);

    QVector<QVector<QString>>sr = calculator1.split(calculator1.expression,false);
    //等号分成左右两部分
    QVector<QVector<QString>>splitleft;
    QVector<QVector<QString>>splitright;

    QString splitleftstring = "";
    QString splitrightstring = "";
    //用等号分割函数的表达式
    int i =0;
    for(;i<sr.size();i++)
    {
        if(sr[i][0]=="=")break;
        splitleft.push_back(sr[i]);
        splitleftstring += " " + sr[i][0] + " ";
    }
    i++;
    if(i>=expression.size())
    {
        m_Proxy->resetArray(dataArray);
        return;
    }
    for(;i<sr.size();i++)
    {
        splitright.push_back(sr[i]);
        splitrightstring += " " + sr[i][0] + " ";

    }

    Calculator calculator2(splitleftstring,value_variable,nullptr);
    calculator2.split(calculator2.expression,false);
    int index = 0;
    calculator2.buildtree(calculator2.headnode,index,false);

    int num_parameter = calculator2.headnode->childs[0]->childs.size();
    if(2<num_parameter)
    {
        m_Proxy->resetArray(dataArray);
        QMessageBox::warning(nullptr,"输入错误","参数数量不允许超过2个");
        return;
    }


    QMap<QString,QString>m;

    dataArray->reserve(sampleCountZ);
    for (int i = 0 ; i < sampleCountZ ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
        // Keep values within range bounds, since just adding step can cause minor drift due
        // to the rounding error1s.
        float z = qMin(sampleMax, (i * stepZ + sampleMin));
        int index = 0;
        //然后映射
        if(num_parameter>0)m[calculator2.headnode->childs[0]->childs[0]->content]=QVariant(z).toString();
        for (int j = 0; j < sampleCountX; j++) {
            float x = qMin(sampleMax, (j * stepX + sampleMin));


            if(num_parameter>1)m[calculator2.headnode->childs[0]->childs[1]->content]=QVariant(x).toString();

            //然后做出映射后的新的表达式，就是从原来函数右边的那个表达式，改变它的参数名
            QString newcode = "";
            for(int i =0;i<splitright.size();i++)
            {
                QString p = splitright[i][0];
                if(m.find(p)!=m.end())
                {
                    p = "(" + m[p] + ")";
                }
                newcode +=" " + p + " ";
            }
            newcode+=" ";
            //计算这个新表达式的值

            Calculator calculator3(newcode,value_variable,nullptr);


            double y = calculator3.solve();
            if(calculator3.status==-1)
            {
                y = INFINITY;
            }

            (*newRow)[index++].setPosition(QVector3D(x, y, z));

        }
        *dataArray << newRow;
    }

    m_Proxy->resetArray(dataArray);
    m_graph->removeSeries(m_Series);

    m_graph->addSeries(m_Series);

}



void SurfaceGraph::adjustXMin(int min)
{
    float minX = m_stepX * float(min) + m_rangeMinX;

    int max = m_axisMaxSliderX->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderX->setValue(max);
    }
    float maxX = m_stepX * max + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraph::adjustXMax(int max)
{
    float maxX = m_stepX * float(max) + m_rangeMinX;

    int min = m_axisMinSliderX->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderX->setValue(min);
    }
    float minX = m_stepX * min + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraph::adjustZMin(int min)
{
    float minZ = m_stepZ * float(min) + m_rangeMinZ;

    int max = m_axisMaxSliderZ->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderZ->setValue(max);
    }
    float maxZ = m_stepZ * max + m_rangeMinZ;

    setAxisZRange(minZ, maxZ);
}

void SurfaceGraph::adjustZMax(int max)
{
    float maxX = m_stepZ * float(max) + m_rangeMinZ;

    int min = m_axisMinSliderZ->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderZ->setValue(min);
    }
    float minX = m_stepZ * min + m_rangeMinZ;

    setAxisZRange(minX, maxX);
}

void SurfaceGraph::setAxisXRange(float min, float max)
{
    m_graph->axisX()->setRange(min, max);
}

void SurfaceGraph::setAxisZRange(float min, float max)
{
    m_graph->axisZ()->setRange(min, max);
}

void SurfaceGraph::changeTheme(int theme)
{
    m_graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}

void SurfaceGraph::setBlackToYellowGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void SurfaceGraph::setGreenToRedGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.8, Qt::red);
    gr.setColorAt(1.0, Qt::darkRed);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}


