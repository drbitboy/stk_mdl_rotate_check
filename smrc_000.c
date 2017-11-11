/*
  Usage:

    [ABCORR=LT] ./smrc_NNN.c meta_kernel.tm [--utcest=2019-01-01T05:33:00] [--abcorr=LT]

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
/* Print vector [Xcomponent, Ycomponent, Zcomponent] */
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
/* Print matrix [vector,
                 vector,
                 vector]]
 */
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
SpiceChar* sAbcorr = "LT";
SpiceChar* sTarget = { "2486958" };
SpiceChar* sSc = { "-98" };

  for (iArg=1; iArg<argc; ++iArg) {

    /* Process command-line arguments */
    pArg = argv[iArg];

    if (!strncmp("--utcest=",pArg,9)) {
      /* UTC estimate for TCA */
      utcEst = pArg + 9;
      fprintf(stdout, "Using [%s] as estimate for TCA\n", utcEst);
      continue;
    }

    if (!strncmp("--abcorr=",pArg,9)) {
      /* Aberration correction */
      sAbcorr = pArg + 9;
      fprintf(stdout, "Using [%s] for aberration correction\n", utcEst);
      continue;
    }

    /* Default:  assume argument is SPICE kernel to be FURNSHed */
    furnsh_c(argv[iArg]);
  }

  /* Convert UTC to ET, set GF search window to ET +/-5d,
   * find TCA (Time of Closest Approach)
   */
   
  str2et_c(utcEst,&etEst);
  wninsd_c(etEst - 5*spd, etEst + 5*spd, &cnfine);
  gfdist_c( sTarget, sAbcorr, sSc, "ABSMIN", refval
          , adjust, step, NINTVL, &cnfine, &result);

  /* there must be one window in the result */
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

    /* Fetch the ETs from the result window */
    wnfetd_c( &result, 0, &etStart, &etStop);

    /* Calculate target state wrt spacecraft at TCA */
    spkezr_c(sTarget, etStart, "J2000", sAbcorr, sSc, stScToTarget, &lt);

    /* Invert state to get spacecraft position and velocity wrt target */
    vminus_c(stScToTarget, scPos);
    vminus_c(stScToTarget+3, scVel);

    /* B-norm vector is Position cross velocity */
    vcrss_c(scPos, scVel, vBnorm);

    /* Create matrix:  align X with velocity; +Y  with B-norm.
     * +Z will align  with B-mag i.e. S/C position vector in B-plane
     */
    twovec_c(scVel, 1, vBnorm, 2, mtxJ2kToUncert);

    /* Convert matrix to Euler angles, scale to degrees */
    m2eul_c(mtxJ2kToUncert, 1, 2, 3, rotXYZ+0, rotXYZ+1, rotXYZ+2);
    vscl_c(dpr, rotXYZ, rotXYZ);

    /* Convert TCA ET to UTC */
    et2utc_c(etStart, "ISOC", 6, sizeof utcTCA, utcTCA);

    /* Output result; errors should be zero */
    fprintf(stdout, "TCA=[%s]; ABCORR=[%s]; tErr=[%lgs]; TCA-Pos-Vel_AngleErr=%.7lgdeg (should be ~zero)\n"
           , utcTCA
           , sAbcorr
           , etStop-etStart
           , 90.0 - (dpr * vsep_c(scPos, scVel))
           );

    /* Output Matrix and Euler angles */
    printMtx3x3(mtxJ2kToUncert[0], "\nMTX[J2000=>ABC]", 0);
    printVec3(rotXYZ, "\nRotateXYZ:", "deg\n\n", 0);
  }

  return 0;
}
