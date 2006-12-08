/****************************************************************
NAME:  deramp.c

SYNOPSIS:  

   deramp [-backward] [-log <file>] <inphase> <baseline> <outphase>

DESCRIPTION:
	Deramp removes the phase signal imposed on the  interfero-
       gram  as  a consequence of the physical separation between
       the two constituent satellite passes, called the baseline.
       The  program  operates by taking an assumed baseline func-
       tion and  calculating  the  consequent  'baseline  driven'
       phase  at  each  pixel.  This phase is subtracted from the
       interferogram phase and the resulting 'deramped' phase  is
       written as the output file.

       The  user  can either generate the baseline by hand or use
       the file generated by resolve(1).  Normally, deramp is run
       by tandem_ifm(1).

EXTERNAL ASSOCIATES:
    NAME:                USAGE:
    ---------------------------------------------------------------

FILE REFERENCES:
    NAME:                USAGE:
    ---------------------------------------------------------------

PROGRAM HISTORY:
    VERS:   DATE:       AUTHOR:       PURPOSE:
    ---------------------------------------------------------------
    1.0     9/96        M. Shindle & R. Fatland - Original Development
    1.1     8/97        O. Lawlor - Added DDR support.
    1.2     3/98        O. Lawlor - Added ability to deramp backwards 
				    (for undo).
    2.0     6/98        O. Lawlor - Switched to new CEOS routines.
    2.01    7/01	R. Gens   - Added log file switch
    2.15    4/02        P. Denny  - Standardized commandline parsing
                                      and usage()
    2.5     6/05        R. Gens   - Getting rid of DDR dependency

HARDWARE/SOFTWARE LIMITATIONS:

ALGORITHM DESCRIPTION:
    Note:  Here, x axis is range, y is azimuth, z is vertical;
            For Ian, z is vertical, y is range, x is azimuth.
            H = distance from Center of Earth to s/c, not from surface 
                to s/c (as in Ian's formalism) 

ALGORITHM REFERENCES:
    The original code comes from Ian's formalism, thesis eqns 3.5, 3.8, etc.

BUGS:

****************************************************************/
/****************************************************************************
*								            *
*   deramp:  removes satellite-baseline phase ramping.			    *
* Copyright (c) 2004, Geophysical Institute, University of Alaska Fairbanks   *
* All rights reserved.                                                        *
*                                                                             *
* You should have received an ASF SOFTWARE License Agreement with this source *
* code. Please consult this agreement for license grant information.          *
*                                                                             *
*                                                                             *
*       For more information contact us at:                                   *
*                                                                             *
*	Alaska Satellite Facility	    	                              *
*	Geophysical Institute			www.asf.alaska.edu            *
*       University of Alaska Fairbanks		uso@asf.alaska.edu	      *
*	P.O. Box 757320							      *
*	Fairbanks, AK 99775-7320					      *
*									      *
******************************************************************************/

#include "asf.h"
#include "asf_meta.h"
#include "ddr.h"
#include "asf_endian.h"
#include "asf_insar.h"

/* local constants */
#define VERSION 2.5

void usage(char *name);

int main(int argc, char *argv[])
{
	char *baseFile;
	meta_parameters *meta;
	baseline base;
	int   wid, len,		/* Width and Length of input scene    */
		ss, sl;		/* Offsets of input scene in original */
	int x,y;
	double xScale,yScale;
	float percent=5.0;
	
	FILE  *fin, *fout;
	char  szInPhase[255], szInAmp[255], szOutPhase[255], szOutAmp[255];
	float *data;
	double *sflat,*cflat;
	double derampDirection=1.0;/*1.0=forward deramping.  -1.0=backward deramping.*/
	logflag=0;

/* process command line args */
	currArg=1; /* from cla.h in asf.h */
	/* optional args */
	while (currArg < (argc-3)) {
		char *key = argv[currArg++];
		if (strmatch(key,"-log")) {
			CHECK_ARG(1);
			strcpy(logFile,GET_ARG(1));
			fLog = FOPEN(logFile, "a");
			logflag=1;
		}
		else if (strmatch(key,"-backward")) {
			derampDirection = -1.0;
		}
		else {printf("**Invalid option: %s\n",argv[currArg-1]); usage(argv[0]);}
	}
	if ((argc-currArg) < 3) {printf("Insufficient arguments.\n"); usage(argv[0]);}
	/* required args */
	create_name(szInAmp, argv[currArg], "_amp.img");
	create_name(szInPhase, argv[currArg], "_phase.img");
	baseFile = argv[currArg+1];

	asfSplashScreen(argc, argv);
	
	/* Get input scene size and windowing info, check validity */
	meta = meta_read(szInPhase);

	wid = meta->general->sample_count;
	len = meta->general->line_count;
	ss = meta->general->start_sample - 1;
	sl = meta->general->start_line - 1;
	xScale = meta->sar->sample_increment;
	yScale = meta->sar->line_increment;
	
	create_name(szOutAmp,argv[currArg+2],"_amp.img");
	meta_write(meta, szOutAmp);
	create_name(szOutPhase,argv[currArg+2],"_phase.img");
	meta_write(meta, szOutPhase);

	/*Link over ".amp" file, if it exists.*/
	if (fileExists(szInAmp)&&!fileExists(szOutAmp))
	{
		char command[1024];
		sprintf(command,"ln -s %s %s\n", szInAmp, szOutAmp);
		system(command);
	}
	
	/* buffer mallocs, read data file */
	data = (float *)MALLOC(sizeof(float)*wid);
	sflat = (double *)MALLOC(sizeof(double)*wid);
	cflat = (double *)MALLOC(sizeof(double)*wid);
	fin = fopenImage(szInPhase,"rb");
	fout = fopenImage(szOutPhase,"wb");
	
	/* read in CEOS parameters & convert to meters */
	base=read_baseline(baseFile);
	
	/* calculate slant ranges and look angles - Ian's thesis eqn 3.10 */
	for (x = 0; x < wid; x++) {
		double flat=meta_flat(meta,0.0,x*xScale+ss);
		sflat[x]=sin(flat);
		cflat[x]=cos(flat);
	}
	/* Deramp 'data' array */
	
/*	printf("\n  starting in-place deramp of input data \n\n");*/
	for (y = 0; y < len; y++)
	{
		double Bn_y,Bp_y;
		double twok=derampDirection*2.0*meta_get_k(meta);
		meta_interp_baseline(meta,base,y*(int)yScale+sl,&Bn_y,&Bp_y);
		/* read in the next row of data */
		get_float_line(fin, meta, y, data);
		
		/* calculate flat-earth range phase term & remove it */ 
		for (x = 0; x < wid; x++)
		{
			double d=data[x];
			if (d!=0.0) /*Ignore points which didn't phase unwrap.*/
				d -= twok*(Bp_y*cflat[x]-Bn_y*sflat[x]);
			/*Was: d-=ceos_flat_phase(ceos,base,x,y);*/
			data[x]=d;
		}
		
		/* write out this row of data */
		put_float_line(fout, meta, y, data);
		if (y*100/len==percent) {
		  printf("   Completed %3.0f percent\n", percent);
		  percent+=5.0;
		}
	}
/*	printf("\nDone with deramp\n\n");*/
	
	/* save and scram */
	FCLOSE(fin); FCLOSE(fout);
	FREE(data); FREE(sflat);FREE(cflat);

	printf("   Completed 100 percent\n\n");
	StopWatch();
	if (logflag) {
	  sprintf(logbuf, "   Wrote %lld bytes of data\n\n", (long long)(len*wid*4));
	  printLog(logbuf);
	  StopWatchLog(fLog);
	}

	return 0;
}


void usage(char *name)
{
 printf("\n"
	"USAGE:\n"
	"   %s [-backward] [-log <file>] <in.phase> <base> <out>\n",name);
 printf("\n"
	"REQUIRED ARGUMENTS:\n"
	"   <in.phase> :  The .phase input file.\n" 
	"   <base>     :  The baseline file.\n"
	"   <out>      :  The deramped output phase file.\n");
 printf("\n"
	"OPTIONAL ARGUMENTS:\n"
	"   -backward   :  Perform deramping backwards (undoes previous deramping).\n"
	"   -log <file> :  Allows the output to be written to a log file.\n");
 printf("\n"
	"DESCRIPTION:\n"
	"   %s removes satellite-baseline phase ramping.\n",name);
 printf("\n"
	"Version %.2f, ASF SAR Tools\n"
	"\n",VERSION);
exit(1);
}

