/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/04/06 19:36:53 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	my_mlx_pixel_put(const t_mlx_data *data, const uint16_t x, const uint16_t y, const t_color color)
{
	uint32_t	*dst;

	dst = (uint32_t *)(data->addr + (y * data->line_length + x * data->bytes_per_pixel));
	if (data->endian == 0)
	{
		dst[0] = color.a;	
		dst[1] = color.b;
		dst[2] = color.g;
		dst[3] = color.r;
	}
	else
	{
		dst[0] = color.r;
		dst[1] = color.g;
		dst[2] = color.b;
		dst[3] = color.a;
	}
}

