#include "defines.h"
#include <math.h>
#include "polygon.h"
#include <stdio.h>
#include <stdlib.h>

#define MIN_SLOPE	0.0005

/**************************************************************************
* Public function bodies
**************************************************************************/
//#define DEBUG_GET_CENTRE
/***************************************************************************
* Name: get_Centre
* IN:	triang			triangle whose circumcentre must be computed
* OUT:		N/A
* IN/OUT:	N/A
* RETURN:	triangle circumcentre
* Description: 	Computes the circumcentre of the triangle formed by the
* 				three points of the griangle
***************************************************************************/
struct Point_T 	get_Centre( struct Triangle_T *triang)
{
	struct 	Point_T point;					// Return value.
	TYPE 	x1=0.0, y1=0.0;
	TYPE  	x2=0.0, y2=0.0;
	TYPE 	slope1=0.0, slope2=0.0;
	TYPE  	n1=0.0, n2=0.0;

	// Get point between two triangle vertex.
	x1 = (triang->vertex[0].x + triang->vertex[1].x) / 2.0;
	y1 = (triang->vertex[0].y + triang->vertex[1].y) / 2.0;

	// Compute slope.
	if ((triang->vertex[0].x - triang->vertex[1].x) != 0)
	{
		slope1 = (triang->vertex[0].y - triang->vertex[1].y) /
				 (triang->vertex[0].x - triang->vertex[1].x);

#ifdef DEBUG_GET_CENTRE
		printf("Slope between points 0 and 1 is %lf.\n", slope1);
#endif
		if (slope1 == 0)
		{
			slope1 = MIN_SLOPE;
		}
		else
		{
			slope1 = -(1/slope1);
		}

		// Compute n1.
		n1 = y1 - slope1*x1;
	}
	else
	{
		slope1 = 0;
		n1 = y1;
	}

	// Get point between two triangle vertex.
	x2 = (triang->vertex[1].x + triang->vertex[2].x) / 2.0;
	y2 = (triang->vertex[1].y + triang->vertex[2].y) / 2.0;

	// Compute slope.
	if ((triang->vertex[1].x - triang->vertex[2].x) != 0)
	{
		slope2 = (triang->vertex[1].y - triang->vertex[2].y) /
				(triang->vertex[1].x - triang->vertex[2].x);

#ifdef DEBUG_GET_CENTRE
		printf("Slope between points 1 and 2 is %lf.\n", slope2);
#endif
		if (slope2 == 0.0)
		{
			slope2 = MIN_SLOPE;
		}
		else
		{
			slope2 = -(1/slope2);
		}

		// Compute n2.
		n2 = y2 - slope2*x2;
	}
	else
	{
		slope2 = 0;
		n2 = y2;
	}

	// Compute x.
	point.x = (n2 - n1) / (slope1 - slope2);

	// Compute y.
	point.y = point.x*slope1 + n1;

#ifdef DEBUG_GET_CENTRE
	if (point.y == 0.0)
	{
		printf("%lf %lf\n", triang->vertex[0].x, triang->vertex[0].y);
		printf("%lf %lf\n", triang->vertex[1].x, triang->vertex[1].y);
		printf("%lf %lf\n", triang->vertex[2].x, triang->vertex[2].y);
		printf("%lf %lf %lf %lf %lf\n", point.x, slope1, slope2, n1, n2);
		exit(0);
	}
	printf("x %f y %f.\n", point.x, point.y);
#endif

	return(point);
}


