/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 15:35:08 by craimond          #+#    #+#             */
/*   Updated: 2024/03/28 23:35:36 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static uint16_t	fill_octree(t_octree *node, t_list *shapes, int depth, t_vector box_top, t_vector box_bottom);
static void		set_world_extremes(t_scene *scene);
static void		set_bb_sphere(t_shape *shape);
static void		set_bb_cylinder(t_shape *shape);
static void		set_bb_plane(t_shape *shape);
static t_list	*get_shapes_inside_box(t_list *shapes, t_vector box_top, t_vector box_bottom);

int  boxes_overlap(t_point box1_top, t_point box1_bottom, t_point box2_top, t_point box2_bottom)
{
    return (box1_bottom.x <= box2_top.x && box1_top.x >= box2_bottom.x &&
            box1_bottom.y <= box2_top.y && box1_top.y >= box2_bottom.y &&
            box1_bottom.z <= box2_top.z && box1_top.z >= box2_bottom.z);
}

bool	ray_intersects_aabb(t_ray ray, t_point bounding_box_max, t_point bounding_box_min)
{
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
	float ray_direction[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
	float bb_max[3] = {bounding_box_max.x, bounding_box_max.y, bounding_box_max.z};
	float bb_min[3] = {bounding_box_min.x, bounding_box_min.y, bounding_box_min.z};
	float ray_origin[3] = {ray.origin.x, ray.origin.y, ray.origin.z};

    for (int i = 0; i < 3; i++)
	{
        float invD = 1.0f / ray_direction[i];
        float t0 = (bb_min[i] - ray_origin[i]) * invD;
        float t1 = (bb_max[i] - ray_origin[i]) * invD;
        if (invD < 0.0f)
		{
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if (tmax <= tmin)
            return (false);
    }
    return (true);
}

void setup_scene(t_scene *scene)
{
	set_world_extremes(scene);
	fill_octree(scene->octree, scene->shapes, OCTREE_DEPTH, scene->world_max, scene->world_min);
}

static uint16_t fill_octree(t_octree *node, t_list *shapes, uint8_t depth, t_vector box_top, t_vector box_bottom)
{
	t_point		center;
	t_vector	size;
	t_point		new_box_top;
	t_point		new_box_bottom;
	t_list		*shapes_inside_box;
	uint16_t	i;

	node->depth = depth;
	if (depth == 0) //foglia
	{
		node->children = NULL; //fondamentale, e' cio' che distingue un nodo foglia da uno interno
		node->shapes = shapes;
		node->box_top = box_top;
		node->box_bottom = box_bottom;
		node->n_shapes = ft_lstsize(shapes);
		return (node->n_shapes);
	}
	//world.min a sx e world.max a dx
	center = (t_vector){(box_top.x + box_bottom.x) / 2, (box_top.y + box_bottom.y) / 2, (box_top.z + box_bottom.z) / 2};
	size = (t_vector){(box_top.x - box_bottom.x) / 2, (box_top.y - box_bottom.y) / 2, (box_top.z - box_bottom.z) / 2};
	i = -1;
	while (++i < 8)
	{
		new_box_top = center;
		new_box_bottom = center;

		new_box_top.x += (i & 1) ? size.x : 0;
		new_box_bottom.x += (i & 1) ? 0 : -size.x;
		new_box_top.y += (i & 2) ? size.y : 0;
		new_box_bottom.y += (i & 2) ? 0 : -size.y;
		new_box_top.z += (i & 4) ? size.z : 0;
		new_box_bottom.z += (i & 4) ? 0 : -size.z;

		shapes_inside_box = get_shapes_inside_box(shapes, new_box_top, new_box_bottom);
		node->children[i] = NULL;
		if (!shapes_inside_box)
			continue ;
		node->children[i] = (t_octree *)malloc(sizeof(t_octree));
		node->n_shapes += fill_octree(node->children[i], shapes_inside_box, depth - 1, new_box_top, new_box_bottom);
	}
	return (node->n_shapes);
}

static t_list *get_shapes_inside_box(t_list *shapes, t_point box_top, t_point box_bottom)
{
    t_list  *inside_shapes = NULL;
    while (shapes)
    {
        t_shape *current_shape = (t_shape*)shapes->content;
        if (boxes_overlap(box_top, box_bottom, current_shape->bb_max, current_shape->bb_min))
			ft_lstadd_back(&inside_shapes, ft_lstnew(current_shape));
        shapes = shapes->next;
    }
    return (inside_shapes);
}

static void	set_world_extremes(t_scene *scene)
{
	t_list		*shapes;
	t_shape		*shape;
	t_point		world_max = {0, 0, 0};
	t_point		world_min = {0, 0, 0};

	//bb min e bb max delle shapes deve partire da 0,0,0

	shapes = scene->shapes;
	if (!shapes)
		return ;
	shape = (t_shape *)shapes->content;
	while (shapes)
	{
		shape = shapes->content;
		world_max = (t_point){fmax(world_max.x, shape->bb_max.x), fmax(world_max.y, shape->bb_max.y), fmax(world_max.z, shape->bb_max.z)};
		world_min = (t_point){fmin(world_min.x, shape->bb_min.x), fmin(world_min.y, shape->bb_min.y), fmin(world_min.z, shape->bb_min.z)};
		shapes = shapes->next;
	}
	scene->world_min = world_min;
	scene->world_max = world_max;
}

void set_bounding_box(t_shape *shape)
{
	void (*const	get_bb_funcs[])(t_shape *) = {&set_bb_sphere, &set_bb_cylinder, &set_bb_plane}; //deve essere lo stesso ordine dell enum type
	const uint8_t	n_shapes = sizeof(get_bb_funcs) / sizeof(get_bb_funcs[0]);
	uint8_t			i;

	i = 0;
	while (i < n_shapes)
	{
		if (shape->type == i)
		{
			get_bb_funcs[i](shape);
			return ;
		}
		i++;
	}
}

//bounding box for sphere
static void	set_bb_sphere(t_shape *shape)
{
	shape->bb_min = (t_point){shape->sphere.center.x - shape->sphere.radius, shape->sphere.center.y - shape->sphere.radius, shape->sphere.center.z - shape->sphere.radius};
	shape->bb_max = (t_point){shape->sphere.center.x + shape->sphere.radius, shape->sphere.center.y + shape->sphere.radius, shape->sphere.center.z + shape->sphere.radius};
}

//bounding box for plane
static void set_bb_plane(t_shape *shape)
{
	t_vector 	u;
	t_vector 	v;
	t_vector 	r;
	const float	size = WORLD_SIZE / 2;
	
	r = (t_vector){1, 0, 0};
	if (fabs(vec_dot(r, shape->plane.normal)) > 0.999)
    	r = (t_vector){0, 1, 0};
	
	u = vec_cross(r, shape->plane.normal);
	v = vec_cross(u, shape->plane.normal);

	u = vec_normalize(u);
	v = vec_normalize(v);

	t_point bb_min = {
    	-(size * fabs(u.x)) - (size * fabs(v.x)),
    	-(size * fabs(u.y)) - (size * fabs(v.y)),
    	-(size * fabs(u.z)) - (size * fabs(v.z))
	};

	t_point bb_max = {
		size * fabs(u.x) + (size * fabs(v.x)),
	 	size * fabs(u.y) + (size * fabs(v.y)),
		size * fabs(u.z) + (size * fabs(v.z))
	};

	shape->bb_max = bb_max;
	shape->bb_min = bb_min;
}

//axis aligned bounding box for cylinder
static void	set_bb_cylinder(t_shape *shape)
{
	const t_vector	orientation = shape->cylinder.normal;
	const t_vector	center = shape->cylinder.center;

	const float r = shape->cylinder.diameter / 2;
	const float h = shape->cylinder.height;

	t_point bb_min;
	t_point bb_max;
	
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

	bb_min = axis_min;
    bb_max = axis_max;

    float expandX = sqrt(1 - orientation.x * orientation.x) * r;
    float expandY = sqrt(1 - orientation.y * orientation.y) * r;
    float expandZ = sqrt(1 - orientation.z * orientation.z) * r;

    bb_min.x = fmin(bb_min.x, center.x - expandX);
    bb_max.x = fmax(bb_max.x, center.x + expandX);

    bb_min.y = fmin(bb_min.y, center.y - expandY);
    bb_max.y = fmax(bb_max.y, center.y + expandY);

    bb_min.z = fmin(bb_min.z, center.z - expandZ);
    bb_max.z = fmax(bb_max.z, center.z + expandZ);

	shape->bb_min = bb_min;
	shape->bb_max = bb_max;
}

