/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   math_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 14:28:36 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 13:55:01 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

inline double	fclamp(const double value, const double min, const double max)
{
	if (value < min)
		return (min);
	if (value > max)
		return (max);
	return (value);
}

inline int32_t	clamp(const int32_t value, const int32_t min, const int32_t max)
{
	if (value < min)
		return (min);
	if (value > max)
		return (max);
	return (value);
}
