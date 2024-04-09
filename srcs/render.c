/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/10 00:10:17 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void				setup_camera(t_camera *cam);
static t_ray			get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_color			ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce, const uint16_t idx);
static t_hit			*trace_ray(const t_scene *scene, const t_ray ray);
static inline t_point	ray_point_at_parameter(const t_ray ray, float t);
static void				check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_vector		 	get_cylinder_normal(t_cylinder cylinder, t_point point);
static t_color 			blend_colors(const t_color color1, const t_color color2, const float ratio);
static void				fill_image(t_mlx_data *win_data, t_scene *scene);
static float 			*get_ratios(uint16_t n_elems);
static t_color			merge_colors(t_color *colors, const uint16_t n_colors, const float *ratios);
static void				set_thread_attr(pthread_attr_t *thread_attr);
static t_vector			get_rand_in_unit_sphere(void);
static void				*render_pixel(void *data);
static t_thread_data	*set_thread_data(t_scene *scene, t_ray ray, t_color *colors_array, const uint16_t i, const uint16_t depth_per_thread);
static void				update_closest_hit(t_hit *closest_hit, const t_shape *shape, const float t, const t_ray ray);
static t_color			compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal);

//TODO implementare un blending tra i pixel vicini per velocizzare il rendering e aumentare la smoothness
//TODO sperimentare con la keyword restrict
//TODO utilizzare mlx_get_screen_size invece di dimensioni fixed
//TODO con le normali negative non funziona ne i cilindri ne i piani

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	pthread_attr_t	thread_attr;
	uint16_t		i;

	setup_camera(scene->camera);
	set_thread_attr(&thread_attr);
	srand(time(NULL));
	i = 0;
	while (i < RAYS_PER_PIXEL)
		scene->random_bias_vectors[i++] = get_rand_in_unit_sphere();
	fill_image(win_data, scene);
	mlx_put_image_to_window(win_data->mlx, win_data->win, win_data->img, 0, 0);
	pthread_attr_destroy(&thread_attr);
}

static void	set_thread_attr(pthread_attr_t *thread_attr)
{
	pthread_attr_init(thread_attr);
	pthread_attr_setschedpolicy(thread_attr, SCHED_FIFO);
	pthread_attr_setscope(thread_attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(thread_attr, PTHREAD_CREATE_JOINABLE);
}

static void	fill_image(t_mlx_data *win_data, t_scene *scene)
{
	t_color					colors[RAYS_PER_PIXEL];
	float					*color_ratios;
	uint16_t				x;
	uint16_t				y;
	uint16_t				i;
	t_ray					ray;
	pthread_t 				thread_ids[N_THREADS];
	t_color					final_color;
	t_thread_data			*thread_data;
	float					depth_per_thread;

	depth_per_thread = roundf((float)RAYS_PER_PIXEL / (float)N_THREADS);
	color_ratios = get_ratios(RAYS_PER_PIXEL);
	i = 0;
	y = 0;
	while (y < WIN_HEIGHT)
	{
		printf("Y: %d\n", y);
		x = 0;
		while (x < WIN_WIDTH)
		{
			ray = get_ray(scene->camera, x, y);
			while (i < N_THREADS)
			{
				thread_data = set_thread_data(scene, ray, colors, i, depth_per_thread);
				pthread_create(&thread_ids[i++], NULL, render_pixel, thread_data);
			}
			while (i)
				pthread_join(thread_ids[--i], NULL);
			final_color = merge_colors(colors, RAYS_PER_PIXEL, color_ratios);
			my_mlx_pixel_put(win_data, x, y, final_color);
			x++;
		}
		y++;
	}
	free(color_ratios);
}

static t_thread_data	*set_thread_data(t_scene *scene, t_ray ray, t_color *colors_array, const uint16_t i, const uint16_t depth_per_thread)
{
	t_thread_data	*thread_data;

	thread_data = (t_thread_data *)malloc(sizeof(t_thread_data));
	thread_data->scene = scene;
	thread_data->ray = ray;
	thread_data->colors_array = colors_array;
	thread_data->start_depth = i * depth_per_thread;
	if (i == N_THREADS - 1)
		thread_data->end_depth = RAYS_PER_PIXEL;
	else
		thread_data->end_depth = (i + 1) * depth_per_thread;
	return (thread_data);
}

static void	*render_pixel(void *data)
{
	t_thread_data	*thread_data = (t_thread_data *)data;
	t_color			*colors_array = thread_data->colors_array;
	uint16_t		i;

	i = thread_data->start_depth;
	while (i < thread_data->end_depth)
	{
		colors_array[i] = ray_bouncing(thread_data->scene, thread_data->ray, 0, i);
		i++;
	}
	return (free(data), NULL);
}

static float *get_ratios(uint16_t n_elems)
{
	uint16_t	i;
	float 		*ratios;
	
	ratios = (float *)malloc(n_elems * sizeof(float));
	i = 0;
	while (i < n_elems)
	{
		ratios[i] = 1.0f / (i + 2);
		i++;
	}
	return (ratios);
}

static t_color	merge_colors(t_color *colors, const uint16_t n_colors, const float *ratios)
{
	t_color			merged_color;
	uint16_t		i;

	merged_color = colors[0];
	i = 0;
	while (++i < n_colors)
		merged_color = blend_colors(merged_color, colors[i], ratios[i]);
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

	const float		half_viewport_width = cam->viewport_width / 2;
	const float		half_viewport_height = cam->viewport_height / 2;
	
	//calcolo del vettore che parte dalla camera verso il pixel (sul piano immaginario viewport)
	const t_vector direction = //vettore direzione
	{
		.x = cam->forward.x + (real_viewport_x * cam->right.x * half_viewport_width) + (real_viewport_y * cam->up.x * half_viewport_height),
		.y = cam->forward.y + (real_viewport_x * cam->right.y * half_viewport_width) + (real_viewport_y * cam->up.y * half_viewport_height),
		.z = cam->forward.z + (real_viewport_x * cam->right.z * half_viewport_width) + (real_viewport_y * cam->up.z * half_viewport_height)
	};
	return ((t_ray){cam->center, vec_normalize(direction)});
}

static t_vector get_rand_in_unit_sphere(void) //metodo di Marsaglia's
{
    double		u;
	double		v;
	double		s;
    t_vector	p;

    do {
        u = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        v = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        s = u*u + v*v;
    } while (s >= 1 || s == 0);

    double multiplier = 2 * sqrt(1 - s);
    p.x = u * multiplier;
    p.y = v * multiplier;
    p.z = 1 - 2 * s;
    return (p);
}

static t_ray	get_reflected_ray(const t_ray incoming_ray, const t_vector normal, const t_point point, const t_vector random_component)
{
	t_ray		reflected_ray =
	{
		.origin = vec_add(point, vec_scale(EPSILON, normal)),
		.direction = vec_sub(vec_scale(2 * vec_dot(normal, incoming_ray.direction), normal), incoming_ray.direction)
	};
	float roughness = 0.1; //TODO implementare la roughness prendendo quella degli oggetti
	reflected_ray.direction = vec_add(reflected_ray.direction, vec_scale(roughness, random_component));
	return (reflected_ray);
}

static inline uint32_t	min(const uint32_t a, const uint32_t b)
{
	return (a < b ? a : b);
}

static t_color	ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce, const uint16_t idx)
{
	t_hit					*hit_info;
	t_color					ray_color;
	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF, 0};
	t_color					hit_color;
	t_color					light_component;

	if (n_bounce > MAX_BOUNCE) //non fa nulla ma meglio lasciare il controllo
		return (bg_color);
	hit_info = trace_ray(scene, ray);
	if (!hit_info)
		return (bg_color);
	ray = get_reflected_ray(ray, hit_info->normal, hit_info->point, scene->random_bias_vectors[idx]);
	ray_color = ray_bouncing(scene, ray, n_bounce + 1, idx);
	hit_color = hit_info->material->color;
	light_component = compute_lights_contribution(scene, hit_info->point, hit_info->normal);
	light_component = blend_colors(light_component, scene->amblight.ambient, 0.2f); //ratio 80/20 tra luce e ambient
	ray_color = (t_color){
        .r = (uint8_t)min(255, (int)(hit_color.r * light_component.r / 255 + ray_color.r)),
        .g = (uint8_t)min(255, (int)(hit_color.g * light_component.g / 255 + ray_color.g)),
        .b = (uint8_t)min(255, (int)(hit_color.b * light_component.b / 255 + ray_color.b)),
        .a = 255
    };
	return (free(hit_info), ray_color);
}

static t_color weigh_color(const t_color color, float brightness, float distance)
{
	distance = fmax(distance, EPSILON); //per evitare divisioni per zero
	const float attenuation = 1.0f / (1.0f + log1p(distance - 1));
	const float adjusted_brightness = fclamp(brightness * attenuation, 0.0f, 1.0f);

	return ((t_color)
	{
		.r = (uint8_t)(color.r * adjusted_brightness),
		.g = (uint8_t)(color.g * adjusted_brightness),
		.b = (uint8_t)(color.b * adjusted_brightness),
		.a = 0
	});
}

//TODO la somma di luci non funge
static t_color	compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal)
{
	t_color			*light_components;
	t_color			light_contribution = {0, 0, 0, 0};
	t_list			*lights;
	t_light			*light;
	t_vector		light_dir;
	float			light_distance;
	t_hit			*hit_info;
	uint16_t		i;
	float			*light_ratios;
	const uint16_t	n_lights = ft_lstsize(scene->lights);
	
	light_ratios = get_ratios(n_lights);
	light_components = (t_color *)malloc(n_lights * sizeof(t_color));
	lights = scene->lights;
	surface_point = vec_add(surface_point, vec_scale(EPSILON, surface_normal));
	i = 0;
	while (lights)
	{
		light = (t_light *)lights->content;
		light_dir = vec_normalize(vec_sub(light->center, surface_point));
		hit_info = trace_ray(scene, (t_ray){surface_point, light_dir});
		//se la luce non e' dietro l'oggetto e se tra l'oggetto e la luce c'e' un altro oggetto
		if (vec_dot(surface_normal, light_dir) > 0 && !hit_info)
		{
			light_distance = vec_length(vec_sub(light->center, surface_point));
			light_components[i++] = weigh_color(light->color, light->brightness, light_distance);
		}
		else
			light_components[i++] = (t_color){0, 0, 0, 0};
		free(hit_info);
		lights = lights->next;
	}
	while (i--)
		light_contribution = merge_colors(light_components, n_lights, light_ratios);
	return (free(light_components), free(light_ratios), light_contribution);
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
		return (free(closest_hit), NULL);
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
	static float (*const	intersect[3])(const t_ray, const t_shape *) = {&intersect_ray_sphere, &intersect_ray_cylinder, &intersect_ray_plane}; //stesso ordine di enum
	t_list	*shapes;
	t_shape	*shape;
	float	t;

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

static void	update_closest_hit(t_hit *closest_hit, const t_shape *shape, const float t, const t_ray ray)
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
	const float			projection_length = vec_dot(vec_from_center_to_point, cylinder.direction);
	t_vector			projection;

	if (fabs(projection_length  + cylinder.half_height) < EPSILON) //bottom cap
		return (vec_negate(cylinder.direction));
	if (fabs(projection_length - cylinder.half_height) < EPSILON) //top cap
		return (cylinder.direction);
	projection = vec_add(cylinder.center, vec_scale(projection_length, cylinder.direction));
	return(vec_normalize(vec_sub(point, projection)));
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

static t_color blend_colors(const t_color color1, const t_color color2, float ratio)
{
	t_color		result;

    ratio = fclamp(ratio, 0.0f, 1.0f);
	result.r = (uint8_t)(color1.r * (1.0f - ratio) + color2.r * ratio);
	result.g = (uint8_t)(color1.g * (1.0f - ratio) + color2.g * ratio);
	result.b = (uint8_t)(color1.b * (1.0f - ratio) + color2.b * ratio);
	return (result);
}

inline float	fclamp(const float value, const float min, const float max)
{
	return (value < min ? min : (value > max ? max : value));
}
