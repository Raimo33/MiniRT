/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 15:35:08 by craimond          #+#    #+#             */
/*   Updated: 2024/03/25 17:47:33 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void fill_octree(t_octree *node, t_list *shapes, int depth, t_vector box_top, t_vector box_bottom);
static void	set_world_extremes(t_scene *scene);
static void	get_bounding_box(t_shape shape, t_vector *bb_min, t_vector *bb_max);
static void	get_bb_sphere(t_shape shape, t_vector *bb_min, t_vector *bb_max);
static void	get_bb_cylinder(t_shape shape, t_vector *bb_min, t_vector *bb_max);

void setup_scene(t_scene *scene)
{
	set_world_extremes(scene);
	fill_octree(scene->octree, scene->shapes, 4, scene->world_max, scene->world_min);
}

static void fill_octree(t_octree *node, t_list *shapes, int depth, t_vector box_top, t_vector box_bottom)
{
	t_vector	center;
	t_vector	size;
	t_vector	new_box_top;
	t_vector	new_box_bottom;
	uint16_t	i;
	t_float3	sign = {0, 0, 0};

	if (depth == 0)
	{
		node->shapes = shapes;
		node->n_shapes = ft_lstsize(shapes);
		node->box_top = box_top;
		node->box_bottom = box_bottom;
		return ;
	}
	//world.min a sx e world.max a dx
	center = (t_vector){(box_top.x + box_bottom.x) / 2, (box_top.y + box_bottom.y) / 2, (box_top.z + box_bottom.z) / 2};
	size = (t_vector){(box_top.x - box_bottom.x) / 2, (box_top.y - box_bottom.y) / 2, (box_top.z - box_bottom.z) / 2};
	i = 0;
	while (i < 8)
	{
		sign.x = (i & 1) ? size.x : -size.x;
		sign.y = (i & 2) ? size.y : -size.y;
		sign.z = (i & 4) ? size.z : -size.z;
		new_box_top = center;
		new_box_bottom = center;
		if (sign.x > 0)
			new_box_top.x += sign.x;
		else
			new_box_bottom.x += sign.x;
		if (sign.y > 0)
			new_box_top.y += sign.y;
		else
			new_box_bottom.y += sign.y;
		if (sign.z > 0)
			new_box_top.z += sign.z;
		else
			new_box_bottom.z += sign.z;
		node->children[i] = (t_octree *)malloc(sizeof(t_octree));
		fill_octree(node->children[i], shapes, depth - 1, new_box_top, new_box_bottom);
		i++;
	}
}

static void	set_world_extremes(t_scene *scene)
{
	t_list		*shapes;
	t_shape		*shape;
	t_vector	world_max = {0, 0, 0};
	t_vector	world_min = {0, 0, 0};
	t_vector	bb_min = world_min;
	t_vector	bb_max = world_max;

	shapes = scene->shapes;
	if (!shapes)
		return ;
	shape = (t_shape *)shapes->content;
	while (shapes)
	{
		shape = shapes->content;
		if (shape->type != PLANE)
		{
			printf("shape type: %d\n", shape->type);
			get_bounding_box(*shape, &bb_min, &bb_max);
			world_max = (t_vector){fmax(world_max.x, bb_max.x), fmax(world_max.y, bb_max.y), fmax(world_max.z, bb_max.z)};
			world_min = (t_vector){fmin(world_min.x, bb_min.x), fmin(world_min.y, bb_min.y), fmin(world_min.z, bb_min.z)};
		}
		else
		{
			world_max = (t_vector){WORLD_SIZE, WORLD_SIZE, WORLD_SIZE};
			world_min = (t_vector){-WORLD_SIZE, -WORLD_SIZE, -WORLD_SIZE};
			break ;
		}
		shapes = shapes->next;
	}
	printf("world min: %f %f %f\n", world_min.x, world_min.y, world_min.z);
	scene->world_min = world_min;
	scene->world_max = world_max;
}

static void get_bounding_box(t_shape shape, t_vector *bb_min, t_vector *bb_max)
{
	void (*const	get_bb_funcs[])(t_shape, t_vector *, t_vector *) = {&get_bb_sphere, &get_bb_cylinder}; //deve essere lo stesso ordine dell enum type
	const uint8_t	n_shapes = sizeof(get_bb_funcs) / sizeof(get_bb_funcs[0]);
	uint8_t			i;

	i = 0;
	while (i < n_shapes)
	{
		if (shape.type == i)
		{
			get_bb_funcs[i](shape, bb_min, bb_max);
			return ;
		}
		i++;
	}
}

//bounding box for sphere
static void	get_bb_sphere(t_shape shape, t_vector *bb_min, t_vector *bb_max)
{
	*bb_min = (t_vector){shape.sphere.center.x - shape.sphere.radius, shape.sphere.center.y - shape.sphere.radius, shape.sphere.center.z - shape.sphere.radius};
	*bb_max = (t_vector){shape.sphere.center.x + shape.sphere.radius, shape.sphere.center.y + shape.sphere.radius, shape.sphere.center.z + shape.sphere.radius};
}

//axis aligned bounding box for cylinder
static void	get_bb_cylinder(t_shape shape, t_vector *bb_min, t_vector *bb_max)
{
	const t_vector	orientation = shape.cylinder.normal;
	const t_vector	center = shape.cylinder.center;

	const float r = shape.cylinder.diameter / 2;
	const float h = shape.cylinder.height;
	
	t_vector axis_max = (t_vector){
        center.x + orientation.x * (h / 2),
        center.y + orientation.y * (h / 2),
        center.z + orientation.z * (h / 2),
	};
	t_vector axis_min = (t_vector){
        center.x - orientation.x * (h / 2),
        center.y - orientation.y * (h / 2),
        center.z - orientation.z * (h / 2),
	};

	*bb_min = axis_min;
    *bb_max = axis_max;

    float expandX = sqrt(1 - orientation.x * orientation.x) * r;
    float expandY = sqrt(1 - orientation.y * orientation.y) * r;
    float expandZ = sqrt(1 - orientation.z * orientation.z) * r;

    bb_min->x = fmin(bb_min->x, center.x - expandX);
    bb_max->x = fmax(bb_max->x, center.x + expandX);

    bb_min->y = fmin(bb_min->y, center.y - expandY);
    bb_max->y = fmax(bb_max->y, center.y + expandY);

    bb_min->z = fmin(bb_min->z, center.z - expandZ);
    bb_max->z = fmax(bb_max->z, center.z + expandZ);
}

