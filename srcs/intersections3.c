/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections3.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:15:20 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 16:22:36 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

inline t_point	ray_point_at_parameter(const t_ray ray, const double t)
{
	return ((t_point)
		{
			.x = ray.origin.x + t * ray.direction.x,
			.y = ray.origin.y + t * ray.direction.y,
			.z = ray.origin.z + t * ray.direction.z
		});
}
