#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qnetworkreply.h>
#include <qcombobox.h>
#include <qtablewidget.h>
#include <qprocess.h>
#include <qlistview.h>
#include <QStandardItemModel>

class Connector : public QObject
  {
  Q_OBJECT
  QNetworkReply *m_pReply;
  QByteArray m_Response;
  QString m_Url;
  QByteArray m_Command;
  public:
    Connector( const QString& Url, const QByteArray& Command ) : m_Url( Url ), m_Command( Command ) {}
    QByteArray Connect();
    public slots:
    void ReadyRead();
  };

class PasswordDialog : public QDialog
  {
  Q_OBJECT
    QLineEdit *m_pRootUrl;
    QLineEdit *m_pPassInput;
    QLabel *m_pPassError;
    QPushButton *m_pPassAccept;
  public:
    static QByteArray sm_RootUrl;
    QByteArray m_UsrId;
    bool m_LokalWork;
    PasswordDialog();
    public slots:
    void accept();
    void PassChanged( const QString& );
    void LokalWork();
  };

class SelectTest : public QDialog
  {
  public:
    QComboBox *m_pListTests;
    SelectTest( const QByteArray& UserCode );
  };

class MainTestDlg;
class MainTestMenu : public QTableWidget
  {
  void mouseMoveEvent( QMouseEvent *event );
  public:
    MainTestMenu( const QByteArray& TstId, const QByteArray& UsrId, MainTestDlg *pDlg );
  };

class MainTestDlg : public QDialog
  {
  Q_OBJECT
  public:
    bool m_BackToSelectTest;
    QByteArray m_SelectedChapter;
    QString m_ChapterName;
    MainTestDlg( const QByteArray& Parms, const QByteArray& UsrId, const QString& TestName  );
    public slots:
    void ClickItem( QTableWidgetItem *pItem );
    void BackToSelectTest();
  };

class SelectTopicDlg;
class SelectTopic : public QTableWidget
  {
  void mouseMoveEvent( QMouseEvent *event );
  public:
    SelectTopic( const QByteArray& PrmId, const QByteArray& UsrId, const QByteArray& Chp_id, SelectTopicDlg *pDlg );
  };

class SelectTopicDlg : public QDialog
  {
  Q_OBJECT
  public:
    bool m_BackToSelectChapter;
    bool m_NoSelection;
    QByteArray m_ETime;
    QByteArray m_DateDiff;
    QByteArray m_SelectedTopic;
    SelectTopicDlg( const QByteArray& PrmId, const QByteArray& UsrId, const QByteArray& Chp_id, const QString& ChapterName );
    public slots:
    void ClickItem( QTableWidgetItem *pItem );
    void BackToSelectChapter();
  };

class OldDriverStarter: public QObject
  {
  Q_OBJECT
    QProcess *m_pProcess;
  public:
    OldDriverStarter() : m_pProcess( new QProcess ) {}
    void Start(const QString& );
    public slots:
    void FinishDriver( int, QProcess::ExitStatus );
  signals:
    void PostFinish();
  };

class CreateMoodleBank : public QDialog
  {
  Q_OBJECT
    QStandardItemModel *m_pAvailableTests;
  QStandardItemModel *m_pSelectedTests;
  QPushButton *m_pCreateBank;
  QLineEdit *m_pQuizName;
  public:
    CreateMoodleBank();
    public slots:
    void SelectTest( const QModelIndex& );
    void UnselectTest( const QModelIndex& );
    void CreateBank();
    void TestQuizName( const QString & );
  };
