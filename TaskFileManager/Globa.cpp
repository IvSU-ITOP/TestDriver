#include "taskfilemanager.h"
#include "Globa.h"
#define ISALPHA(x) ((x >= 'a' && x <= 'z' ) || (x >= 'A' && x <= 'Z' ))

QString ToLanguage( const QByteArray& Text, TLanguages Lang )
  {
  static QHash<TLanguages, QTextCodec*> Codec;
  if( !Codec.contains( Lang ) ) Codec.insert( Lang, QTextCodec::codecForName( Lang == lngHebrew ? "Windows-1255" : "Windows-1251" ) );
  return Codec[Lang]->toUnicode( Text );
  }

QByteArray FromQBB( const QByteArray& CS, TLanguages Language )
  {
//  if( BaseTask::sm_NewHebFormat || Language != lngHebrew ) return CS;
  QByteArray S( CS );
  int iDefis = S.indexOf( '-' );
  if( iDefis > 1 && ( S[iDefis - 1] == ')' || ISALPHA( S[iDefis - 1] ) ) )
    S.insert( iDefis, ' ' );
  if( S.endsWith( '-' ) )
    {
    S = S.trimmed();
    S.chop( 1 );
    if( S.startsWith( '.' ) )
      S.insert( 1, "- " );
    else
      S.insert( 0, "- " );
    }
  bool bWordHeb = false;
  bool bInvert = false;
  QByteArray sWord, sResult, sWord2;
  int iSpaceCount = 0;
  uchar COld = 0, C;
  for( int iPos = 0; iPos < S.length(); iPos++, COld = C )
    {
    C = S[iPos];
    if( iPos == iSpaceCount && ( C == '.' || C == ':' ) )
      {
      sWord.insert( 0, C );
      bInvert = true;
      continue;
      }
    if( C == ' ' )
      {
      if( bInvert )
        {
        sWord.insert( 0, sWord2 );
        sWord2.clear();
        sWord.insert( 0, ' ' );
        continue;
        }
      iSpaceCount++;
      continue;
      }
    if( C >= 224 && C <= 250 || C == ')' || C == '(' || C == '-' )
      {
      if( !bWordHeb )
        {
        sWord.insert( 0, sWord2 );
        sWord2.clear();
        sResult = QByteArray( iSpaceCount, ' ' ) + sWord + sResult;
        sWord.clear();
        bWordHeb = true;
        bInvert = false;
        }
      else
        if( iSpaceCount > 0 ) sWord += QByteArray( iSpaceCount, ' ' );
      sWord += C;
      iSpaceCount = 0;
      continue;
      }
    if( bInvert )
      {
      sWord2 += C;
      continue;
      }
    if( C == ':' )
      {
      sWord.insert( 0, C );
      sWord += QByteArray( iSpaceCount, ' ' );
      iSpaceCount = 0;
      continue;
      }
    if( C >= 'A' && C <= 'Z' || C >= 'a' && C <= 'z' )
      {
      if( bWordHeb )
        {
        sResult = QByteArray( iSpaceCount, ' ' ) + sWord + sResult;
        sWord.clear();
        bWordHeb = false;
        }
      else
        {
        if( sWord[0] == ':' )
          {
          sResult = QByteArray( iSpaceCount, ' ' ) + sWord + sResult;
          sWord.clear();
          iSpaceCount = 0;
          }
        if( iSpaceCount > 0 ) sWord = QByteArray( iSpaceCount, ' ' ) + sWord;
        }
      sWord += C;
      iSpaceCount = 0;
      continue;
      }
    sWord += QByteArray( iSpaceCount, ' ' );
    sWord += C;
    iSpaceCount = 0;
    }
  return QByteArray( iSpaceCount, ' ' ) + sWord + sResult;
  }

QString FromQBA( const QByteArray& CS, TLanguages Language, bool AddChar )
  {
  if( CS.isEmpty() ) return "";
//  if( BaseTask::sm_NewHebFormat || Language != lngHebrew ) return ToLanguage( CS, Language );
  QByteArray sResult = FromQBB( CS, Language );
  if( AddChar ) sResult.push_front( ' ' );
  return ToLanguage( sResult, Language );
  }
