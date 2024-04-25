/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 17:19:07 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	parse_texture(const char *str, t_material *material)
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

void	parse_amblight(t_scene *scene)
{
	t_amblight	amblight;

	amblight.brightness = fclamp(ft_atof(ft_strtok(NULL, spaces)), 0, 1);
	if (amblight.brightness == 0)
		ft_putstr_fd("Warning: ambient light brightness set to 0\n",
			STDERR_FILENO);
	amblight.color = parse_color(ft_strtok(NULL, spaces));
	amblight.ambient.r = amblight.color.r * amblight.brightness;
	amblight.ambient.g = amblight.color.g * amblight.brightness;
	amblight.ambient.b = amblight.color.b * amblight.brightness;
	scene->amblight = (t_amblight *)malloc_p(sizeof(t_amblight));
	*scene->amblight = amblight;
}

void	parse_light(t_scene *scene)
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

void	parse_camera(t_scene *scene)
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

void	parse_sphere(t_shape *shape)
{
	shape->sphere.center = parse_coord(ft_strtok(NULL, spaces));
	shape->sphere.radius = ft_atof(ft_strtok(NULL, spaces)) / 2.0f;
	if (shape->sphere.radius <= 0)
		ft_quit(5, "invalid sphere radius");
	shape->e_type = SPHERE;
}
