/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_operations3.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:52:38 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 17:59:17 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

inline double	vec_length(const t_vector a)
{
	return (sqrt(a.x * a.x + a.y * a.y + a.z * a.z));
}

inline t_vector	project_vector_onto_axis(t_vector vector, t_vector axis)
{
	const t_vector	normalized_axis = vec_normalize(axis);
	const double	dot_product = vec_dot(vector, normalized_axis);
	const t_vector	projected_vec = vec_scale(dot_product, normalized_axis);

	return (projected_vec);
}
