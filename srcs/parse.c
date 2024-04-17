/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/17 15:18:48 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		parse_line(char *line, t_scene *scene);
static void		parse_amblight(t_scene *scene);
static void		parse_light(t_scene *scene);
static void		parse_camera(t_scene *scene);
static void		parse_shape(char *line, t_scene *scene);
static void		parse_material(t_material *material);
static void		parse_sphere(t_shape *shape);
static void		parse_plane(t_shape *shape);
static void		parse_cylinder(t_shape *shape);
static void		parse_triangle(t_shape *shape);
static void		parse_cone(t_shape *shape);
static t_float3	parse_coord(char *str);
static t_color	parse_color(char *str);
static char 	*skip_commas(char *str);
static bool 	is_scene_valid(const t_scene *scene);

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
	if (!is_scene_valid(scene))
	{
		destroy_scene(scene);
		exit(4);
	}
}

static bool is_scene_valid(const t_scene *scene)
{
	if (!scene->cameras)
		ft_putstr_fd("Invalid scene: no cameras found", STDERR_FILENO);
	if (!scene->shapes)
		ft_putstr_fd("Invalid scene: no shapes found", STDERR_FILENO);
	if (!scene->lights)
		ft_putstr_fd("Invalid scene: no lights found", STDERR_FILENO);
	if (!scene->amblight)
		ft_putstr_fd("Invalid scene: no ambient light found", STDERR_FILENO);
	return (scene->cameras && scene->shapes && scene->lights && scene->amblight);
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
	static const char		*prefixes[] = {"sp", "cy", "tr", "co", "pl"};
	void (*const			parse_funcs[])(t_shape *) = {&parse_sphere, &parse_cylinder, &parse_triangle, &parse_cone, &parse_plane};
	static const uint8_t	n_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	uint8_t		i;
	t_shape		*shape;

	i = 0;
	while (i < n_prefixes)
	{
		if (ft_strncmp(line, prefixes[i], 2) == 0)
		{
			shape = (t_shape *)calloc_p(1, sizeof(t_shape));
			shape->material = (t_material *)calloc_p(1, sizeof(t_material));
			ft_strtok(line, spaces); //per skippare la lettera
			parse_funcs[i](shape);
			parse_material(shape->material);
			ft_lstadd_front(&scene->shapes, ft_lstnew(shape));
			return ;
		}
		i++;
	}
	ft_quit(4, "invalid shape prefix");
}

static void	parse_material(t_material *material)
{
	material->color = parse_color(ft_strtok(NULL, spaces));
	material->shininess = fmax(ft_atof(ft_strtok(NULL, spaces)), 0);
	material->specular = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1.0f);
	material->diffuse = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1.0f);
	material->is_checkerboard = (ft_strncmp(ft_strtok(NULL, spaces), "true", 12) == 0);
	if (material->shininess == 0)
		ft_putstr_fd("Warning: material shininess set to 0\n", STDERR_FILENO);
	if (material->specular == 0)
		ft_putstr_fd("Warning: material specular set to 0\n", STDERR_FILENO);
	if (material->diffuse == 0)
		ft_putstr_fd("Warning: material diffuse set to 0\n", STDERR_FILENO);
}

static void	parse_amblight(t_scene *scene)
{
	t_amblight *amblight;

	amblight = (t_amblight *)calloc_p(1, sizeof(t_amblight));
	amblight->brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	if (amblight->brightness == 0)
		ft_putstr_fd("Warning: ambient light brightness set to 0\n", STDERR_FILENO);
	amblight->color = parse_color(ft_strtok(NULL, spaces));
	amblight->ambient.r = amblight->color.r * amblight->brightness;
	amblight->ambient.g = amblight->color.g * amblight->brightness;
	amblight->ambient.b = amblight->color.b * amblight->brightness;
	scene->amblight = amblight;
}

static void	parse_light(t_scene *scene)
{
	t_light		*light;

	light = (t_light *)calloc_p(1, sizeof(t_light));
	light->center = parse_coord(ft_strtok(NULL, spaces));
	light->brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	light->color = parse_color(ft_strtok(NULL, spaces));
	scene->n_lights++;
	if (light->brightness == 0)
		ft_putstr_fd("Warning: light brightness set to 0\n", STDERR_FILENO);
	ft_lstadd_front(&scene->lights, ft_lstnew(light));
}

static void	parse_camera(t_scene *scene)
{
	t_camera	*camera;

	camera = (t_camera *)calloc_p(1, sizeof(t_camera));
	camera->center = parse_coord(ft_strtok(NULL, spaces));
	camera->normal = parse_coord(ft_strtok(NULL, spaces));
	camera->fov = clamp(ft_atoui(ft_strtok(NULL, spaces)), 0, 180);
	ft_lstadd_front(&scene->cameras, ft_lstnew(camera));
}

static void	parse_sphere(t_shape *shape)
{
	shape->sphere.center = parse_coord(ft_strtok(NULL, spaces));
	shape->sphere.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	if (shape->sphere.radius <= 0)
		ft_quit(5, "invalid sphere radius");
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
	shape->cylinder.height = ft_atof(ft_strtok(NULL, spaces));
	if (shape->cylinder.radius <= 0)
		ft_quit(5, "invalid cylinder radius");
	if (shape->cylinder.height <= 0)
		ft_quit(5, "invalid cylinder height");
	shape->type = CYLINDER;
}

static void	parse_cone(t_shape *shape)
{
	shape->cone.intersection_point = parse_coord(ft_strtok(NULL, spaces));
	shape->cone.direction = parse_coord(ft_strtok(NULL, spaces));
	shape->cone.height = ft_atof(ft_strtok(NULL, spaces));
	shape->cone.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	if (shape->cone.radius <= 0)
		ft_quit(5, "invalid cone radius");
	if (shape->cone.height <= 0)
		ft_quit(5, "invalid cone height");
	shape->type = CONE;
}

static void	parse_triangle(t_shape *shape)
{
	shape->triangle.vertices[0] = parse_coord(ft_strtok(NULL, spaces));
	shape->triangle.vertices[1] = parse_coord(ft_strtok(NULL, spaces));
	shape->triangle.vertices[2] = parse_coord(ft_strtok(NULL, spaces));
	shape->type = TRIANGLE;
}

static t_float3	parse_coord(char *str)
{
	t_float3	coord;

	if (!str)
		ft_quit(5, "invalid coordinate syntax");
	coord.x = ft_atof(str);
	str = skip_commas(str);
	coord.y = ft_atof(str);
	str = skip_commas(str);
	coord.z = ft_atof(str);
	return (coord);
}

static t_color	parse_color(char *str)
{
	t_color	color;

	if (!str)
		ft_quit(5, "invalid color syntax");
	color.r = ft_atoui(str);
	str = skip_commas(str);
	color.g = ft_atoui(str);
	str = skip_commas(str);
	color.b = ft_atoui(str);
	return (color);
}

static char *skip_commas(char *str)
{
	while (*str && *str != ',')
		str++;
	if (*str == '\0' || is_empty_line(str))
		ft_quit(5, "invalid syntax: missing comma");
	return (str + 1);
}