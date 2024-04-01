/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/04/01 21:20:51 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	my_mlx_pixel_put(const t_mlx_data data, const uint16_t x, const uint16_t y, const uint32_t color)
{
	void	*dst;

	dst = data.addr + (y * data.line_length + x * (data.bits_per_pixel / 8));
	if (data.endian == 0)
		*(uint32_t *)dst = color;
	else
		*(uint32_t *)dst = color >> 8 | color << 24;
}

