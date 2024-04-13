/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/13 11:53:20 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		parse_line(char *line, t_scene *scene);
static void		parse_amblight(t_scene *scene);
static void		parse_light(t_scene *scene);
static void		parse_camera(t_scene *scene);
static void		parse_shape(char *line, t_scene *scene);
static void		parse_sphere(t_shape *shape);
static void		parse_plane(t_shape *shape);
static void		parse_cylinder(t_shape *shape);
static void		parse_triangle(t_shape *shape);
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
	static const char		*prefixes[] = {"A", "L", "C"};
	void (*const			parse_objects[])(t_scene *) = {&parse_amblight, &parse_light, &parse_camera};
	static uint8_t			n_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	uint8_t					i;
	
	i = 0;
	while (i < n_prefixes)
	{
		if (line[0] == prefixes[i][0])
		{
			ft_strtok(line, spaces);
			parse_objects[i](scene);
			return ;
		}
		i++;
	}
	parse_shape(line, scene);
}

static void	parse_shape(char *line, t_scene *scene)
{
	static const char		*prefixes[] = {"sp", "cy", "tr", "pl"};
	void (*const			parse_funcs[])(t_shape *) = {&parse_sphere, &parse_cylinder, &parse_triangle, &parse_plane};
	static const uint8_t	n_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	uint8_t		i;
	t_shape		*shape;

	i = 0;
	while (i < n_prefixes)
	{
		if (ft_strncmp(line, prefixes[i], 2) == 0)
		{
			shape = (t_shape *)malloc(sizeof(t_shape));
			shape->material = (t_material *)malloc(sizeof(t_material));
			ft_strtok(line, spaces); //per skippare la lettera
			parse_funcs[i](shape);
			shape->material->color = parse_color(ft_strtok(NULL, spaces));
			shape->material->shininess = ft_atof(ft_strtok(NULL, spaces));
			ft_lstadd_front(&scene->shapes, ft_lstnew(shape));
			return ;
		}
		i++;
	}
	ft_quit(4, "invalid shape prefix");
}

static void	parse_amblight(t_scene *scene)
{
	t_amblight	amblight;

	amblight.brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	amblight.color = parse_color(ft_strtok(NULL, spaces));
	amblight.ambient.r = amblight.color.r * amblight.brightness;
	amblight.ambient.g = amblight.color.g * amblight.brightness;
	amblight.ambient.b = amblight.color.b * amblight.brightness;
	scene->amblight = amblight;
}

static void	parse_light(t_scene *scene)
{
	t_light		*light;

	light = (t_light *)malloc(sizeof(t_light));
	light->center = parse_coord(ft_strtok(NULL, spaces));
	light->brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	light->color = parse_color(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->lights, ft_lstnew(light));
	scene->n_lights++;
}

static void	parse_camera(t_scene *scene)
{
	t_camera	*camera;
	
	camera = (t_camera *)malloc(sizeof(t_camera));
	camera->center = parse_coord(ft_strtok(NULL, spaces));
	camera->normal = parse_coord(ft_strtok(NULL, spaces));
	camera->fov = ft_atoui(ft_strtok(NULL, spaces));
	ft_lstadd_front(&scene->cameras, ft_lstnew(camera));
}

static void	parse_sphere(t_shape *shape)
{
	shape->sphere.center = parse_coord(ft_strtok(NULL, spaces));
	shape->sphere.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	shape->type = SPHERE;
}

static void	parse_plane(t_shape *shape)
{
	shape->plane.center = parse_coord(ft_strtok(NULL, spaces));
	shape->plane.normal = parse_coord(ft_strtok(NULL, spaces));
	shape->type = PLANE;
}

static void	parse_cylinder(t_shape *shape)
{
	shape->cylinder.center = parse_coord(ft_strtok(NULL, spaces));
	shape->cylinder.direction = parse_coord(ft_strtok(NULL, spaces));
	shape->cylinder.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	shape->cylinder.half_height = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	shape->type = CYLINDER;
}

static void	parse_triangle(t_shape *shape)
{
	shape->triangle.vertices[0] = parse_coord(ft_strtok(NULL, spaces));
	shape->triangle.vertices[1] = parse_coord(ft_strtok(NULL, spaces));
	shape->triangle.vertices[2] = parse_coord(ft_strtok(NULL, spaces));
	shape->type = TRIANGLE;
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
