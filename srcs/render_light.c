/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_light.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 14:30:16 by craimond          #+#    #+#             */
/*   Updated: 2024/04/16 16:00:08 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static t_color		compute_lights_contribution(const t_scene *scene, const t_hit *hit_info, const double *light_ratios);
static t_color 		get_light_component(t_color light_color, const double brightness, const double angle_of_incidence_cosine, const t_vector view_dir, const t_material *material, const t_vector perfect_reflection);

t_color	add_lighting(const t_scene *scene, t_color color, const t_hit *hit_info, const double *light_ratios)
{
	t_color				light_component;
	static const double	reciproca1255 = 1.0f / 255.0f;

	light_component = compute_lights_contribution(scene, hit_info, light_ratios);
	light_component = blend_colors(light_component, scene->amblight->ambient, 0.5f);
	color.r *= (double)light_component.r * reciproca1255;
	color.g *= (double)light_component.g * reciproca1255;
	color.b *= (double)light_component.b * reciproca1255;

	return (color);
}

static t_color	compute_lights_contribution(const t_scene *scene, const t_hit *hit_info, const double *light_ratios)
{
	t_color			light_component;
	t_color			light_contribution;
	t_list			*lights;
	t_light			*light;
	t_vector		light_dir;
	t_hit			*tmp;
	uint16_t		i;
	double			angle_of_incidence_cosine;
	t_vector		view_dir;
	t_point			surface_point;
	t_vector		surface_normal;
	double			light_distance;
	double			object_distance;
	t_vector		perfect_light_reflection;
	
	light_contribution = (t_color){0, 0, 0};
	lights = scene->lights;
	surface_normal = hit_info->normal;
	surface_point = vec_add(hit_info->point, vec_scale(EPSILON, surface_normal));
	view_dir = vec_normalize(vec_sub(scene->current_camera->center, surface_point));
	i = 0;
	while (lights)
	{
		light = (t_light *)lights->content;
		light_dir = vec_normalize(vec_sub(light->center, surface_point));
		light_distance = vec_length(vec_sub(light->center, surface_point));
		tmp = trace_ray(scene, (t_ray){surface_point, light_dir});
		if (tmp)
			object_distance = vec_length(vec_sub(tmp->point, surface_point));
		angle_of_incidence_cosine = vec_dot(surface_normal, light_dir);
		perfect_light_reflection = vec_normalize(vec_sub(vec_scale(2 * angle_of_incidence_cosine, surface_normal), light_dir));
		if ((!tmp || object_distance > light_distance ) && angle_of_incidence_cosine > 0)
			light_component = get_light_component(light->color, light->brightness, angle_of_incidence_cosine, view_dir, hit_info->shape->material, perfect_light_reflection);
		else
			light_component = (t_color){0, 0, 0};
		light_contribution = blend_colors(light_contribution, light_component, light_ratios[i++]);
		free(tmp);
		lights = lights->next;
	}
	return (light_contribution);
}

//full phong reflectin model
static t_color get_light_component(t_color light_color, const double brightness, const double angle_of_incidence_cosine, const t_vector view_dir, const t_material *material, const t_vector perfect_light_reflection)
{
	const double dot_product = fmax(vec_dot(perfect_light_reflection, view_dir), 0.0f);
	const double diffuse = material->diffuse * angle_of_incidence_cosine * brightness;
	const double specular = material->specular * pow(dot_product, material->shininess) * brightness;
	const double total_light = fclamp(diffuse + specular, 0.0f, 1.0f);

	light_color.r *= total_light;
	light_color.g *= total_light;
	light_color.b *= total_light;
	return (light_color);
}

