/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_light.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 14:30:16 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 14:48:32 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static t_color	compute_lights_contribution(const t_scene *scene,
					const t_hit *hit_info, double *light_ratios);
static t_color	get_light_component(t_glc glc,
					const t_vector view_dir, const t_material *material,
					const t_vector perf_reflec);

t_color	add_lighting(const t_scene *scene, t_color color,
	const t_hit *hit_info, double *light_ratios)
{
	t_color				light_component;
	static const double	reciproca1255 = 1.0f / 255.0f;

	light_component
		= compute_lights_contribution(scene, hit_info, light_ratios);
	light_component
		= blend_colors(light_component, scene->amblight->ambient, 0.5f);
	color.r *= (double)light_component.r * reciproca1255;
	color.g *= (double)light_component.g * reciproca1255;
	color.b *= (double)light_component.b * reciproca1255;
	return (color);
}

static t_color	compute_light_component(t_list *lights, const t_scene *scene,
	const t_hit *hit_info, const t_point surface_point)
{
	t_color			light_component;
	t_light			*light;
	t_vector		light_dir;
	t_hit			*tmp;
	double			cosine;

	light = (t_light *)lights->content;
	light_dir = vec_normalize(vec_sub(light->center, surface_point));
	tmp = trace_ray(scene, (t_ray){surface_point, light_dir});
	cosine = vec_dot(hit_info->normal, light_dir);
	if ((!tmp || vec_length(vec_sub(tmp->point, surface_point))
			> vec_length(vec_sub(light->center, surface_point)))
		&& cosine > 0)
		light_component = get_light_component((t_glc){light->color,
				light->brightness, cosine},
				vec_normalize(vec_sub(scene->current_camera->center,
						surface_point)), hit_info->shape->material,
				vec_normalize(vec_sub(vec_scale(2 * cosine,
							hit_info->normal), light_dir)));
	else
		light_component = (t_color){0, 0, 0};
	free(tmp);
	return (light_component);
}

static t_color	compute_lights_contribution(const t_scene *scene,
	const t_hit *hit_info, double *light_ratios)
{
	t_color			light_component;
	t_color			light_contribution;
	t_list			*lights;
	t_vector		view_dir;
	t_point			surface_point;

	light_contribution = (t_color){0, 0, 0};
	lights = scene->lights;
	surface_point = vec_add(hit_info->point,
			vec_scale(EPSILON, hit_info->normal));
	view_dir = vec_normalize(vec_sub(scene->current_camera->center,
				surface_point));
	while (lights)
	{
		light_component = compute_light_component(lights, scene,
				hit_info, surface_point);
		light_contribution = blend_colors(light_contribution,
				light_component, *(light_ratios++));
		lights = lights->next;
	}
	return (light_contribution);
}

static t_color	get_light_component(t_glc glc,
	const t_vector view_dir, const t_material *material,
	const t_vector perf_reflec)
{
	const double	dot_product = fmax(vec_dot(perf_reflec, view_dir), 0.0f);
	const double	diffuse = material->diffuse * glc.cosine * glc.brightness;
	const double	specular = material->specular
		* pow(dot_product, material->shininess) * glc.brightness;
	const double	total_light = fclamp(diffuse + specular, 0.0f, 1.0f);

	glc.light_color.r *= total_light;
	glc.light_color.g *= total_light;
	glc.light_color.b *= total_light;
	return (glc.light_color);
}
