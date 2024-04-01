/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/04/01 18:00:32 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	my_mlx_pixel_put(const t_mlx_data data, const uint16_t x, const uint16_t y, const uint32_t color)
{
	void	*dst;

	dst = data.addr + (y * data.line_length + x * (data.bits_per_pixel / 8));
	*(unsigned int *)dst = rgb_to_hex(color, data.endian);
}

