/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render3.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:24:04 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 16:10:50 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

inline t_vector	get_cylinder_normal(t_cylinder cylinder, t_point point)
{
	const t_vector		vec_from_center_to_point
		= vec_sub(point, cylinder.center);
	const double		projection_length
		= vec_dot(vec_from_center_to_point, cylinder.direction);
	t_vector			projection;

	if (fabs(projection_length + cylinder.half_height) < EPSILON)
		return (vec_negate(cylinder.direction));
	else if (fabs(projection_length - cylinder.half_height) < EPSILON)
		return (cylinder.direction);
	projection = vec_add(cylinder.center,
			vec_scale(projection_length, cylinder.direction));
	return (vec_normalize(vec_sub(point, projection)));
}

inline t_vector	get_cone_normal(t_cone cone, t_point point)
{
	const t_vector		co = vec_sub(point, cone.base_center);
	const double		cos_alpha_squared
		= pow(cos(atan(cone.radius / cone.height)), 2);
	const double		v_dot_d = vec_dot(cone.direction, co);
	const double		a = v_dot_d * v_dot_d - cos_alpha_squared;
	const double		c = v_dot_d * v_dot_d
		- vec_dot(co, co) * cos_alpha_squared;

	if (((2.0 * c) * (2.0 * c) - 2 * (2.0 * a) * c) < 0)
		return (vec_normalize(co));
	return (vec_normalize(vec_sub(point, vec_add(cone.base_center,
					vec_scale(-v_dot_d + sqrt(((2.0 * c) * (2.0 * c) - 2
								* (2.0 * a) * c)) / (2.0 * a),
						cone.direction)))));
}
