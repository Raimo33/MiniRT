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

#endif