#include "XPressTests.h"
#include <qdebug.h>
#include "../Mathematics/Parser.h"
#include "../TaskFileManager/Globa.h"
#include "TaskWork.h"

//int TExpr::sm_DeletedCount = 0;
//QList<TExpr*> TExpr::sm_CreatedList;

//int TNode::sm_NodeCreated = 0;
//int TNode::sm_NodeDeleted = 0;
int TExMemb::sm_ExMembCreated = 0;
int TExMemb::sm_ExMembDeleted = 0;

int CreatedCountOld = 0;
int DeletedCountOld = 0;
int NodeCreatedOld = 0;
int NodeDeletedOld = 0;

void XPressTests()
  {
  QDir dir("XPressTests","*.xpr");
  if( !dir.exists() )
    {
    QMessageBox::critical( nullptr, "Start XPressTests Error", "XPressTests folder not found.\r\nTest stopped." );
    return;
    }
  QString path = dir.absolutePath();

  QFile FOutput( path + "\\!log.txt" );
  if( !FOutput.open( QIODevice::WriteOnly ) )
    {
    QMessageBox::critical(nullptr, "Start XPressTests Error", "Error opening XPressTests log file.\nTest stopped.");
    return;
    }
  QTextStream Fout( &FOutput );

  QStringList Files = dir.entryList();

  if (Files.isEmpty())
    {
    QMessageBox::warning( nullptr, "XPressTests", "Test files not found.\nTest stopped." );
    return;
    }
  
#if _MSC_VER >= 1800
   for each ( QString FileName in Files )
#else
  foreach (QString FileName, Files)
#endif
    {
    QFile FInput( path + '\\' + FileName );
    if( !FInput.open( QIODevice::ReadOnly ) )
      continue;
    Fout << "---" << FileName << "---" << endl;
    QTextStream Fin(&FInput);

    for( QString Line; !Fin.atEnd(); )
      {
      Line = Fin.readLine();
      if( Line.contains( "COMMENTARY" ) )
        continue;
      if( Line.length() == 1 )
        {
//        Fout << "error. readed: " << Line << " error:  read 1 character" <<endl; // Заглушка от нечитаемого символа
        continue;
        }
      try
        {
        MathExpr ParserResult = Parser::StrToExpr( Line.toLocal8Bit() );
        QByteArray WriteEResult;
        if( !ParserResult.IsEmpty() )
          WriteEResult = ParserResult.WriteE();
        if( Line.toLocal8Bit() == WriteEResult )
          Fout << "ok.    ";
        else
          Fout << "error. ";
        Fout << "readed: " << Line << " result: " << ToLang( WriteEResult );
//        Fout << "; NodeCreated: " << TNode::sm_NodeCreated - NodeCreatedOld << "; NodeDeleted: " << TNode::sm_NodeDeleted - NodeDeletedOld;
//        NodeCreatedOld = TNode::sm_NodeCreated;
//        NodeDeletedOld = TNode::sm_NodeDeleted;
        Fout.flush();
        if( !ParserResult.IsEmpty() )
          {
          Fout << "; Eq Self: " << ( ParserResult.Eq( ParserResult ) ? "True" : "False" );
          MathExpr ReduceResult = ParserResult.Reduce();
          Fout << "; reduce: " << ToLang( ReduceResult.WriteE() );
          Fout << "; Eq Reduce: " << ( ParserResult.Eq( ReduceResult ) ? "True" : "False" );
          Fout << "; Equal: " << ( ParserResult.Equal( ReduceResult ) ? "True" : "False" );
          }
        }
      catch( ErrParser e )
        {
        Fout << " error: " << e.Message();
        //e.ShowMessage();
        }
      catch( QString Err )
        {
        ErrParser e( Err.toLocal8Bit(), peNewErr );
        Fout << " error: " << e.Message();
        //e.ShowMessage();
        }
      Fout << "; ExprCreated: " << TExpr::sm_CreatedCount - CreatedCountOld << "; ExprDeleted: " << TExpr::sm_DeletedCount - DeletedCountOld;
      CreatedCountOld = TExpr::sm_CreatedCount;
      DeletedCountOld = TExpr::sm_DeletedCount;
      Fout << endl;
      Fout.flush();
      }
    FInput.close();
    }
//  Fout << "TotalNodeCreated: " << TNode::sm_NodeCreated << "; TotalNodeDeleted: " << TNode::sm_NodeDeleted;
  Fout << "; TotalExprCreated: " << TExpr::sm_CreatedCount << "; TotalExprDeleted: " << TExpr::sm_DeletedCount;
  }