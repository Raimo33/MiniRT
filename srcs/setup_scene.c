/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:25:57 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 13:53:14 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		fill_octree(t_octree *node, t_list *shapes, uint8_t depth, t_vector box_top, t_vector box_bottom);
static void		set_world_extremes(t_scene *scene);
static void		set_bounding_box(t_shape *shape);
static void		set_bb_sphere(t_shape *shape);
static void		set_bb_cylinder(t_shape *shape);
static void		set_bb_plane(t_shape *shape);
static void		set_bb_triangle(t_shape *shape);
static void		set_bb_cone(t_shape *shape);
static t_list	*get_shapes_inside_box(t_list *shapes, t_vector box_top, t_vector box_bottom);
static void 	set_shapes_data(t_scene *scene);

int  boxes_overlap(const t_point box1_top, const t_point box1_bottom, const t_point box2_top, const t_point box2_bottom)
{
    return (box1_bottom.x <= box2_top.x && box1_top.x >= box2_bottom.x &&
            box1_bottom.y <= box2_top.y && box1_top.y >= box2_bottom.y &&
            box1_bottom.z <= box2_top.z && box1_top.z >= box2_bottom.z);
}

void setup_scene(t_scene *scene)
{
	set_shapes_data(scene);
	set_world_extremes(scene);
	fill_octree(scene->octree, scene->shapes, OCTREE_DEPTH, scene->world_max, scene->world_min);
}

static void set_shapes_data(t_scene *scene)
{
	t_shape		*shape;
	t_list		*node;
	t_cylinder	*cylinder;
	double		cos_theta;

	node = scene->shapes;
	while (node)
	{
		shape = (t_shape *)node->content;
		set_bounding_box(shape);
		switch (shape->e_type)
		{
			case SPHERE:
				shape->sphere.squared_radius = shape->sphere.radius * shape->sphere.radius;
				break ;
			case TRIANGLE:
				shape->triangle.normal = vec_normalize(vec_cross(vec_sub(shape->triangle.vertices[1], shape->triangle.vertices[0]), vec_sub(shape->triangle.vertices[2], shape->triangle.vertices[0])));
				break ;
			case PLANE:
				shape->plane.normal = vec_normalize(shape->plane.normal);
				break ;
			case CYLINDER:
				cylinder = &shape->cylinder;
				cylinder->squared_radius = cylinder->radius * cylinder->radius;
				cylinder->direction = vec_normalize(cylinder->direction);
				cylinder->half_height = cylinder->height / 2;
				const t_vector	center_to_cap = vec_scale(cylinder->half_height, cylinder->direction);
				cylinder->top_cap_center = vec_add(cylinder->center, center_to_cap);
				cylinder->bottom_cap_center = vec_sub(cylinder->center, center_to_cap);
				break ;
			case CONE:
				cos_theta = cos(atan2(shape->cone.radius, shape->cone.height));
				shape->cone.costheta_squared = cos_theta * cos_theta;
				shape->cone.direction = vec_normalize(shape->cone.direction);
				break ;
			default:
				break ;
		}
		node = node->next;
	}
}

static void fill_octree(t_octree *node, t_list *shapes, uint8_t depth, t_vector box_top, t_vector box_bottom)
{
	t_point		center;
	t_vector	size;
	t_point		new_box_top;
	t_point		new_box_bottom;
	t_list		*shapes_inside_box;
	uint16_t	i;

	if (depth == 0)
	{
		node->children = NULL;
		node->shapes = shapes;
		node->box_top = box_top;
		node->box_bottom = box_bottom;
		node->n_shapes = ft_lstsize(shapes);
		return ;
	}
	node->children = (t_octree **)calloc_p(8, sizeof(t_octree *));
	node->box_bottom = box_bottom;
	node->box_top = box_top;
	node->shapes = shapes;
	node->n_shapes = 0;
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
		if (!shapes_inside_box)
		{
			node->children[i] = NULL;
			continue ;
		}
		node->children[i] = (t_octree *)calloc_p(1, sizeof(t_octree));
		node->n_shapes += ft_lstsize(shapes_inside_box);
		fill_octree(node->children[i], shapes_inside_box, depth - 1, new_box_top, new_box_bottom);
	}
}

static t_list *get_shapes_inside_box(t_list *shapes, t_point box_top, t_point box_bottom)
{
	t_list	*inside_shapes;
	t_shape	*current_shape;

	inside_shapes = NULL;
	while (shapes)
	{
		current_shape = (t_shape *)shapes->content;
		if (boxes_overlap(box_top, box_bottom, current_shape->bb_max, current_shape->bb_min))
			ft_lstadd_front(&inside_shapes, ft_lstnew(current_shape));
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

static void	set_bounding_box(t_shape *shape)
{
	void (*const	get_bb_funcs[])(t_shape *) = {&set_bb_sphere, &set_bb_cylinder, &set_bb_triangle, &set_bb_cone, &set_bb_plane}; //deve essere lo stesso ordine dell enum type
	const uint8_t	n_shapes = sizeof(get_bb_funcs) / sizeof(get_bb_funcs[0]);
	uint8_t			i;

	i = 0;
	while (i < n_shapes)
	{
		if (shape->e_type == i)
		{
			get_bb_funcs[i](shape);
			return ;
		}
		i++;
	}
}

//axis aligned bounding box for sphere
static void	set_bb_sphere(t_shape *shape)
{
	shape->bb_min = (t_point){shape->sphere.center.x - shape->sphere.radius, shape->sphere.center.y - shape->sphere.radius, shape->sphere.center.z - shape->sphere.radius};
	shape->bb_max = (t_point){shape->sphere.center.x + shape->sphere.radius, shape->sphere.center.y + shape->sphere.radius, shape->sphere.center.z + shape->sphere.radius};
}

//axis aligned bounding box for plane
static void set_bb_plane(t_shape *shape)
{
	t_vector 			u;
	t_vector 			v;
	t_vector 			r;
	static const double	size = WORLD_SIZE / 2;
	static const double  thickness = EPSILON;
	
	r = (t_vector){1, 0, 0};
	if (fabs(vec_dot(r, shape->plane.normal)) > 0.99f)
		r = (t_vector){0, 1, 0};
	
	u = vec_cross(r, shape->plane.normal);
	v = vec_cross(u, shape->plane.normal);

	u = vec_normalize(u);
	v = vec_normalize(v);

	const t_vector size_by_v = 
	{
		size * fabs(v.x),
		size * fabs(v.y),
		size * fabs(v.z)
	};
	const t_point size_by_u = 
	{
		size * fabs(u.x),
		size * fabs(u.y),
		size * fabs(u.z)
	};

	t_point bb_min = {
		-size_by_u.x - size_by_v.x,
		-size_by_u.y - size_by_v.y,
		-size_by_u.z - size_by_v.z
	};

	t_point	bb_max = {
		size_by_u.x + size_by_v.x,
		size_by_u.y + size_by_v.y,
		size_by_u.z + size_by_v.z
	};

	// Add a small thickness to the bounding box
	const t_vector	normal_by_thickness = 
	{
		shape->plane.normal.x * thickness,
		shape->plane.normal.y * thickness,
		shape->plane.normal.z * thickness
	};
	
	bb_min.x -= normal_by_thickness.x;
	bb_min.y -= normal_by_thickness.y;
	bb_min.z -= normal_by_thickness.z;

	bb_max.x += normal_by_thickness.x;
	bb_max.y += normal_by_thickness.y;
	bb_max.z += normal_by_thickness.z;

	shape->bb_max = bb_max;
	shape->bb_min = bb_min;
}

//axis aligned bounding box for cylinder
static void set_bb_cylinder(t_shape *shape)
{
	const t_vector	orientation = shape->cylinder.direction;
	const t_vector	center = shape->cylinder.center;
	const double	r = shape->cylinder.radius;
	const double	half_height = shape->cylinder.half_height;
	t_vector 		perp1, perp2;

	if (fabs(orientation.x) < fabs(orientation.y) || fabs(orientation.x) < fabs(orientation.z))
		perp1 = (t_vector){0, -orientation.z, orientation.y};
	else
		perp1 = (t_vector){-orientation.y, orientation.x, 0};

	perp1 = vec_normalize(perp1);
	perp2 = vec_cross(orientation, perp1);

	// Calculate extremes along the cylinder axis
	t_vector axis_max = {
		center.x + orientation.x * half_height,
		center.y + orientation.y * half_height,
		center.z + orientation.z * half_height,
	};

	t_vector axis_min = {
		center.x - orientation.x * half_height,
		center.y - orientation.y * half_height,
		center.z - orientation.z * half_height,
	};

	// Adjust for the radius along the perpendicular directions
	// This considers the cylinder's rotation and its impact on the AABB
	t_vector radius_extents = {
		r * sqrt(perp1.x * perp1.x + perp2.x * perp2.x),
		r * sqrt(perp1.y * perp1.y + perp2.y * perp2.y),
		r * sqrt(perp1.z * perp1.z + perp2.z * perp2.z)
	};

	// Update min and max taking into account the radius in all directions
	t_vector bb_min = {
		fmin(axis_min.x, axis_max.x) - radius_extents.x,
		fmin(axis_min.y, axis_max.y) - radius_extents.y,
		fmin(axis_min.z, axis_max.z) - radius_extents.z,
	};
	t_vector bb_max = {
		fmax(axis_min.x, axis_max.x) + radius_extents.x,
		fmax(axis_min.y, axis_max.y) + radius_extents.y,
		fmax(axis_min.z, axis_max.z) + radius_extents.z,
	};

	shape->bb_min = (t_point)bb_min;
	shape->bb_max = (t_point)bb_max;
}

static void set_bb_cone(t_shape *shape)
{
	const t_vector orientation = shape->cone.direction;
	const t_vector base_center = shape->cone.base_center;
	const double r = shape->cone.radius;
	const double height = shape->cone.height;
	t_vector perp1, perp2;

	t_vector tip = {
		base_center.x + orientation.x * height,
		base_center.y + orientation.y * height,
		base_center.z + orientation.z * height,
	};

	if (fabs(orientation.x) < fabs(orientation.y) || fabs(orientation.x) < fabs(orientation.z))
		perp1 = (t_vector){0, -orientation.z, orientation.y};
	else
		perp1 = (t_vector){-orientation.y, orientation.x, 0};
	perp1 = vec_normalize(perp1);
	perp2 = vec_cross(orientation, perp1);
	t_vector radius_extents = {
		r * sqrt(perp1.x * perp1.x + perp2.x * perp2.x),
		r * sqrt(perp1.y * perp1.y + perp2.y * perp2.y),
		r * sqrt(perp1.z * perp1.z + perp2.z * perp2.z)
	};
	t_vector bb_min = {
		fmin(fmin(base_center.x, tip.x) - radius_extents.x, base_center.x - r),
		fmin(fmin(base_center.y, tip.y) - radius_extents.y, base_center.y - r),
		fmin(fmin(base_center.z, tip.z) - radius_extents.z, base_center.z - r),
	};
	t_vector bb_max = {
		fmax(fmax(base_center.x, tip.x) + radius_extents.x, base_center.x + r),
		fmax(fmax(base_center.y, tip.y) + radius_extents.y, base_center.y + r),
		fmax(fmax(base_center.z, tip.z) + radius_extents.z, base_center.z + r),
	};
	shape->bb_min = (t_point)bb_min;
	shape->bb_max = (t_point)bb_max;
}

static void	set_bb_triangle(t_shape *shape)
{
	const t_vector v0 = shape->triangle.vertices[0];
	const t_vector v1 = shape->triangle.vertices[1];
	const t_vector v2 = shape->triangle.vertices[2];

	t_vector min = v0;
	t_vector max = v0;

	min.x = fmin(min.x, v1.x);
	min.y = fmin(min.y, v1.y);
	min.z = fmin(min.z, v1.z);

	max.x = fmax(max.x, v1.x);
	max.y = fmax(max.y, v1.y);
	max.z = fmax(max.z, v1.z);

	min.x = fmin(min.x, v2.x);
	min.y = fmin(min.y, v2.y);
	min.z = fmin(min.z, v2.z);

	max.x = fmax(max.x, v2.x);
	max.y = fmax(max.y, v2.y);
	max.z = fmax(max.z, v2.z);

	shape->bb_min = (t_point)min;
	shape->bb_max = (t_point)max;
}
