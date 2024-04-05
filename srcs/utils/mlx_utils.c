/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 15:23:58 by craimond          #+#    #+#             */
/*   Updated: 2024/04/05 02:21:18 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	my_mlx_pixel_put(const t_mlx_data *data, const uint16_t x, const uint16_t y, const uint32_t color, const uint16_t frame_no)
{
	void	*dst;

	dst = data->frames[frame_no] + (y * data->line_length + x * data->bytes_per_pixel);
	if (data->endian == 0)
		*(uint32_t *)dst = color;
	else
		*(uint32_t *)dst = color >> 8 | color << 24;
}

// char	*my_mlx_get_data_addr(void *img_ptr, int32_t *bits_per_pixel, int32_t *size_line, int32_t *endian)
// {
// 	char		*addr;
// 	int32_t 	bpp;
// 	int32_t		sl;
// 	int32_t		end;

// 	if (!bits_per_pixel)
// 		bits_per_pixel = &bpp;
// 	if (!size_line)
// 		size_line = &sl;
// 	if (!endian)
// 		endian = &end;
// 	addr = mlx_get_data_addr(img_ptr, bits_per_pixel, size_line, endian);
// 	return (addr);
// }

// void	my_mlx_stack_image(t_mlx_data *data)
// {
// 	static const float		transparency_ratio = 1.0f / N_FRAMES;
// 	float					k;
// 	uint16_t				x;
// 	uint16_t				y;

// 	k = data->line_length / data->bytes_per_pixel;
// 	y = 0;
// 	while (y < WIN_HEIGHT)
// 	{
// 		x = 0;
// 		while (x <WIN_WIDTH)
// 		{
// 			int i = x + (y * k);
// 			uint32_t new_pix = data->frame_addr[i];
// 			uint32_t old_pix = data->main_img_addr[i];
			
// 			// Blend RGB components
// 			t_color	new = {new_pix >> 16, (new_pix >> 8) & 0xFF, new_pix & 0xFF, 0};
// 			t_color	old = {old_pix >> 16, (old_pix >> 8) & 0xFF, old_pix & 0xFF, 0};

// 			t_color res;

// 			res.r = old.r + (uint8_t)((new.r - old.r) * transparency_ratio);
// 			res.g = old.g + (uint8_t)((new.g - old.g) * transparency_ratio);
// 			res.b = old.b + (uint8_t)((new.b - old.b) * transparency_ratio);

//             data->main_img_addr[i] = (res.r << 16) | (res.g << 8) | res.b;
// 			x++;
// 		}
// 		y++;
// 	}
// }

