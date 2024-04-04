/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/04 18:59:51 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void				*render_segment(void *data);
static void				setup_camera(t_camera *cam);
static t_ray			get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_color			ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce);
static float			compute_ray_weight(t_vector ray_direction, t_vector normal, const t_material *material);
static t_hit			*trace_ray(const t_scene *scene, const t_ray ray);
static t_ray			*get_reflected_rays(const t_ray incoming_ray, const t_vector normal, const t_point point, const t_material *material, uint16_t *n_rays);
static inline t_point	ray_point_at_parameter(const t_ray ray, float t);
static void				check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_vector 		get_cylinder_normal(t_shape *shape, t_point point);
static t_vector			get_random_in_unit_sphere(void);
static t_vector			generate_random_vector_in_hemisphere(const t_vector normal, const float roughness);
static uint16_t			get_ray_count_based_on_roughness(const float roughness);
// inline static float		fclamp(const float value, const float min, const float max);
static float			rand_float(const float min, const float max);
static t_color			merge_colors(t_color *colors, const uint16_t n_colors);

void render(t_mlx_data *mlx_data, t_scene *scene)
{
	t_thread_data	threads_data[N_THREADS];
	uint16_t		i;
	uint16_t		j;
	struct timeval	time;
	uint64_t		seed;

	setup_camera(&scene->camera);
	j = 0;
	while (j++ < N_FRAMES)
	{
		gettimeofday(&time, NULL); //meglio di srand(TIME(NULL)) perche' si cambia ogni microsecondo non ogni secondo
		seed = (time.tv_sec * 1000000) + time.tv_usec;
		srand(seed);
		i = 0;
		while (i < N_THREADS)
		{
			threads_data[i].win_data = mlx_data;
			threads_data[i].scene = scene;
			threads_data[i].start_y = i * (WIN_HEIGHT / N_THREADS);
			if (i == N_THREADS - 1)
				threads_data[i].end_y = WIN_HEIGHT;
			else
				threads_data[i].end_y = (i + 1) * (WIN_HEIGHT / N_THREADS);
			pthread_create(&threads_data[i].id, NULL, &render_segment, &threads_data[i]);
			i++;
		}
		i = 0;
		while (i < N_THREADS)
			pthread_join(threads_data[i++].id, NULL);
		mlx_put_image_to_window(mlx_data->mlx, mlx_data->win, mlx_data->img, 0, 0);
		mlx_destroy_image(mlx_data->mlx, mlx_data->img);
		mlx_data->img = mlx_new_image(mlx_data->mlx, WIN_WIDTH, WIN_HEIGHT);
		mlx_data->addr = mlx_get_data_addr(mlx_data->img, &mlx_data->bits_per_pixel, &mlx_data->line_length, &mlx_data->endian);
	}
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
			ray = get_ray(&thread_data->scene->camera, x, y);
			while (i < RAYS_PER_PIXEL)
				colors[i++] = ray_bouncing(thread_data->scene, ray, 0);
			final_color = merge_colors(colors, RAYS_PER_PIXEL);
			final_color.a = 0;
			final_color.a += transparency_per_frame;
			my_mlx_pixel_put(thread_data->win_data, x, y, rgb_to_hex(final_color));
			x++;
		}
		y++;
	}
	return (NULL);
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

static t_ray	*get_reflected_rays(const t_ray incoming_ray, const t_vector normal, const t_point point, const t_material *material, uint16_t *n_rays)
{
	uint16_t	n = get_ray_count_based_on_roughness(material->roughness);
	t_ray		*reflected_rays = (t_ray *)malloc(sizeof(t_ray) * (n));

	*n_rays = n;
	while (n--)
	{
		t_vector	perturbation = generate_random_vector_in_hemisphere(normal, material->roughness);
		t_vector	scattered_direction = vec_normalize(vec_add(incoming_ray.direction, perturbation));

		if (vec_dot(scattered_direction, normal) < 0) //se il raggio è entrante, inverti la direzione
    		scattered_direction = vec_negate(scattered_direction);
		reflected_rays[n] = (t_ray){point, scattered_direction};
	}
	return (reflected_rays);
}

static uint16_t	get_ray_count_based_on_roughness(const float roughness)
{
    float base = MIN_REFLECTED_RAYS; // Minimum number of rays for the least rough surfaces
    float factor = ROUGHNESS_SCALING_FACTOR; // Scaling factor for maximum roughness

    // Non-linear scaling: as roughness increases, the number of rays grows more quickly
    return (base + pow(roughness, 2) * (factor - base));
}

static t_vector	generate_random_vector_in_hemisphere(const t_vector normal, const float roughness)
{
	const t_vector	in_unit_sphere = get_random_in_unit_sphere();
    // Scale the vector in the unit sphere by roughness, if roughness is 0, stick with the normal
	const t_vector	perturbation = vec_scale(in_unit_sphere, fmax(0.1, roughness));
	const t_vector	adjusted_direction = vec_normalize(vec_add(perturbation, normal));
	
	if (vec_dot(adjusted_direction, normal) < 0.0f)
		return (vec_scale(adjusted_direction, -1.0f));
	return (adjusted_direction);
}

static t_vector	get_random_in_unit_sphere(void)
{
	t_point	p;

	do
	{
		p = (t_point){rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f)};
	} while (vec_dot(p, p) >= 1.0f);
	return (p);
}

static float	rand_float(const float min, const float max)
{
	return (min + (max - min) * rand() / (float)RAND_MAX);
}

static t_color	ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce)
{
	t_hit					*hit_info;
	float					total_weight;
	t_ray					*rays;
	uint16_t				n_rays;
	uint16_t				accumulated_r;
	uint16_t				accumulated_g;
	uint16_t				accumulated_b;
	uint16_t				accumulated_a;
	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF, 0};

	if (n_bounce > MAX_BOUNCE) //non fa nulla ma meglio lasciare il controllo
		return (bg_color);
	hit_info = trace_ray(scene, ray);
	if (!hit_info)
		return (bg_color);
	rays = get_reflected_rays(ray, hit_info->normal, hit_info->point, hit_info->material, &n_rays);
	if (n_rays == 0 || rays == NULL)
		return (bg_color);
	total_weight = 0;
	accumulated_r = hit_info->material->color.r;
	accumulated_g = hit_info->material->color.g;
	accumulated_b = hit_info->material->color.b;
	accumulated_a = hit_info->material->color.a;
	while (n_rays--)
	{
		t_color		ray_color = ray_bouncing(scene, rays[n_rays], n_bounce + 1);
		float		weight = compute_ray_weight(rays[n_rays].direction, hit_info->normal, hit_info->material);

		accumulated_r += ray_color.r * weight;
		accumulated_g += ray_color.g * weight;
		accumulated_b += ray_color.b * weight;
		accumulated_a += ray_color.a * weight;
		total_weight += weight;
	}
	free(hit_info);
	free(rays);
	if (total_weight <= 0)
		return (bg_color);
	return ((t_color)
	{
		.r = accumulated_r / total_weight,
		.g = accumulated_g / total_weight,
		.b = accumulated_b / total_weight,
		.a = accumulated_a / total_weight
	});
}

// Computes the weight of a ray based on the angle of incidence and material reflectivity
static float	compute_ray_weight(t_vector ray_direction, t_vector normal, const t_material *material)
{
	ray_direction = vec_normalize(ray_direction);
	normal = vec_normalize(normal);

	// compute the cosine of the angle of incidence
	float cos_theta = fabs(vec_dot(ray_direction, normal));
	
	// compute French Schlick approximation
	float RO = material->reflectivity;
	float x = 1 - cos_theta;
	float reflectance = RO + (1 - RO) * (x * x * x * x * x);

	return (reflectance);
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
	const t_vector		projection = vec_add(cylinder.center, vec_scale(cylinder.direction, projection_lenght));
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
