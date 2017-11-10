/*
  Usage:

    ./smrc_NNN.c meta_kernel.tm [--utcest=2019-01-01T05:33:00]

  Compile:

    gcc -Icspice/include smrc_NNN.c cspice/lib/cspice.a -lm

    - cspice/ is the top-level directory of the NAIF/JPL CSPICE library,
      or a symlink to same
      - cf. https://naif.jpl.nasa.gov

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SpiceUsr.h"


/**********************************************************************/
void
printVec3(SpiceDouble* v, char* sPfxArg, char* sSfxArg, char* sFmtArg) {
char* sFmt = sFmtArg ? sFmtArg : "%.12lg";
char* sPfx = sPfxArg ? sPfxArg : "";
char* sSfx = sSfxArg ? sSfxArg : "\n";
int i;
  fprintf(stdout, "%s[", sPfx);
  for (i=0; i<3; ++i) {
    if (i) fprintf(stdout, ", ");
    fprintf(stdout, sFmt, v[i]);
  }
  fprintf(stdout, "]%s", sSfx);
}


/**********************************************************************/
void
printMtx3x3(SpiceDouble* mtx0, char* sLabel, char* sFmtArg) {
int i;
  fprintf(stdout, "%s%s", sLabel ? sLabel : "", sLabel ? "\n" : "");
  for (i=0; i<3; ++i) {
    printVec3( mtx0 + (i*3), i ? " " : "[", i==2 ? "]\n" : ",\n", sFmtArg);
  }
}


/**********************************************************************/
#define MAXWIN 200
#define NINTVL (MAXWIN >> 1)

int
main(int argc, char** argv) {
int iArg;
SpiceChar* utcEst = { "2019-01-01T12:00:00" };
char* pArg;
SpiceDouble etEst;
SpiceDouble dEt;
SPICEDOUBLE_CELL (cnfine, MAXWIN);
SPICEDOUBLE_CELL (result, MAXWIN);
SpiceDouble adjust = 0.0;
SpiceDouble refval = 0.0;
SpiceDouble spd = spd_c();     /* s/d */
SpiceDouble dpr = dpr_c();     /* deg/radian */
SpiceDouble step = 3600;       /* Step size = 1h */
SpiceChar* abcorr = getenv("ABCORR") ? getenv("ABCORR") : "LT";
SpiceChar* sTarget = { "2486958" };
SpiceChar* sSc = { "-98" };

  for (iArg=1; iArg<argc; ++iArg) {

    pArg = argv[iArg];

    if (!strncmp("--utcest=",pArg,9)) {
      utcEst = pArg + 9;
      fprintf(stdout, "Using [%s] as estimate for TCA\n", utcEst);
      continue;
    }

    /* Default:  assume SPICE kernel to be FURNSHed */
    furnsh_c(argv[iArg]);
  }

  str2et_c(utcEst,&etEst);

  wninsd_c(etEst - 5*spd, etEst + 5*spd, &cnfine);

  gfdist_c( sTarget, abcorr, sSc, "ABSMIN", refval
          , adjust, step, NINTVL, &cnfine, &result);

  if (1 != wncard_c(&result)) return 1;

  {
  SpiceDouble etStart;
  SpiceDouble etStop;
  SpiceDouble stScToTarget[6];
  SpiceDouble lt;
  SpiceDouble scVel[3];
  SpiceDouble scPos[3];
  SpiceDouble vBnorm[3];
  SpiceDouble rotXYZ[3];
  SpiceDouble mtxJ2kToUncert[3][3];
  SpiceChar utcTCA[50];

    wnfetd_c( &result, 0, &etStart, &etStop);

    spkezr_c(sTarget, etStart, "J2000", abcorr, sSc, stScToTarget, &lt);

    vminus_c(stScToTarget, scPos);
    vminus_c(stScToTarget+3, scVel);

    vcrss_c(scPos, scVel, vBnorm);
    twovec_c(scVel, 1, vBnorm, 2, mtxJ2kToUncert);

    m2eul_c(mtxJ2kToUncert, 1, 2, 3, rotXYZ+0, rotXYZ+1, rotXYZ+2);
    vscl_c(dpr, rotXYZ, rotXYZ);

    et2utc_c(etStart, "ISOC", 6, sizeof utcTCA, utcTCA);
    fprintf(stdout, "%s; tErr=%lgs; pos-Vel_AngleErr = %.7lgdeg\n"
           , utcTCA
           , etStop-etStart
           , 90.0 - (dpr * vsep_c(scPos, scVel))
           );

    printMtx3x3(mtxJ2kToUncert[0], "\nMTX[J2000=>ABC]", 0);
    printVec3(rotXYZ, "\nRotateXYZ:", "deg\n\n", 0);
  }

  return 0;
}
