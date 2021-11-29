#include "Multiplot.h"

//#include <OptionMenuPlotter.h>
#include <Parser.h>
#include <QPainter>
#include <QColorDialog>

PlotData::PlotData( const QByteArray& Formula, QString Label, QColor Color, MultiPlotter &Plotter ) : m_Formula(Formula),
  m_Color(Color), m_LineWidth(1), m_Label(Label), m_pCursor(new QScatterSeries), m_pLinesCursor(new QLineSeries)
  {
  if( m_Formula.isEmpty() || m_Formula.contains("/0") ) throw QString("Formula is empty");
  int iEq = m_Formula.indexOf('=');
  if(iEq != -1)
    {
    m_Label = Formula.left(iEq);
    m_Formula = m_Formula.mid(iEq + 1);
    }
  m_Font.setBold(true);
  m_Formula = m_Formula.toLower();
  if(!m_Formula.contains('x')) throw QString("Bad formula:") + m_Formula;
  m_pCursor->setColor(Qt::blue);
  m_pCursor->setBorderColor(Qt::blue);
  m_pCursor->setMarkerSize(8);
  QPen PenLine(Qt::red);
  PenLine.setWidth(1);
  m_pLinesCursor->setPen(PenLine);
  Plotter.m_pChart->addSeries(m_pCursor);
  Plotter.m_pChart->addSeries(m_pLinesCursor);
  }

void PlotData::setAxisName(QGraphicsTextItem* pAxisName, int x, int y)
  {
  m_pAxisName = pAxisName;
  m_pAxisName->setFont(m_Font);
  m_pAxisName->setPos(x, y);
  m_pAxisName->setDefaultTextColor(m_Color);
  }

QLabel* PlotData::AddValueFunc()
  {
  m_pValueFunc = new  QLabel;
  m_pValueFunc->setAlignment(Qt::AlignHCenter);
  m_pValueFunc->setStyleSheet( "QLabel {font-size:16px}" );
  return m_pValueFunc;
  }

void PlotData::ShowValue(int Prec, int index)
  {
  m_pValueFunc->setText(QString::number(m_Points[index].y(),10,Prec));
  }

QPointF PlotData::SetCursor( MultiPlotter &Plotter, int index)
  {
  m_pLinesCursor->clear();
  m_pCursor->clear();
  QPointF Point(m_Points[index]);
  if(Point.y() != Plotter.cm_Break && index != 0)
    {
    m_pLinesCursor->attachAxis(Plotter.m_pValueAxisY);
    m_pLinesCursor->attachAxis(Plotter.m_pValueAxisX);
    m_pLinesCursor->append(Point.x(), 0);
    m_pLinesCursor->append(Point);
    m_pLinesCursor->append(0, Point.y());//
    }
  m_pCursor->attachAxis(Plotter.m_pValueAxisY);
  m_pCursor->attachAxis(Plotter.m_pValueAxisX);
  m_pCursor->append(Point);
  return Point;
  }

const  unsigned Colors[15] = {0,0xff0000,0x00ff00,0x0000ff,0xffff00,0x00ffff,0xff00ff,0x008080,0x800080,0x000080,0x008000,0x800000};

MultiPlotter::MultiPlotter(const QByteArray& Formula) : QWidget(nullptr), m_Formula(Formula), m_iCurrentPoint(0), m_pPathAsymptote(nullptr)
  {
/*
  m_pUi=new Ui::Plotter;
  m_pUi->setupUi(this);

  connect(m_pPlotterMenu,&OptionMenuPlotter::sendDataClass,this,&Plotter::on_SetChartSettings);
*/
  resize(721, 580);
  setWindowTitle("Graph");
  setWindowIcon( QIcon( ":/Resources/plotter.png" ) );
  QByteArrayList Formuls(Formula.split(';'));
  if(Formuls.count() == 1)
    m_Plots.append(new PlotData(Formula, "Y", Qt::black, *this ) );
  else
    for(int i = 0; i < Formuls.count(); i++)
      m_Plots.append(new PlotData(Formuls[i], "Y" + QString::number(i + 1), Colors[i], *this ) );
  m_pGraphicsView = new QGraphicsView;
  m_pGraphicsView->setFixedSize(580,580);
  m_pGraphicsView->setFrameShape(QFrame::WinPanel);
  m_pGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_pGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QHBoxLayout *pLayout = new QHBoxLayout;
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->addWidget( m_pGraphicsView );
  QFrame *pFrame = new QFrame;
  pFrame->setFixedHeight(150);
  pFrame->setFrameShape(QFrame::StyledPanel);
  QVBoxLayout *pFrameLayout = new QVBoxLayout(pFrame);
  QLabel *pInterval = new QLabel("Interval");
  pInterval->setAlignment(Qt::AlignHCenter);
  pInterval->setStyleSheet( "QLabel {font-size:16px}" );
  pFrameLayout->addWidget(pInterval);
  m_pXMin = new QDoubleSpinBox;
  m_pXMax = new QDoubleSpinBox;
  m_pYMin = new QDoubleSpinBox;
  m_pYMax = new QDoubleSpinBox;
  m_pXMin->setMinimum(-100000);
  m_pXMax->setMaximum(100000);
  m_pYMax->setMaximum(10000000);
  m_pYMin->setMinimum(-10000000);
  m_pXMin->setValue(-10);
  m_pXMax->setValue(10);
  connect(m_pXMin, SIGNAL(valueChanged(double)), SLOT(on_xmin_valueChanged(double)));
  connect(m_pXMax, SIGNAL(valueChanged(double)), SLOT(on_xmax_valueChanged(double)));
  connect(m_pYMin, SIGNAL(valueChanged(double)), SLOT(on_ymin_valueChanged(double)));
  connect(m_pYMax, SIGNAL(valueChanged(double)), SLOT(on_ymax_valueChanged(double)));
  QFormLayout *pFLayout = new QFormLayout(pFrame);
  pFLayout->addRow("Xmin", m_pXMin);
  pFLayout->addRow("Xmax", m_pXMax);
  pFLayout->addRow("Ymin", m_pYMin);
  pFLayout->addRow("Ymax", m_pYMax);
  pFrameLayout->addLayout(pFLayout);
  QVBoxLayout *pVlayout = new QVBoxLayout;
  pVlayout->addWidget(pFrame);
  m_pRefresh = new QPushButton("Refresh");
  m_pRefresh->setEnabled(false);
  connect(m_pRefresh, SIGNAL(clicked()), SLOT(on_ResetPressed()));
  pVlayout->addWidget(m_pRefresh);
  QLabel *pCurrval = new QLabel("Current Value");
  pCurrval->setAlignment(Qt::AlignHCenter);
  pCurrval->setStyleSheet( "QLabel {font-size:16px}" );
  pVlayout->addWidget(pCurrval);
  m_pCurSliderValue = new QSlider;
  m_pCurSliderValue->setOrientation(Qt::Horizontal);
  m_pCurSliderValue->setFixedSize(120,25);
  connect(m_pCurSliderValue, SIGNAL(valueChanged(int)), SLOT(on_cur_val_slider_valueChanged(int)));
  pVlayout->addWidget(m_pCurSliderValue);
  QLabel *pX = new QLabel("X: ");
  pX->setAlignment(Qt::AlignHCenter);
  pX->setStyleSheet( "QLabel {font-size:16px}" );
  QHBoxLayout *pHlayout = new QHBoxLayout;
  pHlayout->addWidget(pX);
  m_pValueX = new QLabel;
  m_pValueX->setAlignment(Qt::AlignLeft);
  m_pValueX->setStyleSheet( "QLabel {font-size:16px}" );
  pHlayout->addWidget(m_pValueX);
  pVlayout->addLayout(pHlayout);
  QLabel *pY = new QLabel("Y1");
  pY->setAlignment(Qt::AlignHCenter);
  pY->setStyleSheet( "QLabel {font-size:16px}" );
  pVlayout->addWidget(pY);
  for(int i = 0; i < m_Plots.count(); i++)
    {
    QLineEdit *pFormula = new QLineEdit(m_Plots[i]->m_Formula);
    pFormula->setAlignment(Qt::AlignHCenter);
    pFormula->setReadOnly(true);
    pVlayout->addWidget(pFormula);
    pVlayout->addWidget(m_Plots[i]->AddValueFunc());
    }
  QLabel *pPrecision = new QLabel("Precision F(x)");
  pPrecision->setAlignment(Qt::AlignHCenter);
  pPrecision->setStyleSheet( "QLabel {font-size:16px}" );
  pVlayout->addWidget(pPrecision);
  m_pPrecisionFx = new QSlider;
  m_pPrecisionFx->setOrientation(Qt::Horizontal);
  m_pPrecisionFx->setFixedSize(120,25);
  m_pPrecisionFx->setMaximum(6);
  m_pPrecisionFx->setMinimum(1);
  m_pPrecisionFx->setValue(1);
  connect(m_pPrecisionFx, SIGNAL(valueChanged(int)), SLOT(on_precision_Fx_valueChanged(int)));
  pVlayout->addWidget(m_pPrecisionFx);
  pVlayout->addSpacing(300);
  pLayout->addLayout(pVlayout);
  setLayout( pLayout );
  m_pScene = new QGraphicsScene(m_pGraphicsView);
  m_pScene->installEventFilter(this);
//  m_pMainChart->clear();
//  m_pSeriesBreakPoints->setBorderColor(m_pMainChart->BreakPointColor);
  m_pSeriesBreakPoints->setBorderColor(Qt::red);
  m_pSeriesBreakPoints->setColor(Qt::white);
  m_pSeriesBreakPoints->setMarkerSize(8);
//  m_pSeriesBreakPoints->setMarkerSize(qreal(m_pMainChart->ThinknessBreakPoint));
  QRectF Geometry = m_pGraphicsView->rect();
  m_pChart->setGeometry(Geometry);
  m_pChart->setPlotArea(Geometry);
  m_pChart->addAxis(m_pValueAxisX,Qt::AlignBottom);
  m_pChart->addAxis(m_pValueAxisY,Qt::AlignLeft);
  m_pValueAxisX->hide();
  m_pValueAxisY->hide();
  m_pChart->legend()->hide();
  }

MultiPlotter::~MultiPlotter()
  {
  for(int i = 0; i < m_Plots.count(); i++)
    delete m_Plots[i];
  }

void MultiPlotter::CalculatePoint(PlotData& PData)
  {

  double X_start(m_pXMin->value()), X_end(m_pXMax->value()), X_step;

  int NumberX=abs(X_start)+abs(X_end);

  if(NumberX<=25)X_step=0.1;
//  if(NumberX<=25)X_step=0.02;
  else if(NumberX<=50)X_step=0.05;
  else if(NumberX<=100)X_step=0.1;
  else if(NumberX<=250)X_step=0.2;
  else if(NumberX<=500)X_step=5;
  else if(NumberX<=5000)X_step=20;
  else X_step=100;

  X_end += 0.1 * X_step;
  MathExpr Expr = MathExpr( Parser::StrToExpr( PData.m_Formula));
  if(s_GlobalInvalid || Expr.IsEmpty()) throw QString("Bad formula:") + PData.m_Formula;
  int nPrevBreakpoint = 0;
  double MaxExtr = 0;
  double MinExtr = 0;
  double dY = 0;
  bool bStart = true;
  for( double YOld, Dy, Y, X = X_start; X <= X_end; X += X_step, YOld = Y, dY = Dy)
    {
//    if(fabs(X) < 0.5 * X_step ) X = 0;
    MathExpr Value;
    try
      {
      Value = Expr.Substitute("x", Constant(X) ).SimplifyFull();
      }
    catch( ErrParser& ErrMsg )
      {
      }
    Y = 0;
    if( !(IsType( TConstant, Value )) || s_GlobalInvalid && s_LastError=="INFVAL" && !bStart)
      {
      if( !nPrevBreakpoint )
        {
        Y = 0;
        if( !Value.IsEmpty() && Value.Constan(Y))
          if(Y > MaxExtr)
            MaxExtr = Y;
          else
            if(Y < MinExtr)
              MinExtr = Y;
        m_BreakPoints.append(QPointF(X, Y));
        PData.m_Points.append(QPointF(X, Y));
        }
      else
        PData.m_Points.append(QPointF(X, cm_Break));
      nPrevBreakpoint++;
      continue;
      }
    if(!(Value.IsEmpty()) && !s_GlobalInvalid && Value.Constan(Y))
      {
      if(Value.IsLimit())
        {
        PData.m_Points.append(QPointF(X, cm_Break));
        if( !nPrevBreakpoint ) m_BreakPoints.append(QPointF(X, Y));
        nPrevBreakpoint++;
        continue;
        }
      if( Value.HasComplex() )
        {
        if( !nPrevBreakpoint ) m_BreakPoints.append(QPointF(X, Y));
        nPrevBreakpoint++;
        PData.m_Points.append(QPointF(X, cm_Break));
        continue;
        }
      if(nPrevBreakpoint > 1)
        {
        Y = 0;
        m_BreakPoints.append(QPointF(X, 0 ));
        dY = 0;
        }
      Dy = Y - YOld;
      if( X == X_start )
        m_YMin = m_YMax = Y;
      else
         if( Y * YOld < 0 && Dy * dY < 0  )
           {
           Y = PData.m_Points.last().ry();
           if( Y > m_YMax  ) m_YMax = Y;
           if( Y < m_YMin ) m_YMin = Y;
           PData.m_Points.append(QPointF(X - X_step, cm_Break));
           m_BreakPoints.append(QPointF(X - X_step, 0 ));
           X += X_step;
           if(X > X_end) break;
           Value = Expr.Substitute("x", Constant(X) ).SimplifyFull();
           if(Value.IsEmpty() || s_GlobalInvalid || !Value.Constan(Y)) break;
           PData.m_Points.append(QPointF(X, Y));
           continue;
           }
       if(Y < m_YMin) m_YMin = Y;
       if( Y > m_YMax ) m_YMax = Y;
       PData.m_Points.append(QPointF(X, Y));
       nPrevBreakpoint = 0;
       if( X == X_start || Dy * dY >= 0 ) continue;
       if(Dy > 0)
         {
         if(Y < MinExtr) MinExtr = Y;
         continue;
         }
       if(Y > MaxExtr) MaxExtr = Y;
       }
     }
    double delta = ( m_YMax - m_YMin ) / 20;
    if( m_YMin == 0 ) m_YMin -= delta;
    if(fabs(m_YMin - MinExtr) < delta ) m_YMin -= delta;
    if(fabs(m_YMax - MaxExtr) < delta ) m_YMax += delta;
  m_pCurSliderValue->setMaximum(PData.m_Points.length());
  }

bool MultiPlotter::Plot()
  {
  for( int i = 0; i < m_Plots.count(); i++)
    CalculatePoint(*m_Plots[i]);

 m_pScene->addItem(m_pChart);
 m_pChartView->setRenderHint(QPainter::Antialiasing);
 m_pGraphicsView->setScene(m_pScene);

 m_pYMin->blockSignals(true);
 m_pYMax->blockSignals(true);
 m_pYMin->setValue( m_YMin );
// m_pYMin->setValue( floor(m_YMin) );
 m_pYMax->setValue( m_YMax );
// m_pYMax->setValue( ceil(m_YMax) );
 m_pYMin->blockSignals(false);
 m_pYMax->blockSignals(false);

 m_pValueAxisX->setRange(m_pXMin->value(),m_pXMax->value());
 if(m_YMin==m_YMax){m_YMax++;m_YMin--;}
// m_pValueAxisY->setRange(floor(m_YMin), ceil(m_YMax) );
 m_pValueAxisY->setRange(m_YMin, m_YMax );

 SetCursor(0);
 UpdateGraph();

 QPoint xmax = m_pChartView->mapFromParent(QPoint(
  static_cast<int>(m_pChart->mapToPosition(QPointF(m_pXMax->value(),0)).x()),
  static_cast<int>(m_pChart->mapToPosition(QPointF(m_pXMax->value(),0)).y())));
 m_GraphWidth = xmax.x();

 m_pValueX->setText(QString::number(m_Plots[0]->m_Points[0].x(),10,2));
 for(int i = 0; i < m_Plots.count(); i++)
   m_Plots[i]->ShowValue(m_Prec, 0);
 return true;
}

void MultiPlotter::ReCalculateAndUpdate()
  {
  m_BreakPoints.clear();
  s_LastError="";
  for( int i = 0; i < m_Plots.count(); i++)
    CalculatePoint(*m_Plots[i]);
  UpdateGraph();
  }

void MultiPlotter::PaintGraph(PlotData& PData)
  {
  auto MapPoint = [&] (QPointF &P)
    {
    return m_pChartView->mapFromParent( QPoint(
      static_cast<int>(m_pChart->mapToPosition(P).x()),
    static_cast<int>(m_pChart->mapToPosition(P).y())));
    };
  PData.m_PathGraph.clear();
  bool bWasBreak = false;
  PData.m_PathGraph.moveTo(MapPoint(PData.m_Points[0]));
  for(int i = 1;  i < PData.m_Points.length();  i++)
    {
    QPointF P( PData.m_Points[i]);
    if(P.y() == cm_Break )
      {
      bWasBreak = true;
      continue;
      }
    if(P.y() > m_YMax ) P.setY(m_YMax);
    if(P.y() < m_YMin ) P.setY(m_YMin);
    if(bWasBreak)
        PData.m_PathGraph.moveTo(MapPoint(P));
      else
        PData.m_PathGraph.lineTo(MapPoint(P));
    bWasBreak = false;
    }
  QPen P;
  P.setColor(PData.m_Color);
  P.setWidth(PData.m_LineWidth);
  m_pPathItemGraph = m_pScene->addPath(PData.m_PathGraph, P);
  }

void MultiPlotter::RefreshValues(int index)
  {
  m_pValueX->setText(QString::number(m_Plots[0]->m_Points[index].x(),10,2));
  for(int i = 0; i < m_Plots.count(); i++)
    m_Plots[i]->ShowValue(m_Prec, index);
  SetCursor(index);
  }

bool MultiPlotter::eventFilter(QObject *obj, QEvent *e)
  {
  QVector<QPointF>& Points = m_Plots[0]->m_Points;

  auto Cursor = [&] (QPointF& p)
    {
    int Count = Points.count();
    int index = p.x() * Count / m_GraphWidth;
    if( index < 0 ) index = 0;
    if(index >= Count ) index = Count - 1;
    m_pCurSliderValue->setSliderPosition(index);
    };

  if(e->type() == QEvent::GraphicsSceneMouseMove)
    {
    if( !m_MousePressed ) return true;
    QGraphicsSceneMouseEvent* gmev = static_cast<QGraphicsSceneMouseEvent*>(e);
    QPointF ps = gmev->scenePos();
    m_MousePressed = ps.x() <= m_GraphWidth;
    if(!m_MousePressed) return true;
    Cursor(ps);
    }
  if(e->type() == QEvent::GraphicsSceneMousePress)
    {
    QGraphicsSceneMouseEvent* gmev = static_cast<QGraphicsSceneMouseEvent*>(e);
    QPointF ps = gmev->scenePos();
    if(gmev->buttons() == Qt::RightButton)
      {
      on_ContextMenuCall(ps.toPoint());
      return true;
      }
    if(ps.x() > m_GraphWidth) return true;
    Cursor(ps);
    m_MousePressed = true;
    return true;
    }
  if(e->type() == QEvent::GraphicsSceneMouseRelease)
    {
    QGraphicsSceneMouseEvent* gmev = static_cast<QGraphicsSceneMouseEvent*>(e);
    if(gmev->buttons() == Qt::RightButton) return false;
    m_MousePressed = false;
    return true;
    }
  return false;
  }

void MultiPlotter::PaintAxis()
  {
  m_pScene->removeItem(m_pPathItem);
  for(int i{};i<TextAxisY.length();i++)
    m_pScene->removeItem(TextAxisY[i]);
  for(int i{};i<TextAxisX.length();i++)
    m_pScene->removeItem(TextAxisX[i]);
  m_Path.clear();
  QPointF ymin = m_pChartView->mapFromParent( QPoint(
   static_cast<int>(m_pChart->mapToPosition(QPointF(0,m_pYMin->value())).x()),
   static_cast<int>(m_pChart->mapToPosition(QPointF(0,m_pYMin->value())).y())
     ));
  QPointF ymax = m_pChartView->mapFromParent( QPoint(
    static_cast<int>(m_pChart->mapToPosition(QPointF(0,m_pYMax->value())).x()),
    static_cast<int>(m_pChart->mapToPosition(QPointF(0,m_pYMax->value())).y())
    ));
  qreal YMin = ymin.y();
  QPointF const xmin = m_pChartView->mapFromParent( QPoint(
    static_cast<int>(m_pChart->mapToPosition(QPointF(m_pXMin->value(),0)).x()),
    static_cast<int>(m_pChart->mapToPosition(QPointF(m_pXMin->value(),0)).y())
    ));
  qreal XY = xmin.y();
  if(XY > YMin) ymin.setY(XY);
  QPointF const xmax = m_pChartView->mapFromParent(QPoint(
     static_cast<int>(m_pChart->mapToPosition(QPointF(m_pXMax->value(),0)).x()),
     static_cast<int>(m_pChart->mapToPosition(QPointF(m_pXMax->value(),0)).y())
     ));
   ymax.ry() = xmin.x();
   m_Path.moveTo(ymax);
   m_Path.lineTo(ymin);
   m_Path.moveTo(ymax);
   m_Path.lineTo(ymax.x()+5,ymax.y()+10);
   m_Path.moveTo(ymax);
   m_Path.lineTo(ymax.x()-5,ymax.y()+10);
   int Posx = ymax.x() - 25;
   for(int i = 0; i < m_Plots.count(); i++)
     {
     m_Plots[i]->setAxisName(m_pScene->addText(m_Plots[i]->m_Label), Posx, ymax.y() + 10);
     if(i == 0)
       Posx += 50;
     else
       Posx += 20;
     }
   m_Path.moveTo(xmin);
   m_Path.lineTo(xmax);
   m_Path.lineTo(xmax.x()-10,xmax.y()-5);
   m_Path.moveTo(xmax);
   m_Path.lineTo(xmax.x()-10,xmax.y()+5);
   TextAxisX.append(m_pScene->addText("X"));
   TextAxisX.last()->setPos(xmax.x()-20,xmax.y()+5);
   QFont LabelFont = TextAxisX.last()->font();
   LabelFont.setBold(true);
   TextAxisX.last()->setFont(LabelFont);
   int PrecAxis;
   auto CalcDiv = [&] (double div)
     {
     double kDiv = pow(10.0, round(log10(div)));
     div /= kDiv;
     if(div >= 2.0)
       div = 2.0;
     else
       if(div >= 0.74)
         div = 1.0;
       else
         div = 0.5;
     div *= kDiv;
     PrecAxis = 1;
     if(div < 0.1) PrecAxis = 2;
     return div;
     };

    double LabelPoint;
    auto CalcLabel = [&] ()
      {
      QByteArray Label = QByteArray::number(LabelPoint,10,PrecAxis);
      if(Label.endsWith(".0"))
        Label = Label.left(Label.count() - 2);
      return Label;
      };

    double div = CalcDiv( double(abs(m_pXMax->value())+abs(m_pXMin->value()))/20.0 );
    QPointF val;
    for(LabelPoint=m_pXMin->value();LabelPoint<m_pXMax->value() && div!=0;LabelPoint+=div)
      {
      val=m_pChartView->mapFromParent(
      QPoint( static_cast<int>(m_pChart->mapToPosition(QPointF(LabelPoint,0)).x()),
         static_cast<int>(m_pChart->mapToPosition(QPointF(LabelPoint,0)).y()))
      );
      m_Path.moveTo(val.x(),val.y()+10);
      m_Path.lineTo(val.x(),val.y()-10);
      m_Path.moveTo(val.x(),val.y());
      if(LabelPoint == 0 || LabelPoint == m_pXMin->value())
//        TextAxisY.append(m_pScene->addText("",m_pMainChart->FontAxisX));
        TextAxisY.append(m_pScene->addText(""));
      else
        TextAxisY.append(m_pScene->addText(CalcLabel()));
//      TextAxisY.append(m_pScene->addText(CalcLabel(),m_pMainChart->FontAxisX));
      if(LabelPoint >= 0 )
         TextAxisY.last()->setPos(val.x()-10,val.y()-25);
      else
        TextAxisY.last()->setPos(val.x()-15,val.y()-25);
    }

    for(LabelPoint=m_pXMin->value() + div/2.0;LabelPoint<m_pXMax->value() && div!=0;LabelPoint+=div)
      {
       val=m_pChartView->mapFromParent(
       QPoint(
              static_cast<int>(m_pChart->mapToPosition(QPointF(LabelPoint,0)).x()),
              static_cast<int>(m_pChart->mapToPosition(QPointF(LabelPoint,0)).y())
              )
              );
       m_Path.moveTo(val.x(),val.y()+7);
       m_Path.lineTo(val.x(),val.y()-7);
       m_Path.moveTo(val.x(),val.y());
    }

  div = CalcDiv ((abs(m_pYMax->value())+abs(m_pYMin->value()))/20.0);
  for(LabelPoint=m_pYMin->value(); LabelPoint < m_pYMax->value() && div!=0; LabelPoint+=div)
    {
    val=m_pChartView->mapFromParent(
      QPoint(static_cast<int>(m_pChart->mapToPosition(QPointF(0,LabelPoint)).x()),
        static_cast<int>(m_pChart->mapToPosition(QPointF(0,LabelPoint)).y())));
    m_Path.moveTo(val.x()+10,val.y());
    m_Path.lineTo(val.x()-10,val.y());
    m_Path.moveTo(val.x(),val.y());
    if(LabelPoint == 0 || LabelPoint == m_pYMin->value())
//      TextAxisY.append(m_pScene->addText("",m_pMainChart->FontAxisY));
      TextAxisY.append(m_pScene->addText(""));
    else
      TextAxisY.append(m_pScene->addText(CalcLabel()));
//    TextAxisY.append(m_pScene->addText(CalcLabel(),m_pMainChart->FontAxisY));
    TextAxisY.last()->setPos(val.x()+7,val.y()-15);
    }

    for(LabelPoint=m_pYMin->value() + div/2.0; LabelPoint <= m_pYMax->value() && div!=0; LabelPoint+=div)
      {
       val=m_pChartView->mapFromParent(
       QPoint(
              static_cast<int>(m_pChart->mapToPosition(QPointF(0,LabelPoint)).x()),
              static_cast<int>(m_pChart->mapToPosition(QPointF(0,LabelPoint)).y())
              )
              );
       m_Path.moveTo(val.x()+7,val.y());
       m_Path.lineTo(val.x()-7,val.y());
       m_Path.moveTo(val.x(),val.y());
    }
    m_pPathItem = m_pScene->addPath(m_Path);
//    m_pPathItem = m_pScene->addPath(m_Path,m_pMainChart->AxisXPen);
}

void MultiPlotter::UpdateGraph()
  {
   if(!m_BreakPoints.isEmpty())
    {
    m_pChart->addSeries(m_pSeriesBreakPoints);
    m_pSeriesBreakPoints->attachAxis(m_pValueAxisY);
    m_pSeriesBreakPoints->attachAxis(m_pValueAxisX);
    m_pSeriesBreakPoints->replace(m_BreakPoints);
    }
  PaintAxis();
  m_pScene->removeItem(m_pPathItemGraph);
  for( int i = 0; i < m_Plots.count(); i++)
    {
    PaintGraph(*m_Plots[i]);
    m_pChart->update(m_pGraphicsView->rect());
    m_pScene->update(m_pScene->sceneRect());
    }
  }

void MultiPlotter::on_xmin_valueChanged(double val)
  {
  m_pXMax->setMinimum(val);
  m_pValueAxisX->setRange(val,m_pXMax->value());
  m_pRefresh->setEnabled(true);
  }

void MultiPlotter::on_xmax_valueChanged(double val)
  {
  m_pXMin->setMaximum(val);
  m_pValueAxisX->setRange(m_pXMin->value(),val);
  m_pRefresh->setEnabled(true);
  }

void MultiPlotter::on_ymin_valueChanged(double val)
  {
  m_pYMax->setMinimum(val);
  m_pValueAxisY->setRange(val,m_pYMax->value());
  m_pRefresh->setEnabled(true);
  }

void MultiPlotter::on_ymax_valueChanged(double val)
  {
  m_pYMin->setMaximum(val);
  m_pValueAxisY->setRange(m_pYMin->value(),val);
  m_pRefresh->setEnabled(true);
  }

void MultiPlotter::on_ResetPressed()
  {
  ReCalculateAndUpdate();
  m_pRefresh->setEnabled(false);
  }

void MultiPlotter::SetCursor(int index)
  {
  if( m_pPathAsymptote != nullptr) m_pScene->removeItem(m_pPathAsymptote);
  m_pScene->removeItem(m_pPathAsymptote);
  m_pPathAsymptote = nullptr;
  QPointF Point;
  Point.setY(cm_Break);
  for(int i = 0; i < m_Plots.count(); i++)
    {
    QPointF P = m_Plots[i]->SetCursor(*this, index);
    if( P.y() != cm_Break )
      Point = P;
    else
      if( Point.y() == cm_Break ) Point = P;
    }
  if(Point.y() == cm_Break )
    {
    m_PathGraph.clear();
    QPointF P1, P2;
    P1.setX(Point.x());
    P1.setY(m_pValueAxisY->min());
    P2.setX(Point.x());
    P2.setY(m_pValueAxisY->max());
    auto p1 = m_pChartView->mapFromParent( QPoint(
      static_cast<int>(m_pChart->mapToPosition(P1).x()),
      static_cast<int>(m_pChart->mapToPosition(P1).y())));
    auto p2 = m_pChartView->mapFromParent( QPoint(
      static_cast<int>(m_pChart->mapToPosition(P2).x()),
      static_cast<int>(m_pChart->mapToPosition(P2).y())));
    m_PathGraph.moveTo(p1);
    m_PathGraph.lineTo(p2);
    QPen P;
    P.setColor(Qt::darkRed);
    P.setStyle( Qt::DashDotDotLine);
    P.setWidth(1);
    m_pPathAsymptote = m_pScene->addPath(m_PathGraph, P);
    }
  m_pChart->update(m_pGraphicsView->rect());
  m_pScene->update(m_pScene->sceneRect());
  }

void MultiPlotter::on_cur_val_slider_valueChanged(int value)
  {
  if(value < m_Plots[0]->m_Points.length() && value >= 0)
    {
    m_iCurrentPoint = value;
    RefreshValues(value);
    }
  }

void MultiPlotter::on_precision_Fx_valueChanged(int value)
  {
  switch(value)
    {
    case 1: {m_Prec=1; break;}
    case 2: {m_Prec=2; break;}
    case 3: {m_Prec=3; break;}
    case 4: {m_Prec=4; break;}
    case 5: {m_Prec=5; break;}
    case 6: {m_Prec=6; break;}
    }
  RefreshValues(m_iCurrentPoint);
  }

void MultiPlotter::on_ContextMenuCall(const QPoint& val)
  {
  QMenu * menu = new QMenu(this);

  QAction *options = new QAction("Options", this);
  QAction *hide_numbers;
  if(m_NumberAxisIsHidden)
    hide_numbers = new QAction("Hide numbers", this);
  else
    hide_numbers = new QAction("Show numbers", this);
  QAction *hide_names;
  if(m_NamesAxisIsHidden)
    hide_names = new QAction("Hide names of axis", this);
  else
    hide_names = new QAction("Show names of axis", this);
  QAction *save_graph = new QAction("Save graph as PNG", this);
  connect(hide_numbers, SIGNAL(triggered()), this, SLOT(on_HideNumbers()));
  connect(hide_names, SIGNAL(triggered()), this, SLOT(on_HideNames()));
  connect(save_graph, SIGNAL(triggered()), this, SLOT(on_SaveGraph()));
  connect(options, SIGNAL(triggered()), this, SLOT(on_Options()));

  menu->addAction(options);
  menu->addAction(hide_numbers);
  menu->addAction(hide_names);
  menu->addAction(save_graph);

  menu->popup(mapToGlobal(val));
  }

void MultiPlotter::on_HideNumbers()
  {
  if(m_NumberAxisIsHidden)
    {
    for(int i = 1;i<TextAxisY.length();i++)
      m_pScene->removeItem(TextAxisY[i]);
    for(int i{};i<TextAxisX.length()-1;i++)
      m_pScene->removeItem(TextAxisX[i]);
    m_NumberAxisIsHidden=false;
    }
  else
    {
    for(int i = 1;i<TextAxisY.length();i++)
      m_pScene->addItem(TextAxisY[i]);
    for(int i{};i<TextAxisX.length() - 1;i++)
      m_pScene->addItem(TextAxisX[i]);
    m_NumberAxisIsHidden=true;
    }
  }

void MultiPlotter::on_HideNames()
  {
  for(int i = 0; i < m_Plots.count(); i++)
    m_Plots[i]->m_pAxisName->setVisible(!m_NamesAxisIsHidden);
  m_NamesAxisIsHidden = !m_NamesAxisIsHidden;
  }

void MultiPlotter::on_SaveGraph()
  {
  QPixmap* picture=new QPixmap;
  *picture = m_pGraphicsView->grab();
  QString filename = QFileDialog::getSaveFileName(this, tr("Save file"), "", tr("Images (*.png)"));
  *picture = picture->copy(0,0,750,560);
  picture->save(filename,"PNG");
  delete picture;
  }

void MultiPlotter::on_Options()
  {
  QStringList ListObjects(QString("Background,x-Axis,y-Axis,Cursor point,Cursor line").split(','));
  for(int i = 0; i < m_Plots.count(); i++)
    ListObjects << "Graph " + m_Plots[i]->m_Formula;
  while(true)
    {
    bool Ok;
    QString Selected = QInputDialog::getItem(nullptr, "Graph objects", "Select object", ListObjects, 0, false, &Ok);
    if(!Ok) return;
    int index = ListObjects.indexOf(Selected);
    switch(index)
      {
      case 0:
        {
        QColor color = QColorDialog::getColor(m_pChart->plotAreaBackgroundBrush().color());
        if (!color.isValid() ) break;
        m_pChart->setPlotAreaBackgroundBrush(QBrush(color));
        m_pChart->setPlotAreaBackgroundVisible(true);
        break;
        }
      case 1:
        {
        break;
        }
      case 2: {break;}
      case 3: {break;}
      case 4: {break;}
      default: {break;}
      }
    }
  }

void MultiPlotter::on_HideLegend()
{
    if(m_ChartLegendIsHidden)
    {
        m_pChart->legend()->setVisible(false);
        m_ChartLegendIsHidden=false;
    }
    else
    {
        m_pChart->legend()->setVisible(true);
        m_ChartLegendIsHidden=true;
    }
}
/*
SettingsChart::SettingsChart(MultiPlotter *pPlotter): QDialog( nullptr, Qt::WindowSystemMenuHint ),
  m_pPlotter(pPlotter)
  {

  }
*/
void MultiPlotter::on_SetChartSettings()
  {
  }
