/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/03/28 23:25:57 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

static inline uint16_t	rgb_to_int(const t_color color);

void	my_mlx_pixel_put(const t_mlx_data data, const uint16_t x, const uint16_t y, const t_color color)
{
	void	*dst;

	dst = data.addr + (y * data.line_length + x * (data.bits_per_pixel / 8));
	*(unsigned int *)dst = rgb_to_int(color);
}

static inline uint16_t	rgb_to_int(const t_color color)
{
	return ((color.r << 16) + (color.g << 8) + color.b);
}

inline t_color	hex_to_rgb(const uint32_t hex) //TODO da testare
{
	return ((t_color){.r = (hex >> 16) & 0xFF, .g = (hex >> 8) & 0xFF, .b = hex & 0xFF});
}