/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/11 19:21:58 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void				set_thread_attr(pthread_attr_t *thread_attr);
static void				fill_image(t_mlx_data *win_data, const uint16_t img_idx, t_scene *scene);
static t_thread_data	*set_thread_data(const uint16_t i, t_scene *scene, t_mlx_data *win_data, double *light_ratios, double *attenuation_factors, const uint16_t lines_per_thread, const uint16_t img_idx);
static void				*render_line(void *data);
static t_color			render_pixel(const t_scene *scene, const t_ray ray, const double *attenuation_factors, const double *light_ratios);
static double			*precompute_ratios(uint16_t n_elems);
static double			*precoumpute_attenuation_factors(void);
static void				setup_camera(t_camera *cam);
static t_ray			get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_vector			get_rand_in_unit_sphere(void);
static t_ray			get_reflected_ray(const t_ray incoming_ray, const t_vector normal, const t_point point);
static t_color			ray_bouncing(const t_scene *scene, t_ray ray, t_hit *hit_info, const uint16_t n_bounce, const double *attenuation_factors, const double *light_ratios);
static t_color			add_lighting(const t_scene *scene, t_color color, const t_hit *hit_info, const double *light_ratios);
static t_color			get_light_component(t_color color, const double brightness, double distance, const double angle_of_incidence_cosine);
static t_color			compute_lights_contribution(const t_scene *scene, t_point surface_point, const t_vector surface_normal, const double *light_ratios);
static t_hit			*trace_ray(const t_scene *scene, const t_ray ray);
static void				traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				update_closest_hit(t_hit *closest_hit, const t_shape *shape, const double t, const t_ray ray);
static t_vector			get_cylinder_normal(t_cylinder cylinder, t_point point);
static t_point			ray_point_at_parameter(const t_ray ray, double t);
static t_color			blend_colors(const t_color color1, const t_color color2, double ratio);

//TODO implementare un blending tra i pixel vicini per velocizzare il rendering e aumentare la smoothness
//TODO sperimentare con la keyword restrict
//TODO utilizzare mlx_get_screen_size invece di dimensioni fixed

void render_scene(t_mlx_data *win_data, t_scene *scene)
{
	t_list		*cameras;
	uint16_t	i;

	cameras = scene->cameras;
	srand(time(NULL));
	i = 0;
	while (cameras)
	{
		scene->current_camera = cameras->content;
		setup_camera(scene->current_camera);
		fill_image(win_data, i++, scene);
		cameras = cameras->next;
	}
	mlx_put_image_to_window(win_data->mlx, win_data->win, win_data->images[0], 0, 0);
}

static void	set_thread_attr(pthread_attr_t *thread_attr)
{
	pthread_attr_init(thread_attr);
	pthread_attr_setschedpolicy(thread_attr, SCHED_FIFO);
	pthread_attr_setscope(thread_attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(thread_attr, PTHREAD_CREATE_JOINABLE);
}

static void	fill_image(t_mlx_data *win_data, const uint16_t img_idx, t_scene *scene)
{
	pthread_t 		thread_ids[N_THREADS];
	pthread_attr_t	thread_attr;
	t_thread_data	*thread_data;
	double			*light_ratios;
	double			*attenuation_factors;
	uint16_t		i;
	const uint16_t	lines_per_thread = (uint16_t)roundf((double)WIN_HEIGHT / (double)N_THREADS);

	light_ratios = precompute_ratios(ft_lstsize(scene->lights));
	attenuation_factors = precoumpute_attenuation_factors();
	set_thread_attr(&thread_attr);
	i = 0;
	while (i < N_THREADS)
	{
		thread_data = set_thread_data(i, scene, win_data, light_ratios, attenuation_factors, lines_per_thread, img_idx);
		pthread_create(&thread_ids[i], &thread_attr, render_line, thread_data);
		i++;
	}
	while (i > 0)
		pthread_join(thread_ids[--i], NULL);
	free(light_ratios);
	free(attenuation_factors);
	pthread_attr_destroy(&thread_attr);
}

static t_thread_data *set_thread_data(const uint16_t i, t_scene *scene, t_mlx_data *win_data, double *light_ratios, double *attenuation_factors, const uint16_t lines_per_thread, const uint16_t img_idx)
{
	t_thread_data			*thread_data;

	thread_data = (t_thread_data *)malloc(sizeof(t_thread_data));
	thread_data->scene = scene;
	thread_data->win_data = win_data;
	thread_data->img_idx = img_idx;
	thread_data->light_ratios = light_ratios;
	thread_data->attenuation_factors = attenuation_factors;
	thread_data->start_y = i * lines_per_thread;
	if (i == N_THREADS - 1)
		thread_data->end_y = WIN_HEIGHT;
	else
		thread_data->end_y = (i + 1) * lines_per_thread;
	return (thread_data);
}

static void	*render_line(void *data)
{
	t_thread_data	*thread_data;
	uint16_t		y;
	uint16_t		x;
	t_ray			ray;
	t_color			color;

	thread_data = (t_thread_data *)data;
	y = thread_data->start_y;
	while (y < thread_data->end_y)
	{
		x = 0;
		while (x < WIN_WIDTH)
		{
			ray = get_ray(thread_data->scene->current_camera, x, y);
			color = render_pixel(thread_data->scene, ray, thread_data->attenuation_factors, thread_data->light_ratios);
			my_mlx_pixel_put(thread_data->win_data, thread_data->img_idx, x, y, color);
			x++;
		}
		y++;
	}
	return (free(data), NULL);
}

static t_color	render_pixel(const t_scene *scene, const t_ray ray, const double *attenuation_factors, const double *light_ratios)
{
	t_hit			*first_hit;
	t_color			color;

	first_hit = trace_ray(scene, ray);
	color = ray_bouncing(scene, ray, first_hit, 1, attenuation_factors, light_ratios);
	//la luce viene calcolata solo sul primo hit_point, non su tutti i bounce (stesso effetto visivo, provare per credere)
	color = add_lighting(scene, color, first_hit, light_ratios);
	return (free(first_hit), color);
}

static double *precompute_ratios(uint16_t n_elems)
{
	uint16_t	i;
	double 		*ratios;
	
	ratios = (double *)malloc(n_elems * sizeof(double));
	ratios[0] = 1.0f;
	i = 1;
	while (i < n_elems)
	{
		ratios[i] = 1.0f / (i + 1);
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

static void	setup_camera(t_camera *cam)
{
	const t_vector		world_up = {0, 1, 0};
	const double		aspect_ratio = (double)WIN_WIDTH / (double)WIN_HEIGHT;
	const double		rad_fov = cam->fov * M_PI / 180; //fov da grad in radianti

	cam->viewport_height = 2 * tan(rad_fov / 2);
	cam->viewport_width = aspect_ratio * cam->viewport_height;
	//calcolo dei vettori base della camera
	cam->forward = vec_normalize(cam->normal); //la normale punta verso il punto di interesse
	if (are_vectors_parallel(cam->forward, world_up)) //se la normale è uguale al vettore up del mondo
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

static t_vector get_rand_in_unit_sphere(void) //metodo di Marsaglia (meno leggibile ma piu' efficiente)
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
	static const t_color	bg_color = {BACKGROUND_COLOR >> 16 & 0xFF, BACKGROUND_COLOR >> 8 & 0xFF, BACKGROUND_COLOR & 0xFF, 0};
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
	ray_color.a = 0;
	return(free(new_hit), blend_colors(hit_color, ray_color, 0.5f));
}

static t_color	add_lighting(const t_scene *scene, t_color color, const t_hit *hit_info, const double *light_ratios)
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

static t_color get_light_component(t_color color, const double brightness, double distance, const double angle_of_incidence_cosine)
{
	distance = fmax(distance, EPSILON); //per evitare divisioni per zero
	const double	attenuation = fclamp(1.0f - log(distance) / log(WORLD_SIZE), EPSILON, 1.0f);
	const double	diffusion = fmax(angle_of_incidence_cosine, 0.0f);
	const double	variation = brightness * diffusion * attenuation;

	color.r *= variation;
	color.g *= variation;
	color.b *= variation;
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
	
	light_contribution = (t_color){0, 0, 0, 0};
	light_component = (t_color){0, 0, 0, 0};
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
			light_component = get_light_component(light->color, light->brightness, light_distance, angle_of_incidence_cosine);
		}
		else
			light_component = (t_color){0, 0, 0, 0};
		light_contribution = blend_colors(light_contribution, light_component, light_ratios[i++]);
		free(hit_info);
		lights = lights->next;
	}
	return (light_contribution);
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
	double		negative_ratio;

    ratio = fclamp(ratio, 0.0f, 1.0f);
	negative_ratio = 1.0f - ratio;
	result.r = (color1.r * negative_ratio + color2.r * ratio);
	result.g = (color1.g * negative_ratio + color2.g * ratio);
	result.b = (color1.b * negative_ratio + color2.b * ratio);
	return (result);
}

inline double	fclamp(const double value, const double min, const double max)
{
	return (value < min ? min : (value > max ? max : value));
}
