#include "../FormulaPainter/XPTypes.h"
#include "../FormulaPainter/Messtrs.h"
#include "ExpOb.h"

bool s_TruePolynom = false;
double s_Precision = 0.01;
double s_OldPrecision = 0.01;
bool s_bSuppressErrorMsg = false;
char s_DecimalSeparator = '.';
bool s_GlobalInvalid = false;
bool s_InvalidExpand = false;
bool s_CheckError = false;
memory_mode s_MemorySwitch = SWtask;
bool s_NoLogReduce = false;
bool s_NoExpReduce = false;
bool s_GlobalTrigonomError = false;
bool s_TanCotError = false;
int s_FuncArgListDepth = 0;
bool s_ShowMinute = 0;
TXPStatus s_XPStatus;
bool s_ClassicTrinom = false;
double s_XPSimNumber = -1.0;
bool s_GeoActivate = false;
bool s_RestoreObjectPicture = true;
bool s_XPMonitorMode = false;
bool s_WasDefinition = false;
bool s_XPTestMarksMode = false;
bool s_XPOwnerTestMode = false;
QByteArrayList s_EStrList;
bool s_ExpandDetailed = false;
bool s_IsLogEquation = false;
bool s_CanExchange = true;
int s_IntegralCount = 0;
int s_OpenMultAmbiguity = false;
bool s_EqualPicture = true;
bool s_IntegralError = false;
bool s_SmartReduceLogArg = false;
bool s_RootToPower = false;
bool s_PowerToMult = false;
bool s_IsIntegral = false;
bool s_FractionToPower = false;
bool s_PowerToFraction = false;
bool s_EquationSolving = false;
QByteArray s_GlobalVarName;
bool s_IsTrinom = false;
bool s_SummExpFactorize = false;
bool s_NoRootReduce = false;
bool s_PutAnswer = false;
MathExpr s_Answer;
MathExpr s_AnswerTrigo;
bool s_ReductionMustBeContinued = false;
bool s_PowerToRoot = false;
bool s_NoLogSquarReduce = false;
bool s_ExpandPower = false;
bool s_ExpandLog = false;
bool s_FinalComment = true;
bool s_Calculations = true;
bool s_FactorizedSolving = false;
int s_RootCount;
int s_FactorCount = 0;
int s_DetailDegree = liPutAll;








