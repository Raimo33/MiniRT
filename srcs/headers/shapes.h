/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shapes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:47:12 by craimond          #+#    #+#             */
/*   Updated: 2024/04/20 19:17:07 by craimond         ###   ########.fr       */
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
	double		squared_radius;
	double		half_height;
	double		height;
	t_point		top_cap_center;
	t_point		bottom_cap_center;
}	t_cylinder;

typedef struct s_triangle
{
	t_point		vertices[3];
	uint8_t		u[3];
	uint8_t		v[3];
	t_vector	normal;
}	t_triangle;

typedef struct s_cone
{
	t_point		base_center;
	t_vector	direction;
	double		radius;
	double		height;
	double		costheta_squared;
}	t_cone;

typedef struct s_texture_data
{
	char		*addr;
	char		*path;
	int32_t		width;
	int32_t		height;
	int32_t		line_length;
	int32_t		endian;
	uint8_t		bytes_per_pixel;
}	t_texture_data;

typedef struct s_material
{
	t_texture_data	*texture;
    t_color			color;
	double			shininess;
	double			specular;
	double			diffuse;
	bool			is_checkerboard : 1;
}   t_material;

typedef struct s_shape
{
	enum {SPHERE, CYLINDER, TRIANGLE, CONE, PLANE}	type; //PLANE DEVE ESSERE L'ULTIMO DELLA SERIE
	union
	{
		t_sphere	sphere;
		t_cylinder	cylinder;
		t_triangle	triangle;
		t_cone		cone;
		t_plane		plane;
	};
	t_material	*material;
	//bounding box
	t_point	bb_min;
	t_point	bb_max;
}	t_shape;

typedef struct s_hit
{
    t_point     point;
    t_vector    normal;
    double      distance;
    t_shape		*shape;
}   t_hit;

#endif