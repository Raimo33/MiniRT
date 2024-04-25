/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_operations2.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:52:38 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 17:59:25 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

inline t_vector	vec_cross(const t_vector a, t_vector b)
{
	return ((t_vector){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	});
}

inline t_vector	vec_normalize(const t_vector a)
{
	double		len;

	len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	return ((t_vector){a.x / len, a.y / len, a.z / len});
}

inline t_vector	vec_negate(const t_vector a)
{
	return ((t_vector){-a.x, -a.y, -a.z});
}

inline t_vector	vec_scale(const double scale, const t_vector a)
{
	return ((t_vector){a.x * scale, a.y * scale, a.z * scale});
}

inline double	vec_dot(const t_vector a, const t_vector b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}
