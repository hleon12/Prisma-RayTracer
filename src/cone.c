#include <stdlib.h>
#include <math.h>
#include "vector.h"
#include "graphics.h"
#include "object.h"
#include "cone.h"

int cgIsInsideFiniteCone(cgPoint3f intersection_point, cgCone information);

#define MIN(X,Y) ((X < Y) ? X : Y)
#define MAX(X,Y) ((X > Y) ? X : Y)

extern const long double EPSILON;
extern const long double NO_INTERSECTION_T_VALUE;

cgIntersection * cgConeIntersection(cgPoint3f eye, cgVector3f ray_direction, void * information){
	cgCone cone_information = (*(cgCone*) (information));
	cgIntersection * intersection = NULL;

	cgVector3f vector_q = cone_information.direction;
	cgPoint3f anchor = cone_information.anchor;
	long double radius_k = cone_information.radius_k;
	long double distance_k = cone_information.distance_k;

	long double alpha = (cgDotProduct(vector_q, vector_q) - 2) * powl(cgDotProduct(vector_q, ray_direction), 2)
		+ powl(ray_direction.x, 2) + powl(ray_direction.y, 2) + powl(ray_direction.z, 2)
		- powl((radius_k / distance_k) * cgDotProduct(ray_direction, vector_q),2 );

	long double beta = 2
		* ( ( (vector_q.x * cgDotProduct(ray_direction, vector_q) - ray_direction.x)
		* ( (anchor.x - eye.x) * (1 - powl(vector_q.x, 2)) + vector_q.x
		* (vector_q.y * (eye.y - anchor.y) + vector_q.z * (eye.z - anchor.z) ) ) )
		+ ( (vector_q.y * cgDotProduct(vector_q, ray_direction) - ray_direction.y)
		* ( (anchor.y - eye.y) * (1 - powl(vector_q.y, 2)) + vector_q.y
		* (vector_q.x * (eye.x - anchor.x) + vector_q.z * (eye.z - anchor.z) ) ) )
		+ ( (vector_q.z * cgDotProduct(vector_q, ray_direction) - ray_direction.z)
		* ( (anchor.z - eye.z) * (1 - powl(vector_q.z, 2)) + vector_q.z
		* (vector_q.x * (eye.x - anchor.x) + vector_q.y * (eye.y - anchor.y) ) ) )
	 	- (powl(radius_k / distance_k, 2) * cgDotProduct(ray_direction, vector_q)
		* ((eye.x * vector_q.x) - (anchor.x * vector_q.x) + (eye.y * vector_q.y) - (anchor.y * vector_q.y)
	 	+ (eye.z * vector_q.z) - (anchor.z * vector_q.z) ) ) );

	long double delta = powl((anchor.x - eye.x) * (1 - powl(vector_q.x, 2))
 		+ vector_q.x * ((eye.y * vector_q.y) - (anchor.y * vector_q.y) + (eye.z * vector_q.z) - (anchor.z * vector_q.z)), 2)
		+ powl((anchor.y - eye.y) * (1 - powl(vector_q.y, 2))
	 	+ vector_q.y * ((eye.x * vector_q.x) - (anchor.x * vector_q.x) + (eye.z * vector_q.z) - (anchor.z * vector_q.z)), 2)
		+ powl((anchor.z - eye.z) * (1 - powl(vector_q.z, 2))
	 	+ vector_q.z * ((eye.x * vector_q.x) - (anchor.x * vector_q.x) + (eye.y * vector_q.y) - (anchor.y * vector_q.y)), 2)
		- powl((radius_k / distance_k) * ((eye.x * vector_q.x) - (anchor.x * vector_q.x) + (eye.y * vector_q.y) - (anchor.y * vector_q.y)
	 	+ (eye.z * vector_q.z) - (anchor.z * vector_q.z)), 2);

	long double discriminant = (beta * beta) - (4 * alpha * delta);
	long double t = NO_INTERSECTION_T_VALUE;
	long double first_t = NO_INTERSECTION_T_VALUE;
	long double second_t = NO_INTERSECTION_T_VALUE;
	cgPoint3f point_t;

	if(discriminant > EPSILON){
		long double discriminant_root = sqrtl(discriminant);
		long double t1 = ((long double) -beta + discriminant_root) / (2 * alpha);
		long double t2 = ((long double) -beta - discriminant_root) / (2 * alpha);

		if(t1 > EPSILON && t2 > EPSILON) {
			first_t = MIN(t1, t2);
			second_t = MAX(t1, t2);
		}
		else if(t1 > EPSILON){
			first_t = t1;
		}
		else if(t2 > EPSILON){
			first_t = t2;
		}
	}

	cgPoint3f first_point = {
		eye.x + (first_t * ray_direction.x),
		eye.y + (first_t * ray_direction.y),
		eye.z + (first_t * ray_direction.z)
	};

	cgPoint3f second_point = {
		eye.x + (second_t * ray_direction.x),
		eye.y + (second_t * ray_direction.y),
		eye.z + (second_t * ray_direction.z)
	};

	if(first_t > EPSILON && cgIsInsideFiniteCone(first_point, cone_information)){
		t = first_t;
		point_t = first_point;
	}
	else if(second_t > EPSILON && cgIsInsideFiniteCone(second_point, cone_information)){
		t = second_t;
		point_t = second_point;
	}

	if(t > (NO_INTERSECTION_T_VALUE + EPSILON)){
		intersection = (cgIntersection *) malloc(sizeof(cgIntersection));

		intersection->distance = t;

		cgPoint3f intersection_point = point_t;

		intersection->point = intersection_point;
	}

	return intersection;
}

int cgIsInsideFiniteCone(cgPoint3f intersection_point, cgCone information){
	cgPoint3f anchor = information.anchor;
	cgVector3f direction = information.direction;

	cgVector3f h = cgDirectionVector(anchor, intersection_point);
	long double distance_m = cgDotProduct(h, direction);

	long double d_min, d_max;

	d_min = MIN(information.distance_a, information.distance_b);
	d_max = MAX(information.distance_a, information.distance_b);

	if(d_min <= distance_m && distance_m <= d_max){
		return 1;
	}

	return 0;
}

cgVector3f cgConeNormalVector(cgPoint3f point, void * information){
	cgCone cone_information = (*(cgCone*) (information));

	cgPoint3f anchor = cone_information.anchor;
	cgVector3f direction = cone_information.direction;

	cgVector3f h = cgDirectionVector(anchor, point);
	long double distance_m = cgDotProduct(h, direction);

	cgPoint3f point_m = {
		anchor.x + (distance_m * direction.x),
		anchor.y + (distance_m * direction.y),
		anchor.z + (distance_m * direction.z)
	};

	cgVector3f normal_vector = cgDirectionVector(point_m, point);
	cgVector3f unit_vector = cgNormalizedVector(normal_vector, cgVectorMagnitude(normal_vector));

	return unit_vector;
}