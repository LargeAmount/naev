

#include "physics.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>


#ifndef M_PI
#define M_PI	3.14159265358979323846f
#endif /* M_PI */


/*
 * set the vector value using cartesian coordinates
 */
void vect_cset( Vector2d* v, double x, double y )
{
	v->x = x;
	v->y = y;
	v->mod = MOD(x,y);
	v->angle = ANGLE(x,y);
}
/*
 * set the vector value using polar coordinates
 */
void vect_pset( Vector2d* v, double mod, double angle )
{
	v->mod = mod;
	v->angle = angle;
	v->x = v->mod*cos(v->angle);
	v->y = v->mod*sin(v->angle);
}
/*
 * copies vector src to dest
 */
void vectcpy( Vector2d* dest, const Vector2d* src )
{
	dest->x = src->x;
	dest->y = src->y;
	dest->mod = src->mod;
	dest->angle = src->angle;
}
/*
 * makes a vector NULL
 */
void vectnull( Vector2d* v )
{
	v->x = v->y = v->mod = v->angle = 0.;
}


/*
 * Simple method
 *
 *   d^2 x(t) / d t^2 = a, a = constant (acceleration)
 *   x'(0) = v, x(0) = p
 *
 *   d x(t) / d t = a*t + v, v = constant (initial velocity)
 *   x(t) = a/2*t + v*t + p, p = constant (initial position)
 *
 *   since d t isn't actually diferential this gives us ERROR!
 *   so watch out with big values for dt
 *
 */
#if 0
static void simple_update (Solid *obj, const double dt)
{
	/* make sure angle doesn't flip */
	obj->dir += obj->dir_vel/360.*dt;
	if (obj->dir > 2*M_PI) obj->dir -= 2*M_PI;
	if (obj->dir < 0.) obj->dir += 2*M_PI;

	double px, py, vx, vy;
	px = VX(obj->pos);
	py = VY(obj->pos);
	vx = VX(obj->vel);
	vy = VY(obj->vel);

	if (obj->force.mod) { /* force applied on object */
		double ax, ay;
		ax = VX(obj->force)/obj->mass;
		ay = VY(obj->force)/obj->mass;

		vx += ax*dt;
		vy += ay*dt;

		px += vx*dt + 0.5*ax * dt*dt;
		py += vy*dt + 0.5*ay * dt*dt;

		obj->vel.mod = MOD(vx,vy);
		obj->vel.angle = ANGLE(vx,vy);
	}
	else {
		px += vx*dt;
		py += vy*dt;
	}
	obj->pos.mod = MOD(px,py);
	obj->pos.angle = ANGLE(px,py);
}
#endif

/*
 * Runge-Kutta 4 method
 *
 *   d^2 x(t) / d t^2 = a, a = constant (acceleration)
 *   x'(0) = v, x(0) = p
 *   x'' = f( t, x, x' ) = ( x' , a )
 *
 *   x_{n+1} = x_n + h/6 (k1 + 2*k2 + 3*k3 + k4)
 *    h = (b-a)/2
 *    k1 = f(t_n, X_n ), X_n = (x_n, x'_n)
 *    k2 = f(t_n + h/2, X_n + h/2*k1)
 *    k3 = f(t_n + h/2, X_n + h/2*k2)
 *    k4 = f(t_n + h, X_n + h*k3)
 *
 *   x_{n+1} = x_n + h/6*(6x'_n + 3*h*a, 4*a)
 */
#define RK4_MIN_H	0.01 /* minimal pass we want */
static void rk4_update (Solid *obj, const double dt)
{
	/* make sure angle doesn't flip */
	obj->dir += obj->dir_vel/360.*dt;
	if (obj->dir > 2*M_PI) obj->dir -= 2*M_PI;
	if (obj->dir < 0.) obj->dir += 2*M_PI;

	int N = (dt>RK4_MIN_H) ? (int)(dt/RK4_MIN_H) : 1 ;
	double h = dt / (double)N; /* step */

	double px, py, vx, vy;
	px = VX(obj->pos);
	py = VY(obj->pos);
	vx = VX(obj->vel);
	vy = VY(obj->vel);


	if (obj->force.mod) { /* force applied on object */
		int i;
		double ix, iy, tx, ty; /* initial and temporary cartesian vector values */

		double ax, ay;
		ax = VX(obj->force)/obj->mass;
		ay = VY(obj->force)/obj->mass;

		for (i=0; i < N; i++) { /* iterations */

			/* x component */
			tx = ix = vx;
			tx += 2*ix + h*tx;
			tx += 2*ix + h*tx;
			tx += ix + h*tx;
			tx *= h/6;

			px += tx;
			vx += ax*h;

			/* y component */
			ty = iy = vy; 
			ty += 2*(iy + h/2*ty);
			ty += 2*(iy + h/2*ty);
			ty += iy + h*ty;
			ty *= h/6;

			py += ty;
			vy += ay*h;
		}
		vect_cset( &obj->vel, vx, vy );
	}
	else {
		px += dt*vx;
		py += dt*vy;
	}
	vect_cset( &obj->pos, px, py );
}


/*
 * Initializes a new Solid
 */
void solid_init( Solid* dest, const double mass, const Vector2d* vel, const Vector2d* pos )
{
	dest->mass = mass;

	dest->force.mod = 0;
	dest->dir = 0;

	if (vel == NULL) vectnull( &dest->vel );
	else vectcpy( &dest->vel, vel );

	if (pos == NULL) vectnull( &dest->pos );
	else vectcpy( &dest->pos, pos);

	dest->update = rk4_update;
}

/*
 * Creates a new Solid
 */
Solid* solid_create( const double mass, const Vector2d* vel, const Vector2d* pos )
{
	Solid* dyn = MALLOC_ONE(Solid);
	assert(dyn != NULL);
	solid_init( dyn, mass, vel, pos );
	return dyn;
}

/*
 * Frees an existing solid
 */
void solid_free( Solid* src )
{
	free( src );
	src = NULL;
}
