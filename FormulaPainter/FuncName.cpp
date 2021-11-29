#include "FuncName.h"

QByteArray funct_name[][3] = 
  {
    { "abs", "abs", "abs" },
    { "cosec", "cosec", "cosec" },
    { "csc", "cosec", "cosec" },
    { "sc", "sec", "sec" },
    { "sec", "sec", "sec" },
    { "arcsin", "arcsin", "arcsin" },
    { "arccos", "arccos", "arccos" },
    { "arctan", "arctan", "arctan" },
    { "arctg", "arctan", "arctan" },
    { "arccot", "arccot", "arccot" },
    { "arcctg", "arccot", "arccot" },
    { "sin", "sin", "sin" },
    { "cos", "cos", "cos" },
    { "exp", "exp", "exp" },
    { "tan", "tan", "tan" },
    { "ctg", "cot", "cot" },
    { "tg", "tan", "tan" },
    { "cot", "cot", "cot" },
    { "ctn", "cot", "cot" },
    { "diff", "diff", "diff" },
    { "ln", "ln", "ln" },
    { "lg", "lg", "lg" },
    { "log", "log", "log" },
    { "sign", "sign", "sign" },
    { "div", "div", "div" },
    { "mod", "mod", "mod" },
    { "int", "int", "int" },
    { "round", "round", "round" },
    { "frac", "frac", "frac" },
    { "mixer", "mixer", "" },
    { "fcase", "fcase", "case" },
    { "case", "fcase", "case" },
    { "RtoDEC", "RtoDEC", "RDS" },
    { "RDS", "RtoDEC", "RDS" },
    { "SortA", "SortListA", "SortA" },
    { "SortD", "SortListD", "SortD" },
    { "SimplifyEquation", "SimplifyEquation", "SimplifyEquation" },
//    { "SolutionSystNonEq", "SolutionSystNonEq", "" },
    { "TrigoComplex", "TrigoComplex", "" },
    { "delta", "delta", "" },
    { "UnvisibleTable", "UnvisibleTable", "" },
    { "PartlyVisibleTable", "PartlyVisibleTable", "" },
    { "Table", "Table", "" },
    { "Picture", "Picture", "" },
    { "PlotDots", "PlotDots", "PlotDots" },
    { "PlotLine", "PlotLine", "PlotLine" },
    { "PlotSpline", "PlotSpline", "PlotSpline" },
    { "Chart", "Chart", "" },
    { "CreateObject", "CreateObject", "" },
    { "SimplifyFull", "SimplifyFull", "SimplifyFull" },
    { "Simplify", "Simplify", "Simplify" },
    { "PolinomSmplf", "PolinomSmplf", "PolinomSmplf" },
    { "ListOrd", "ListOrd", "ListOrd" },
    { "RandList", "RandList", "RandList" },
    { "List", "List", "List" },
    { "SimplifyInDetail", "SimplifyInDetail", "Detail" },
    { "Detail", "SimplifyInDetail", "Detail" },
    { "GenChainInEq", "GenChainInEq", "GenChainInEq" },
    { "ReduceToMultiplicators", "ReduceToMultiplicators", "FACTOR" },
    { "FACTOR", "ReduceToMultiplicators", "FACTOR" },
    { "ToLeastCommDenom", "ToLeastCommDenom", "ToLCD" },
    { "ToLCD", "ToLeastCommDenom", "ToLCD" },
    { "FindLeastCommDenom", "FindLeastCommDenom", "LCD" },
    { "LCD", "FindLeastCommDenom", "LCD" },
    { "FindLeastCommMult", "FindLeastCommMult", "LCM" },
    { "LCM", "FindLeastCommMult", "LCM" },
    { "FindGreatestCommDivisor", "FindGreatestCommDivisor", "GCF" },
    { "GCD", "FindGreatestCommDivisor", "GCF" },
    { "GCF", "FindGreatestCommDivisor", "GCF" },
    { "DVR", "Divisor", "DVR" },
    { "DVD", "Dividend", "DVD" },
    { "Der", "Der", "Der" },
    { "DrStroke", "DrStroke", "DrStroke" },
    { "DoubleIntegral", "DoubleIntegral", "DoubleIntegral" },
    { "TripleIntegral", "TripleIntegral", "TripleIntegral" },
    { "CurveIntegral", "CurveIntegral", "CurveIntegral" },
    { "SurfaceIntegral", "SurfaceIntegral", "SurfaceIntegral" },
    { "ContourIntegral", "ContourIntegral", "ContourIntegral" },
    { "Integral", "Integral", "Integral" },
    { "Zumma", "Zumma", "Zumma" },
    { "Product", "Product", "Product" },
    { "Vector", "Vector", "Vector" },
    { "Matric", "Matric", "Matric" },
    { "ERF", "ERF", "ERF" },
    { "GetEl", "GetEl", "GetEl" },
    { "Lim", "Lim", "Lim" },
    { "subst", "subst", "subst" },
    { "syst", "syst", "syst" },
    { "factorial", "factorial", "factorial" },
    { "PerCount", "PerCount", "PerCount" },
    { "BinomCoeff", "BinomCoeff", "BinomCoeff" },
    { "ACoeff", "ACoeff", "ACoeff" },
    { "SetTrigoSystem", "SetTrigoSystem", "SetTrigoSystem" },
    { "SetShowMinute", "SetShowMinute", "SetShowMinute" },
    { "SetShowMultSign", "SetShowMultSign", "SetShowMultSign" },
    { "Polygon", "Polygon", "Polygon" },
    { "SaveBrackets", "SaveBrackets", "SaveBrackets" },
    { "func", "func", "func" },
    { "S", "S", "S" },
    { "rand_with_prohib", "rand_with_prohib", "rand_with_prohib" },
    { "signum", "signum", "signum" }
  };

int count_funct_name = sizeof( funct_name ) / sizeof( funct_name[0] );

int s_TaskEditorOn = false;
int s_iDogOption = 0;

bool IsFunctName( const QByteArray &S, int &fnum )
  {
  if( S.isEmpty() ) return false;
  int p = 0;
  if( (char) S[0] == msMetaSign ) p++;
  for( fnum = 0; fnum < count_funct_name; fnum++ )
    {
    int len = funct_name[fnum][0].length();
    if( len != S.length() - p ) continue;
    int i = 0;
    for( ; i < len && S[p + i] == funct_name[fnum][0][i]; i++ );
    if( i == len ) 
      return true;
    }
  return false;
  }

QByteArray InputFunctName( int fnum )
  {
  if( fnum >= 0 && fnum < count_funct_name ) return funct_name[fnum][0];
  return "";
  };

QByteArray OutputFunctName( const QByteArray& StartName )
  {
  int iName = 0;
  QByteArray sResult;
  uchar FirstChar = StartName[0];
  if( s_TaskEditorOn && s_iDogOption > 0 && FirstChar == '@' )
    {
    for( ; iName < count_funct_name && '@' + funct_name[iName][1] != StartName; iName++ );
    if( iName < count_funct_name )
      sResult = '@' + funct_name[iName][2];
    return sResult;
    }
  for( ; iName < count_funct_name && funct_name[iName][1] != StartName; iName++ );
  if( iName < count_funct_name ) sResult = funct_name[iName][2];
  return sResult;
  }

QByteArray InterNameByInputName( const QByteArray& Name )
  { 
  int i = 0;
  char FirstChar = Name[0];
  if( s_TaskEditorOn && s_iDogOption > 0 && FirstChar == '@' )
    {
    for( i = 0;  i < count_funct_name &&  '@' + funct_name[i][0] != Name;  i++);
    if( i < count_funct_name ) return '@' + funct_name[i][s_iDogOption];
    }
  else
    {
    for( ; i < count_funct_name && funct_name[i][0] != Name; i++ );
    if( i < count_funct_name ) return funct_name[i][1];
    }
  return "";
  };
