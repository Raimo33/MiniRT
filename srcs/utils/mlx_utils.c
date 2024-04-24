/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/04/24 21:11:20 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	my_mlx_pixel_put(const t_mlx_data *data, const uint16_t x, const uint16_t y, const t_color color)
{
	void	*dst;

	dst = data->addrresses[data->current_img] + (y * data->line_length + x * data->bytes_per_pixel);
	if (data->endian == 0)
		*(uint32_t *)dst = (color.r << 16) | (color.g << 8) | color.b;
	else
		*(uint32_t *)dst = (color.b << 24) | (color.g << 16) | (color.r << 8);
}

t_color my_mlx_pixel_get(const t_texture_data *data, const uint32_t x, const uint32_t y)
{
	t_color		color;
	uint32_t	*dst;

	dst = (uint32_t *)(data->addr + (y * data->line_length + x * data->bytes_per_pixel));
	if (data->endian == 0)
	{
		color.r = (*dst & 0x00FF0000) >> 16;
		color.g = (*dst & 0x0000FF00) >> 8;
		color.b = (*dst & 0x000000FF);
	}
	else
	{
		color.b = (*dst & 0xFF000000) >> 24;
		color.g = (*dst & 0x00FF0000) >> 16;
		color.r = (*dst & 0x0000FF00) >> 8;
	}
	return (color);
}
