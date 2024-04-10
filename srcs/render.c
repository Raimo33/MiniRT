/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/10 22:50:15 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void				setup_camera(t_camera *cam);
static t_ray			get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_color			ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce, const uint16_t idx, const double *attenuation_factors, const double *light_ratios, const t_vector *random_bias_vectors, const uint16_t n_lights);
static t_hit			*trace_ray(const t_scene *scene, const t_ray ray);
static inline t_point	ray_point_at_parameter(const t_ray ray, double t);
static void				check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_vector		 	get_cylinder_normal(t_cylinder cylinder, t_point point);
static t_color 			blend_colors(const t_color color1, const t_color color2, const double ratio);
static void				fill_image(t_mlx_data *win_data, t_scene *scene);
static double 			*precompute_ratios(uint16_t n_elems);
static t_color			merge_colors(t_color *colors, const uint16_t n_colors, const double *ratios);
static void				set_thread_attr(pthread_attr_t *thread_attr);
static t_vector			get_rand_in_unit_sphere(void);
static void				*render_pixel(void *data);
static t_thread_data	*set_thread_data(t_scene *scene, t_ray ray, t_color *colors_array, const uint16_t i, const uint16_t depth_per_thread, double *light_ratios, double *attenuation_factors, t_vector *random_bias_vectors);
static void				update_closest_hit(t_hit *closest_hit, const t_shape *shape, const double t, const t_ray ray);
static t_color			compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal, const double *light_ratios, const uint16_t n_lights);
static double			*precoumpute_attenuation_factors(void);
static t_vector			*precompute_random_bias_vectors(void);

//TODO implementare un blending tra i pixel vicini per velocizzare il rendering e aumentare la smoothness
//TODO sperimentare con la keyword restrict
//TODO utilizzare mlx_get_screen_size invece di dimensioni fixed
//TODO con le normali negative non funzionano ne i cilindri ne i piani

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	pthread_attr_t	thread_attr;

	setup_camera(scene->camera);
	set_thread_attr(&thread_attr);
	srand(time(NULL));
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
	t_color			colors[RAYS_PER_PIXEL];
	pthread_t 		thread_ids[N_THREADS];
	double			*color_ratios;
	double			*light_ratios;
	double			*attenuation_factors;
	t_thread_data	*thread_data;
	t_vector		*random_bias_vectors;
	t_ray			ray;
	double			depth_per_thread;
	t_color			final_color;
	uint16_t		x;
	uint16_t		y;
	uint16_t		i;
	

	depth_per_thread = roundf((double)RAYS_PER_PIXEL / (double)N_THREADS);
	color_ratios = precompute_ratios(RAYS_PER_PIXEL);
	light_ratios = precompute_ratios(ft_lstsize(scene->lights));
	attenuation_factors = precoumpute_attenuation_factors();
	random_bias_vectors = precompute_random_bias_vectors();
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
				thread_data = set_thread_data(scene, ray, colors, i, depth_per_thread, light_ratios, attenuation_factors, random_bias_vectors);
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

static t_thread_data	*set_thread_data(t_scene *scene, t_ray ray, t_color *colors_array, const uint16_t i, const uint16_t depth_per_thread, double *light_ratios, double *attenuation_factors, t_vector *random_bias_vectors)
{
	t_thread_data	*thread_data;

	thread_data = (t_thread_data *)malloc(sizeof(t_thread_data));
	thread_data->scene = scene;
	thread_data->ray = ray;
	thread_data->colors_array = colors_array;
	thread_data->random_bias_vectors = random_bias_vectors;
	thread_data->attenuation_factors = attenuation_factors;
	thread_data->start_depth = i * depth_per_thread;
	thread_data->light_ratios = light_ratios;
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
	double			*attenuation_factors = thread_data->attenuation_factors;
	double			*light_ratios = thread_data->light_ratios;
	t_vector		*random_bias_vectors = thread_data->random_bias_vectors;
	t_scene			*scene = thread_data->scene;
	uint16_t		i;

	i = thread_data->start_depth;
	while (i < thread_data->end_depth)
	{
		colors_array[i] = ray_bouncing(scene, thread_data->ray, 0, i, attenuation_factors, light_ratios, random_bias_vectors, scene->n_lights);
		i++;
	}
	return (free(data), NULL);
}

static double *precompute_ratios(uint16_t n_elems)
{
	uint16_t	i;
	double 		*ratios;
	
	ratios = (double *)malloc(n_elems * sizeof(double));
	i = 0;
	while (i < n_elems)
	{
		ratios[i] = 1.0f / (i + 2);
		i++;
	}
	return (ratios);
}

static double	*precoumpute_attenuation_factors(void)
{
	uint16_t	i;
	double		*attenuation_factors;
	double		safety_limit = EPSILON * 5;

	attenuation_factors = (double *)malloc((MAX_BOUNCE + 1) * sizeof(double));
	attenuation_factors[0] = 1.0f;
	i = 1;
	while (i <= MAX_BOUNCE)
	{
		attenuation_factors[i] = attenuation_factors[i - 1] * ATTENUATION_FACTOR;
		if (attenuation_factors[i] <= safety_limit) //safety measure to avoid division by zero and floating point issues
		{
			while (i <= MAX_BOUNCE)
				attenuation_factors[i++] = safety_limit;
			break ;
		}
		i++;
	}
	return (attenuation_factors);
}

static t_vector	*precompute_random_bias_vectors(void)
{
	uint16_t	i;
	t_vector	*random_bias_vectors;
	
	random_bias_vectors = (t_vector *)malloc(RAYS_PER_PIXEL * sizeof(t_vector));
	i = 0;
	while (i < RAYS_PER_PIXEL)
		random_bias_vectors[i++] = get_rand_in_unit_sphere();
	return (random_bias_vectors);
}

static t_color	merge_colors(t_color *colors, const uint16_t n_colors, const double *ratios)
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
	const double		aspect_ratio = (double)WIN_WIDTH / (double)WIN_HEIGHT;
	const double		rad_fov = cam->fov * M_PI / 180; //fov da grad in radianti

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
	const double		screen_viewport_x = x / (double)(WIN_WIDTH - 1);
	const double		screen_viewport_y = y / (double)(WIN_HEIGHT - 1);
	//coordinate sul piano cartesiano vero [-1, 1]
	const double		real_viewport_x = screen_viewport_x * 2 - 1;
	const double		real_viewport_y = 1 - screen_viewport_y * 2;

	const double		half_viewport_width = cam->viewport_width / 2;
	const double		half_viewport_height = cam->viewport_height / 2;
	
	//calcolo del vettore che parte dalla camera verso il pixel (sul piano immaginario viewport)
	const t_vector direction = //vettore direzione
	{
		.x = cam->forward.x + (real_viewport_x * cam->right.x * half_viewport_width) + (real_viewport_y * cam->up.x * half_viewport_height),
		.y = cam->forward.y + (real_viewport_x * cam->right.y * half_viewport_width) + (real_viewport_y * cam->up.y * half_viewport_height),
		.z = cam->forward.z + (real_viewport_x * cam->right.z * half_viewport_width) + (real_viewport_y * cam->up.z * half_viewport_height)
	};
	return ((t_ray){cam->center, vec_normalize(direction)});
}

static t_vector get_rand_in_unit_sphere(void) //metodo di Marsaglia
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
		.direction = vec_normalize(vec_sub(vec_scale(2 * vec_dot(normal, incoming_ray.direction), normal), incoming_ray.direction))
	};
	reflected_ray.direction = vec_normalize(vec_add(reflected_ray.direction, vec_scale(0.1, random_component))); //TODO random component influenza troppo i layer, che una volta uniti si scazzano
	return (reflected_ray);
}

//TODO aggiungere il calcolo della luce solo sul primo oggetto colpito, non su tutti gli oggetti piu' avanti nella ricorsione
static t_color	ray_bouncing(const t_scene *scene, t_ray ray, const uint16_t n_bounce, const uint16_t idx, const double *attenuation_factors, const double *light_ratios, const t_vector *random_bias_vectors, const uint16_t n_lights)
{
	t_hit					*hit_info;
	t_color					ray_color;
	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF, 0};
	t_color					light_component;
	static const double		reciproca1255 = 1.0f / 255.0f;

	if (n_bounce > MAX_BOUNCE)
		return (bg_color);
	hit_info = trace_ray(scene, ray);
	if (!hit_info)
		return (bg_color);
	ray = get_reflected_ray(ray, hit_info->normal, hit_info->point, random_bias_vectors[idx]);
	ray_color = ray_bouncing(scene, ray, n_bounce + 1, idx, attenuation_factors, light_ratios, random_bias_vectors, n_lights);
	const t_color	hit_color = hit_info->material->color;
	light_component = compute_lights_contribution(scene, hit_info->point, hit_info->normal, light_ratios, n_lights);
	light_component = blend_colors(light_component, scene->amblight.ambient, 0.2f); //ratio 80/20 tra luce e ambient
	const double 	attenuation = attenuation_factors[n_bounce];
	const t_color	attenuated_light = {
        .r = light_component.r * attenuation,
        .g = light_component.g * attenuation,
        .b = light_component.b * attenuation,
        .a = 0
    };
    ray_color = (t_color) {
        .r = fmin(255.0f, (hit_color.r * attenuated_light.r * reciproca1255 + ray_color.r)),
        .g = fmin(255.0f, (hit_color.g * attenuated_light.g * reciproca1255 + ray_color.g)),
        .b = fmin(255.0f, (hit_color.b * attenuated_light.b * reciproca1255 + ray_color.b)),
        .a = 0
    };
	return (free(hit_info), ray_color);
}

static t_color weigh_color(const t_color color, double brightness, double distance, const double angle_of_incidence_cosine)
{
	distance = fmax(distance, EPSILON); //per evitare divisioni per zero
	const double attenuation = 1.0f / (1.0f + log1p(distance - 1));
	const double adjusted_brightness = fclamp(brightness * attenuation * angle_of_incidence_cosine, 0.0f, 1.0f);

	return ((t_color)
	{
		.r = (uint8_t)(color.r * adjusted_brightness),
		.g = (uint8_t)(color.g * adjusted_brightness),
		.b = (uint8_t)(color.b * adjusted_brightness),
		.a = 0
	});
}

static t_color	compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal, const double *light_ratios, const uint16_t n_lights)
{
	t_color			*light_components;
	t_color			light_contribution = {0, 0, 0, 0};
	t_list			*lights;
	t_light			*light;
	t_vector		light_dir;
	double			light_distance;
	t_hit			*hit_info;
	uint16_t		i;
	double			angle_of_incidence_cosine;
	
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
			angle_of_incidence_cosine = vec_dot(surface_normal, light_dir);			
			light_components[i++] = weigh_color(light->color, light->brightness, light_distance, angle_of_incidence_cosine);
		}
		else
			light_components[i++] = (t_color){0, 0, 0, 0};
		free(hit_info);
		lights = lights->next;
	}
	while (i--)
		light_contribution = merge_colors(light_components, n_lights, light_ratios);
	return (free(light_components), light_contribution);
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
	const double			projection_length = vec_dot(vec_from_center_to_point, cylinder.direction);
	t_vector			projection;

	if (fabs(projection_length  + cylinder.half_height) < EPSILON) //bottom cap
		return (vec_negate(cylinder.direction));
	if (fabs(projection_length - cylinder.half_height) < EPSILON) //top cap
		return (cylinder.direction);
	projection = vec_add(cylinder.center, vec_scale(projection_length, cylinder.direction));
	return(vec_normalize(vec_sub(point, projection)));
}

inline static t_point ray_point_at_parameter(const t_ray ray, double t)
{
    return ((t_point)
	{
		.x = ray.origin.x + t * ray.direction.x,
		.y = ray.origin.y + t * ray.direction.y,
		.z = ray.origin.z + t * ray.direction.z
	});
}

static t_color blend_colors(const t_color color1, const t_color color2, double ratio)
{
	t_color		result;

    ratio = fclamp(ratio, 0.0f, 1.0f);
	result.r = (uint8_t)(color1.r * (1.0f - ratio) + color2.r * ratio);
	result.g = (uint8_t)(color1.g * (1.0f - ratio) + color2.g * ratio);
	result.b = (uint8_t)(color1.b * (1.0f - ratio) + color2.b * ratio);
	return (result);
}

inline double	fclamp(const double value, const double min, const double max)
{
	return (value < min ? min : (value > max ? max : value));
}
