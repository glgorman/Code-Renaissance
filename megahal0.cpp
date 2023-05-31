#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if 0
#include <unistd.h>
#include <getopt.h>
#endif

#if !defined(AMIGA) && !defined(__mac_os)
#include <malloc.h>
#endif
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#if defined(__mac_os)
#include <types.h>
#include <Speech.h>
#else
#include <sys/types.h>
#endif
#include "megahal.h"
//#if defined(DEBUG)
//#include "debug.h"
//#endif

/* extern errorfp;
extern statusfp;
  */
/* extern noprompt;
extern nowrap;
extern nobanner;
extern typing_delay;
extern speech;
extern quiet;
  */

#if 0
static struct option long_options[] = {
    {"no-prompt", 0, NULL, 'p'},
    {"no-wrap", 0, NULL, 'w'},
    {"no-banner", 0, NULL, 'b'},
    {"help", 0, NULL, 'h'},
    {0, 0, 0, 0}
};
#endif

void usage()
{
    puts("usage: megahal [-[pqrgwbh]]\n" \
	 "\t-h : show usage\n" \
	 "\t-p : inhibit prompts\n" \
	 "\t-q : quiet mode (no replies) enabled at start\n" \
	 "\t-r : inhibit progress display\n" \
	 "\t-g : inhibit initial greeting\n" \
	 "\t-b : inhibit banner display at startup\n");
}

/*===========================================================================*/

/*
 *		Function:	Main
 *
 *		Purpose:		Initialise everything, and then do an infinite loop.  In
 *						the loop, we read the user's input and reply to it, and
 *						do some housekeeping task such as responding to special
 *						commands.
 */
int megahal (int argc, char **argv)
{
    char *input=NULL;
    char *output=NULL;
    int c, fd, option_index = 0;

#if 0
    while(1) 
	{
	if((c = getopt_long(argc, argv, "hpwb", long_options,
			    &option_index)) == -1)
	    break;
	switch(c) {
	case 'p':
	    megahal_setnoprompt();
	    break;
	case 'w':
	    megahal_setnowrap();
	    break;
	case 'b':
	    megahal_setnobanner();
	    break;
	case 'h':
	    usage();
	    return 0;
	}
    }
#endif
	
    /*
     *		Do some initialisation 
     */
    megahal_initialize();
    
	
    /*
     *		Create a dictionary which will be used to hold the segmented
     *		version of the user's input.
     */

    /*
     *		Load the default MegaHAL personality.
     */
    output = megahal_initial_greeting();
    megahal_output(output);
    /*
     *		Read input, formulate a reply and display it as output
     */
    while(1) {

	input = megahal_input("> ");
	
	/*
	 *		If the input was a command, then execute it
	 */

	if (megahal_command(input) != 0)
	    continue;

	output = megahal_do_reply(input, 1);

	megahal_output(output);
    }

    megahal_cleanup();
    exit(0);
}
