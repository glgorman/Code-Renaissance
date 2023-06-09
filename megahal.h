#ifndef MEGAHAL_H
#define MEGAHAL_H 1

/*===========================================================================*/

/*
 *  Copyright (C) 1998 Jason Hutchens
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the Gnu Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*===========================================================================*/

/*
 *		$Id: megahal.h,v 1.5 2000/10/16 19:48:44 davidw Exp $
 *
 *		File:			megahal.h
 *
 *		Program:		MegaHAL
 *
 *		Purpose:		To simulate a natural language conversation with a psychotic
 *						computer.  This is achieved by learning from the user's
 *						input using a third-order Markov model on the word level.
 *						Words are considered to be sequences of characters separated
 *						by whitespace and punctuation.  Replies are generated
 *						randomly based on a keyword, and they are scored using
 *						measures of surprise.
 *
 *		Author:		Mr. Jason L. Hutchens
 *
 *		WWW:			http://megahal.sourceforge.net
 *
 *		E-Mail:		hutch@ciips.ee.uwa.edu.au
 *
 *		Contact:		The Centre for Intelligent Information Processing Systems
 *						Department of Electrical and Electronic Engineering
 *						The University of Western Australia
 *						AUSTRALIA 6907
 *
 */

/*===========================================================================*/

/*===========================================================================*/


/*===========================================================================*/

#ifdef SUNOS
extern double drand48(void);
extern void srand48(long);
#endif

/*===========================================================================*/

/*
 *		$Log: megahal.h,v $
 *		Revision 1.5  2000/10/16 19:48:44  davidw
 *		Moved docs to subdirectory.
 *		
 *		Added man page for 'library' interface.
 *		
 *		Revision 1.4  2000/09/07 21:51:12  davidw
 *		Created some library functions that I think are workable, and moved
 *		everything else into megahal.c as static variables/functions.
 *		
 *		Revision 1.3  2000/09/07 11:43:43  davidw
 *		Started hacking:
 *		
 *		Reduced makefile targets, eliminating non-Linux OS's.  There should be
 *		a cleaner way to do this.
 *		
 *		Added Tcl and Python C level interfaces.
 *		
 *		Revision 1.2  1998/04/21 10:10:56  hutch
 *		Fixed a few little errors.
 *
 *		Revision 1.1  1998/04/06 08:02:01  hutch
 *		Initial revision
 */

/*===========================================================================*/

/* public functions  */

#if 0
void megahal_setnoprompt ();
void megahal_setnowrap ();
void megahal_setnobanner ();
void megahal_seterrorfile(char *filename);
void megahal_setstatusfile(char *filename);
void megahal_initialize();
char *megahal_initial_greeting();
int megahal_command(char *input);
char *megahal_do_reply(char *input, int log);
void megahal_output(char *output);
char *megahal_input(char *prompt);
void megahal_cleanup();

/*===========================================================================*/

#endif

class megahal
{
public:
	static void setnoprompt(void);
	static void setnowrap (void);
	static void setnobanner (void);
	static void seterrorfile(char *filename);
	static void setstatusfile(char *filename);
	static void initialize(void);
	static char *do_reply(char *input, int log);
	static char *initial_greeting(void);
	static void output(char *output);
	static char *input(char *input);
	static int command(char *command);
	static void cleanup(void);
};

#endif /* MEGAHAL_H  */
