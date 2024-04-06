/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/06 18:02:12 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void				*render_segment(void *data);
static void				setup_camera(t_camera *cam);
static t_ray			get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_color			ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce);
static t_hit			*trace_ray(const t_scene *scene, const t_ray ray);
static inline t_point	ray_point_at_parameter(const t_ray ray, float t);
static void				check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_vector 		get_cylinder_normal(t_shape *shape, t_point point);
// inline static float		fclamp(const float value, const float min, const float max);
static t_color			merge_colors(t_color *colors, const uint16_t n_colors);
static void				set_thread_attr(pthread_attr_t *thread_attr);
static t_vector			get_random_in_unit_sphere(void);

//TODO sperimentare con la keyword restrict
//TODO utilizzare mlx_get_screen_size invece di dimensioni fixed

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	t_thread_data	*thread_data;
	uint16_t		i;
	uint16_t		j;
	struct timeval	time;
	uint64_t		seed;
	pthread_attr_t	thread_attr;
	pthread_t		thread_ids[N_THREADS];

	setup_camera(scene->camera);
	set_thread_attr(&thread_attr);
	i = 0;
	j = 0;
	while (j < N_FRAMES)
	{
		printf("Rendering frame %d\n", j);
		gettimeofday(&time, NULL); //meglio di srand(TIME(NULL)) perche' si cambia ogni microsecondo non ogni secondo
		seed = (time.tv_sec * 1000000) + time.tv_usec;
		srand(seed);
		scene->random_bias_vector = get_random_in_unit_sphere();
		while (i < N_THREADS)
		{
			thread_data = (t_thread_data *)malloc(sizeof(t_thread_data));
			thread_data->scene = scene;
			thread_data->win_data = win_data;
			thread_data->frame_no = j;
			thread_data->start_y = i * (WIN_HEIGHT / N_THREADS);
			if (i == N_THREADS - 1)
				thread_data->end_y = WIN_HEIGHT;
			else
				thread_data->end_y = (i + 1) * (WIN_HEIGHT / N_THREADS);
			pthread_create(&thread_ids[i], &thread_attr, &render_segment, thread_data);
			i++;
		}
		while (i)
			pthread_join(thread_ids[--i], NULL);
		mlx_put_image_to_window(win_data->mlx, win_data->win, win_data->frames[j], 0, 0);
		j++;
	}
	pthread_attr_destroy(&thread_attr);
}

static void	set_thread_attr(pthread_attr_t *thread_attr)
{
	pthread_attr_init(thread_attr);
	pthread_attr_setschedpolicy(thread_attr, SCHED_FIFO);
	pthread_attr_setscope(thread_attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(thread_attr, PTHREAD_CREATE_JOINABLE);
}

static inline uint32_t rgb_to_hex(t_color color)
{
	return ((color.a << 24) | (color.r << 16) | (color.g << 8) | color.b);
}

static void		*render_segment(void *data)
{
	t_color					colors[RAYS_PER_PIXEL];
	t_color					final_color;
	uint16_t				x;
	uint16_t				y;
	uint16_t				i;
	t_ray					ray;
	t_thread_data			*thread_data = (t_thread_data *)data;
	static const uint8_t	transparency_per_frame = 255 / N_FRAMES;

	y = thread_data->start_y;
	while (y < thread_data->end_y)
	{
		x = 0;
		while (x < WIN_WIDTH)
		{
			i = 0;
			ray = get_ray(thread_data->scene->camera, x, y);
			while (i < RAYS_PER_PIXEL)
				colors[i++] = ray_bouncing(thread_data->scene, ray, 0);
			final_color = merge_colors(colors, RAYS_PER_PIXEL);
			final_color.a = 0;
			final_color.a += transparency_per_frame;
			my_mlx_pixel_put(thread_data->win_data, x, y, rgb_to_hex(final_color), thread_data->frame_no);
			x++;
		}
		y++;
	}
	return (free(data), NULL);
}

static t_color	merge_colors(t_color *colors, const uint16_t n_colors)
{
	t_color			merged_color = {0, 0, 0, 0};
	uint16_t		i;
	const float		weight = 1.0f / n_colors;

	if (n_colors == 1)
		return (colors[0]);
	i = 0;
	while (i < n_colors)
	{
		merged_color.r += (colors[i].r * weight);
		merged_color.g += (colors[i].g * weight);
		merged_color.b += (colors[i].b * weight);
		merged_color.a += (colors[i].a * weight);
		i++;
	}
	return (merged_color);
}

static void	setup_camera(t_camera *cam)
{
	const t_vector	world_up = {0, 1, 0};
	const float		aspect_ratio = (float)WIN_WIDTH / (float)WIN_HEIGHT;
	const float		rad_fov = cam->fov * M_PI / 180; //fov da grad in radianti

	cam->viewport_height = 2 * tan(rad_fov / 2);
	cam->viewport_width = aspect_ratio * cam->viewport_height;
	//calcolo dei vettori base della camera
	cam->forward = vec_normalize(cam->normal); //la normale punta verso il punto di interesse
	if (is_vec_equal(cam->forward, world_up)) //se la normale è uguale al vettore up del mondo
		cam->right = (t_vector){1, 0, 0};
	else
		cam->right = vec_normalize(vec_cross(world_up, cam->forward)); //trova il terzo vettore perpendicolare ad entrambi
	cam->up = vec_normalize(vec_cross(cam->forward, cam->right)); //trova il terzo vettore perpendicolare ad entrambi
}

static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y)
{
	//centro del viewport: (0, 0)
	//coordinate sul piano del monitor [0, 1]
	const float		screen_viewport_x = x / (float)(WIN_WIDTH - 1);
	const float		screen_viewport_y = y / (float)(WIN_HEIGHT - 1);
	//coordinate sul piano cartesiano vero [-1, 1]
	const float		real_viewport_x = screen_viewport_x * 2 - 1;
	const float		real_viewport_y = 1 - screen_viewport_y * 2;
	//calcolo del vettore che parte dalla camera verso il pixel (sul piano immaginario viewport)
	const t_vector direction = //vettore direzione
	{
		.x = cam->forward.x + (real_viewport_x * cam->right.x * cam->viewport_width / 2) + (real_viewport_y * cam->up.x * cam->viewport_height / 2),
		.y = cam->forward.y + (real_viewport_x * cam->right.y * cam->viewport_width / 2) + (real_viewport_y * cam->up.y * cam->viewport_height / 2),
		.z = cam->forward.z + (real_viewport_x * cam->right.z * cam->viewport_width / 2) + (real_viewport_y * cam->up.z * cam->viewport_height / 2)
	};
	const t_ray	ray_direction = //raggio (vettore direzione spostato per partire dalla camera)
	{
		.origin = cam->center,
		.direction = vec_normalize(direction)
	};
	return (ray_direction);
}

static t_vector	get_random_in_unit_sphere(void)
{
    t_vector	p;
	float		rand_n;

    do {
        // Generate a random point in the unit cube by scaling the random values
        // and then translate them to the range [-1, 1]
		rand_n = (2.0 * ((double)rand() / RAND_MAX) - 1.0);
        p.x = rand_n;
		p.y = rand_n;
		p.z = rand_n;
    // Repeat until we get a point inside the unit sphere
    } while (vec_dot(p, p) >= 1.0);
    return p;
}

static t_ray	get_reflected_ray(const t_ray incoming_ray, const t_vector normal, const t_point point, const t_vector random_component)
{
	t_ray		reflected_ray =
	{
		.origin = point,
		.direction = vec_sub(vec_scale(2 * vec_dot(normal, incoming_ray.direction), normal), incoming_ray.direction)
	};
	float roughness = 0.1;
	reflected_ray.direction = vec_add(reflected_ray.direction, vec_scale(roughness, random_component));
	return (reflected_ray);
}

static inline uint32_t	min(const uint32_t a, const uint32_t b)
{
	return (a < b ? a : b);
}

static t_color	ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce)
{
	t_hit					*hit_info;
	t_color					ray_color;
	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF, 0};
	t_color					hit_color;
	t_color					ambient_light;

	if (n_bounce > MAX_BOUNCE) //non fa nulla ma meglio lasciare il controllo
		return (bg_color);
	hit_info = trace_ray(scene, ray);
	if (!hit_info)
		return (bg_color);
	ray = get_reflected_ray(ray, hit_info->normal, hit_info->point, scene->random_bias_vector);
	ray_color = ray_bouncing(scene, ray, n_bounce + 1);
	hit_color = hit_info->material->color;
	ambient_light = (t_color){
		.r = (uint8_t)(scene->amblight.color.r * scene->amblight.brightness * hit_color.r / 255),
		.g = (uint8_t)(scene->amblight.color.g * scene->amblight.brightness * hit_color.g / 255),
		.b = (uint8_t)(scene->amblight.color.b * scene->amblight.brightness * hit_color.b / 255),
		.a = 255
	};
	ray_color = (t_color){
		.r = (uint8_t)min(255, ambient_light.r + (hit_color.r * ray_color.r / 255)),
		.g = (uint8_t)min(255, ambient_light.g + (hit_color.g * ray_color.g / 255)),
		.b = (uint8_t)min(255, ambient_light.b + (hit_color.b * ray_color.b / 255)),
		.a = 255
	};
	return (free(hit_info), ray_color);
}

static t_hit	*trace_ray(const t_scene *scene, const t_ray ray)
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
		return (NULL);
    return (closest_hit);
}

static void	traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	uint8_t	i;

	// Controllo se il raggio interseca il bounding box del nodo
	if (!node || !ray_intersects_aabb(ray, node->box_top, node->box_bottom))
		return ;
	// Se c'e' solo una shape all'interno o se il nodo è una leaf, controlla le intersezioni con le forme
	if (node->n_shapes == 1 || node->children == NULL)
		check_shapes_in_node(node, ray, closest_hit);
	else
	{
		// Altrimenti, continua a scendere nell'albero
		i = 0;
		while (i < 8)
			traverse_octree(node->children[i++], ray, closest_hit);
	}
}

static void check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	static float (*const intersect[3])(const t_ray, const t_shape *) = //stesso ordine di enum e struct
	{
		&intersect_ray_sphere,
		&intersect_ray_cylinder,
		&intersect_ray_plane
	};
	t_list	*shapes;
	t_shape	*shape;
	float	t;

	shapes = node->shapes;
	while (shapes)
	{
		shape = (t_shape *)shapes->content;
		t = intersect[shape->type](ray, shape);
		if (t > 0 && t < closest_hit->distance)
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
					closest_hit->normal = get_cylinder_normal(shape, closest_hit->point);
					break;
				case PLANE:
					closest_hit->normal = shape->plane.normal;
					break;
			}
		}
		shapes = shapes->next;
	}
}

static t_vector get_cylinder_normal(t_shape *shape, t_point point)
{
	const t_cylinder	cylinder = shape->cylinder;
	const t_vector		vec_from_center_to_point = vec_sub(point, cylinder.center);
	const float			projection_lenght = vec_dot(vec_from_center_to_point, cylinder.direction);
	const t_vector		projection = vec_add(cylinder.center, vec_scale(projection_lenght, cylinder.direction));
	const t_vector		normal = vec_normalize(vec_sub(point, projection));

	return (normal);
}

inline static t_point ray_point_at_parameter(const t_ray ray, float t)
{
    return ((t_point)
	{
		.x = ray.origin.x + t * ray.direction.x,
		.y = ray.origin.y + t * ray.direction.y,
		.z = ray.origin.z + t * ray.direction.z
	});
}

// static uint32_t blend_colors(uint32_t color1, uint32_t color2, float ratio)
// {
// 	uint8_t result_r;
// 	uint8_t result_g;
// 	uint8_t result_b;
	
//     ratio = fclamp(ratio, 0.0f, 1.0f);
// 	result_r = (uint8_t)(((color1 >> 16) & 0xFF) * (1.0f - ratio) + ((color2 >> 16) & 0xFF) * ratio);
// 	result_g = (uint8_t)(((color1 >> 8) & 0xFF) * (1.0f - ratio) + ((color2 >> 8) & 0xFF) * ratio);
// 	result_b = (uint8_t)((color1 & 0xFF) * (1.0f - ratio) + (color2 & 0xFF) * ratio);

//     return ((result_r << 16) | (result_g << 8) | result_b);
// }

// inline static float	fclamp(const float value, const float min, const float max)
// {
// 	return (value < min ? min : (value > max ? max : value));
// }
