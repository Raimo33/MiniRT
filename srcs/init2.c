/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init2.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:27:35 by craimond          #+#    #+#             */
/*   Updated: 2024/05/01 15:22:24 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static int	key_hook(int keycode, t_hook_data *hook_data);

void	destroy_and_quit(t_mlx_data *mlx_data)
{
	destroy_mlx_data(mlx_data);
	ft_quit(41, "mlx: failed to load texture");
}

void	init_textures(const t_scene *scene, t_mlx_data *mlx_data)
{
	t_list			*shapes;
	t_shape			*shape;
	t_material		*m;
	void			*ti;
	int32_t			bps;

	shapes = scene->shapes;
	while (shapes)
	{
		shape = shapes->content;
		m = shape->material;
		if (m->texture)
		{
			ti = mlx_xpm_file_to_image(mlx_data->mlx, m->texture->path,
					&m->texture->width, &m->texture->height);
			if (!ti)
				destroy_and_quit(mlx_data);
			m->texture->addr = mlx_get_data_addr(ti, &bps,
					&m->texture->line_length, &m->texture->endian);
			if (!m->texture->addr)
				destroy_and_quit(mlx_data);
			m->texture->bytes_per_pixel = bps / 8;
		}
		shapes = shapes->next;
	}
}

void	init_hooks(t_mlx_data *win_data, t_scene *scene)
{
	t_hook_data	*hook_data;

	hook_data = calloc_p(1, sizeof(t_hook_data));
	if (!hook_data)
		ft_quit(3, "hook data initialization failed");
	hook_data->win_data = win_data;
	hook_data->scene = scene;
	mlx_hook(win_data->win, 2, 1L << 0, key_hook, hook_data);
	mlx_hook(win_data->win, 17, 1L << 17, close_win, hook_data);
}

static int	key_hook(const int keycode, t_hook_data *hook_data)
{
	t_mlx_data	*win_data;

	win_data = hook_data->win_data;
	if (keycode == KEY_ESC)
		close_win(hook_data);
	else if (keycode == KEY_SPACE && win_data->n_images > 1)
	{
		win_data->current_img--;
		if (win_data->current_img < 0)
			win_data->current_img = win_data->n_images - 1;
		mlx_clear_window(hook_data->win_data->mlx, hook_data->win_data->win);
		mlx_put_image_to_window(win_data->mlx, win_data->win,
			win_data->images[win_data->current_img], 0, 0);
	}
	return (0);
}
