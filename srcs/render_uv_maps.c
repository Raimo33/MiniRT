/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_uv_maps.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 10:38:44 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 17:45:59 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void	get_sphere_uv(const t_hit *hit_info, double *u, double *v);
static void	get_triangle_uv(const t_hit *hit_info, double *u, double *v);
static void	get_cone_uv(const t_hit *hit_info, double *u, double *v);
static void	get_plane_uv(const t_hit *hit_info, double *u, double *v);

void	get_uv(const t_hit *hit_info, double *u, double *v)
{
	uint8_t					i;
	static void				(*get_uv_funcs[])(const t_hit *, double *, double *)
		= {&get_sphere_uv, &get_cylinder_uv,
		&get_triangle_uv, &get_cone_uv, &get_plane_uv};
	static const uint8_t	n_shapes
		= sizeof(get_uv_funcs) / sizeof(get_uv_funcs[0]);

	i = 0;
	while (i < n_shapes)
	{
		if (hit_info->shape->e_type == i)
		{
			get_uv_funcs[i](hit_info, u, v);
			break ;
		}
		i++;
	}
}

static void	get_sphere_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_vector	normal = hit_info->normal;
	const double	phi = atan2(normal.z, normal.x);
	const double	theta = asin(normal.y);

	*u = 1 - (phi + M_PI) / DOUBLE_PI;
	*v = (theta + HALF_PI) / M_PI;
}

static void	get_triangle_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_triangle	triangle = hit_info->shape->triangle;
	const t_vector		edge1 = vec_sub(triangle.vertices[1],
			triangle.vertices[0]);
	const t_vector		edge2 = vec_sub(triangle.vertices[2],
			triangle.vertices[0]);
	const t_vector		h = vec_cross(hit_info->normal, edge2);
	const double		a = vec_dot(edge1, h);
	t_vector			s;
	double				inv_a;
	double				b;
	t_vector			q;
	double				c;
	double				d;

	if (fabs(a) < EPSILON)
	{
		*u = 0;
		*v = 0;
		return ;
	}
	s = vec_sub(hit_info->point, triangle.vertices[0]);
	inv_a = 1.0 / a;
	b = vec_dot(s, h) * inv_a;
	q = vec_cross(s, edge1);
	c = vec_dot(hit_info->normal, q) * inv_a;
	d = 1.0 - b - c;
	*u = b * triangle.u[1] + c * triangle.u[2] + d * triangle.u[0];
	*v = b * triangle.v[1] + c * triangle.v[2] + d * triangle.v[0];
}

static void	get_cone_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_cone	cone = hit_info->shape->cone;
	const t_vector	vec_to_i = vec_sub(hit_info->point, cone.base_center);
	double			height_projection;
	t_vector		rotated_dir;
	t_vector		ortho_to_dir;

	height_projection = vec_dot(vec_to_i, cone.direction);
	height_projection = fmax(0, height_projection);
	height_projection = fmin(cone.height, height_projection);
	rotated_dir = vec_cross(cone.direction, (t_vector){0, 1, 0});
	if (vec_length(rotated_dir) < EPSILON)
		rotated_dir = vec_cross(cone.direction, (t_vector){1, 0, 0});
	rotated_dir = vec_normalize(rotated_dir);
	ortho_to_dir = vec_normalize(vec_cross(cone.direction, rotated_dir));
	*u = (atan2(vec_dot(vec_to_i, ortho_to_dir),
				vec_dot(vec_to_i,
					rotated_dir)) + M_PI) / (2.0 * M_PI);
	*v = 1.0 - (height_projection / cone.height);
}

static void	get_plane_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_vector	normal = hit_info->normal;
	t_vector		perp1;
	t_vector		perp2;
	double			u_temp;
	double			v_temp;

	if (fabs(normal.x) > fabs(normal.y))
		perp1 = (t_vector){normal.z, 0, -normal.x};
	else
		perp1 = (t_vector){0, -normal.z, normal.y};
	perp1 = vec_normalize(perp1);
	perp2 = vec_normalize(vec_cross(normal, perp1));
	u_temp = vec_dot(hit_info->point, perp1) / (2.0 * WORLD_SIZE) + 0.5;
	v_temp = vec_dot(hit_info->point, perp2) / (2.0 * WORLD_SIZE) + 0.5;
	*u = fmax(0.0, fmin(u_temp, 1.0));
	*v = fmax(0.0, fmin(v_temp, 1.0));
}
