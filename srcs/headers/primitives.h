/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   primitives.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:49:38 by craimond          #+#    #+#             */
/*   Updated: 2024/03/23 22:17:14 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef primitives_H
# define primitives_H

# include <stdint.h>

# define EPSILON 1e-5

typedef struct s_float3
{
	double		x;
	double		y;
	double		z;
}	t_float3;

typedef struct s_int3
{
	uint16_t	x;
	uint16_t	y;
	uint16_t	z;
}	t_uint3;

typedef struct s_color
{
	uint8_t		r;
	uint8_t		g;
	uint8_t		b;
}	t_color;

typedef t_float3 t_point;
typedef t_float3 t_vector;

typedef struct s_ray
{
	t_point		origin;
	t_vector	direction;
}	t_ray;

typedef struct s_material
{
    t_color		color;
	double		shininess;
	double		specular;
	double		diffuse;
	bool		is_checkerboard;
}   t_material;

t_vector	vec_add(const t_vector a, const t_vector b);
t_vector	vec_add(const t_vector a, const t_vector b);
t_vector	vec_sub(const t_vector a, const t_vector b);
t_vector	vec_scale(const double scalar, const t_vector v);
t_vector	vec_cross(const t_vector a, const t_vector b);
t_vector	vec_normalize(const t_vector v);
t_vector	vec_negate(const t_vector a);
double		vec_dot(const t_vector a, const t_vector b);
double		vec_length(const t_vector a);
bool		are_vectors_parallel(t_vector v1, t_vector v2);


#endif