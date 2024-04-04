/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/04/04 12:36:26 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	my_mlx_pixel_put(const t_mlx_data *data, const uint16_t x, const uint16_t y, const uint32_t color)
{
	void		*dst;

	dst = data->frame_addr + (y * data->line_length + x * data->bytes_per_pixel);
	if (data->endian == 0)
		*(uint32_t *)dst = color;
	else
		*(uint32_t *)dst = color >> 8 | color << 24;
}

char	*my_mlx_get_data_addr(void *img_ptr, int32_t *bits_per_pixel, int32_t *size_line, int32_t *endian)
{
	char		*addr;
	int32_t 	bpp;
	int32_t		sl;
	int32_t		end;

	if (!bits_per_pixel)
		bits_per_pixel = &bpp;
	if (!size_line)
		size_line = &sl;
	if (!endian)
		endian = &end;
	addr = mlx_get_data_addr(img_ptr, bits_per_pixel, size_line, endian);
	return (addr);
}

void	my_mlx_stack_image(t_mlx_data *data)
{
	static const float		transparency_ratio = 1.0f / N_FRAMES;

	uint16_t	x;
	uint16_t	y;

	y = 0;
	while (y < WIN_HEIGHT)
	{
		x = 0;
		while (x <WIN_WIDTH)
		{
			int i = x + (y * data->line_length / data->bytes_per_pixel);
			uint32_t new_pix = data->frame_addr[i];
			uint32_t old_pix = data->main_img_addr[i];
			
			// Blend RGB components
            uint8_t nr = (new_pix >> 16) & 0xFF, ng = (new_pix >> 8) & 0xFF, nb = new_pix & 0xFF;
            uint8_t or = (old_pix >> 16) & 0xFF, og = (old_pix >> 8) & 0xFF, ob = old_pix & 0xFF;

            uint8_t br = or + (uint8_t)((nr - or) * transparency_ratio);
            uint8_t bg = og + (uint8_t)((ng - og) * transparency_ratio);
            uint8_t bb = ob + (uint8_t)((nb - ob) * transparency_ratio);

            data->main_img_addr[i] = (br << 16) | (bg << 8) | bb;
			x++;
		}
		y++;
	}
}

