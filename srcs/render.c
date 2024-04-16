/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/16 17:59:18 by craimond         ###   ########.fr       */
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
// static t_ray	get_reflected_ray(const t_ray incoming_ray, const t_hit *hit_info);
// static t_color	ray_bouncing(const t_scene *scene, t_ray ray, t_hit *hit_info, const uint16_t n_bounce, const double *attenuation_factors, const double *light_ratios);

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	t_thread_data	**threads_data;
	pthread_attr_t	thread_attr;
	t_list			*cameras;
	double			*light_ratios;
	double			*attenuation_factors;
	const uint16_t	lines_per_thread = roundf((float)win_data->win_height / (float)N_THREADS);

	cameras = scene->cameras;
	light_ratios = precompute_ratios(ft_lstsize(scene->lights));
	attenuation_factors = precoumpute_attenuation_factors();
	threads_data = set_threads_data(scene, win_data, light_ratios, attenuation_factors, lines_per_thread, &thread_attr);
	while (cameras)
	{
		printf("Rendering camera %d\n", win_data->current_img);
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
		printf("Rendering line %d\n", y);
		x = 0;
		while (x < win_data->win_width)
		{
			ray = get_ray(scene->current_camera, x, y, win_data);
			first_hit = trace_ray(scene, ray);
			// color = ray_bouncing(scene, ray, first_hit, 1, thread_data->attenuation_factors, thread_data->light_ratios);
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

static void get_sphere_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_vector normal = hit_info->normal; // Assuming normalized
    const double phi = atan2(normal.z, normal.x);
    const double theta = asin(normal.y); // Asin is safe if normal is normalized
    *u = 1 - (phi + M_PI) / (2 * M_PI);
    *v = (theta + M_PI / 2) / M_PI;
}

static bool	is_point_on_cylinder_cap(const t_point point, const t_cylinder cylinder, const t_vector dir, const double hit_height)
{
	const t_vector center = (hit_height <= 0) ? cylinder.bottom_cap_center : vec_add(cylinder.bottom_cap_center, vec_scale(2 * cylinder.half_height, dir));
	const t_vector from_center_to_hit = vec_sub(point, center);
	const double distance = vec_length(from_center_to_hit);
	return (distance <= cylinder.radius);
}

static void get_cylinder_uv(const t_hit *hit_info, double *u, double *v)
{
    t_vector dir = vec_normalize(hit_info->shape->cylinder.direction);
    t_vector bottom_center = hit_info->shape->cylinder.bottom_cap_center;
    double height = 2 * hit_info->shape->cylinder.half_height;
    double radius = hit_info->shape->cylinder.radius;
    t_vector local_hit = vec_sub(hit_info->point, bottom_center);
    double hit_height = vec_dot(local_hit, dir);
    bool hit_caps = is_point_on_cylinder_cap(hit_info->point, hit_info->shape->cylinder, dir, hit_height);

    if (hit_caps)
    {
        t_vector center = (hit_height <= 0) ? bottom_center : vec_add(bottom_center, vec_scale(height, dir));
        t_vector from_center_to_hit = vec_sub(hit_info->point, center);
        t_vector arbitrary = (fabs(dir.x) < 0.9) ? (t_vector){1, 0, 0} : (t_vector){0, 1, 0};
        t_vector perp_dir = vec_cross(dir, arbitrary);
        t_vector perp_dir2 = vec_cross(dir, perp_dir);
        double x = vec_dot(from_center_to_hit, perp_dir);
        double z = vec_dot(from_center_to_hit, perp_dir2);

        *u = atan2(z, x) / (2 * M_PI) + 0.5;
        *v = vec_length(from_center_to_hit) / radius;
    }
    else
    {
        t_vector hit_plane = vec_sub(local_hit, vec_scale(hit_height, dir));
        double angle = atan2(hit_plane.z, hit_plane.x);
        *u = (angle + M_PI) / (2 * M_PI);
        *v = hit_height / height;
    }
}

static void get_triangle_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_triangle triangle = hit_info->shape->triangle;
    const t_vector edge1 = vec_sub(triangle.vertices[1], triangle.vertices[0]);
    const t_vector edge2 = vec_sub(triangle.vertices[2], triangle.vertices[0]);
    const t_vector h = vec_cross(hit_info->normal, edge2);
    const double a = vec_dot(edge1, h);

    if (fabs(a) < 1e-8) { // Avoid division by zero
        *u = 0;
        *v = 0;
        return;
    }

    const t_vector s = vec_sub(hit_info->point, triangle.vertices[0]);
    const double b = vec_dot(s, h) / a;
    const t_vector q = vec_cross(s, edge1);
    const double c = vec_dot(hit_info->normal, q) / a;

    *u = b;
    *v = c;
}

static void get_cone_uv(const t_hit *hit_info, double *u, double *v)
{
    const t_vector normal = hit_info->normal; // Assuming normalized
    const double phi = atan2(normal.z, normal.x);
    const double theta = asin(normal.y);
    *u = 1 - (phi + M_PI) / (2 * M_PI);
    *v = (theta + M_PI / 2) / M_PI;
}

static void get_plane_uv(const t_hit *hit_info, double *u, double *v)
{
    t_vector normal = hit_info->normal;  // Assuming normalized
    t_vector perp1, perp2;

    if (fabs(normal.x) > fabs(normal.y))
        perp1 = (t_vector){normal.z, 0, -normal.x};
    else
        perp1 = (t_vector){0, -normal.z, normal.y};

    perp1 = vec_normalize(perp1);
    perp2 = vec_cross(normal, perp1);
    perp2 = vec_normalize(perp2);

    const t_vector hit_point = hit_info->point;
    const double uv_scale_factor_u = 0.1; // Adjust to fit the desired scale of your checkerboard
    const double uv_scale_factor_v = 0.1;
    *u = vec_dot(hit_point, perp1) * uv_scale_factor_u;
    *v = vec_dot(hit_point, perp2) * uv_scale_factor_v;	
}


static void	get_uv(const t_hit *hit_info, double *u, double *v)
{
	void					(* get_uv_funcs[])(const t_hit *, double *, double *) = {&get_sphere_uv, &get_cylinder_uv, &get_triangle_uv, &get_cone_uv, &get_plane_uv}; //stesso ordine di enum
	static const uint8_t	n_shapes = sizeof(get_uv_funcs) / sizeof(get_uv_funcs[0]);
	uint8_t					i;

	i = 0;
	while (i < n_shapes)
	{
		if (hit_info->shape->type == i)
		{
			get_uv_funcs[i](hit_info, u, v);
			break ;
		}
		i++;
	}
}

static t_color	add_color_disruption(const t_hit *hit_info)
{
	double	u;
	double	v;
	static const t_color	color1 = {CHECKERBOARD_COLOR1 >> 16 & 0xFF, CHECKERBOARD_COLOR1 >> 8 & 0xFF, CHECKERBOARD_COLOR1 & 0xFF};
	static const t_color	color2 = {CHECKERBOARD_COLOR2 >> 16 & 0xFF, CHECKERBOARD_COLOR2 >> 8 & 0xFF, CHECKERBOARD_COLOR2 & 0xFF};

	if (hit_info->shape->material->is_checkerboard)
	{
		get_uv(hit_info, &u, &v);
		int iu = (int)(u * CHECKERBOARD_WIDTH);
		int iv = (int)(v * CHECKERBOARD_WIDTH);
		if ((iu + iv) % 2 == 0)
			return (color1);
		else
			return (color2);
	}
	return (hit_info->shape->material->color);
}

t_hit	*trace_ray(const t_scene *scene, const t_ray ray)
{
    t_hit		*closest_hit = (t_hit *)malloc_p(sizeof(t_hit));

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
	const double		V_dot_CO = vec_dot(V, CO);
	const double		a = V_dot_d * V_dot_d - cos_alpha_squared;
	const double		b = 2.0 * (V_dot_d * V_dot_CO - vec_dot(CO, CO) * cos_alpha_squared);
	const double		c = V_dot_CO * V_dot_CO - vec_dot(CO, CO) * cos_alpha_squared;
	const double		discriminant = b * b - 4 * a * c;
	t_vector			normal;

	if (discriminant < 0)
		return (vec_normalize(vec_sub(point, cone.intersection_point)));
	normal = vec_normalize(vec_sub(point, vec_add(cone.intersection_point, vec_scale(-V_dot_d + sqrt(discriminant) / (2 * a), V))));
	return (normal);
}

// static t_ray	get_reflected_ray(const t_ray incoming_ray, const t_hit *hit_info)
// {
// 	t_ray	reflected_ray;

// 	reflected_ray.origin = vec_add(hit_info->point, vec_scale(EPSILON, hit_info->normal));
// 	reflected_ray.direction = vec_normalize(vec_sub(vec_scale(2 * vec_dot(hit_info->normal, incoming_ray.direction), hit_info->normal), incoming_ray.direction));
// 	return (reflected_ray);
// }

// static t_color	ray_bouncing(const t_scene *scene, t_ray ray, t_hit *hit_info, const uint16_t n_bounce, const double *attenuation_factors, const double *light_ratios)
// {
// 	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF};
// 	t_color					ray_color;
// 	t_color					hit_color;
// 	t_hit					*new_hit;

// 	if (n_bounce > MAX_BOUNCE || !hit_info)
// 		return (bg_color);
// 	hit_color = hit_info->material->color;
// 	ray = get_reflected_ray(ray, hit_info);
// 	new_hit = trace_ray(scene, ray);
// 	ray_color = ray_bouncing(scene, ray, new_hit, n_bounce + 1, attenuation_factors, light_ratios);
// 	ray_color.r *= attenuation_factors[n_bounce - 1];
// 	ray_color.g *= attenuation_factors[n_bounce - 1];
// 	ray_color.b *= attenuation_factors[n_bounce - 1];
// 	return(free(new_hit), blend_colors(hit_color, ray_color, 0.5f));
// }