#ifndef TASKFILEMANAGER_H
#define TASKFILEMANAGER_H

#include "taskfilemanager_global.h"
#include "../Mathematics/ExpOb.h"
#include "../Mathematics/ExpStore.h"
#include "../Mathematics/Statistics.h"
#include "../Mathematics/Parser.h"
#include <qtextdocument.h>
#include <QHash>
#include <qtexttable.h>
#define ISALPHA(x) ((x >= 'a' && x <= 'z' ) || (x >= 'A' && x <= 'Z' ))

class TASKFILEMANAGER_EXPORT TaskFileManager
{
public:
    TaskFileManager();
    ~TaskFileManager();

private:

};

enum TCompareType { AsExpression, AsConstraint };
enum XDescr_type { tXDtext, tXDpicture, tXDexpress, tXDnewline, tXDrandom };

typedef QSharedPointer<class TXDescrMemb> PDescrMemb;
typedef QSharedPointer<class TXDescrList> PDescrList;
typedef QSharedPointer<class TXPSubTask> PSubTask;
typedef QSharedPointer<class TXPSubStep> PSubStep;
typedef QSharedPointer<class TXStepMemb> PStepMemb;
typedef QSharedPointer<class TXStepList> PStepList;

typedef QSharedPointer<class TTrack> PTrack;
typedef QSharedPointer<class TXPSubTask> PSubTask;
typedef QSharedPointer<class TXPPromptDescr> PPromptDescr;

class BaseTask;

class TEqualBase
  {
  TCompareType m_CompareType;
  TEqualBase( TCompareType CompType ) {}
  bool Eq( const MathExpr& E2 ) const { return false; }
  bool Equal( const MathExpr& E2 ) const { return false; }
  };

class TaskFile : public QFile
  {
  BaseTask *m_pTask;
  int m_iLine;
  public:
    TaskFile( BaseTask *pTask ) : QFile(), m_pTask( pTask ), m_iLine(0) {}
    bool Step_exist( const QByteArray& StepID );
    void SkipLinesUntilSelectedTrack( const QByteArray& LKeyWord );
    bool LoadNameFromTaskFile( const QByteArray& LKeyWord, QByteArray& );
    bool LoadRValueFromTaskFile( const QByteArray& LKeyWord, double &Value );
    bool LoadBooleanFromTaskFile( const QByteArray& LKeyWord, bool &Value );
    TASKFILEMANAGER_EXPORT QByteArray ReadLine();
    TASKFILEMANAGER_EXPORT void Reset( int iLine );
    TASKFILEMANAGER_EXPORT virtual bool reset() { m_iLine = 0; return QFile::reset(); }
    TASKFILEMANAGER_EXPORT int CurrentLine() { return m_iLine; }
    TASKFILEMANAGER_EXPORT void Save(class QByteStream &, const QString& FName );
  };

class TXDescrMemb
  {
  friend BaseTask;
  friend class TXDescrList;
  QPoint m_Start;			//{ graphics start coordinates				}
  QSize m_Size;
  int m_Axis;
  int m_WBlank;
  public:
    XDescr_type m_Kind;
    QByteArray m_Content;	//		{ string with method source					}
    PDescrMemb m_pNext;
    PDescrMemb m_pPrev;
    TCompareType m_CompareType;
    bool m_Already;		//{ true  if( this step was decided				}
    TXDescrMemb( XDescr_type K, const QByteArray& C, const PDescrMemb& P, const PDescrMemb& N );
  };

class RichTextDocument;
class TXDescrList
  {
  friend BaseTask;
  friend RichTextDocument;
  BaseTask& m_Task;
  int m_QuestionCount;
  bool m_bHasEndLeft;
  static bool GetPict( QByteArray& L, QByteArray& LO );
  static bool GetExpr( QByteArray& L, QByteArray& LO );
  public:
    PDescrMemb m_pFirst;
    PDescrMemb m_pLast;
    RichTextDocument *m_pDocument;
    bool m_Already;    //      { true  if( this step was decided already  true }
    static bool ExtractValue( QByteArray& L, QByteArray& LO, int Start );
    static void ProcessMacro( QByteArray& S, char );
    static void Save( class QByteStream&, PDescrList );
    TXDescrList( BaseTask& Task ) : m_Already( false ), m_QuestionCount( -1 ), m_Task( Task ),
        m_pDocument(nullptr), m_bHasEndLeft(false) {}
    ~TXDescrList() { Clear(); }
    TASKFILEMANAGER_EXPORT void Add( XDescr_type Type_arg, const QByteArray& S );
    void Update( QByteArray& S );
    TASKFILEMANAGER_EXPORT void Delete( PDescrMemb M );
    TASKFILEMANAGER_EXPORT void Clear();
    TASKFILEMANAGER_EXPORT void Assign( PDescrList& A );
    void LoadFromTaskFile( const QByteArray& LKeyWord );
    void LoadFromQuestionVariables(const QByteArrayList& );
    BaseTask &GetTask() { return m_Task; }
    TASKFILEMANAGER_EXPORT QByteArray GetText();
    QByteArray* GetTextPtr();
    TASKFILEMANAGER_EXPORT int GetExpressCount();
  };

class TXPSubStep
  {
  friend class TXPSubTask;
  friend class TXPStep;
  QByteArray m_Name;
  PDescrList m_pMethodL;  // List of method description
  PSubTask m_pSubTask;
  PSubStep m_pPrev;
  PSubStep m_pNext;
  public:
    TXPSubStep( BaseTask& Taskile ) : m_pMethodL( new TXDescrList( Taskile ) ) {}
    void AssignTo( PSubStep& A );
  };

class TXPSubTask
  {
  friend class TXPStep;
  PSubStep m_pHostSubStep;  // Reference to host SubStep. Used only  for(reding SubTask
  PSubTask m_pHostSubTask;  // Reference to host SubTask. Used only  for(reding SubTask
  int m_Level;     // Nested level. 1  for(first level of nesting, 2  for(subSubTask, etc.
  QByteArray m_Name;	    // Name of SubTask
  PDescrList m_pSubMethodL; // List of methods
  PSubStep m_pSubStepsL;  // First element of List of steps
  public:
    TXPSubTask( BaseTask& Task ) : m_Level( 0 ), m_pSubMethodL( new TXDescrList( Task ) ) {}
    ~TXPSubTask();
    void AddSubStep( PSubStep& NewSubStep );
    void AssignTo( PSubTask& A );
    void AdjustHosting( int iLevel, const PSubTask&, const PSubStep& );
  };

class TXPStep
  {
  friend class BaseTask;
  friend class TXStepList;
  PSubTask m_pSubTask;
  public:
    BaseTask& m_Task;
    struct ShowParms
      {
      bool m_ShowMinusByAddition;
      bool m_HideMinusByAddition;
      bool m_ShowUnarMinus;
      bool m_HideUnarMinus;
      bool m_ShowMultSign;
      bool m_HideMultSign;
      bool m_ShowRad;
      bool m_ShowDeg;
      bool m_OnExactCompare;
      bool m_NoHint;
      double m_HeightEditorWindow;
      ShowParms() : m_ShowUnarMinus( false ), m_HideUnarMinus( false ), m_ShowMultSign( false ), m_HideMultSign( false ), m_NoHint(false),
        m_ShowRad( false ), m_ShowDeg( false ), m_OnExactCompare(false), m_ShowMinusByAddition(false), m_HideMinusByAddition(false),
        m_HeightEditorWindow(0.5) {}
      void Save( QByteStream & Stream, QByteArray& Id );
      }  m_ShowParms;

    PDescrList m_pResultE; //{	step result( answer )	}
    PDescrList m_pMethodL; // List of methods
    PDescrList m_pComm;	// {	P.S. - comment			}
    QByteArray m_Name;	//	{	The STEP name			}
    PDescrList m_pF1, m_pF2, m_pF3;	// {	false variants lists	}
    PDescrList m_pFComm1, m_pFComm2, m_pFComm3;	// {	comments to false variants lists	}
    PDescrList m_pAnswerTemplate; //{ template of answer } 
    PDescrList m_pAnswerPrompt; 
    bool m_ShowUnarm, m_ShowMSign, m_ShowRad, m_ShowMinusByAdd;
    int m_Mark;
    TXPStep(BaseTask& Task);
    void LoadFromTaskFile( const QByteArray& StepID );
    void LoadSubTaskFromTaskFile( const QByteArray& KeyWord );
    TASKFILEMANAGER_EXPORT QByteArray GetComment();
    TASKFILEMANAGER_EXPORT QByteArray* GetCommentPtr();
    void Save( QByteStream&, int );
    void SetGlobalSWhowRad( bool Value );
    TASKFILEMANAGER_EXPORT void ResetMark( int NewMark );
  };

class RichText
  {
  public:
    TASKFILEMANAGER_EXPORT RichText() {}
    virtual QByteArray GetText() = 0;
  };

class TXStepMemb : public TXPStep
  {
  public:
    PStepMemb m_pPrev;
    PStepMemb m_pNext;
    RichText *m_pRichText;
    RichText *m_pPromptText;
    TXStepMemb( PStepMemb Prev, BaseTask& Task, PStepMemb Next = PStepMemb() );
  };

class TXStepList
  {
  public:
    PStepMemb m_pFirst;
    PStepMemb m_pLast;
    int m_Counter;
    int m_SummMarksTotal;
    TXStepList() : m_Counter( 0 ) {}
    ~TXStepList() { Clear(); }
    void Add( const QByteArray& StepID, BaseTask& Task );
    TASKFILEMANAGER_EXPORT void AddNewStep( const QByteArray& StepName, BaseTask& Task, PStepMemb pPrew = PStepMemb() );
    TASKFILEMANAGER_EXPORT void DeleteStep( PStepMemb pPstep );
    void Clear();
    void Save( QByteStream& );
    void SetGlobalSWhowRad( bool Value );
    bool ResetMarks();
  };

class TXPPromptDescr
  {
  friend class BaseTask;
  QByteArray m_Name;
  public:
    TXPPromptDescr() {}
    TXPPromptDescr( QByteArray& AName ) : m_Name( AName ) {}
    virtual ~TXPPromptDescr() {}
    const QByteArray& Name() { return m_Name; }
  };

class TXPStepPromptDescr : public TXPPromptDescr
  {
  public:
    MathExpr m_ResExpr;
    TXPStepPromptDescr( QByteArray& AName, const MathExpr& AExpr ) : TXPPromptDescr( AName ), m_ResExpr( AExpr ) {}
  };

typedef QVector<MathExpr> TOptExpr;

class TXPSOptPromptDescr : public TXPPromptDescr
  {
  public:
    TOptExpr m_OptExpr;
    TXPSOptPromptDescr( QByteArray& AName, TOptExpr& OExpr ) : TXPPromptDescr( AName ), m_OptExpr( OExpr ) {}
  };

class TTrack
  {
  friend BaseTask;
  friend class ExpStore;
  friend TaskFile;
  BaseTask& m_Task;
  public:
    bool m_MultyTrack;         //True,  if(there are 2||more tracks  if(false, none of variables below!used.
    bool m_TrackSelected;         //True,  if(user selected some track
    int m_SelectedTrack;         //Number of selected track (the first track is #1)
    QByteArrayList m_NameOfTrack; //Names of tracks
    PDescrList m_TracksDescription;     //Prompt  for(track selection (like method section)
    TTrack( BaseTask& Task );
    void Clear();
    void LoadTrackFromTaskFile();
    void Save( QByteStream& );
  };

typedef QSet<int>  BusyValues;
typedef QQueue<BusyValues> ArrBusy;

class CalcPair
  {
  class CalcList *m_pOwner;
  public:
    XDescr_type m_Kind;
    QByteArray m_Variable;
  QByteArray m_Expression;
  CalcPair() {}
  CalcPair( CalcList *pOwner ) : m_pOwner( pOwner) {}
  TASKFILEMANAGER_EXPORT CalcPair( const QByteArray& Pair, CalcList *pOwner, XDescr_type Kind );
  TASKFILEMANAGER_EXPORT void Calculate();
  bool CalcExpress();
  void Save( QByteStream& );
  };

class CalcList : public QVector<CalcPair>
  {
  BaseTask *m_pTask;
  public:
    CalcList() : m_pTask( nullptr ) {}
    CalcList( BaseTask* pTask ) : m_pTask( pTask ) {}
    bool GetRand( QByteArray& L, QByteArray& LO );
    TASKFILEMANAGER_EXPORT MathExpr Random_gen( const QByteArray& S );
    void LoadFromTaskFile( const QByteArray& LKeyWord );
    void LoadFromQuestionVariables(const QByteArrayList& );
    TASKFILEMANAGER_EXPORT CalcList& operator =( const CalcList& );
    TASKFILEMANAGER_EXPORT void Calculate();
    void Save( QByteStream& );
  };

class QByteStream
  {
  public:
  virtual QByteStream& operator << ( const QByteArray& ) = 0;
  virtual QByteStream& operator << ( const char* ) = 0;
  virtual QByteStream& operator << ( double ) = 0;
  virtual QByteStream& operator >> ( QByteArray& ) = 0;
  virtual bool reset() = 0;
  virtual bool AtEnd() = 0;
  };

class QByteFStream : public QFile, public QByteStream
  {
  public:
    TASKFILEMANAGER_EXPORT QByteFStream( const QString& Name );
    QByteStream& operator << ( const QByteArray& );
    QByteStream& operator << ( const char* );
    QByteStream& operator << ( double );
    QByteStream& operator >> ( QByteArray& );
    virtual bool reset();
    virtual bool AtEnd();
  };

class QByteAStream : public QByteArrayList, public QByteStream
  {
  int m_CurrentLine;
  QByteArray m_Line;
  void AppText( const QByteArray& );
  public:
    TASKFILEMANAGER_EXPORT QByteAStream();
    TASKFILEMANAGER_EXPORT ~QByteAStream();
    QByteStream& operator << ( const QByteArray& );
    QByteStream& operator << ( const char* );
    QByteStream& operator << ( double );
    QByteStream& operator >> ( QByteArray& );
    virtual bool reset();
    virtual bool AtEnd();
  };

class BaseTask
  {
  friend TXStepMemb;
  friend TXPStep;
  friend TXStepList;
  PStepMemb m_pMarker;	// {	Index of the current step			}
  bool m_PictureUpPos;
  TLanguages m_Language;
  static QHash<TLanguages, QString > sm_FileExtensions;
  ArrBusy m_Busy;
  QByteArray m_NewBusy;
  bool m_MultiTask;
  protected:
    bool m_Exiting;
    bool m_Something_skipped;
    bool m_WasConstraint;
    PStepMemb m_pCurr;
    double m_SummMarks;
    bool m_OutTemplate;
    bool m_ExactCompare;
    TASKFILEMANAGER_EXPORT void SetFirstStep();
    PDescrList m_pComment;
    TASKFILEMANAGER_EXPORT virtual void Clear();
    TASKFILEMANAGER_EXPORT virtual void ResetParms() {}
  public:
    struct TaskHeader
      {
      bool m_ShowUnarMinus;
      bool m_HideUnarMinus;
      bool m_ShowMultSign;
      bool m_HideMultSign;
      bool m_ShowRad;
      bool m_ShowMinusByAddition;
      bool m_HideMinusByAddition;
      bool m_ShowDeg;
      double m_Accuracy;
      QByteArray* m_pName;
      bool* m_pMultiTask;
      TaskHeader() : m_ShowUnarMinus( false ), m_HideUnarMinus( false ), m_ShowMultSign( false ), m_HideMultSign( false ), m_pName(nullptr),
        m_ShowRad( false ), m_ShowDeg( false ), m_Accuracy( 0.0 ), m_ShowMinusByAddition(false), m_HideMinusByAddition(false) {}
      void Save( QByteStream & Stream );
      }  m_Header;
    TaskFile *m_pFile;
    CalcList *m_pCalc;	//{	List expression to calculate		}
    QByteArray m_Name;
    bool m_NewTask;
    TASKFILEMANAGER_EXPORT static bool sm_NewHebFormat;
    TASKFILEMANAGER_EXPORT static bool sm_GlobalShowMultSign;
    TASKFILEMANAGER_EXPORT static bool sm_GlobalShowUnarMinus;
    TASKFILEMANAGER_EXPORT static bool sm_GlobalShowMinusByAddition;
    TASKFILEMANAGER_EXPORT static bool sm_GlobalShowRad;
    TASKFILEMANAGER_EXPORT static EditSets* sm_pEditSets;
    TASKFILEMANAGER_EXPORT static int sm_LastCreatedTrack;
    TASKFILEMANAGER_EXPORT static TLanguages GetLangByFileName( const QString& File );
    TASKFILEMANAGER_EXPORT static QString GetFileExtByLang( TLanguages Lang );
    TWorkMode m_WorkMode;
    static QByteArray ExtractNameFromString( const QByteArray& );
    TASKFILEMANAGER_EXPORT void SetFileName( const QString& FName );
    TASKFILEMANAGER_EXPORT void SetGlobalSWhowRad( bool );
    void SetWorkMode( TWorkMode WMode );
    TASKFILEMANAGER_EXPORT void SetWorkMode( const QString& Mode );
    virtual void SetCurr( PStepMemb NewCurr ) = 0;
    virtual void OutTemplate( const QByteArray& sTemp ) = 0;
    PTrack m_pTrack;
    PDescrList m_pQuestion;	//{	Task definition						}
    PDescrList m_pPrompt;	//{	Prompt in OUTwindow					}
    PDescrList m_pMethodL;	//{	List of methods						}
    PStepList m_pStepsL; // { List of steps }
    QByteArray m_Template;
    bool m_NeedPrompt;
    TASKFILEMANAGER_EXPORT BaseTask();
    TASKFILEMANAGER_EXPORT virtual ~BaseTask();
    TASKFILEMANAGER_EXPORT void ClearTrackDependent();
    TASKFILEMANAGER_EXPORT void LoadFromFile();
    TASKFILEMANAGER_EXPORT void LoadFromStack(QString&);
    TASKFILEMANAGER_EXPORT virtual void LoadTrackDependentFromFile();
    TASKFILEMANAGER_EXPORT void CalcRead();
    PPromptDescr StepPrompt();
    virtual PPromptDescr OneStepPrompt() = 0;
    virtual PPromptDescr OptStepPrompt() = 0;
    PSubTask ExtractSubTask( int indStep );
    TLanguages GetLanguage() { return m_Language; }
    TaskFile& GetFile() { return *m_pFile; }
    TASKFILEMANAGER_EXPORT QByteArray GetTemplate(int Step);
    TASKFILEMANAGER_EXPORT QByteArray GetComment();
    TASKFILEMANAGER_EXPORT QByteArray* GetCommentPtr();
    TASKFILEMANAGER_EXPORT void SetBusy( const ArrBusy& Busy ) { m_Busy = Busy; }
    TASKFILEMANAGER_EXPORT QByteArray& GetBusy() { return m_NewBusy; }
    TASKFILEMANAGER_EXPORT int Random( int MaxVal );
    TASKFILEMANAGER_EXPORT void Save( QByteStream& );
    TASKFILEMANAGER_EXPORT TaskHeader& GetHeader();
    TASKFILEMANAGER_EXPORT int TrackCount();
    TASKFILEMANAGER_EXPORT bool IsMultitask() { return m_MultiTask; }
    TASKFILEMANAGER_EXPORT QString GetCurrstepName() { return ToLang( m_pCurr->m_Name ); }
  };

inline TXPStep::TXPStep(BaseTask& Task) : m_pMethodL(new TXDescrList(Task)), m_pResultE(new TXDescrList(Task)),
  m_pAnswerTemplate(new TXDescrList(Task)), m_pComm(new TXDescrList(Task)), m_pF1(new TXDescrList(Task)),
  m_pF2(new TXDescrList(Task)), m_pF3(new TXDescrList(Task)), m_pFComm1(new TXDescrList(Task)),
  m_pFComm2(new TXDescrList(Task)), m_pFComm3(new TXDescrList(Task)),
  m_Task(Task), m_ShowRad(Task.sm_GlobalShowRad),
  m_ShowUnarm(Task.sm_GlobalShowUnarMinus), m_ShowMSign(Task.sm_GlobalShowMultSign), m_ShowMinusByAdd(Task.sm_GlobalShowMinusByAddition),
  m_pAnswerPrompt(new TXDescrList(Task)),
  m_Mark(Task.m_MultiTask ? 1 : 0){}

class RichTextDocument : public QTextDocument
  {
  ViewSettings m_ViewSettings;
  QString m_TempPath;
  QTextTable *m_pTable;
  TLanguages m_Language;
  int m_FixedWidth;
  public:
    TASKFILEMANAGER_EXPORT static QString sm_TempPath;
    TASKFILEMANAGER_EXPORT static int sm_NumTmp;
    TASKFILEMANAGER_EXPORT RichTextDocument( const QString& TempPath = sm_TempPath );
    TASKFILEMANAGER_EXPORT void SetContent( PDescrList );
    TASKFILEMANAGER_EXPORT void ResetLanguage();
    TASKFILEMANAGER_EXPORT const ViewSettings& ViewSettings() { return m_ViewSettings; }
    TASKFILEMANAGER_EXPORT void AddRow();
    TASKFILEMANAGER_EXPORT void DeleteRow();
    TASKFILEMANAGER_EXPORT void SetFixedWidth( int Width ) { m_FixedWidth = Width; }
    void Save( QByteStream& );
    TASKFILEMANAGER_EXPORT static QString GetTempPath();
  };

#endif // TASKFILEMANAGER_H
