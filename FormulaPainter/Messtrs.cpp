#include "Messtrs.h"
#include "InEdit.h"

bool s_CalcOnly = false;
QString s_LastError;
void( *s_AfterMsg )( ) = nullptr;
bool TXPStatus::sm_ShowErrMessage = false;

Translator Trans( ":/Resources/Settings.eng" );
Translator TransHeb( ":/Resources/Settings.heb" );
Translator TransRus( ":/Resources/Settings.rus" );

Translator::Translator( const QString& File )
  {
  QFile Fin( File );
  Fin.open( QIODevice::ReadOnly );
  while( !Fin.atEnd() )
    {
    QByteArray S( Fin.readLine().trimmed() );
    if( S.indexOf( '[' ) != -1 ) continue;
    int iPos = S.indexOf( '=' );
    if( iPos == -1 ) continue;
    insert( S.left( iPos ), S.mid( iPos + 1 ) );
    }
  }

QString X_Str( const QByteArray& Ident, const QByteArray& Default )
  {
  return ToLang( X_BStr( Ident, Default ) );
  }

QByteArray X_BStr( const QByteArray& Ident, const QByteArray& Default )
  {
  QByteArray Result( XPInEdit::sm_Language == lngHebrew ? TransHeb[Ident] :
     XPInEdit::sm_Language == lngRussian ? TransRus[Ident] : Trans[Ident]);
  if( Result.isEmpty() ) return Default;
  return Result;
  }

void Error_m( const QString& S )
  {
  if( XPInEdit::sm_Message == nullptr )
    {
    if( s_CalcOnly )
      QMessageBox::critical( nullptr, Trans["HMMsgDlg_dlgError"], S );
    }
  else
    XPInEdit::sm_Message( S );
  s_LastError = S;
  }

void Error_m( const QByteArray& S )
  {
  Error_m( ToLang( S ) );
  }

void Info_m( const QByteArray& S )
  {
  Info_m( ToLang( S ) );
  }

void Info_m( const QString& S )
  {
  if( XPInEdit::sm_Message == nullptr )
    QMessageBox::information( nullptr, "Information", S );
  else
    XPInEdit::sm_Message( S );
  if( s_AfterMsg != nullptr ) s_AfterMsg();
  }

QString ToLocal( const QByteArray& Text )
  {
  return ToLang( Text );
  }

void TXPStatus::SetMessage( const QByteArray& AMessage ) 
  {
  m_CurrentMessage = ToLang( AMessage );
  }

QString TXPStatus::GetCurrentMessage()
  {
  QString Result = m_CurrentMessage;
  m_CurrentMessage.clear();
  return Result;
  }

void TXPStatus::SetMessage( const QString& AMessage )
  {
  m_CurrentMessage = AMessage;
  }

/*

Unit MessStrs;
(****************************************************
****************************************************)

Interface

{============================================================================}

Uses
	ExtCtrls;

Type

TXPStatus = class
  private
	CurrentMessage,					{	message string			}
	DefaultMessage	:	string;		{	default message string	}

	FPanel			:	TPanel;

void ShowMessage() {}

void SetMessage(QByteArray AMessage) {}
void SetPanel(tpanel APanel) {}

  public

	property StMessage : string read CurrentMessage write SetMessage;
	property DfMessage : string read DefaultMessage write DefaultMessage;

	property Panel : TPanel read FPanel write SetPanel;

end;

{-------------------------------------------------------------------------}

void Error_m(QByteArray s) {}
void CError_m(QByteArray s) {}
void Info_m(QByteArray s) {}
(*
void Information() {}
*)
QByteArray T_str(word N, word L) {return "";}

QByteArray X_Str(QByteArray const Section, QByteArray Ident, QByteArray Default) {return "";}

{============================================================================}

Implementation

Uses
	Classes,
	Dialogs,
	StrsFunc,
	GlobVars,	{...}
  SysUtils,
	HMGlbVrs;	{...}

{============================================================================}

void TXPStatus.ShowMessage() {}
begin
	if not Assigned( FPanel ) then
		exit;

	if Language > 0 then
		Panel.Alignment := taLeftJustify
	else
		Panel.Alignment := taRightJustify;
	Panel.Caption := CurrentMessage;
end;

{-------------------------------------------------------------------------}

void TXPStatus.SetMessage(QByteArray AMessage) {}
begin
	if AMessage = '' then
		AMessage := DefaultMessage;
	if AMessage = CurrentMessage then
		exit;

	CurrentMessage := AMessage;

	ShowMessage;
end;

{-------------------------------------------------------------------------}

void TXPStatus.SetPanel(tpanel APanel) {}
begin
	FPanel := APanel;

	ShowMessage;
end;

{============================================================================}

void Info_m(QByteArray s) {}
begin

	MessageDlg( s, mtInformation, [mbOK], 0 );

end;

{-------------------------------------------------------------------------}

void Error_m(QByteArray s) {}
begin
        if not CalcQuadrEqu and not CalcOnly then  // kag 14.03.2000 prevent Msg DivByZero
	MessageDlg( s, mtError, [mbOK], 0 );

	LastError := s;
end;

{-------------------------------------------------------------------------}

void CError_m(QByteArray s) {}
begin
	if ( s <> '' ) and ( LastError = '' ) then
		Error_m( s );

	LastError := s;
end;

{-------------------------------------------------------------------------}
(*
{V.15.07.96}
void Information() {}
Begin
{#$31.07.97	if ( ErrorNmb <> 0 ) and ( s_XPStatus.SetMessage( '' ) then	}
		s_XPStatus.StMessage := t_str( ErrorNmb, Language );

	{ if (message = '') or (memory_switch <> SWcalculator) then exit;}

	case ErrorNmb of
		26,47,49,56,69,70,71,96:		error_m ( s_XPStatus.StMessage );
		25,27,64,65,68,77,78,92,94,98:	info_m  ( s_XPStatus.StMessage );
	else
		exit;
	end;

	s_XPStatus.StMessage := '';
	ErrorNmb := 0;
End;
*)
{-------------------------------------------------------------------------}
(*
{	...	Exceptions handling	!!!...	}
QByteArray T_str(word N, word L) {return "";}
var T : TextFile;
	S : string;
	C : Integer;
begin
	AssignFile( T, 'text_str.txt' );	{...}
	Reset( T );

	C := 0;
	while not EOF( T ) and ( C < ( 2 * N + L - 1 ) ) do		{...}
	begin
		Readln( T, S );
		Inc( C );
	end;

	CloseFile( T );

	{	And if NOT FOUND ?!!...	}
	t_str := CopyStrTail( S, pos( '%', S ) + 1 );

end;
*)
{-------------------------------------------------------------------------}

QByteArray X_Str(QByteArray const Section, QByteArray Ident, QByteArray Default) {return "";}
begin
        Result:= '';
  ReadSettings;
	if Language = 0 then
      Result := trim( SetsIniLstHeb.Values[ Ident] )  // kag 24.02.2000   .ReadString( Section, Ident, Default )
  else
      Result := trim( SetsIniLstEng.Values[ Ident] );  // kag 24.02.2000   .ReadString( Section, Ident, Default )
	if Result = '' then
           Result := Default;
{	if Assigned( SetsIni )
           then Result := SetsIni.ReadString( Section, Ident, Default )
	   else Result := Default;}
end;

{============================================================================}

end.
*/
