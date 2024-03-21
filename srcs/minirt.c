/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 17:33:18 by craimond          #+#    #+#             */
/*   Updated: 2024/03/21 18:01:12 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

static void		check_args(uint16_t argc, char **argv);
static void		init_scene(t_scene *scene, int fd);
static void		init_window(t_mlx_data *data);
static void		init_hooks(t_mlx_data *win_data, t_scene scene);
static int		key_hook(int keycode, t_hook_data *hook_data);
static void		destroy_scene(t_scene scene);
static void		parse(int fd, t_scene *scene);
static void		parse_line(char *line, t_scene *scene);
static int		close_win(t_hook_data *hook_data);
static void		parse_amblight(char *line, t_scene *scene);
static void		parse_light(char *line, t_scene *scene);
static void		parse_camera(char *line, t_scene *scene);
static void		parse_sphere(char *line, t_scene *scene);
static void		parse_plane(char *line, t_scene *scene);
static void		parse_cylinder(char *line, t_scene *scene);
static t_coord	parse_coord(char *str);
static t_color	parse_color(char *str);

int	main(int argc, char **argv)
{
	t_mlx_data	mlx_data;
	t_scene		scene;
	int			fd;

	check_args(argc, argv);
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		ft_quit(4, NULL);
	init_scene(&scene, fd);
	init_window(&mlx_data);
	init_hooks(&mlx_data, scene);
	mlx_loop(mlx_data.mlx);
	return (0);
}

static void	init_scene(t_scene *scene, int fd)
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

static bool is_comment(char *line)
{
	return (*line == '#');
}

static bool	ft_isspace(char c)
{
	return (ft_strchr(spaces, c) != NULL);
}

static bool is_empty_line(char *line)
{
	while (*line)
	{
		if (!ft_isspace(*line))
			return (false);
		line++;
	}
	return (true);
}

static void parse(int fd, t_scene *scene)
{
	char *line;
	
	line = get_next_line(fd);
    while (line)
	{
		if (!is_empty_line(line) && !is_comment(line))
		{
			parse_line(line, scene);
			free(line);
		}
		line = get_next_line(fd);
	}
	close(fd);
}

static void	parse_line(char *line, t_scene *scene)
{
	const char		*prefixes[] = {"A", "L", "C", "sp", "pl", "cy"};
	void (*const	parse_funcs[])(char *, t_scene *) = {&parse_amblight, &parse_light, &parse_camera, &parse_sphere, &parse_plane, &parse_cylinder};
	uint8_t		i;
	uint8_t		n_prefixes;

	n_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	i = 0;
	while (i < n_prefixes)
	{
		if (ft_strncmp(line, prefixes[i], ft_strlen(prefixes[i])) == 0)
		{
			parse_funcs[i](line, scene);
			return ;
		}
		i++;
	}
}

static void	parse_amblight(char *line, t_scene *scene)
{
	t_amblight	amblight;

	amblight.brightness = ft_atof(ft_strtok(line, spaces));
	amblight.color = parse_color(ft_strtok(NULL, spaces));
	scene->amblight = amblight;
}

static void	parse_light(char *line, t_scene *scene)
{
	t_light		light;
	
	light.point = parse_coord(ft_strtok(line, spaces));
	light.brightness = ft_atof(ft_strtok(NULL, spaces));
	light.color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->lights, ft_lstnew(&light));
}

static void	parse_camera(char *line, t_scene *scene)
{
	t_camera	camera;
	
	camera.point = parse_coord(ft_strtok(line, spaces));
	camera.normal = parse_coord(ft_strtok(NULL, spaces));
	camera.fov = ft_atoui(ft_strtok(NULL, spaces));
	scene->camera = camera;
}

static void	parse_sphere(char *line, t_scene *scene)
{
	t_sphere	sphere;
	
	sphere.center = parse_coord(ft_strtok(line, spaces));
	sphere.radius = ft_atof(ft_strtok(NULL, spaces));
	sphere.color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->spheres, ft_lstnew(&sphere));
}

static void	parse_plane(char *line, t_scene *scene)
{
	t_plane		plane;
	
	plane.point = parse_coord(ft_strtok(line, spaces));
	plane.normal = parse_coord(ft_strtok(NULL, spaces));
	plane.color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->planes, ft_lstnew(&plane));
}

static void	parse_cylinder(char *line, t_scene *scene)
{
	t_cylinder	cylinder;
	
	cylinder.point = parse_coord(ft_strtok(line, spaces));
	cylinder.normal = parse_coord(ft_strtok(NULL, spaces));
	cylinder.diameter = ft_atof(ft_strtok(NULL, spaces));
	cylinder.height = ft_atof(ft_strtok(NULL, spaces));
	cylinder.color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->cylinders, ft_lstnew(&cylinder));
}

static t_coord	parse_coord(char *str)
{
	t_coord	coord;
	
	coord.x = ft_atof(str);
	coord.y = ft_atof(ft_strtok(NULL, ","));
	coord.z = ft_atof(ft_strtok(NULL, ","));
	return (coord);
}

static t_color	parse_color(char *str)
{
	t_color	color;
	
	color.r = ft_atoui(str);
	color.g = ft_atoui(ft_strtok(NULL, ","));
	color.b = ft_atoui(ft_strtok(NULL, ","));
	return (color);
}

static void	check_args(uint16_t argc, char **argv)
{
	if (argc != 2)
		ft_quit(1, "invalid number of arguments");
	if (ft_strncmp(argv[1] + ft_strlen(argv[1]) - 3, ".rt", 3) != 0)
		ft_quit(2, "invalid file extension");
}

static void	init_window(t_mlx_data *win_data)
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

static void init_hooks(t_mlx_data *win_data, t_scene scene)
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

static int close_win(t_hook_data *hook_data)
{
	destroy_scene(hook_data->scene);
	mlx_destroy_window(hook_data->win_data->mlx, hook_data->win_data->win);
	mlx_destroy_image(hook_data->win_data->mlx, hook_data->win_data->img);
	mlx_destroy_display(hook_data->win_data->mlx);
	free(hook_data);
	exit(0);
	return (0);
}

static void destroy_scene(t_scene scene)
{
	ft_lstclear(&scene.lights, NULL);
	ft_lstclear(&scene.spheres, NULL);
	ft_lstclear(&scene.planes, NULL);
	ft_lstclear(&scene.cylinders, NULL);
}