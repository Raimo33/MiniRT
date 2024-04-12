/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shapes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:47:12 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 17:35:20 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHAPES_H
# define SHAPES_H

# include "primitives.h"

typedef struct s_sphere
{
	t_point		center;
	double		radius;
}	t_sphere;

typedef struct s_plane
{
	t_point		center;
	t_vector	normal;
	t_point		vertices[4];
}	t_plane;

typedef struct s_cylinder
{
	t_point		center;
	t_vector	direction;
	double		radius;
	double		sqr_radius;
	double		half_height;
	t_point		top_cap_center;
	t_point		bottom_cap_center;
}	t_cylinder;

typedef struct s_triangle
{
	t_point		vertices[3];
	t_vector	normal;
}	t_triangle;

typedef struct s_shape
{
	enum {SPHERE, CYLINDER, TRIANGLE, PLANE}	type; //PLANE DEVE ESSERE L'ULTIMO DELLA SERIE
	union
	{
		t_sphere	sphere;
		t_cylinder	cylinder;
		t_triangle	triangle;
		t_plane		plane;
	};
	t_material	*material;
	//bounding box
	t_point	bb_min;
	t_point	bb_max;
}	t_shape;

#endif