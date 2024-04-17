/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/17 15:19:16 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

//TODO sperimentare con la keyword restrict
//TODO utilizzare mlx_get_screen_size invece di dimensioni fixed

static void		fill_image(t_thread_data **threads_data, pthread_attr_t *thread_attr);
static void		*render_segment(void *data);
static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y, const t_mlx_data *win_data);
static void		traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void		check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void		update_closest_hit(t_hit *closest_hit, t_shape *shape, const double t, const t_ray ray);
static t_vector	get_cylinder_normal(t_cylinder cylinder, t_point point);
static t_vector	get_cone_normal(t_cone cone, t_point point);
static t_color	add_color_disruption(const t_hit *hit_info);

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	t_thread_data	**threads_data;
	pthread_attr_t	thread_attr;
	t_list			*cameras;
	double			*light_ratios;
	const uint16_t	lines_per_thread = roundf((float)win_data->win_height / (float)N_THREADS);

	cameras = scene->cameras;
	light_ratios = precompute_ratios(ft_lstsize(scene->lights));
	threads_data = set_threads_data(scene, win_data, light_ratios, lines_per_thread, &thread_attr);
	while (cameras)
	{
		printf("\nRendering camera %d\n", win_data->current_img);
		scene->current_camera = cameras->content;
		setup_camera(scene->current_camera, win_data);
		fill_image(threads_data, &thread_attr);
		cameras = cameras->next;
		win_data->current_img++;
	}
	win_data->current_img = win_data->n_images - 1;
	mlx_put_image_to_window(win_data->mlx, win_data->win, win_data->images[win_data->current_img], 0, 0);
	pthread_attr_destroy(&thread_attr);
	free(light_ratios);
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
	t_mlx_data		*win_data;
	uint16_t		y;
	uint16_t		x;
	t_ray			ray;
	t_color			color;
	t_hit			*first_hit;
	t_scene			*scene;
	t_color			bg_color;

	bg_color = (t_color){BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF};
	thread_data = (t_thread_data *)data;
	win_data = thread_data->win_data;
	scene = thread_data->scene;
	y = thread_data->start_y;
	while (y < thread_data->end_y)
	{
		write(1, ".", 1);
		x = 0;
		while (x < win_data->win_width)
		{
			ray = get_ray(scene->current_camera, x, y, win_data);
			first_hit = trace_ray(scene, ray);
			if (first_hit)
			{
				color = add_color_disruption(first_hit);
				color = add_lighting(scene, color, first_hit, thread_data->light_ratios);
			}
			else
				color = bg_color;
			my_mlx_pixel_put(win_data, x, y, color);
			free(first_hit);
			x++;
		}
		y++;
	}
	return (NULL);
}

static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y, const t_mlx_data *win_data)
{	
	const t_vector direction =
	{
		.x = cam->forward.x + (win_data->viewport_x[x] * cam->right_by_half_viewport_width.x) + (win_data->viewport_y[y] * cam->up_by_half_viewport_height.x),
		.y = cam->forward.y + (win_data->viewport_x[x] * cam->right_by_half_viewport_width.y) + (win_data->viewport_y[y] * cam->up_by_half_viewport_height.y),
		.z = cam->forward.z + (win_data->viewport_x[x] * cam->right_by_half_viewport_width.z) + (win_data->viewport_y[y] * cam->up_by_half_viewport_height.z)
	};
	return ((t_ray){cam->center, vec_normalize(direction)});
}

static t_color	add_color_disruption(const t_hit *hit_info)
{
	double		u;
	double		v;
	static const t_color	checkerboard_color1 = {CHECKERBOARD_COLOR1 >> 16 & 0xFF, CHECKERBOARD_COLOR1 >> 8 & 0xFF, CHECKERBOARD_COLOR1 & 0xFF};
	static const t_color	checkerboard_color2 = {CHECKERBOARD_COLOR2 >> 16 & 0xFF, CHECKERBOARD_COLOR2 >> 8 & 0xFF, CHECKERBOARD_COLOR2 & 0xFF};

	if (hit_info->shape->material->is_checkerboard)
	{
		get_uv(hit_info, &u, &v);
		int iu = floor(u * CHECKERBOARD_TILE_DENSITY);
		int iv = floor(v * CHECKERBOARD_TILE_DENSITY);
		if ((iu + iv) % 2 == 0)
			return (checkerboard_color1);
		else
			return (checkerboard_color2);
	}
	return (hit_info->shape->material->color);
}

t_hit	*trace_ray(const t_scene *scene, const t_ray ray)
{
    t_hit		*closest_hit = (t_hit *)calloc_p(1, sizeof(t_hit));

	*closest_hit = (t_hit)
	{
		.distance = FLT_MAX,
		.point = {0, 0, 0},
		.normal = {0, 0, 0},
		.shape = NULL,
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

static inline void	check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	static double (*const	intersect[])(const t_ray, const t_shape *) = {&intersect_ray_sphere, &intersect_ray_cylinder, &intersect_ray_triangle, &intersect_ray_cone, &intersect_ray_plane}; //stesso ordine di enum
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

static void	update_closest_hit(t_hit *closest_hit, t_shape *shape, const double t, const t_ray ray)
{
	closest_hit->distance = t;
	closest_hit->point = ray_point_at_parameter(ray, t);
	closest_hit->shape = shape;
	switch (shape->type)
	{
		case TRIANGLE:
			closest_hit->normal = shape->triangle.normal;
			break;
		case SPHERE:
			closest_hit->normal = vec_normalize(vec_sub(closest_hit->point, shape->sphere.center));
			break;
		case CYLINDER:
			closest_hit->normal = get_cylinder_normal(shape->cylinder, closest_hit->point);
			break;
		case PLANE:
			closest_hit->normal = shape->plane.normal;
			break;
		case CONE:
			closest_hit->normal = get_cone_normal(shape->cone, closest_hit->point);
			break;
	}
}

static inline t_vector	get_cylinder_normal(t_cylinder cylinder, t_point point)
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

static inline t_vector	get_cone_normal(t_cone cone, t_point point)
{
	const t_vector		CO = vec_sub(point, cone.intersection_point);
	const double		cos_alpha = cos(atan(cone.radius / cone.height));
	const double		cos_alpha_squared = cos_alpha * cos_alpha;
	const t_vector		V = vec_normalize(cone.direction);
	const double		V_dot_d = vec_dot(V, CO);
	const double		V_dot_d_squared = V_dot_d * V_dot_d;
	const double		a = V_dot_d_squared - cos_alpha_squared;
	const double 		co_dot_co = vec_dot(CO, CO);
	const double		c = V_dot_d_squared - co_dot_co * cos_alpha_squared;
	const double		b = 2.0 * c;
	const double		two_a = 2.0 * a;
	const double		discriminant = b * b - 2 * two_a * c;
	t_vector			normal;

	if (discriminant < 0)
		return (vec_normalize(vec_sub(point, cone.intersection_point)));
	normal = vec_normalize(vec_sub(point, vec_add(cone.intersection_point, vec_scale(-V_dot_d + sqrt(discriminant) / two_a, V))));
	return (normal);
}
