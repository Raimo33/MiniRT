/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/04/01 18:51:16 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void				setup_camera(t_camera *cam);
static t_ray			get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static uint32_t			ray_bouncing(const t_scene scene, t_ray ray);
static t_hit			*trace_ray(const t_scene scene, t_ray ray);
static t_ray			get_reflected_ray(const t_ray ray, const t_vector normal, const t_point point, const t_material *material);
static inline t_point	ray_point_at_parameter(const t_ray ray, float t);
static void				check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static void				traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_vector 		get_cylinder_normal(t_shape *shape, t_point point);
static uint32_t 		compute_color_at_intersection(const uint32_t prev_color, const t_material *material, const float attenuation_factor);
static uint32_t 		blend_colors(uint32_t color1, uint32_t color2, float ratio);
inline static float		fclamp(float value, float min, float max);

void render(const t_mlx_data mlx_data, t_scene scene)
{
	t_ray	ray;
	uint32_t	color;
	int		x;
	int		y;
	
	setup_camera(&scene.camera);
	y = 0;
	while (y < WIN_HEIGHT)
	{
		x = 0;
		//implementare threads (ognuno fa un tot di righe)
		while (x < WIN_WIDTH)
		{
			ray = get_ray(&scene.camera, x, y);
			color = ray_bouncing(scene, ray);
			my_mlx_pixel_put(mlx_data, x, y, color);
			x++;
		}
		y++;
	}
	mlx_put_image_to_window(mlx_data.mlx, mlx_data.win, mlx_data.img, 0, 0);
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

static t_ray	get_reflected_ray(const t_ray ray, const t_vector normal, const t_point point, const t_material *material)
{
	const float		dot = vec_dot(ray.direction, normal);
	const t_vector	reflected = vec_sub(ray.direction, vec_scale(normal, 2 * dot));
	const t_ray		reflected_ray = {point, reflected};
	(void)material;
	//TODO materiali piu grezzi (es. roccia) riflettono il raggio in modo diverso (randomicita' minima)

	return (reflected_ray);
}

static uint32_t ray_bouncing(const t_scene scene, t_ray ray)
{
	uint16_t 	i;
	uint32_t	color;
	t_hit		*hit_info;
	float		attenuation_factor;

	color = 0x000000;
	attenuation_factor = 1.0f;
	i = 0;
	while (i < MAX_BOUNCE)
	{
		hit_info = trace_ray(scene, ray);
		if (!hit_info)
			break ;
		color = compute_color_at_intersection(color, &hit_info->material, attenuation_factor);
		// attenuation_factor *= hit_info->material.reflectivity;
		attenuation_factor *= 0.8f;
		ray = get_reflected_ray(ray, hit_info->normal, hit_info->point, &hit_info->material);
		i++;
	}
	return (color);
}

static t_hit	*trace_ray(const t_scene scene, const t_ray ray)
{
    t_hit		*closest_hit = (t_hit *)malloc(sizeof(t_hit));

	*closest_hit = (t_hit)
	{
		.distance = FLT_MAX,
		.point = {0, 0, 0},
		.normal = {0, 0, 0},
		.material = {0, 0, 0}
	};
    traverse_octree(scene.octree, ray, closest_hit);
	if (closest_hit->distance == FLT_MAX)
		return (NULL);
    // Calcolare il colore del pixel in base all'intersezione più vicina
	// Senza considerare le luci e i materiali per ora
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

static uint32_t	compute_color_at_intersection(const uint32_t prev_color, const t_material *material, const float attenuation_factor)
{
    // // Assuming a white light source for simplicity
    // const uint32_t light_color = 0xFFFFFF;

    // // Base color affected by material color and light color
    // uint32_t base_color = blend_colors(prev_color, material->color, 1.0f - material->reflectivity);
    
    // // Specular highlight (simplified calculation)
    // // This would normally depend on the angle between the light source, the normal at the point of intersection, and the view direction
    // // For this example, assume a fixed specular contribution for demonstration purposes
    // uint32_t specular_highlight = blend_colors(light_color, 0x000000, 1.0f - material->specular_strength);

    // // Combine base color with specular highlight, assuming the highlight affects the overall color subtly
    // uint32_t final_color = blend_colors(base_color, specular_highlight, 0.1f); // Specular highlights are usually subtle

    return (blend_colors(prev_color, material->color, attenuation_factor));
}

static uint32_t blend_colors(uint32_t color1, uint32_t color2, float ratio)
{
	uint8_t result_r;
	uint8_t result_g;
	uint8_t result_b;
	
    ratio = fclamp(ratio, 0.0f, 1.0f);
	result_r = (uint8_t)(((color1 >> 16) & 0xFF) * (1.0f - ratio) + ((color2 >> 16) & 0xFF) * ratio);
	result_g = (uint8_t)(((color1 >> 8) & 0xFF) * (1.0f - ratio) + ((color2 >> 8) & 0xFF) * ratio);
	result_b = (uint8_t)((color1 & 0xFF) * (1.0f - ratio) + (color2 & 0xFF) * ratio);

    return ((result_r << 16) | (result_g << 8) | result_b);
}

inline static float	fclamp(float value, float min, float max)
{
	return (value < min ? min : (value > max ? max : value));
}
