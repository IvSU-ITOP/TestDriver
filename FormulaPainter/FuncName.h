#ifndef MATH_FUNCNAME
#define MATH_FUNCNAME
#include <qbytearray.h>
#include "XPSymbol.h"
#include "formulapainter_global.h"
#pragma once


const char IntegrName[]	= "Integral"; 
const char DoubleIntegrName[] = "DoubleIntegral";
const char TripleIntegrName[] = "TripleIntegral";
const char CurveIntegrName[] = "CurveIntegral";
const char SurfaceIntegrName[] = "SurfaceIntegral";
const char ContourIntegrName[] = "ContourIntegral";
const char 	GSummName[] = "Zumma";
const char 	LimitName[]	= "Lim";
const char 	GMultName[]	= "Product";
const char 	SubstName[]	= "subst";
const char 	DerivName[]	= "Der";
const char 	AbsName[]	= "abs";
const char 	SystName[]	= "syst";
const char 	VectName[]	= "Vector";
const char 	MatrName[]	= "Matric";
const char 	DetSummName[]	= "detsumm";
const char 	DetMultName[]	= "detmult";
const char 	DetRMultName[]	= "detrmult";
const char  DetDivName[]	= "detdiv";
const char  DetRDivName[]	= "detrdiv";
const char 	DetRSummName[]	= "detrsumm";
FORMULAPAINTER_EXPORT extern int s_TaskEditorOn;
FORMULAPAINTER_EXPORT extern int s_iDogOption;
FORMULAPAINTER_EXPORT extern int count_funct_name;
//Use to indicate option to process @ in OutPut from Parser
//O - replace known variable @VarName by their value
// 1 - Don't replace

FORMULAPAINTER_EXPORT bool IsFunctName( const QByteArray &S, int &fnum );
FORMULAPAINTER_EXPORT QByteArray InputFunctName( int );
FORMULAPAINTER_EXPORT QByteArray OutputFunctName( const QByteArray& StartName );
FORMULAPAINTER_EXPORT QByteArray InterNameByInputName( const QByteArray& Name );

#endif
