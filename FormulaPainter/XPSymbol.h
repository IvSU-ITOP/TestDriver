#ifndef MATH_XPSYMBOL
#define MATH_XPSYMBOL
#include <qmap.h>
#include <qset.h>
#include <initializer_list>
#include "formulapainter_global.h"

#define _infinity(C) C == msInfinity
#define UCHAR(x) x + 0xF000

FORMULAPAINTER_EXPORT bool In( uchar C, uchar *S );
FORMULAPAINTER_EXPORT bool inline In( uchar C, const char *S ) { return In( C, ( uchar* ) S ); }
bool _operation( uchar C );
bool _relation( uchar C);
FORMULAPAINTER_EXPORT bool _printable( uchar C );
FORMULAPAINTER_EXPORT bool _litera( uchar C );
bool _bracket( uchar C );
FORMULAPAINTER_EXPORT bool _separator( uchar C );
FORMULAPAINTER_EXPORT bool _inside_ident( uchar C );
bool _start_ident( uchar C );
FORMULAPAINTER_EXPORT QByteArray charToTex( uchar Ch );
FORMULAPAINTER_EXPORT QByteArray Encode( const QByteArray& Formula );
bool SplitOperSign( uchar SignChar );

const unsigned char
msBaseLang = 15,
msIdle = 16,
msSystem = 123,
msCharNewLine = 254,
msPlus = 43,
msMinus = 45,
msDivi = 58,
msMetaSign = '@',
msDegree = 221,
msMinute = 222,
msInvisible = 128,
msLowReg = '_',
msHighReg = '\\',
msMultSign1 = '*',
msMultSign2 = 228,
msBigAlpha = 161,
msBigBetta = 162,
msBigGamma = 163,
msBigDelta = 164,
msBigEpsilon = 165,
msBigDZeta = 166,
msBigEta = 167,
msBigTeta = 168,
msBigIota = 169,
msBigKappa = 170,
msBigLambda = 171,
msBigMu = 172,
msBigNu = 173,
msBigKsi = 174,
msBigOmicron = 175,
msBigPi = 176,
msBigRo = 177,
msBigSigma = 178,
msBigTau = 179,
msBigYpsilon = 180,
msBigFi = 181,
msBigHi = 182,
msBigPsi = 183,
msBigOmega = 184,
msAlpha = 185,
msBetta = 186,
msGamma = 187,
msDelta = 188,
msEpsilon = 189,
msDZeta = 190,
msEta = 191,
msTeta = 192,
msIota = 193,
msKappa = 194,
msLambda = 195,
msMu = 196,
msNu = 197,
msKsi = 198,
msOmicron = 199,
msPi = 200,
msRo = 201,
msSigmaX = 202,
msSigma = 203,
msTau = 204,
msYpsilon = 205,
msFi = 206,
msHi = 207,
msPsi = 208,
msOmega = 209,
msBird = 210,
msApprox = 211,
msRound = 212,
msAround = 213,
msParallel = 214,
msNotequal = 215,
msIdentity = 216,
msMinequal = 217,
msMaxequal = 218,
msPlusMinus = 219,
msMinusPlus = 220,
msCross = 223,
msTriangle = 224,
ms3Points = 226,
msAddition = 227,
msInfinity = 229,
msImUnit = 230,
msCConjugation = 232,
msMean = 233,
msArrowToRight = 234,
msDoubleArrow = 235,
msIntersection = 236,
msStrictInclusion = 237,
msUnstrictInclusion = 238,
msGenQuantifier = 239,
msExQuantifier = 240,
msArrowsUp = 241,
msArrowsUpDown = 242,
msNotBelong = 243,
msIntegral = 244,
msDoubleIntegral = 127,
msTripleIntegral = 129,
msContourIntegral = 231,
msUnion = 245,
msBrackLeftUpper = 246,
msBrackLeftLower = 247,
msBrackRightUpper = 248,
msBrackRightLower = 249,
msPartialDiff = 225,
msPrime = 251,
msDoublePrime = 252,
msTriplePrime = 253,
msLongMinus = 130,
msLongPlus = 131;

class SetOfChars : public QSet<uchar>
  {
  public:
    FORMULAPAINTER_EXPORT SetOfChars( const uchar *Set );
    SetOfChars( const char *Set ) : SetOfChars( ( uchar* ) Set ) {}
    FORMULAPAINTER_EXPORT SetOfChars( const std::initializer_list<uchar> &Set );
    SetOfChars( const QSet<uchar> &Set ) : QSet<uchar>( Set ) {}
  };

const unsigned char dftMultSign = msMultSign1;

class TexNames : public QMap<QByteArray, uchar>
  {
  public:
    TexNames();
    static TexNames Symbols;
  };
#endif
