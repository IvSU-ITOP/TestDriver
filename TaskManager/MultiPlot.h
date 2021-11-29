#ifndef MULTIPLOT_H
#define MULTIPLOT_H

#include <QAction>
#include <QMainWindow>
#include <QtCharts/QtCharts>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>

using namespace QtCharts;

class PlotData
  {
  friend class MultiPlotter;
//  friend class SettingsChart;
  QByteArray m_Formula;
  QFont m_Font;
  QString m_Label;
  QColor m_Color;
  int m_LineWidth;
  QPainterPath m_PathGraph;
  QVector<QPointF> m_Points;
  QGraphicsTextItem *m_pAxisName;
  QLabel *m_pValueFunc;
  QScatterSeries *m_pCursor;
  QLineSeries *m_pLinesCursor;
  PlotData( const QByteArray& Formula, QString Label, QColor Color, class MultiPlotter &Plotter );
  void setAxisName(QGraphicsTextItem* pAxisName, int x, int y);
  QLabel *AddValueFunc();
  void ShowValue(int Prec, int index);
  QPointF SetCursor(class MultiPlotter &Plotter, int index);
  };

class MultiPlotter : public QWidget
  {
  Q_OBJECT
    friend PlotData;
//    friend class SettingsChart;
    const double cm_Break = 1e-34;
    QScatterSeries *m_pSeriesBreakPoints=new QScatterSeries;
    QValueAxis *m_pValueAxisX = new QValueAxis;
    QValueAxis *m_pValueAxisY = new QValueAxis;
    QGraphicsScene *m_pScene = nullptr;
    QGraphicsPathItem* m_pPathItem = nullptr;
    QGraphicsPathItem* m_pPathItemGraph = nullptr;
    QVector <QGraphicsTextItem *>TextAxisY, TextAxisX;
    QChart *m_pChart = new QChart;
    QChartView *m_pChartView = new QChartView(m_pChart);
    QPainterPath m_Path, m_PathGraph;
    QGraphicsPathItem* m_pPathAsymptote;
    QVector<QPointF> m_BreakPoints;
    QVector<PlotData*> m_Plots;
    QByteArray m_Formula;
    QGraphicsView* m_pGraphicsView;
    QDoubleSpinBox *m_pXMin;
    QDoubleSpinBox *m_pXMax;
    QDoubleSpinBox *m_pYMin;
    QDoubleSpinBox *m_pYMax;
    QSlider *m_pCurSliderValue;
    QSlider *m_pPrecisionFx;
    QLabel *m_pValueX;
    QPushButton *m_pRefresh;
    int m_iCurrentPoint;
//    SettingsChart *m_pMainChart=new SettingsChart;
//    OptionMenuPlotter *m_pPlotterMenu=new OptionMenuPlotter(nullptr);
    double m_YMin = 1.79769e+308, m_YMax = 2.22507e-308;
    int m_Prec = 1;
    bool m_NumberAxisIsHidden=true;
    bool m_NamesAxisIsHidden=true;
    bool m_GridAxisIsHidden=true;
    bool m_ChartLegendIsHidden=true;
    double m_GraphWidth;
    bool m_MousePressed = false;

    void ReCalculateAndUpdate();
    void UpdateGraph();
    void SetCursor(int index);
    void CalculatePoint(PlotData& PData);
    void PaintAxis();
    void PaintGraph(PlotData& PData);
    protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void on_ContextMenuCall(const QPoint& val);
    void RefreshValues(int index);

public:
     MultiPlotter(const QByteArray& Formula);
     ~MultiPlotter();
     bool Plot();

private slots:
     void on_precision_Fx_valueChanged(int value);
     void on_cur_val_slider_valueChanged(int value);
     void on_ymax_valueChanged(double val);
     void on_ymin_valueChanged(double val);
     void on_xmax_valueChanged(double val);
     void on_xmin_valueChanged(double val);
     void on_ResetPressed();
     void on_HideNumbers();
     void on_HideNames();
     void on_SaveGraph();
     void on_Options();
     void on_HideLegend();

public slots:
     void on_SetChartSettings();
};

/*
class SettingsChart : public QDialog
  {
  MultiPlotter *m_pPlotter;
  SettingsChart(MultiPlotter *pPlotter);
  };
*/

#endif // MULTIPLOT_H
