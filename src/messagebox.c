/* MessageBox routines for XFITSview */
/* uses a ScrollText to display messages from XFITSview      */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996
*  Associated Universities, Inc. Washington DC, USA.
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*-----------------------------------------------------------------------*/

#include <Xm/Xm.h> 
#include "scrolltext.h"
#include "messagebox.h"
#include "xfitsview.h"
#include <stdio.h> 

/* MessageBox ScrollText, if non NULL then message box active */
ScrollTextPtr MessScroll = NULL; 

/* internal prototypes */
/* shutdown messagebox, arg not used */
void MessageDismiss(XtPointer arg);

/* public functions */
/* display message in message box, creating if necessary */
void MessageShow (char *message)
{
  int next, length, new = 0;

/* new ScrollText? */
  if (!MessScroll)
    {
      /* make ScrollText */
      new = 1;
      MessScroll = ScrollTextMake (Display_shell, "XFITSview Messages");
      if (!MessScroll) { /* error, print to stderr */
	fprintf (stderr, message); fprintf (stderr,"\n");
	return;
      } /* end create error */
      /* add dismiss callback */
      MessScroll->DismissProc = (TextFileProc)MessageDismiss;
    } /* end create ScrollText */

  /* add text */
  next = MessScroll->num_lines;
  if (next>=MAX_LINE) next = MAX_LINE - 1; /* add to end no matter */
  length = strlen(message);
  MessScroll->lines[next] = (char*)malloc(length+1);
  strcpy (MessScroll->lines[next], message);
  next++;
  MessScroll->num_lines = next; /* save number in ScrollText */
  /* make some noise */
  XBell(XtDisplay(MessScroll->Parent), 50); 
  /*  setup */
  ScrollTextInit (MessScroll);

  if (!new) { /* pop to front */
    if (XtIsRealized (MessScroll->ScrollBox))
      XMapRaised (XtDisplay(MessScroll->Parent), 
		  XtWindow(MessScroll->ScrollBox));
    /* redraw */
    STextExposeCB (MessScroll->ScrollBox, (XtPointer)MessScroll, NULL);
  }

} /* end MessageShow */

/* shutdown messagebox, arg not used */
/* as this is called when the ScrollText self destructs it does not delete
   the ScrollText */
void MessageDismiss(XtPointer arg)
{
  MessScroll = NULL;
} /* end MessageDismiss */

/* redraw message box if it exists */
void MessageRefresh (void)
{
/* does it exist? */
  if (!MessScroll) return;

  /*  setup */
  ScrollTextInit (MessScroll);
  if (XtIsRealized (MessScroll->ScrollBox))
    XMapRaised (XtDisplay(MessScroll->Parent), 
		XtWindow(MessScroll->ScrollBox));
  /* redraw */
  STextExposeCB (MessScroll->ScrollBox, (XtPointer)MessScroll, NULL);
} /* end MessageRefresh */
