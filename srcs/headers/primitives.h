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

typedef struct s_color
{
	uint8_t		r;
	uint8_t		g;
	uint8_t		b;
}	t_color;

typedef struct s_coord
{
	float		x;
	float		y;
	float		z;
}	t_coord;

typedef t_coord t_point;
typedef t_coord t_vector;

typedef struct s_ray
{
	t_point		origin;
	t_vector	direction;
}	t_ray;

t_vector	vec_add(t_vector a, t_vector b);
t_vector	vec_sub(t_vector a, t_vector b);
t_vector	vec_mul(t_vector a, t_vector b);
t_vector	vec_div(t_vector a, t_vector b);
t_vector	vec_cross(t_vector a, t_vector b);
t_vector	vec_normalize(t_vector a);
t_vector	vec_negate(t_vector a);

#endif