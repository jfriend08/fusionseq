#include <bios/log.h>
#include <bios/format.h>
#include <bios/intervalFind.h>
#include "gfr.h"

/**
   @file gfrRepeatMaskerFilter.c
   @brief Filter to remove artifacts due to mis-alignment to repetitive regions.
   @details It removes candidates with reads overlapping repetitive sequences. It looks at non-exonic reads and, if some overlap exists with repetitive regions, the reads are excluded. If the final number of reads is below the threshold (minNumberOfReads), the fusion candidate is removed.
   
   @author Andrea Sboner  (andrea.sboner.w [at] gmail.com).  
   @version 0.8
   @date 2012.08.23
   @remarks WARNings will be output to stdout to summarize the filter results.
   @pre A valid GFR file as input, including stdin.
   @pre repeatMasker.interval The repetitive regions in interval format; typically from RepeatMasker.
   @pre [in] minNumberInterReads An integer representing the minimum number of reads to keep the fusion candidate.
 */


static float getNumInter( GfrInterRead* currInter, int readLength ) { // computes the correct number of the inters by considering split reads on splice junctions.
  float numInter=0.0;
  if( (currInter->readEnd1 - currInter->readStart1 + 1) != readLength &
      (currInter->readEnd2 - currInter->readStart2 + 1) != readLength  ) {
    numInter += 0.25;
  } else if ( (currInter->readEnd1 - currInter->readStart1 + 1) != readLength  |
	      (currInter->readEnd2 - currInter->readStart2 + 1) != readLength ) {
    numInter += 0.5;
  } else {
    numInter += 1.0;
  }
  return( numInter );
}


int main (int argc, char *argv[]) {
	GfrEntry *currGE;
	int count,countRemoved;
	int i;
	Array intervals;
	GfrInterRead *currGIR;
	int totalOverlaps;
	int minNumInterReads;
	float numberOfInters;
	if (argc != 3) {
		usage ("%s <repeatMasker.interval> <minNumInterReads>",argv[0]);
	}
	intervalFind_addIntervalsToSearchSpace (argv[1],0);
	minNumInterReads = atoi (argv[2]);
	count = 0;
	countRemoved = 0;
	gfr_init ("-");
	puts (gfr_writeHeader ());
	while (currGE = gfr_nextEntry ()){
	  int readLength = strlen( arru( currGE->readsTranscript1, 0, Texta ) );
	  numberOfInters = (float) currGE->numInter;
	  for (i = 0; i < arrayMax (currGE->interReads); i++) {
	    currGIR = arrp (currGE->interReads,i,GfrInterRead);
	    if (currGIR->pairType == GFR_PAIR_TYPE_EXONIC_EXONIC) {
	      continue;
	    }
	    totalOverlaps = 0;
	    intervals = intervalFind_getOverlappingIntervals (currGE->chromosomeTranscript1,currGIR->readStart1,currGIR->readEnd1);
	    totalOverlaps += arrayMax (intervals);
	    intervals = intervalFind_getOverlappingIntervals (currGE->chromosomeTranscript2,currGIR->readStart2,currGIR->readEnd2);
	    totalOverlaps += arrayMax (intervals);
	    if (totalOverlaps > 0) {
	      //currGE->numInter-= getNumInter( currGIR, readLength ); // currGE->numInter is an integer, numberOfInters is float!
	      numberOfInters-= getNumInter( currGIR, readLength );
	      currGIR->flag = 1;
	    }
	  }
	  
	  // if (currGE->numInter < minNumInterReads) // numberOfInters correctly determine the splice number numInter, w
	  if( numberOfInters < minNumInterReads ) {
	    countRemoved++;
	    continue;
	  }
	  puts (gfr_writeGfrEntry (currGE));
	  count++;
	}
	gfr_deInit ();
	warn ("%s_numRemoved: %d",argv[0],countRemoved);
	warn ("%s_numGfrEntries: %d",argv[0],count);
	return 0;
}
