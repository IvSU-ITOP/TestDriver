#include "StrHebEng.h"
#include "XPSymbol.h"
#include <ctype.h>
#include <string>
#include <qstring.h>

using namespace std;

bool IsEngChar(uchar C)
  {
  return isalpha( C ) || C == '@' || C == '$';
  }

bool IsNoHebrew(uchar c)
  {
  return isdigit(c) || IsEngChar(c) || _operation(c) || _relation(c) || _infinity(c) || _bracket(c);
  }

bool IsHebChar(uchar c)
  {
  return c >= 224 && c <= 250;
//  return c >= 192 && c <= 255; //also Russian
  }

map<string, string> XMessages = { 
    { "MDenomIs0", "Denominator equals 0" } 
  };
/*
QString X_Str( const uchar* Key, const uchar* Default )
  {
  string Msg = XMessages[(char*) Key];
  if( Msg.empty() ) return (char*) Default;
  return Msg.data();
  }
  */

/*

implementation
uses
  Sysutils,
  HMGlbVrs,
  TaskClass,
  XPSymbol;


function  IsDigit(c:char) : boolean;
{
  IsDigit := (c in ['0'..'9']);
};



//******************************************************************************
//Invert order of characters in subsequences inside
// s[iStart]..[sEnd]  (including first and last characters of segment)
//Decimal point could be only between digits
//******************************************************************************
Procedure InvertDigitSequences(Recode[s : string; iStart, iEnd : integer);
var
  i,j,k1,k2 : integer;
  c: char;
{
  i:=iStart;
  while i<iEnd do
    {
      if not (s[i] in ['0'..'9']) then
        {
          Inc(i);
          Continue;
        };
      //Now i - index of first digit of subsequence
      j:=i;
      while ( ((j+1)<=iEnd) and (s[j+1] in ['0'..'9'])  )  or
            ( ((j+2)<=iEnd) and (s[j+1]='.') and (s[j+2] in ['0'..'9'])  )  do
        Inc(j);
      //now j - index of last digit of subsequence
      if i<j then
        {
          //Invert subsequnce [i..j]
          k1:=i;
          k2:=j;
          while k1<k2 do
            {
              c:=s[k1];
              s[k1]:=s[k2];
              s[k2]:=c;
              Inc(k1);
              Dec(k2);
            };
        };
      i:=j+1;
    };
};
//******************************************************************************
{
 Transformation while Write srting
  If string contains hbrew and English letters, then write it in inverse order
  and inverse each homolanguage fragmen separately
}
//Version 030930
//******************************************************************************
function  WInvertHebrewEnglishString(ss : string) : string;
var
  i,
  k,      //index of destination in result string r
  j1,j2,  //start and } of monolanguage fragment, start position os space tail
  nInitBlank,    //number of spaces in the { of the string
  nEndBlank,     //number of spaces in the } of the string
  nEndNonHeb,    //number of symbols after last Hebrew character and before } blanks (like ":", "-")
  iNonHebrewPrefix //number of symbols between initial blank spaces and first hebrew
                   //characters of monolingual segment. They can occur, if
                   //} of previous sentence is a formula and a point and afterward
                   //goes Hebrew sentence.
    : integer;
  r,s  : string;
  CurLang : integer; //0 - Heb, 1 - Eng
{
  if (Length(Trim(ss))=0) or (Language=1) then
    {
      WInvertHebrewEnglishString:=ss;
      Exit;
    };
  s:=ss;

  r:='';
  for i:=1 to Length(s) do
    r:=r+' ';

  CurLang:=-1;
  //Find language of the first fragment
  for i:=1 to Length(s) do
    {
      if IsHebChar(s[i]) then
        {
          CurLang:=0;
          Break;
*/