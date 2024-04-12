/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:27:35 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 13:56:56 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static int key_hook(int keycode, t_hook_data *hook_data);

void	check_args(const uint16_t argc, char **argv)
{
	//TODO controllare se fov sta tra 0 e 180
	if (argc != 2)
		ft_quit(1, "invalid number of arguments");
	if (ft_strncmp(argv[1] + ft_strlen(argv[1]) - 3, ".rt", 4) != 0)
		ft_quit(2, "invalid file extension");
}

void	init_scene(t_scene *scene)
{
	scene->amblight.brightness = 0;
	scene->amblight.color = (t_color){0, 0, 0};
	scene->current_camera = NULL;
	scene->cameras = NULL;
	scene->lights = NULL;
	scene->n_lights = 0;
	scene->shapes = NULL;
	scene->octree = (t_octree *)malloc(sizeof(t_octree));	
	scene->world_max.x = 0;
	scene->world_max.y = 0;
}

void	init_window(t_mlx_data *win_data, t_scene *scene)
{
	uint16_t	i;
	
	win_data->mlx = mlx_init();
	win_data->win = mlx_new_window(win_data->mlx,
			WIN_WIDTH, WIN_HEIGHT, "miniRT");
	if (!win_data->win)
		ft_quit(3, "window initialization failed");
	win_data->n_images = ft_lstsize(scene->cameras);
	win_data->images = (void **)malloc(sizeof(void *) * win_data->n_images);
	win_data->addrresses = (char **)malloc(sizeof(char *) * win_data->n_images);
	i = 0;
	while (i < win_data->n_images)
	{
		win_data->images[i] = mlx_new_image(win_data->mlx, WIN_WIDTH, WIN_HEIGHT);
		if (!win_data->images[i])
			ft_quit(3, "image initialization failed");
		win_data->addrresses[i] = mlx_get_data_addr(win_data->images[i],
				&win_data->bits_per_pixel, &win_data->line_length, &win_data->endian);
		i++;
	}
	win_data->current_img = 0;
	win_data->bytes_per_pixel = win_data->bits_per_pixel / 8;
}

void init_hooks(t_mlx_data *win_data, t_scene *scene)
{
	t_hook_data *hook_data;
	
	hook_data = malloc(sizeof(t_hook_data));
	if (!hook_data)
		ft_quit(3, "hook data initialization failed");
	hook_data->win_data = win_data;
	hook_data->scene = scene;
	mlx_hook(win_data->win, 2, 1L << 0, key_hook, hook_data);
	mlx_hook(win_data->win, 17, 1L << 17, close_win, hook_data);
}

static int key_hook(const int keycode, t_hook_data *hook_data)
{
	t_mlx_data *win_data;
	uint16_t	idx;

	win_data = hook_data->win_data;
	if (keycode == KEY_ESC)
		close_win(hook_data);
	else if (keycode == KEY_SPACE && win_data->n_images > 1)
	{
		win_data->current_img++;
		idx = win_data->current_img % win_data->n_images;
		mlx_clear_window(hook_data->win_data->mlx, hook_data->win_data->win);
		mlx_put_image_to_window(win_data->mlx, win_data->win, win_data->images[idx], 0, 0);
	}
	return (0);
}