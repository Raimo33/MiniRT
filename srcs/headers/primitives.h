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

# define EPSILON 1e-6

typedef struct s_float3
{
	float		x;
	float		y;
	float		z;
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
	uint8_t		a;
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
    t_color    	color; //hex
    float       specular_strength; // Specular highlight strength
    float       reflectivity; // 0 (not reflective) to 1 (perfect mirror)
	float       roughness; // 0 (perfectly smooth) to 1 (rough)
}   t_material;

typedef struct s_hit
{
    t_point     point;
    t_vector    normal;
    float       distance;
    t_material	*material;
}   t_hit;

t_vector	vec_add(const t_vector a, const t_vector b); //somma di vettori
t_vector	vec_add(const t_vector a, const t_vector b);
t_vector	vec_sub(const t_vector a, const t_vector b);
t_vector	vec_scale(const float scalar, const t_vector v);
t_vector	vec_cross(const t_vector a, const t_vector b);
t_vector	vec_normalize(const t_vector v);
t_vector	vec_negate(const t_vector a); //restituisce il vettore opposto
float		vec_dot(const t_vector a, const t_vector b);
bool 		is_vec_equal(const t_vector a, const t_vector b); //confronto di vettori


#endif