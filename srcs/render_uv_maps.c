/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_uv_maps.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 10:38:44 by craimond          #+#    #+#             */
/*   Updated: 2024/04/17 12:25:50 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void get_sphere_uv(const t_hit *hit_info, double *u, double *v);
static void get_cylinder_uv(const t_hit *hit_info, double *u, double *v);
static void get_triangle_uv(const t_hit *hit_info, double *u, double *v);
static void get_cone_uv(const t_hit *hit_info, double *u, double *v);
static void get_plane_uv(const t_hit *hit_info, double *u, double *v);

void	get_uv(const t_hit *hit_info, double *u, double *v)
{
	void					(* get_uv_funcs[])(const t_hit *, double *, double *) = {&get_sphere_uv, &get_cylinder_uv, &get_triangle_uv, &get_cone_uv, &get_plane_uv}; //stesso ordine di enum
	static const uint8_t	n_shapes = sizeof(get_uv_funcs) / sizeof(get_uv_funcs[0]);
	uint8_t					i;

	i = 0;
	while (i < n_shapes)
	{
		if (hit_info->shape->type == i)
		{
			get_uv_funcs[i](hit_info, u, v);
			break ;
		}
		i++;
	}
}

static void get_sphere_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_vector normal = hit_info->normal;
    const double phi = atan2(normal.z, normal.x);
    const double theta = asin(normal.y);

    *u = 1 - (phi + M_PI) / DOUBLE_PI;
    *v = (theta + HALF_PI) / M_PI;
}

static void get_cylinder_uv(const t_hit *hit_info, double *u, double *v)
{
	//TODO get cylinder uv
	(void)hit_info;
	(void)u;
	(void)v;
}

static void get_triangle_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_triangle triangle = hit_info->shape->triangle;
    const t_vector edge1 = vec_sub(triangle.vertices[1], triangle.vertices[0]);
    const t_vector edge2 = vec_sub(triangle.vertices[2], triangle.vertices[0]);
    const t_vector h = vec_cross(hit_info->normal, edge2);
    const double a = vec_dot(edge1, h);

    if (fabs(a) < 1e-8) { // Astatic void division by zero
        *u = 0;
        *v = 0;
        return;
    }

    const t_vector s = vec_sub(hit_info->point, triangle.vertices[0]);
    const double b = vec_dot(s, h) / a;
    const t_vector q = vec_cross(s, edge1);
    const double c = vec_dot(hit_info->normal, q) / a;

    *u = b;
    *v = c;
}

static void get_cone_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_vector			normal = hit_info->normal; // Assuming normalized
    const double			phi = atan2(normal.z, normal.x);
    const double			theta = asin(normal.y);
	
    *u = 1 - (phi + M_PI) / DOUBLE_PI;
    *v = (theta + HALF_PI) / M_PI;
}

static void get_plane_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_vector	normal = hit_info->normal;  // Assuming normalized
    t_vector 		perp1;
	t_vector 		perp2;

    if (fabs(normal.x) > fabs(normal.y))
        perp1 = (t_vector){normal.z, 0, -normal.x};
    else
        perp1 = (t_vector){0, -normal.z, normal.y};

    perp1 = vec_normalize(perp1);
    perp2 = vec_cross(normal, perp1);
    perp2 = vec_normalize(perp2);
	
    const t_vector hit_point = hit_info->point;

    *u = vec_dot(hit_point, perp1) * CHECKERBOARD_TILE_DENSITY / WORLD_SIZE;
    *v = vec_dot(hit_point, perp2) * CHECKERBOARD_TILE_DENSITY / WORLD_SIZE;	
}