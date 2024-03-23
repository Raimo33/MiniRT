/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:27:35 by craimond          #+#    #+#             */
/*   Updated: 2024/03/23 21:29:48 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

static int key_hook(int keycode, t_hook_data *hook_data);

void	check_args(uint16_t argc, char **argv)
{
	if (argc != 2)
		ft_quit(1, "invalid number of arguments");
	if (ft_strncmp(argv[1] + ft_strlen(argv[1]) - 3, ".rt", 3) != 0)
		ft_quit(2, "invalid file extension");
}

void	init_scene(t_scene *scene, int fd)
{
	scene->amblight.brightness = 0;
	scene->amblight.color.r = 0;
	scene->amblight.color.g = 0;
	scene->amblight.color.b = 0;
	scene->camera.point.x = 0;
	scene->camera.point.y = 0;
	scene->camera.point.z = 0;
	scene->camera.normal.x = 0;
	scene->camera.normal.y = 0;
	scene->camera.normal.z = 0;
	scene->camera.fov = 0;
	scene->lights = NULL;
	scene->spheres = NULL;
	scene->planes = NULL;
	scene->cylinders = NULL;
	parse(fd, scene);
	get_next_line(-1);
}

void	init_window(t_mlx_data *win_data)
{
	win_data->mlx = mlx_init();
	win_data->win = mlx_new_window(win_data->mlx,
			WIN_WIDTH, WIN_HEIGHT, "miniRT");
	win_data->img = mlx_new_image(win_data->mlx, WIN_WIDTH, WIN_HEIGHT);
	win_data->addr = mlx_get_data_addr(win_data->img, &win_data->bits_per_pixel,
			&win_data->line_length, &win_data->endian);
	if (!win_data->win)
		ft_quit(3, "window initialization failed");
}

void init_hooks(t_mlx_data *win_data, t_scene scene)
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

static int key_hook(int keycode, t_hook_data *hook_data)
{
	if (keycode == KEY_ESC)
		close_win(hook_data);
	return (0);
}