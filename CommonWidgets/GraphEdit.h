#ifndef QGRAPHEDIT
#define QGRAPHEDIT

#include <qscrollarea.h>
#include "../Mathematics/ExpObjOp.h"
#include "commonwidgets_global.h"

class GraphEdit : public QScrollArea
  {
  Q_OBJECT
  public:
    enum TrainingType { NoTraining, LinInEq, MathGraph, Tangram };
    GraphEdit( QWidget *pParent = nullptr ) : QScrollArea( pParent ) {}
    void CancelComparison() {}
    void SetTraining( TrainingType Type ) {}
    QByteArray ComparisonResult() { return ""; }
    COMMONWIDGETS_EXPORT void SetConstraints(const MathExpr&) {}
    MathExpr RestoreObject( const QString& HistFile ) { return nullptr; }
    COMMONWIDGETS_EXPORT bool IsMathGraph() { return false; }
    COMMONWIDGETS_EXPORT void Reset() {}
    public slots :
  };

#endif

