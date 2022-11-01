#include "OptionMenuPlotter.h"
//#include "ui_OptionMenuPlotter.h"
#include <QColorDialog>
#include <QGraphicsScene>
#include <QPainter>

OptionMenuPlotter::OptionMenuPlotter(QWidget *parent) :
  QWidget(parent)//, ui(new Ui::OptionMenuPlotter)
  {
 // ui->setupUi(this);
  ChartToSet.clear();
//  HideAllSettings();
  QGraphicsScene* Scene=new QGraphicsScene();
//  ui->graphicsView->setScene(Scene);
  }
/*
OptionMenuPlotter::~OptionMenuPlotter()
  {
  delete ui;
  }
*/
void OptionMenuPlotter::on_select_color_btn_clicked()
  {
  QColor color = QColorDialog::getColor();
  if (color.isValid() )
    {
    switch (m_SelectColorToSet)
      {
      case 0:{ChartToSet.Background=color;break;}
      case 1:{ChartToSet.BackgroundGraph=color;break;}
      case 2:{ChartToSet.GraphColor=color;break;}
      case 4:{ChartToSet.BreakPointColor=color;break;}
      case 5:{ChartToSet.AxisColorX=color;break;}
      case 6:{ChartToSet.AxisColorY=color;break;}
      default:break;
      }
//    ui->graphicsView->setBackgroundBrush(QBrush(color));
  }
}


void OptionMenuPlotter::on_ok_btn_clicked()
  {
  ChartToSet.GraphPen.setWidth(ChartToSet.ThinknessGraph);
  ChartToSet.GraphPen.setColor(ChartToSet.GraphColor);
  ChartToSet.AxisXPen.setWidth(ChartToSet.ThinknessAxisX);
  ChartToSet.AxisXPen.setColor(ChartToSet.AxisColorX);
  ChartToSet.AxisYPen.setWidth(ChartToSet.ThinknessAxisY);
  ChartToSet.AxisYPen.setColor(ChartToSet.AxisColorY);
  ChartToSet.isChange=true;
  emit sendDataClass();
  this->hide();
  }


void OptionMenuPlotter::on_cancel_btn_clicked()
  {
  this->hide();
  }


void OptionMenuPlotter::on_font_box_currentFontChanged(const QFont &f)
{
    switch (m_SelectColorToSet)
    {
    case 5:{ChartToSet.FontAxisY=f;break;}
    case 6:{ChartToSet.FontAxisX=f;break;}
    default:{ChartToSet.GraphFont=f; break;}
    }
}


void OptionMenuPlotter::on_thinkness_valueChanged(const QString &arg1)
  {
  switch (m_SelectColorToSet)
    {
    case 4:{ChartToSet.ThinknessBreakPoint=arg1.toInt();break;}
    case 5:{ChartToSet.ThinknessAxisX=arg1.toInt();break;}
    case 6:{ChartToSet.ThinknessAxisY=arg1.toInt();break;}
    default: {ChartToSet.ThinknessGraph=arg1.toInt(); break;}
    }
  }

void SettingsChart::clear()
  {
  isChange=false;
  ThinknessAxisX=1;
  ThinknessAxisY=1;
  ThinknessBreakPoint=8;
  ThinknessGraph=1;
  Background=QColor(Qt::white);
  BackgroundGraph=QColor(Qt::white);
  BreakPointColor=QColor(Qt::red);
  GraphColor=QColor(Qt::black);
  AxisColorX=QColor(Qt::black);
  AxisColorY=QColor(Qt::black);
  GraphFont=QFont("Verdana",7);
  FontAxisX=QFont("Verdana",7);
  FontAxisY=QFont("Arial",7);
  GraphPen.setColor(GraphColor);
  GraphPen.setWidth(ThinknessGraph);
  AxisXPen.setColor(AxisColorX);
  AxisXPen.setWidth(ThinknessAxisX);
  AxisYPen.setColor(AxisColorY);
  AxisYPen.setWidth(ThinknessAxisY);
  }

/*
void OptionMenuPlotter::on_object_to_set_currentIndexChanged(int index)
{
    m_SelectColorToSet=index;
    HideAllSettings();
    switch (m_SelectColorToSet)
      {
      case 0:{ui->graphicsView->setBackgroundBrush(QBrush(ChartToSet.Background));break;}
      case 1:{ui->graphicsView->setBackgroundBrush(QBrush(ChartToSet.BackgroundGraph));break;}
      case 2:
        {
        ui->graphicsView->setBackgroundBrush(QBrush(ChartToSet.GraphColor));
        ui->font->setText("Font of title Graph");
        ui->thinkness->setMinimum(1);
        ui->thinkness->setValue(ChartToSet.ThinknessGraph);
        ui->thinkness->setMaximum(5);
        break;
        }
        case 4:
        {
        ui->graphicsView->setBackgroundBrush(QBrush(ChartToSet.BreakPointColor));
        ui->thinkness->setMinimum(5);
        ui->thinkness->setValue(ChartToSet.ThinknessBreakPoint);
        ui->thinkness->setMaximum(10);
        break;
        }
        case 5:
        {
        ui->graphicsView->setBackgroundBrush(QBrush(ChartToSet.AxisColorX));
        ui->thinkness->setMinimum(1);
        ui->thinkness->setValue(ChartToSet.ThinknessAxisX);
        ui->thinkness->setMaximum(5);
        ui->font->setText("Font of title Axis X");
        break;
        }
        case 6:
        {
        ui->graphicsView->setBackgroundBrush(QBrush(ChartToSet.AxisColorY));
        ui->thinkness->setMinimum(1);
        ui->thinkness->setValue(ChartToSet.ThinknessAxisY);
        ui->thinkness->setMaximum(5);
        ui->font->setText("Font of title Axis Y");
        break;
        }
        default:break;
    }
    if(m_SelectColorToSet==5 ||m_SelectColorToSet==6)
    {
        ui->thinkness->show();
        ui->label_thinkness->show();
        ui->font->show();
        ui->font_box->show();
        ui->pixel->show();
     }
    if(m_SelectColorToSet==2 || m_SelectColorToSet==4)
      {
      ui->thinkness->show();
      ui->label_thinkness->show();
      ui->font->show();
      ui->font_box->show();
      ui->pixel->show();
     }
}

void OptionMenuPlotter::HideAllSettings()
{
    ui->thinkness->setMinimum(1);
    ui->thinkness->setMaximum(5);
    ui->thinkness->hide();
    ui->label_thinkness->hide();
    ui->font->hide();
    ui->font_box->hide();
    ui->pixel->hide();
    ui->retranslateUi(this);
}
*/

