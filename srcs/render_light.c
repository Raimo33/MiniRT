/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_light.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 14:30:16 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 22:58:43 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static t_color		compute_lights_contribution(const t_scene *scene, const t_hit *hit_info, const double *light_ratios);
static t_color		get_light_component(t_color color, const double brightness, double distance, const double angle_of_incidence_cosine, const t_vector view_dir, const t_vector light_dir, const t_vector surface_normal, const double shininess);
static double		calculate_specular(const t_vector view_dir, const t_vector light_dir, const t_vector surface_normal, const double shininess);
static bool			check_shapes_in_node(const t_octree *node, const t_ray ray);
static bool			ray_hits_object(const t_ray ray, const t_octree *octree);

t_color	add_lighting(const t_scene *scene, t_color color, const t_hit *hit_info, const double *light_ratios)
{
	t_color				light_component;
	static const double	reciproca1255 = 1.0f / 255.0f;

	if (!hit_info)
		return (color);
	light_component = compute_lights_contribution(scene, hit_info, light_ratios);
	light_component = blend_colors(light_component, scene->amblight.ambient, 0.2f); //ratio 80/20 tra luce e ambient
	//TODO la divisione per 255 abbassa troppo la luminosita'
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
	double			light_distance;
	bool			is_object_blocking_light;
	uint16_t		i;
	double			angle_of_incidence_cosine;
	t_vector		view_dir;
	t_point			surface_point;
	t_vector		surface_normal;
	
	light_contribution = (t_color){0, 0, 0};
	light_component = (t_color){0, 0, 0};
	lights = scene->lights;
	surface_normal = hit_info->normal;
	surface_point = vec_add(hit_info->point, vec_scale(EPSILON, surface_normal));
	view_dir = vec_normalize(vec_sub(scene->current_camera->center, surface_point));
	i = 0;
	while (lights)
	{
		light = (t_light *)lights->content;
		light_dir = vec_normalize(vec_sub(light->center, surface_point));
		is_object_blocking_light = ray_hits_object((t_ray){surface_point, light_dir}, scene->octree);
		if (!is_object_blocking_light && vec_dot(surface_normal, light_dir) > 0)
		{
			light_distance = vec_length(vec_sub(light->center, surface_point));
			angle_of_incidence_cosine = vec_dot(surface_normal, light_dir);
			light_component = get_light_component(light->color, light->brightness, light_distance, angle_of_incidence_cosine, view_dir, light_dir, surface_normal, hit_info->material->shininess);
		}
		else
			light_component = (t_color){0, 0, 0};
		light_contribution = blend_colors(light_contribution, light_component, light_ratios[i++]);
		lights = lights->next;
	}
	return (light_contribution);
}

static inline bool	ray_hits_object(const t_ray ray, const t_octree *octree)
{
	uint8_t	i;

	if (!octree || !ray_intersects_aabb(ray, octree->box_top, octree->box_bottom))
		return (false);
	if (octree->n_shapes == 1 || octree->children == NULL)
		return (check_shapes_in_node(octree, ray));
	else
	{
		i = 0;
		while (i < 8)
		{
			if (ray_hits_object(ray, octree->children[i++]))
				return (true);	
		}
		return (false);
	}
}

static inline bool	check_shapes_in_node(const t_octree *node, const t_ray ray)
{
	static double (*const	intersect[])(const t_ray, const t_shape *) = {&intersect_ray_sphere, &intersect_ray_cylinder, &intersect_ray_triangle ,&intersect_ray_plane}; //stesso ordine di enum
	t_list	*shapes;
	t_shape	*shape;
	double	t;

	shapes = node->shapes;
	while (shapes)
	{
		shape = (t_shape *)shapes->content;
		t = intersect[shape->type](ray, shape);
		if (t > 0)
			return (true);
		shapes = shapes->next;
	}
	return (false);
}

static t_color get_light_component(t_color color, const double brightness, double distance, const double angle_of_incidence_cosine, const t_vector view_dir, const t_vector light_dir, const t_vector surface_normal, const double shininess)
{
	distance = fmax(distance, EPSILON);
	const double	attenuation = fclamp(1.0f - log(distance) / log(WORLD_SIZE), EPSILON, 1.0f);
	const double	diffuse = fmax(angle_of_incidence_cosine, 0.0f); //angolo di incidenza raggio di luce rispettoa  normale della superficie
	const double	specular = calculate_specular(view_dir, light_dir, surface_normal, shininess); 	//in base alla direzione rispetto alla camera
	const double	variation = brightness * (diffuse + specular) * attenuation;

	color.r *= variation;
	color.g *= variation;
	color.b *= variation;
	return (color);
}

static double	calculate_specular(const t_vector view_dir, const t_vector light_dir, const t_vector surface_normal, const double shininess)
{
	t_vector		reflected_light_dir;
	const double	dot_nl_2 = 2.0f * fmax(vec_dot(surface_normal, light_dir), 0.0f);

	reflected_light_dir.x = dot_nl_2 * surface_normal.x - light_dir.x;
	reflected_light_dir.y = dot_nl_2 * surface_normal.y - light_dir.y;
	reflected_light_dir.z = dot_nl_2 * surface_normal.z - light_dir.z;

	reflected_light_dir = vec_normalize(reflected_light_dir);
	const double spec_angle = fmax(vec_dot(reflected_light_dir, view_dir), 0.0f);
	return (pow(spec_angle, shininess));
}