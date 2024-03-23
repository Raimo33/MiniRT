/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/03/23 22:26:07 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		parse_line(char *line, t_scene *scene);
static void		parse_amblight(char *line, t_scene *scene);
static void		parse_light(char *line, t_scene *scene);
static void		parse_camera(char *line, t_scene *scene);
static void		parse_sphere(char *line, t_scene *scene);
static void		parse_plane(char *line, t_scene *scene);
static void		parse_cylinder(char *line, t_scene *scene);
static t_coord	parse_coord(char *str);
static t_color	parse_color(char *str);

void parse(int fd, t_scene *scene)
{
	char *line;

	line = get_next_line(fd);
    while (line)
	{
		if (!is_empty_line(line) && !is_comment(line))
			parse_line(line, scene);
		free(line);
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

	light.center = parse_coord(ft_strtok(line, spaces));
	light.brightness = ft_atof(ft_strtok(NULL, spaces));
	light.color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->lights, ft_lstnew(&light));
}

static void	parse_camera(char *line, t_scene *scene)
{
	t_camera	camera;

	camera.center = parse_coord(ft_strtok(line, spaces));
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

	plane.center = parse_coord(ft_strtok(line, spaces));
	plane.normal = parse_coord(ft_strtok(NULL, spaces));
	plane.color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->planes, ft_lstnew(&plane));
}

static void	parse_cylinder(char *line, t_scene *scene)
{
	t_cylinder	cylinder;

	cylinder.center = parse_coord(ft_strtok(line, spaces));
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
