/*  Axisdescriptor class implementation  */
/*  An axis descriptor  and labeling information for an  axis of of an 
    Image.  AxisDescriptors are elements of an  ImageDescriptor.  */ 
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998,2002
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

  
#include "axisdesc.h" 
   
/*  constructor  */ 
  AxisDescriptor*  MakeAxisDescriptor() 
{ 
  AxisDescriptor *me = (AxisDescriptor *) malloc (sizeof(AxisDescriptor)); 
  me->axis_name = MakeString("unknown"); 
  me->axis_length = 0; 
  me->axis_ref_pixel = 1.0; 
  me->axis_coord = 0.0; 
  me->axis_increment = 1.0; 
  me->axis_rotation = 0.0; 
  return me;} /* end MakeAxisDescriptor */ 
  
/*  destructor  */ 
void KillAxisDescriptor(AxisDescriptor* me) 
{ 
  if (!me) return;  /* anybody home? */ 
  KillString (me->axis_name); 
  if (me) free (me); me = NULL;
}  /* end KillAxisDescriptor */ 
  
/* compare */ 
Logical AxisDescriptorComp (AxisDescriptor *desc1, AxisDescriptor *desc2) 
{
 if (!desc1) return 0;
 if (!desc2) return 0;
 if (desc1->axis_length != desc2->axis_length) return 0; 
 return StringComp(desc1->axis_name, desc2->axis_name); 
}  /*  end of AxisDescriptorComp  */ 
  
/* copy desc2 to desc1 */ 
void AxisDescriptorCopy (AxisDescriptor *desc1, AxisDescriptor *desc2) 
{ 
 if (!desc1) return;
 if (!desc2) return;
  StringCopy (desc1->axis_name, desc2->axis_name); 
  desc1->axis_length    = desc2->axis_length; 
  desc1->axis_ref_pixel = desc2->axis_ref_pixel; 
  desc1->axis_coord     = desc2->axis_coord; 
  desc1->axis_increment = desc2->axis_increment; 
  desc1->axis_rotation  = desc2->axis_rotation; 
} /* end AxisDescriptorCopy */ 
  
