/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/01 15:48:06 by craimond         ###   ########.fr       */
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
static t_float3	parse_coord(const char *str);
static t_color	parse_color(const char *str);

void parse_scene(const int fd, t_scene *scene)
{
	char	*line;

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

	ft_strtok(line, spaces); //per skippare la lettera
	amblight.brightness = ft_atof(ft_strtok(NULL, spaces));
	amblight.color = parse_color(ft_strtok(NULL, spaces));
	scene->amblight = amblight;
}

static void	parse_light(char *line, t_scene *scene)
{
	t_light		*light;

	light = (t_light *)malloc(sizeof(t_light));
	ft_strtok(line, spaces); //per skippare la lettera
	light->center = parse_coord(ft_strtok(NULL, spaces));
	light->brightness = ft_atof(ft_strtok(NULL, spaces));
	light->color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->lights, ft_lstnew(light));
}

static void	parse_camera(char *line, t_scene *scene)
{
	t_camera	camera;

	ft_strtok(line, spaces); //per skippare la lettera
	camera.center = parse_coord(ft_strtok(NULL, spaces));
	camera.normal = parse_coord(ft_strtok(NULL, spaces));
	camera.fov = ft_atoui(ft_strtok(NULL, spaces));
	scene->camera = camera;
}

static void	parse_sphere(char *line, t_scene *scene)
{
	t_sphere	sphere;
	t_shape		*shape;

	shape = (t_shape *)malloc(sizeof(t_shape));
	ft_strtok(line, spaces); //per skippare la lettera
	sphere.center = parse_coord(ft_strtok(NULL, spaces));
	sphere.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	shape->material.color = parse_color(ft_strtok(NULL, spaces));
	shape->type = SPHERE;
	shape->sphere = sphere;
	ft_lstadd_front(&scene->shapes, ft_lstnew(shape));
}

static void	parse_plane(char *line, t_scene *scene)
{
	t_plane		plane;
	t_shape		*shape;

	shape = (t_shape *)malloc(sizeof(t_shape));
	ft_strtok(line, spaces); //per skippare la lettera
	plane.center = parse_coord(ft_strtok(NULL, spaces));
	plane.normal = parse_coord(ft_strtok(NULL, spaces));
	shape->material.color = parse_color(ft_strtok(NULL, spaces));
	shape->type = PLANE;
	shape->plane = plane;
	ft_lstadd_front(&scene->shapes, ft_lstnew(shape));
}

static void	parse_cylinder(char *line, t_scene *scene)
{
	t_cylinder	cylinder;
	t_shape		*shape;

	shape = (t_shape *)malloc(sizeof(t_shape));
	ft_strtok(line, spaces); //per skippare la lettera
	cylinder.center = parse_coord(ft_strtok(NULL, spaces));
	cylinder.direction = parse_coord(ft_strtok(NULL, spaces));
	cylinder.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	cylinder.height = ft_atof(ft_strtok(NULL, spaces));
	shape->material.color = parse_color(ft_strtok(NULL, spaces));
	shape->type = CYLINDER;
	shape->cylinder = cylinder;
	ft_lstadd_front(&scene->shapes, ft_lstnew(shape));
}

static t_float3	parse_coord(const char *str)
{
	t_float3	coord;

	//TODO abbellire, check syntax
	coord.x = ft_atof(str);
	str = ft_strchr(str, ',') + 1;
	coord.y = ft_atof(str);
	str = ft_strchr(str, ',') + 1;
	coord.z = ft_atof(str);
	return (coord);
}

static t_color	parse_color(const char *str)
{
	t_color	color;

	color.r = ft_atoui(str);
	str = ft_strchr(str, ',') + 1;
	color.g = ft_atoui(str);
	str = ft_strchr(str, ',') + 1;
	color.b = ft_atoui(str);
	return (color);
}
