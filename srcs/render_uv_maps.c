/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_uv_maps.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 10:38:44 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 13:53:14 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void	get_sphere_uv(const t_hit *hit_info, double *u, double *v);
static void	get_cylinder_uv(const t_hit *hit_info, double *u, double *v);
static void	get_triangle_uv(const t_hit *hit_info, double *u, double *v);
static void	get_cone_uv(const t_hit *hit_info, double *u, double *v);
static void	get_plane_uv(const t_hit *hit_info, double *u, double *v);

void	get_uv(const t_hit *hit_info, double *u, double *v)
{
	void					(* get_uv_funcs[])(const t_hit *, double *, double *) = {&get_sphere_uv, &get_cylinder_uv, &get_triangle_uv, &get_cone_uv, &get_plane_uv}; //stesso ordine di enum
	static const uint8_t	n_shapes = sizeof(get_uv_funcs) / sizeof(get_uv_funcs[0]);
	uint8_t					i;

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

static void get_sphere_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_vector	normal = hit_info->normal;
	const double	phi = atan2(normal.z, normal.x);
	const double	theta = asin(normal.y);

	*u = 1 - (phi + M_PI) / DOUBLE_PI;
	*v = (theta + HALF_PI) / M_PI;
}

static void get_cylinder_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_cylinder	cylinder = hit_info->shape->cylinder;
	t_vector			vec_to_intersection = vec_sub(hit_info->point, cylinder.center);
	double				height_projection = vec_dot(vec_to_intersection, cylinder.direction);
	bool				is_on_cap = fabs(height_projection) >= (cylinder.half_height - EPSILON);

	if (is_on_cap)
	{
		t_point cap_center = (height_projection > 0) ? cylinder.top_cap_center : cylinder.bottom_cap_center;
		t_vector from_cap_to_intersection = vec_sub(hit_info->point, cap_center);
		t_vector ortho1, ortho2;
		if (fabs(cylinder.direction.x) < fabs(cylinder.direction.y) || fabs(cylinder.direction.x) < fabs(cylinder.direction.z))
			ortho1 = vec_normalize(vec_cross(cylinder.direction, (t_vector){1, 0, 0}));
		else
			ortho1 = vec_normalize(vec_cross(cylinder.direction, (t_vector){0, 1, 0}));
		ortho2 = vec_normalize(vec_cross(cylinder.direction, ortho1));
		double x = vec_dot(from_cap_to_intersection, ortho1);
		double y = vec_dot(from_cap_to_intersection, ortho2);
		double theta = atan2(y, x);
		*u = (theta + M_PI) / DOUBLE_PI;
		*v = sqrt(x * x + y * y) / cylinder.radius;
	}
	else
	{
		t_vector ortho_to_dir;
		if (fabs(cylinder.direction.x) < fabs(cylinder.direction.y) || fabs(cylinder.direction.x) < fabs(cylinder.direction.z))
			ortho_to_dir = vec_normalize(vec_cross(cylinder.direction, (t_vector){1, 0, 0}));
		else
			ortho_to_dir = vec_normalize(vec_cross(cylinder.direction, (t_vector){0, 1, 0}));
		t_vector base_circle_normal = vec_normalize(vec_cross(cylinder.direction, ortho_to_dir));
		t_vector on_base_circle_plane = vec_sub(vec_to_intersection, vec_scale(height_projection, cylinder.direction));
		double angle = atan2(vec_dot(on_base_circle_plane, ortho_to_dir), vec_dot(on_base_circle_plane, base_circle_normal));
		*u = (angle + M_PI) / DOUBLE_PI;
		*v = (height_projection + cylinder.half_height) / cylinder.height;
	}
}

static void get_triangle_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_triangle	triangle = hit_info->shape->triangle;
	const t_vector		edge1 = vec_sub(triangle.vertices[1], triangle.vertices[0]);
	const t_vector		edge2 = vec_sub(triangle.vertices[2], triangle.vertices[0]);
	const t_vector		h = vec_cross(hit_info->normal, edge2);
	const double		a = vec_dot(edge1, h);

	if (fabs(a) < EPSILON)
	{
		*u = 0;
		*v = 0;
		return ;
	}
	const t_vector s = vec_sub(hit_info->point, triangle.vertices[0]);
	const double inv_a = 1.0 / a;
	const double b = vec_dot(s, h) * inv_a;
	const t_vector q = vec_cross(s, edge1);
	const double c = vec_dot(hit_info->normal, q) * inv_a;
	double d = 1.0 - b - c;
	*u = b * triangle.u[1] + c * triangle.u[2] + d * triangle.u[0];
	*v = b * triangle.v[1] + c * triangle.v[2] + d * triangle.v[0];
}

static void get_cone_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_cone	cone = hit_info->shape->cone;
	t_vector		vec_to_intersection = vec_sub(hit_info->point, cone.base_center);
	double			height_projection = vec_dot(vec_to_intersection, cone.direction);

	height_projection = fmax(0, height_projection);
	height_projection = fmin(cone.height, height_projection);
	t_vector rotated_dir = vec_cross(cone.direction, (t_vector){0, 1, 0});
	if (vec_length(rotated_dir) < EPSILON)
		rotated_dir = vec_cross(cone.direction, (t_vector){1, 0, 0});
	rotated_dir = vec_normalize(rotated_dir);
	t_vector ortho_to_dir = vec_normalize(vec_cross(cone.direction, rotated_dir));
	double angle = atan2(vec_dot(vec_to_intersection, ortho_to_dir), vec_dot(vec_to_intersection, rotated_dir));
	*u = (angle + M_PI) / (2.0 * M_PI);
	double normalized_height = height_projection / cone.height;
	*v = 1.0 - normalized_height;
}


static void get_plane_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_vector	normal = hit_info->normal;
	t_vector		perp1;
	t_vector		perp2;

	if (fabs(normal.x) > fabs(normal.y))
		perp1 = (t_vector){normal.z, 0, -normal.x};
	else
		perp1 = (t_vector){0, -normal.z, normal.y};
	perp1 = vec_normalize(perp1);
	perp2 = vec_normalize(vec_cross(normal, perp1));
	double	u_temp = vec_dot(hit_info->point, perp1) / (2.0 * WORLD_SIZE) + 0.5;
	double	v_temp = vec_dot(hit_info->point, perp2) / (2.0 * WORLD_SIZE) + 0.5;
	*u = fmax(0.0, fmin(u_temp, 1.0));
	*v = fmax(0.0, fmin(v_temp, 1.0));
}
