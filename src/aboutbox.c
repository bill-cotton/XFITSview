/* about dialog box  for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1998-2017
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
#include <stdio.h>
#include "xfitsview.h"
#include "scrolltext.h"

/* internal prototype */
void HelpAbout (ScrollTextPtr STextPtr);

void HelpAboutCB (Widget w, XtPointer clientData, XtPointer callData) {
  ScrollTextPtr STextPtr;
/* make ScrollText */
  STextPtr = ScrollTextMake (Display_shell, "About XFITSview");

/* copy text */
   if (STextPtr) HelpAbout(STextPtr);
/* final setup */
  ScrollTextInit (STextPtr);
  } /* end HelpAboutCB */

/* Supply About text */
void HelpAbout (ScrollTextPtr STextPtr)  {
    int loop, next, length;
    char *line[] = {
"XFITSview 3.0 Viewer for images in FITS format ",
"Copyright NRAO/AUI 1996-2017 ",
" ",
"   This software is distributed free of charge by NRAO. ",
"The (USA) National Radio Astronomy Observatory (http://www.nrao.edu/) ",
"is a facility of the (USA) National Science Foundation operated under ",
"cooperative agreement by Associated Universities, Inc.  ",
"(http://www.aui.edu).  The FITSview home page is ", 
"http://www.nrao.edu/software/fitsview/ .",
"Only very limited user support of this software is available.",
"Suggestions and comments should be sent to Bill Cotton at NRAO ",
"(bcotton@nrao.edu). NRAO's headquarters address is:  ",
"National Radio Astronomy Observatory ",
"520 Edgemont Road, ",
"Charlottesville, VA 22903, USA",
" ",
"This Software is distributed in the hope that it will be useful, but ",
"WITHOUT ANY WARRANTY; without even the implied warranty of ",
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. ",
" ",
"*** FINISHED ***" }; /* end of text */
    loop = 0;
    next = STextPtr->num_lines;
    while (1) { /* loop till done */
      if (!strcmp (line[loop], "*** FINISHED ***")) break; /* Finished */
      if (next>=MAX_LINE) break;
      /* copy */
      length = strlen(line[loop]);
      STextPtr->lines[next] = (char*)malloc(length+1);
      strcpy (STextPtr->lines[next], line[loop]);
      loop++; next++;
    } /* end loop loading info */
    STextPtr->num_lines = next; /* save number in ScrollText */
  } /* end HelpAbout */


