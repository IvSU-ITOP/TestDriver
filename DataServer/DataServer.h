#include <QTcpServer>
#include <QThread>  //Эта программа работает в многопоточном режиме.
//Для каждого пользователя, который подключился к программе,
//создается отделний поток. Выполнение потоков может происходить
//одновременно, если это позволяют средства сервера.
//вместе с тем, могут быть такие ресурсы, использованаие которых
//допускается лишь в монопольном режиме. Таким образом
//реализация многопточного режима предусматривет наличие
//каких-то средств синхронизации выполнения потоков.
#include <QTcpSocket>  //Этот класс обеспечивет так называемую
//сокетную связь. По этому протоколу обмениваются информацией
//веб-скрипт и ТСР-сервер.
//#include <qtextdocument.h>
//#include <QTextImageFormat>
#include "../TaskFileManager/taskfilemanager.h"

const char s_Temp[] = "C:/ProgramData/Halomda/DataServer/";
void ReadyRead( const QByteArray& Args ); //функция проверяет готовность
//к чтению.

class DataServer : public QTcpServer
  {
  protected:
    int m_Port;
  public:
    int m_ConnectionsCount;  //число подключенных пользователей (работает многопоточный режим)
    DataServer();
    bool StartServer();
  protected:
    void incomingConnection( qintptr socketDescriptor ) override;
//Эта функция наследуемая от QTcpServer выполняется при подключении
//очередного пользователя.
  };


class RefServer : public DataServer
  {
  public:
    RefServer();
  protected:
 //   void incomingConnection( qintptr socketDescriptor ) override;
  };

class DataTask : public BaseTask
  {
  public:
    virtual void SetCurr( PStepMemb NewCurr ) {}
    virtual void OutTemplate( const QByteArray& sTemp ) {};
    virtual PPromptDescr OneStepPrompt() { return PPromptDescr(); }
    virtual PPromptDescr OptStepPrompt() { return PPromptDescr(); }
  };

class Thread : public QThread 
  {
  Q_OBJECT
    QTcpSocket* m_pSocket; //указатель на тот сокет, с которым работает поток.
  int m_SocketDescriptor; //Каждый сокет имеет свой личный идентификатор - дескриптор
  QByteArray m_User;  //Имя подключившегося пользователя.
  QByteArray m_Topic;
  QByteArray m_PathFile;
  QByteArray m_Password; //пароль пользователя
  QByteArray m_TaskType;
  QByteArray m_Charset;
  QString m_TempPath; //?
  QTime m_Time; //?
  bool m_Critical; //Признак монопольного режима.

  enum Parms { prmUser, prmTopic, prmHid, prmPathFile, prmPassword, prmTaskType, prmDatabase, prmURL, prmCharset };
  public:
    Thread( int iSockID, QObject *parent ) : QThread( parent ), m_SocketDescriptor( iSockID ), m_Critical(false) {}
    virtual void run();  //Эта функция выполнения поток.
  signals:
    void Error( QTcpSocket::SocketError socketerror ); 
    public slots:
    void ReadyRead();  //Готовность к чтению.
    void Disconnected(); //Отключение потока
  };

class ContentCreator : public RichTextDocument
  {
  public:
    ContentCreator( const QString& TempPath ) : RichTextDocument( TempPath ) {}
    QByteArray GetContent( PDescrList );
  };
