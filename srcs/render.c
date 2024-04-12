/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 15:22:43 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

//TODO implementare un blending tra i pixel vicini per velocizzare il rendering e aumentare la smoothness
//TODO sperimentare con la keyword restrict
//TODO utilizzare mlx_get_screen_size invece di dimensioni fixed

static void		fill_image(t_thread_data **threads_data, pthread_attr_t *thread_attr);
static void		*render_segment(void *data);
static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static void		traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void		check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void		update_closest_hit(t_hit *closest_hit, const t_shape *shape, const double t, const t_ray ray);
static t_vector	get_cylinder_normal(t_cylinder cylinder, t_point point);
static t_ray	get_reflected_ray(const t_ray incoming_ray, const t_vector normal, const t_point point);
static t_color	ray_bouncing(const t_scene *scene, t_ray ray, t_hit *hit_info, const uint16_t n_bounce, const double *attenuation_factors, const double *light_ratios);

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	t_thread_data	**threads_data;
	pthread_attr_t	thread_attr;
	t_list			*cameras;
	double			*light_ratios;
	double			*attenuation_factors;
	const uint16_t	lines_per_thread = roundf((float)WIN_HEIGHT / (float)N_THREADS);

	cameras = scene->cameras;
	light_ratios = precompute_ratios(ft_lstsize(scene->lights));
	attenuation_factors = precoumpute_attenuation_factors();
	threads_data = set_threads_data(scene, win_data, light_ratios, attenuation_factors, lines_per_thread, &thread_attr);
	srand(time(NULL));
	while (cameras)
	{
		scene->current_camera = cameras->content;
		setup_camera(scene->current_camera);
		fill_image(threads_data, &thread_attr);
		cameras = cameras->next;
		win_data->current_img++;
	}
	mlx_put_image_to_window(win_data->mlx, win_data->win, win_data->images[win_data->current_img - 1], 0, 0);
	pthread_attr_destroy(&thread_attr);
	free(light_ratios);
	free(attenuation_factors);
	ft_freematrix((void **)threads_data);
}

static void	fill_image(t_thread_data **threads_data, pthread_attr_t *thread_attr)
{
	pthread_t 	thread_ids[N_THREADS];
	uint16_t	i;

	i = 0;
	while (i < N_THREADS)
	{
		pthread_create(&thread_ids[i], thread_attr, render_segment, threads_data[i]);
		i++;
	}
	while (i > 0)
		pthread_join(thread_ids[--i], NULL);
}

static void	*render_segment(void *data)
{
	t_thread_data	*thread_data;
	uint16_t		y;
	uint16_t		x;
	t_ray			ray;
	t_color			color;
	t_hit			*first_hit;
	t_scene			*scene;

	thread_data = (t_thread_data *)data;
	scene = thread_data->scene;
	y = thread_data->start_y;
	while (y < thread_data->end_y)
	{
		x = 0;
		while (x < WIN_WIDTH)
		{
			ray = get_ray(scene->current_camera, x, y);
			first_hit = trace_ray(scene, ray);
			color = ray_bouncing(scene, ray, first_hit, 1, thread_data->attenuation_factors, thread_data->light_ratios);
			color = add_lighting(scene, color, first_hit, thread_data->light_ratios);
			free(first_hit);
			my_mlx_pixel_put(thread_data->win_data, x, y, color);
			x++;
		}
		y++;
	}
	return (NULL);
}

static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y)
{
	const double		screen_viewport_x = x / (double)(WIN_WIDTH - 1);
	const double		screen_viewport_y = y / (double)(WIN_HEIGHT - 1);
	const double		real_viewport_x = screen_viewport_x * 2 - 1;
	const double		real_viewport_y = 1 - screen_viewport_y * 2;
	const double		half_viewport_width = cam->viewport_width / 2;
	const double		half_viewport_height = cam->viewport_height / 2;
	
	const t_vector direction =
	{
		.x = cam->forward.x + (real_viewport_x * cam->right.x * half_viewport_width) + (real_viewport_y * cam->up.x * half_viewport_height),
		.y = cam->forward.y + (real_viewport_x * cam->right.y * half_viewport_width) + (real_viewport_y * cam->up.y * half_viewport_height),
		.z = cam->forward.z + (real_viewport_x * cam->right.z * half_viewport_width) + (real_viewport_y * cam->up.z * half_viewport_height)
	};
	return ((t_ray){cam->center, vec_normalize(direction)});
}

t_hit	*trace_ray(const t_scene *scene, const t_ray ray)
{
    t_hit		*closest_hit = (t_hit *)malloc(sizeof(t_hit));

	*closest_hit = (t_hit)
	{
		.distance = FLT_MAX,
		.point = {0, 0, 0},
		.normal = {0, 0, 0},
		.material = NULL
	};
    traverse_octree(scene->octree, ray, closest_hit);
	if (closest_hit->distance == FLT_MAX)
		return (free(closest_hit), NULL);
    return (closest_hit);
}

static void	traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	uint8_t	i;

	if (!node || !ray_intersects_aabb(ray, node->box_top, node->box_bottom))
		return ;
	if (node->n_shapes == 1 || node->children == NULL)
		check_shapes_in_node(node, ray, closest_hit);
	else
	{
		i = 0;
		while (i < 8)
			traverse_octree(node->children[i++], ray, closest_hit);
	}
}

static void	check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	static double (*const	intersect[3])(const t_ray, const t_shape *) = {&intersect_ray_sphere, &intersect_ray_cylinder, &intersect_ray_plane}; //stesso ordine di enum
	t_list	*shapes;
	t_shape	*shape;
	double	t;

	shapes = node->shapes;
	while (shapes)
	{
		shape = (t_shape *)shapes->content;
		t = intersect[shape->type](ray, shape);
		if (t > 0 && t < closest_hit->distance)
			update_closest_hit(closest_hit, shape, t, ray);
		shapes = shapes->next;
	}
}

static void	update_closest_hit(t_hit *closest_hit, const t_shape *shape, const double t, const t_ray ray)
{
	closest_hit->distance = t;
	closest_hit->point = ray_point_at_parameter(ray, t);
	closest_hit->material = shape->material;
	switch (shape->type)
	{
		case SPHERE:
			closest_hit->normal = vec_normalize(vec_sub(closest_hit->point, shape->sphere.center));
			break;
		case CYLINDER:
			closest_hit->normal = get_cylinder_normal(shape->cylinder, closest_hit->point);
			break;
		case PLANE:
			closest_hit->normal = shape->plane.normal;
			break;
	}
}

static t_vector get_cylinder_normal(t_cylinder cylinder, t_point point)
{
	const t_vector		vec_from_center_to_point = vec_sub(point, cylinder.center);
	const double		projection_length = vec_dot(vec_from_center_to_point, cylinder.direction);
	t_vector			projection;

	if (fabs(projection_length  + cylinder.half_height) < EPSILON)
		return (vec_negate(cylinder.direction));
	if (fabs(projection_length - cylinder.half_height) < EPSILON)
		return (cylinder.direction);
	projection = vec_add(cylinder.center, vec_scale(projection_length, cylinder.direction));
	return(vec_normalize(vec_sub(point, projection)));
}

static t_ray	get_reflected_ray(const t_ray incoming_ray, const t_vector normal, const t_point point)
{
	t_ray				reflected_ray;
	t_vector			random_component;
	static const double	roughness = 0.1f;

	random_component = get_rand_in_unit_sphere();
	reflected_ray.origin = vec_add(point, vec_scale(EPSILON, normal));
	reflected_ray.direction = vec_normalize(vec_sub(vec_scale(2 * vec_dot(normal, incoming_ray.direction), normal), incoming_ray.direction));
	reflected_ray.direction = vec_normalize(vec_add(reflected_ray.direction, vec_scale(roughness, random_component)));
	return (reflected_ray);
}

static t_color	ray_bouncing(const t_scene *scene, t_ray ray, t_hit *hit_info, const uint16_t n_bounce, const double *attenuation_factors, const double *light_ratios)
{
	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF};
	t_color					ray_color;
	t_color					hit_color;
	t_hit					*new_hit;

	if (n_bounce > MAX_BOUNCE || !hit_info)
		return (bg_color);
	hit_color = hit_info->material->color;
	ray = get_reflected_ray(ray, hit_info->normal, hit_info->point);
	new_hit = trace_ray(scene, ray);
	ray_color = ray_bouncing(scene, ray, new_hit, n_bounce + 1, attenuation_factors, light_ratios);
	ray_color.r *= attenuation_factors[n_bounce - 1];
	ray_color.g *= attenuation_factors[n_bounce - 1];
	ray_color.b *= attenuation_factors[n_bounce - 1];
	return(free(new_hit), blend_colors(hit_color, ray_color, 0.5f));
}