/*BHEADER**********************************************************************
 * (c) 1997   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
 *********************************************************************EHEADER*/
/******************************************************************************
 *
 * Structured inner product routine
 *
 *****************************************************************************/

#ifdef HYPRE_USE_PTHREADS
#undef HYPRE_USE_PTHREADS
#include "headers.h"
#define HYPRE_USE_PTHREADS
#include <pthread.h>
extern pthread_t  initial_thread;
extern pthread_t *hypre_thread;
#else
#include "headers.h"
#endif

/*--------------------------------------------------------------------------
 * hypre_StructInnerProd
 *--------------------------------------------------------------------------*/

#ifdef HYPRE_USE_PTHREADS
double global_result;
#endif

double
hypre_StructInnerProd(  hypre_StructVector *x,
                        hypre_StructVector *y )
{
   double           result;
   double           local_result;
                   
   hypre_Box       *x_data_box;
   hypre_Box       *y_data_box;
                   
   int              xi;
   int              yi;
                   
   double          *xp;
   double          *yp;
                   
   hypre_BoxArray  *boxes;
   hypre_Box       *box;
   hypre_Index      loop_size;
   hypre_IndexRef   start;
   hypre_Index      unit_stride;
                   
   int              i;
   int              loopi, loopj, loopk;

   local_result = 0.0;

   hypre_SetIndex(unit_stride, 1, 1, 1);

   boxes = hypre_StructGridBoxes(hypre_StructVectorGrid(y));
   hypre_ForBoxI(i, boxes)
      {
         box   = hypre_BoxArrayBox(boxes, i);
         start = hypre_BoxIMin(box);

         x_data_box = hypre_BoxArrayBox(hypre_StructVectorDataSpace(x), i);
         y_data_box = hypre_BoxArrayBox(hypre_StructVectorDataSpace(y), i);

         xp = hypre_StructVectorBoxData(x, i);
         yp = hypre_StructVectorBoxData(y, i);

         hypre_GetBoxSize(box, loop_size);

         hypre_BoxLoop2(loopi, loopj, loopk, loop_size,
                        x_data_box, start, unit_stride, xi,
                        y_data_box, start, unit_stride, yi,
                        {
                           local_result += xp[xi] * yp[yi];
                        });
      }

#ifdef HYPRE_USE_PTHREADS
   MPI_Allreduce(&local_result, &global_result, 1,
                 MPI_DOUBLE, MPI_SUM, hypre_StructVectorComm(x));

   result = global_result;
#else
   MPI_Allreduce(&local_result, &result, 1,
                 MPI_DOUBLE, MPI_SUM, hypre_StructVectorComm(x));
#endif


#ifdef HYPRE_USE_PTHREADS
   if (pthread_equal(pthread_self(), initial_thread) ||
       pthread_equal(pthread_self(), hypre_thread[0]))
#endif
   hypre_IncFLOPCount(2*hypre_StructVectorGlobalSize(x));

   return result;
}

