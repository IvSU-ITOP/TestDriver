#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qnetworkreply.h>
#include <qcombobox.h>
#include <qtablewidget.h>
#include <qprocess.h>
#include <qlistview.h>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QProgressBar>

#define MyServer 0
#define HalomdaOrg 1
#define HalomdaCom 2

#define WorkServer MyServer
//#define WorkServer HalomdaCom
//#define WorkServer HalomdaOrg

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
    QString m_UsrLogin;
    bool m_LokalWork;
    PasswordDialog();
    public slots:
    void accept();
    void PassChanged( const QString& );
    void LokalWork();
  };

struct SelectTest : public QDialog
  {
  QComboBox *m_pListTests;
  SelectTest( const QByteArray& UserCode );
  };

class SelectGroup : public QDialog
  {
Q_OBJECT
  public:
  QComboBox *m_pListGroups;
  QString m_NewGroup;
  QString m_LectLogin;
  SelectGroup( const QByteArray& UserCode, const QString& LectLogin );
  public slots:
    void NewGroup();
  };

class NewGroup : public QDialog
  {
Q_OBJECT
  QLineEdit *m_pGroupName;
  QLineEdit *m_pInstitution;
  QLineEdit *m_pCity;
  QLineEdit *m_pSubject;
  QLineEdit *m_pYearTerm;
  QString *m_pNewGroup;
  QString m_LectName;
  QPushButton *m_pOK;
  public:
    NewGroup(QString*, const QString&);
  public slots:
    void ChangeValue();
    void SetValue();
  };

class WaitExcel : public QDialog
  {
Q_OBJECT
  QLabel *m_pLabel;
  QLabel *m_pProgressLabel;
  QProgressBar *m_pProgressBar;
  int m_Count;
  QPushButton *m_pCancel;
  QString SetProgress( int Current, int Max )
    {
    return QString( "Recorded %1 result of %2" ).arg( QString::number( Current ) ).arg( QString::number( Max ) );
    }
  public:
    WaitExcel();
    void SetMax( const QString &Label, int UserCount );
    void StepProgress();
    public slots:
      void Cancel();
  };

class SelectWork : public QDialog
  {
Q_OBJECT
  void SetNameEnabled(bool);
  void SetFlagsEnabled(bool);
  QPushButton *m_pOK;
  static const int xl3DColumnClustered = 54;
  static const long xlColumns = 2;
  static const long xlLocationAsObject = 2;
  static const long xlCategory = 1;
  static const long xlPrimary = 1;
  public:
  QLineEdit *m_pGroupName;
  QLineEdit *m_pFirstName;
  QLineEdit *m_pLastName;
  QLineEdit *m_pPassword;
  QCheckBox *m_pLoadList;
  QCheckBox *m_pLoadResult;
  static int LoadResult(int);
  SelectWork(QString &GroupName);
  public slots:
  void PassChanged( const QString& );
  void FirstNameChanged( const QString& );
  void LastNameChanged( const QString& );
  void LoadListChanged( int );
  void LoadResultChanged( int );
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
