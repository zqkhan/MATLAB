/* 
 * Ir library of image restoration programs.
 * 
 * Copyright (C) 1998  P. J. Verveer and the Max Planc Society, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <math.h>
#include "ir.h"
#ifdef DOUBLE_PRECISION
#include "rfftw.h"
#else
#ifdef FFT_FLOAT
#include "rfftw.h"
#else
#include "srfftw.h"
#endif
#endif


ir_Error ir_ForwardRft(ir_Image *in, ir_Image *out)
{
  ir_Error error = IR_OK;
  int *n=0, d, x, s, i, j, sx, id, od, is, os;
  int *idims = 0, *odims = 0;
  ir_Float *pi, *po;
  rfftwnd_plan plan;
  int type;
  fftw_real *tmp = 0, *pt;
  
  IRXJ(ir_Check(in));
  IRXJ(ir_ImageType(in, &type));
  IRTJ(type != IR_TYPE_IMAGE, IR_TYPE_UNSUPPORTED);
  IRXJ(ir_ImageType(out, &type));
  IRTJ(type != IR_TYPE_SPECTRUM, IR_TYPE_UNSUPPORTED);
  IRXJ(ir_ImageDimensions(in, &idims, &id));
  IRXJ(ir_ImageDimensions(out, &odims, &od));
  IRXJ(ir_ImageRealX(out, &sx));
  odims[0] = sx;
  IRTJ(id != od, IR_SIZES_DIFFER);
  for(x=0; x<id; x++) {
    IRTJ(idims[x] != odims[x], IR_SIZES_DIFFER);
  }
  
  IRXJ(ir_ImageData(in, &pi));
  IRXJ(ir_ImageData(out, &po));

  IRXJ(ir_ImageSize(in, &is));
  IRXJ(ir_ImageSize(out, &os));

  tmp = (fftw_real *) malloc(2*os * sizeof(fftw_real));   /* Why is the os*2 necessary?? change by R.H. to avoid crash for odd sizeX */
  
  pt = tmp;
  for(i=0; i<is/sx; i++) {
    for(j=0; j<sx; j++)
      *pt++ = (fftw_real) *pi++;
    *pt++ = 0.0;
    *pt++ = 0.0;
  }
    
  d = id;
  n = (int *) malloc(d * sizeof(int));
  s = 1;
  for(i=0; i<d; i++) {
    n[i] = idims[d-i-1];
    s *= n[i];
  }

  plan = rfftwnd_create_plan(d, n, FFTW_REAL_TO_COMPLEX, 
			     FFTW_ESTIMATE|FFTW_IN_PLACE);
  rfftwnd_one_real_to_complex(plan, (fftw_real *) tmp, NULL);
  rfftwnd_destroy_plan(plan);

  for(i=0; i<2*os; i++)
    po[i] = (ir_Float) tmp[i];
  
 ir_error:

  if (idims) free(idims);
  if (odims) free(odims);
  
  if (n) free(n);
  if (tmp) free(tmp);
  
  return error;
}

ir_Error ir_InverseRft(ir_Image *in, ir_Image *out)
{
  ir_Error error = IR_OK;
  int *n=0, d, x, s, i, j, sx, id, od, is, os;
  ir_Float *pi, *po;
  double norm;
  rfftwnd_plan plan;
  int *idims = 0, *odims = 0;
  int type;
  fftw_real *tmp = 0, *pt;

  IRXJ(ir_Check(in));
  IRXJ(ir_ImageType(in, &type));
  IRTJ(type != IR_TYPE_SPECTRUM, IR_TYPE_UNSUPPORTED);
  IRXJ(ir_ImageType(out, &type));
  IRTJ(type != IR_TYPE_IMAGE, IR_TYPE_UNSUPPORTED);
  IRXJ(ir_ImageDimensions(in, &idims, &id));
  IRXJ(ir_ImageDimensions(out, &odims, &od));
  IRXJ(ir_ImageRealX(in, &sx));
  idims[0] = sx;
  IRTJ(id != od, IR_SIZES_DIFFER);
  for(x=0; x<id ; x++) {
    IRTJ(idims[x] != odims[x], IR_SIZES_DIFFER);
  }

  IRXJ(ir_ImageData(in, &pi));
  IRXJ(ir_ImageData(out, &po));
       
  IRXJ(ir_ImageSize(in, &is));
  IRXJ(ir_ImageSize(out, &os));

  tmp = (fftw_real *) malloc(2*is * sizeof(fftw_real));
  for(i=0; i<2*is; i++)
    tmp[i] = (fftw_real) pi[i];

  d = od;
  n = (int *) malloc(d * sizeof(int));
  s = 1;
  for(i=0; i<d; i++) {
    n[i] = odims[d-i-1];
    s *= n[i];
  }

  norm = 1.0 / s;

  plan = rfftwnd_create_plan(d, n, FFTW_COMPLEX_TO_REAL, 
			     FFTW_ESTIMATE|FFTW_IN_PLACE);
  rfftwnd_one_complex_to_real(plan, (fftw_complex *) tmp, 
			      (fftw_real *) tmp);
  rfftwnd_destroy_plan(plan);

  pt = tmp;
  for(i=0; i<os/sx; i++) {
    for(j=0; j<sx; j++)
      *po++ = (ir_Float) *pt++ * norm;
    pt+=2;
  }

 ir_error:

  if (n) free(n);
  if (idims) free(idims);
  if (odims) free(odims);
  if (tmp) free(tmp);

  return error;
}
