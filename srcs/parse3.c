/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse3.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 16:20:40 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	parse_plane(t_shape *shape)
{
	shape->plane.center = parse_coord(ft_strtok(NULL, g_spaces));
	shape->plane.normal = parse_coord(ft_strtok(NULL, g_spaces));
	shape->e_type = PLANE;
}

void	parse_cylinder(t_shape *shape)
{
	shape->cylinder.center = parse_coord(ft_strtok(NULL, g_spaces));
	shape->cylinder.direction = parse_coord(ft_strtok(NULL, g_spaces));
	shape->cylinder.radius = ft_atof(ft_strtok(NULL, g_spaces)) / 2.0f;
	shape->cylinder.height = ft_atof(ft_strtok(NULL, g_spaces));
	if (shape->cylinder.radius <= 0)
		ft_quit(5, "invalid cylinder radius");
	if (shape->cylinder.height <= 0)
		ft_quit(5, "invalid cylinder height");
	shape->e_type = CYLINDER;
}

void	parse_cone(t_shape *shape)
{
	shape->cone.base_center = parse_coord(ft_strtok(NULL, g_spaces));
	shape->cone.direction = parse_coord(ft_strtok(NULL, g_spaces));
	shape->cone.radius = ft_atof(ft_strtok(NULL, g_spaces)) / 2.0f;
	shape->cone.height = ft_atof(ft_strtok(NULL, g_spaces));
	if (shape->cone.radius <= 0)
		ft_quit(5, "invalid cone radius");
	if (shape->cone.height <= 0)
		ft_quit(5, "invalid cone height");
	shape->e_type = CONE;
}

void	parse_triangle(t_shape *shape)
{
	shape->triangle.vertices[0] = parse_coord(ft_strtok(NULL, g_spaces));
	shape->triangle.vertices[1] = parse_coord(ft_strtok(NULL, g_spaces));
	shape->triangle.vertices[2] = parse_coord(ft_strtok(NULL, g_spaces));
	shape->triangle.u[0] = 0;
	shape->triangle.v[0] = 0;
	shape->triangle.u[1] = 1;
	shape->triangle.v[1] = 0;
	shape->triangle.u[2] = 0;
	shape->triangle.v[2] = 1;
	shape->e_type = TRIANGLE;
}

t_float3	parse_coord(char *str)
{
	t_float3	coord;

	if (!str)
		ft_quit(5, "invalid coordinate syntax");
	coord.x = ft_atof(str);
	str = skip_commas(str);
	coord.y = ft_atof(str);
	str = skip_commas(str);
	coord.z = ft_atof(str);
	return (coord);
}
