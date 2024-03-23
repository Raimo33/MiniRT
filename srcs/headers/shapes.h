/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shapes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:47:12 by craimond          #+#    #+#             */
/*   Updated: 2024/03/23 22:22:29 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHAPES_H
# define SHAPES_H

# include "primitives.h"

typedef struct s_sphere
{
	t_point		center;
	float		radius;
	t_color		color;
}	t_sphere;

typedef struct s_plane
{
	t_point		center;
	t_vector	normal;
	t_color		color;
}	t_plane;

typedef struct s_cylinder
{
	t_point		center;
	t_vector	normal;
	float		diameter;
	float		height;
	t_color		color;
}	t_cylinder;

#endif