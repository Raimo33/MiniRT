/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 16:20:40 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	parse_scene(const int fd, t_scene *scene)
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

bool	is_scene_valid(const t_scene *scene)
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

void	parse_line(char *line, t_scene *scene)
{
	static const char		*prefixes[] = {"A", "L", "C"};
	static void (*const		parse_objects[])(t_scene *)
		= {&parse_amblight, &parse_light, &parse_camera};
	static uint8_t			n_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	uint8_t					i;

	i = 0;
	while (i < n_prefixes)
	{
		if (line[0] == prefixes[i][0])
		{
			ft_strtok(line, g_spaces);
			parse_objects[i](scene);
			return ;
		}
		i++;
	}
	parse_shape(line, scene);
}

void	parse_shape(char *line, t_scene *scene)
{
	static const char		*prefixes[] = {"sp", "cy", "tr", "co", "pl"};
	static void (*const		parse_funcs[])(t_shape *)
		= {&parse_sphere, &parse_cylinder,
		&parse_triangle, &parse_cone, &parse_plane};
	static const uint8_t	n_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	uint8_t					i;
	t_shape					*shape;

	i = 0;
	while (i < n_prefixes)
	{
		if (ft_strncmp(line, prefixes[i], 2) == 0)
		{
			shape = (t_shape *)calloc_p(1, sizeof(t_shape));
			shape->material = (t_material *)calloc_p(1, sizeof(t_material));
			ft_strtok(line, g_spaces);
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

void	parse_material(t_material *material)
{
	char	*tmp;

	material->color = parse_color(ft_strtok(NULL, g_spaces));
	material->shininess = fmax(ft_atof(ft_strtok(NULL, g_spaces)), 0);
	material->specular = fclamp(ft_atof(ft_strtok(NULL, g_spaces)), 0, 1.0f);
	material->diffuse = fclamp(ft_atof(ft_strtok(NULL, g_spaces)), 0, 1.0f);
	tmp = ft_strtok(NULL, g_spaces);
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
