/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/21 17:59:08 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		parse_line(char *line, t_scene *scene);
static void		parse_amblight(t_scene *scene);
static void		parse_light(t_scene *scene);
static void		parse_camera(t_scene *scene);
static void		parse_shape(char *line, t_scene *scene);
static void		parse_material(t_material *material);
static void		parse_texture(const char *str, t_material *material);
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
		ft_putstr_fd("Invalid scene: no cameras found\n", STDERR_FILENO);
	if (!scene->shapes)
		ft_putstr_fd("Invalid scene: no shapes found\n", STDERR_FILENO);
	if (!scene->lights)
		ft_putstr_fd("Warning: no directional lights found\n", STDERR_FILENO);
	if (!scene->amblight)
		ft_putstr_fd("Invalid scene: no ambient light found\n", STDERR_FILENO);
	return (scene->cameras && scene->shapes && scene->amblight);
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
	free(line);
	ft_quit(4, "invalid shape prefix");
}

static void	parse_material(t_material *material)
{
	char	*tmp;
	
	material->color = parse_color(ft_strtok(NULL, spaces));
	material->shininess = fmax(ft_atof(ft_strtok(NULL, spaces)), 0);
	material->specular = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1.0f);
	material->diffuse = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1.0f);
	tmp = ft_strtok(NULL, spaces);
	material->is_checkerboard = (ft_strncmp(tmp, "checkerboard", 12) == 0);
	material->texture = NULL;
	if (!material->is_checkerboard)
		parse_texture(tmp, material);
	if (material->shininess == 0)
		ft_putstr_fd("Warning: material shininess set to 0\n", STDERR_FILENO);
	if (material->specular == 0)
		ft_putstr_fd("Warning: material specular set to 0\n", STDERR_FILENO);
	if (material->diffuse == 0)
		ft_putstr_fd("Warning: material diffuse set to 0\n", STDERR_FILENO);
}

static void	parse_texture(const char *str, t_material *material)
{
	char		*texture_path;
	uint16_t	len;

	if (!str)
		return ;
	material->texture = (t_texture_data *)calloc_p(1, sizeof(t_texture_data));
	len = ft_strlen(TEXTURE_ROOT) + ft_strlen(str) + ft_strlen(".xpm") + 1;
	texture_path = (char *)calloc_p(len, sizeof(char));
	ft_strlcpy(texture_path, TEXTURE_ROOT, len);
	ft_strlcat(texture_path, str, len);
	ft_strlcat(texture_path, ".xpm", len);
	material->texture->path = texture_path;
}

static void	parse_amblight(t_scene *scene)
{
	t_amblight	amblight;

	amblight.brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	if (amblight.brightness == 0)
		ft_putstr_fd("Warning: ambient light brightness set to 0\n", STDERR_FILENO);
	amblight.color = parse_color(ft_strtok(NULL, spaces));
	amblight.ambient.r = amblight.color.r * amblight.brightness;
	amblight.ambient.g = amblight.color.g * amblight.brightness;
	amblight.ambient.b = amblight.color.b * amblight.brightness;
	scene->amblight = (t_amblight *)malloc_p(sizeof(t_amblight));
	*scene->amblight = amblight;
}

static void	parse_light(t_scene *scene)
{
	t_light		light;
	t_light		*light_ptr;
	
	light.center = parse_coord(ft_strtok(NULL, spaces));
	light.brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	if (light.brightness == 0)
		ft_putstr_fd("Warning: light brightness set to 0\n", STDERR_FILENO);
	light.color = parse_color(ft_strtok(NULL, spaces));
	scene->n_lights++;
	light_ptr = (t_light *)malloc_p(sizeof(t_light));
	*light_ptr = light;
	ft_lstadd_front(&scene->lights, ft_lstnew(light_ptr));
}

static void	parse_camera(t_scene *scene)
{
	t_camera	camera;

	t_camera	*camera_ptr;
	
	camera.center = parse_coord(ft_strtok(NULL, spaces));
	camera.normal = parse_coord(ft_strtok(NULL, spaces));
	camera.fov = clamp(ft_atoui(ft_strtok(NULL, spaces)), 1, 180);
	if (camera.fov == 1 || camera.fov == 180)
		ft_putstr_fd("Warning: camera FOV set to 1 or 180\n", STDERR_FILENO);
	camera_ptr = (t_camera *)malloc_p(sizeof(t_camera));
	*camera_ptr = camera;
	ft_lstadd_front(&scene->cameras, ft_lstnew(camera_ptr));
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
	shape->cone.base_center = parse_coord(ft_strtok(NULL, spaces));
	shape->cone.direction = parse_coord(ft_strtok(NULL, spaces));
	shape->cone.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	shape->cone.height = ft_atof(ft_strtok(NULL, spaces));
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
	shape->triangle.u[0] = 0;
	shape->triangle.v[0] = 0;
	shape->triangle.u[1] = 1;
	shape->triangle.v[1] = 0;
	shape->triangle.u[2] = 0;
	shape->triangle.v[2] = 1;
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
	if (*str == '\0')
		ft_quit(5, "invalid syntax: missing comma");
	return (str + 1);
}