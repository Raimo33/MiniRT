/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_light.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 14:30:16 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 15:09:07 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static t_color	compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal, const double *light_ratios);
static t_color	get_light_component(t_color color, const double brightness, double distance, const double angle_of_incidence_cosine);

t_color	add_lighting(const t_scene *scene, t_color color, const t_hit *hit_info, const double *light_ratios)
{
	t_color				light_component;
	static const double	reciproca1255 = 1.0f / 255.0f;

	if (!hit_info)
		return (color);
	light_component = compute_lights_contribution(scene, hit_info->point, hit_info->normal, light_ratios);
	light_component = blend_colors(light_component, scene->amblight.ambient, 0.2f); //ratio 80/20 tra luce e ambient
	//TODO la divisione per 255 abbassa troppo la luminosita'
	color.r *= (double)light_component.r * reciproca1255;
	color.g *= (double)light_component.g * reciproca1255;
	color.b *= (double)light_component.b * reciproca1255;
	return (color);
}

static t_color	compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal, const double *light_ratios)
{
	t_color			light_component;
	t_color			light_contribution;
	t_list			*lights;
	t_light			*light;
	t_vector		light_dir;
	double			light_distance;
	t_hit			*hit_info;
	uint16_t		i;
	double			angle_of_incidence_cosine;
	
	light_contribution = (t_color){0, 0, 0};
	light_component = (t_color){0, 0, 0};
	lights = scene->lights;
	surface_point = vec_add(surface_point, vec_scale(EPSILON, surface_normal));
	i = 0;
	while (lights)
	{
		light = (t_light *)lights->content;
		light_dir = vec_normalize(vec_sub(light->center, surface_point));
		hit_info = trace_ray(scene, (t_ray){surface_point, light_dir});
		if (vec_dot(surface_normal, light_dir) > 0 && !hit_info)
		{
			light_distance = vec_length(vec_sub(light->center, surface_point));
			angle_of_incidence_cosine = vec_dot(surface_normal, light_dir);
			light_component = get_light_component(light->color, light->brightness, light_distance, angle_of_incidence_cosine);
		}
		else
			light_component = (t_color){0, 0, 0};
		light_contribution = blend_colors(light_contribution, light_component, light_ratios[i++]);
		free(hit_info);
		lights = lights->next;
	}
	return (light_contribution);
}

static t_color get_light_component(t_color color, const double brightness, double distance, const double angle_of_incidence_cosine)
{
	distance = fmax(distance, EPSILON);
	const double	attenuation = fclamp(1.0f - log(distance) / log(WORLD_SIZE), EPSILON, 1.0f);
	const double	diffusion = fmax(angle_of_incidence_cosine, 0.0f);
	const double	variation = brightness * diffusion * attenuation;

	color.r *= variation;
	color.g *= variation;
	color.b *= variation;
	return (color);
}