/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shapes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:47:12 by craimond          #+#    #+#             */
/*   Updated: 2024/03/28 14:55:26 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHAPES_H
# define SHAPES_H

# include "primitives.h"

typedef struct s_sphere
{
	t_point		center;
	float		radius;
	t_material	material;
}	t_sphere;

typedef struct s_plane
{
	t_point		center;
	t_vector	normal;
	t_material	material;
	t_point		vertices[4];
}	t_plane;

typedef struct s_cylinder
{
	t_point		center;
	t_vector	normal;
	float		diameter;
	float		height;
	t_material	material;
}	t_cylinder;

typedef struct s_shape
{
	enum {SPHERE, CYLINDER, PLANE}	type; //PLANE DEVE ESSERE L'ULTIMO DELLA SERIE
	union
	{
		t_sphere	sphere;
		t_cylinder	cylinder;
		t_plane		plane;
	};
	//bounding box
	t_point	bb_min;
	t_point	bb_max;
}	t_shape;

#endif