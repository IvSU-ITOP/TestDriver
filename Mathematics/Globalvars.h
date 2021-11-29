#ifndef MATH_GLOBALVARS
#define MATH_GLOBALVARS
#include "../FormulaPainter/XPTypes.h"
#include "../FormulaPainter/Messtrs.h"
#include "mathematics_global.h"

#pragma once

const int
  liOK = 0,  // ok
  liNotCorrect = -1,  // not correct input of equations system
  liNotLinear = -2,  // input system it is not linear
  liNot1Decision = -3,  // system have not one decision
  liNotSUBST = -4,  // not possibility to solve by method Substitution
  liNoFinish = -5,  // process of solving have not yet finish
  liNotFoundRoots = -6,  // roots are not found
  liNotEXCHANGE = -7,  // not possibility to solve by method EXCHANGE
  liEquationSolved = -8, // logarithmic or exponential equation was solved
  liEmpty = -9,  // empty object (no input information)
  liInfinityDecision = -10,  // system have Infinite set of solutions // NP
  liSystemSolved = -11,
  liSystemOverdetermined = -12,
  liWasEmptyName = -13,
  liSquareOnly = 1,  //a1x^2+b1y^2=c1; a2x^2+b2y^2=c2
  liProductAndSimple = 2, //a1x+b1y+c1xy=d1;a2x+b2y+c2xy=d2
  liSquareAndSimple = 3, //a1x^2+b1y^2+c1x+d1y=e1;a2x^2+b2y^2+c2x+d2y=e3
  liSquareAndProduct = 4, //a1x^2+b1y^2+c1xy=d1;a2x^2+b2y^2+c2xy=d2
  liSquareAndProduct1 = 5, //a1x^2+b1y^2+c1xy=d1;c2xy=d2
  liDenomsPQPQ = 6, //a1/P+b1/Q=c1; a2/P+b2/Q=c2,
  liDenomsPQP2Q2 = 7, //a1/P+b1/Q=c1; a2/P^2+b2/Q^2=c2;
  liDenomsP2Q2P2Q2 = 8, //a1/P^2+b1/Q^2=c1; a2/P^2+b2/Q^2=c2;
  liSquareAndSimpleAndProduct = 9, //a1x^2+b1y^2+c1x+d1y+f1xy=e1;a2x^2+b2y^2+c2x+d2y+f2xy=e3
  liPutNone = 0,
  liPutInitialSystem = 1,
  liPutFirstStage = 2,
  liPutSecondStage = 4,
  liPutAll = 0xFFFF;

  extern bool s_TruePolynom;
  MATHEMATICS_EXPORT extern double s_Precision;
  MATHEMATICS_EXPORT extern double s_OldPrecision;
  extern bool s_bSuppressErrorMsg;
  extern char s_DecimalSeparator;
  MATHEMATICS_EXPORT extern bool s_GlobalInvalid;
  MATHEMATICS_EXPORT extern bool s_InvalidExpand;

  MATHEMATICS_EXPORT extern bool s_CheckError;
  MATHEMATICS_EXPORT extern memory_mode s_MemorySwitch;
  extern bool s_NoLogReduce;
  extern bool s_NoExpReduce;
  extern bool s_GlobalTrigonomError;
  extern bool s_TanCotError;
  extern int s_FuncArgListDepth;
  MATHEMATICS_EXPORT extern bool s_ShowMinute;
  MATHEMATICS_EXPORT extern TXPStatus s_XPStatus;
  MATHEMATICS_EXPORT extern double s_XPSimNumber;
  extern bool s_GeoActivate;
  MATHEMATICS_EXPORT  extern bool s_RestoreObjectPicture;
  MATHEMATICS_EXPORT  extern bool s_XPMonitorMode;
  MATHEMATICS_EXPORT extern bool s_WasDefinition;
  MATHEMATICS_EXPORT extern QByteArrayList s_EStrList;
  MATHEMATICS_EXPORT extern bool s_ExpandDetailed;
  extern bool s_IsLogEquation;
  extern bool s_CanExchange;
  extern int s_IntegralCount;
  extern int s_OpenMultAmbiguity;
  extern bool s_EqualPicture;
  extern bool s_IntegralError;
  extern bool s_SmartReduceLogArg;
  extern bool s_RootToPower;
  extern bool s_PowerToMult;
  extern bool s_IsIntegral;
  extern bool s_FractionToPower;
  extern bool s_PowerToFraction;
  extern bool s_EquationSolving;
  extern QByteArray s_GlobalVarName;
  extern bool s_IsTrinom;
  extern bool s_SummExpFactorize;
  MATHEMATICS_EXPORT extern bool s_NoRootReduce;
  extern bool s_PutAnswer;
  extern bool s_ReductionMustBeContinued;
  extern bool s_PowerToRoot;
  extern bool s_NoLogSquarReduce;
  extern bool s_ExpandPower;
  extern bool s_ExpandLog;
  extern bool s_ClassicTrinom;
  extern bool s_ReductionMustBeContinued;
  extern bool s_FinalComment;
  extern bool s_Calculations;
  extern class MathExpr s_Answer;
  extern MathExpr s_AnswerTrigo;
  extern bool s_FactorizedSolving;
  extern int s_RootCount;
  extern int s_FactorCount;
  extern int s_DetailDegree;
  const int sc_NumError = -2147483647;

#endif
