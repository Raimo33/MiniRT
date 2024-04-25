/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_uv_maps2.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 10:38:44 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 17:44:28 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void	first_if(const double h_proj, const t_cylinder cyl,
		const t_hit *hit_info, double *u_and_v[2])
{
	double		xy[2];
	t_vector	orthos12[2];
	t_point		cap_center;

	if (h_proj > 0)
		cap_center = cyl.top_cap_center;
	else
		cap_center = cyl.bottom_cap_center;
	if (fabs(cyl.direction.x) < fabs(cyl.direction.y)
		|| fabs(cyl.direction.x) < fabs(cyl.direction.z))
		orthos12[0]
			= vec_normalize(vec_cross(cyl.direction,
					(t_vector){1, 0, 0}));
	else
		orthos12[0] = vec_normalize(vec_cross(cyl.direction,
					(t_vector){0, 1, 0}));
	orthos12[1] = vec_normalize(vec_cross(cyl.direction, orthos12[0]));
	xy[0] = vec_dot(vec_sub(hit_info->point, cap_center), orthos12[0]);
	xy[1] = vec_dot(vec_sub(hit_info->point, cap_center), orthos12[1]);
	*u_and_v[0] = (atan2(xy[1], xy[0]) + M_PI) / DOUBLE_PI;
	*u_and_v[1] = sqrt(xy[0] * xy[0] + xy[1] * xy[1]) / cyl.radius;
}

static void	first_else(const t_cylinder cyl, const t_vector vec_to_i,
		const double h_proj, double *u_and_v[2])
{
	t_vector			ortho_to_dir;
	t_vector			obcn;

	if (fabs(cyl.direction.x) < fabs(cyl.direction.y)
		|| fabs(cyl.direction.x) < fabs(cyl.direction.z))
		ortho_to_dir = vec_normalize(vec_cross(cyl.direction,
					(t_vector){1, 0, 0}));
	else
		ortho_to_dir = vec_normalize(vec_cross(cyl.direction,
					(t_vector){0, 1, 0}));
	obcn = vec_sub(vec_to_i, vec_scale(h_proj, cyl.direction));
	*u_and_v[0] = (atan2(vec_dot(obcn, ortho_to_dir),
				vec_dot(obcn, vec_normalize(vec_cross(
							cyl.direction, ortho_to_dir)))) + M_PI) / DOUBLE_PI;
	*u_and_v[1] = (h_proj + cyl.half_height) / cyl.height;
}

void	get_cylinder_uv(const t_hit *hit_info, double *u, double *v)
{
	const t_cylinder	cyl = hit_info->shape->cylinder;
	const t_vector		vec_to_i
		= vec_sub(hit_info->point, cyl.center);
	const double		h_proj
		= vec_dot(vec_to_i, cyl.direction);
	double				*u_and_v[2];

	u_and_v[0] = u;
	u_and_v[1] = v;
	if (fabs(h_proj) >= (cyl.half_height - EPSILON))
		first_if(h_proj, cyl, hit_info, u_and_v);
	else
		first_else(cyl, vec_to_i, h_proj, u_and_v);
}
