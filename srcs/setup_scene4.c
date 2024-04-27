/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene4.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:25:57 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 15:14:11 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	set_bb_plane(t_shape *shape)
{
	t_vector			u;
	t_vector			v;
	t_vector			r;

	r = (t_vector){1, 0, 0};
	if (fabs(vec_dot(r, shape->plane.normal)) > 0.99f)
		r = (t_vector){0, 1, 0};
	u = vec_cross(r, shape->plane.normal);
	v = vec_cross(u, shape->plane.normal);
	u = vec_normalize(u);
	v = vec_normalize(v);
	shape->bb_min.x = -HWS * fabs(u.x) - HWS * fabs(v.x);
	shape->bb_min.y = -HWS * fabs(u.y) - HWS * fabs(v.y);
	shape->bb_min.z = -HWS * fabs(u.z) - HWS * fabs(v.z);
	shape->bb_max.x = HWS * fabs(u.x) + HWS * fabs(v.x);
	shape->bb_max.y = HWS * fabs(u.y) + HWS * fabs(v.y);
	shape->bb_max.z = HWS * fabs(u.z) + HWS * fabs(v.z);
	shape->bb_min.x -= shape->plane.normal.x * EPSILON;
	shape->bb_min.y -= shape->plane.normal.y * EPSILON;
	shape->bb_min.z -= shape->plane.normal.z * EPSILON;
	shape->bb_max.x += shape->plane.normal.x * EPSILON;
	shape->bb_max.y += shape->plane.normal.y * EPSILON;
	shape->bb_max.z += shape->plane.normal.z * EPSILON;
}

void	set_bb_cylinder(t_shape *shape)
{
	const t_cylinder	c = shape->cylinder;
	t_vector			perp[2];
	t_vector			mimr[3];

	perp[0] = (t_vector){-c.direction.y, c.direction.x, 0};
	if (fabs(c.direction.x) < fabs(c.direction.y)
		|| fabs(c.direction.x) < fabs(c.direction.z))
		perp[0] = (t_vector){0, -c.direction.z, c.direction.y};
	perp[0] = vec_normalize(perp[0]);
	perp[1] = vec_cross(c.direction, perp[0]);
	mimr[1].x = c.center.x + c.direction.x * c.half_height;
	mimr[1].y = c.center.y + c.direction.y * c.half_height;
	mimr[1].z = c.center.z + c.direction.z * c.half_height;
	mimr[0].x = c.center.x - c.direction.x * c.half_height;
	mimr[0].y = c.center.y - c.direction.y * c.half_height;
	mimr[0].z = c.center.z - c.direction.z * c.half_height;
	mimr[2].x = c.radius * sqrt(perp[0].x * perp[0].x + perp[1].x * perp[1].x);
	mimr[2].y = c.radius * sqrt(perp[0].y * perp[0].y + perp[1].y * perp[1].y);
	mimr[2].z = c.radius * sqrt(perp[0].z * perp[0].z + perp[1].z * perp[1].z);
	shape->bb_min.x = fmin(mimr[0].x, mimr[1].x) - mimr[2].x;
	shape->bb_min.y = fmin(mimr[0].y, mimr[1].y) - mimr[2].y;
	shape->bb_min.z = fmin(mimr[0].z, mimr[1].z) - mimr[2].z;
	shape->bb_max.x = fmax(mimr[0].x, mimr[1].x) + mimr[2].x;
	shape->bb_max.y = fmax(mimr[0].y, mimr[1].y) + mimr[2].y;
	shape->bb_max.z = fmax(mimr[0].z, mimr[1].z) + mimr[2].z;
}

void	set_bb_cone(t_shape *shape)
{
	const t_cone	c = shape->cone;
	const t_vector	b_c = c.base_center;
	t_vector		pe[2];
	t_vector		tip;
	t_vector		r_ext;

	tip.x = b_c.x + c.direction.x * c.height;
	tip.y = b_c.y + c.direction.y * c.height;
	tip.z = b_c.z + c.direction.z * c.height;
	if (fabs(c.direction.x) < fabs(c.direction.y)
		|| fabs(c.direction.x) < fabs(c.direction.z))
		pe[0] = (t_vector){0, -c.direction.z, c.direction.y};
	else
		pe[0] = (t_vector){-c.direction.y, c.direction.x, 0};
	pe[0] = vec_normalize(pe[0]);
	pe[1] = vec_cross(c.direction, pe[0]);
	r_ext.x = c.radius * sqrt(pe[0].x * pe[0].x + pe[1].x * pe[1].x);
	r_ext.y = c.radius * sqrt(pe[0].y * pe[0].y + pe[1].y * pe[1].y);
	r_ext.z = c.radius * sqrt(pe[0].z * pe[0].z + pe[1].z * pe[1].z);
	shape->bb_min.x = fmin(fmin(b_c.x, tip.x) - r_ext.x, b_c.x - c.radius);
	shape->bb_min.y = fmin(fmin(b_c.y, tip.y) - r_ext.y, b_c.y - c.radius);
	shape->bb_min.z = fmin(fmin(b_c.z, tip.z) - r_ext.z, b_c.z - c.radius);
	shape->bb_max.x = fmax(fmax(b_c.x, tip.x) + r_ext.x, b_c.x + c.radius);
	shape->bb_max.y = fmax(fmax(b_c.y, tip.y) + r_ext.y, b_c.y + c.radius);
	shape->bb_max.z = fmax(fmax(b_c.z, tip.z) + r_ext.z, b_c.z + c.radius);
}

void	set_bb_triangle(t_shape *shape)
{
	const t_vector	v0 = shape->triangle.vertices[0];
	const t_vector	v1 = shape->triangle.vertices[1];
	const t_vector	v2 = shape->triangle.vertices[2];
	t_vector		min;
	t_vector		max;

	min = v0;
	max = v0;
	min.x = fmin(min.x, v1.x);
	min.y = fmin(min.y, v1.y);
	min.z = fmin(min.z, v1.z);
	max.x = fmax(max.x, v1.x);
	max.y = fmax(max.y, v1.y);
	max.z = fmax(max.z, v1.z);
	min.x = fmin(min.x, v2.x);
	min.y = fmin(min.y, v2.y);
	min.z = fmin(min.z, v2.z);
	max.x = fmax(max.x, v2.x);
	max.y = fmax(max.y, v2.y);
	max.z = fmax(max.z, v2.z);
	shape->bb_min = (t_point)min;
	shape->bb_max = (t_point)max;
}
