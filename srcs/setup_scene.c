/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:25:57 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 18:28:34 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		fill_octree(t_octree *node, t_list *shapes,
					uint8_t depth, t_vector box_top, t_vector box_bottom);
static void		set_world_extremes(t_scene *scene);
static void		set_bounding_box(t_shape *shape);
static void		set_bb_sphere(t_shape *shape);
static void		set_bb_cylinder(t_shape *shape);
static void		set_bb_plane(t_shape *shape);
static void		set_bb_triangle(t_shape *shape);
static void		set_bb_cone(t_shape *shape);
static t_list	*get_shapes_inside_box(t_list *shapes,
					t_vector box_top, t_vector box_bottom);
static void		set_shapes_data(t_scene *scene);

int	boxes_overlap(const t_point box1_top, const t_point box1_bottom, const t_point box2_top, const t_point box2_bottom)
{
	return (box1_bottom.x <= box2_top.x && box1_top.x >= box2_bottom.x
		&& box1_bottom.y <= box2_top.y && box1_top.y >= box2_bottom.y
		&& box1_bottom.z <= box2_top.z && box1_top.z >= box2_bottom.z);
}

void	setup_scene(t_scene *scene)
{
	set_shapes_data(scene);
	set_world_extremes(scene);
	fill_octree(scene->octree, scene->shapes, OCTREE_DEPTH,
		scene->world_max, scene->world_min);
}

static void	set_cylinder_data(t_shape *shape)
{
	t_cylinder	*cylinder;
	t_vector	center_to_cap;

	cylinder = &shape->cylinder;
	cylinder->squared_radius = cylinder->radius * cylinder->radius;
	cylinder->direction = vec_normalize(cylinder->direction);
	cylinder->half_height = cylinder->height / 2;
	center_to_cap = vec_scale(cylinder->half_height,
			cylinder->direction);
	cylinder->top_cap_center = vec_add(cylinder->center, center_to_cap);
	cylinder->bottom_cap_center = vec_sub(cylinder->center,
			center_to_cap);
}

static void	set_cone_data(t_shape *shape)
{
	double	cos_theta;

	cos_theta = cos(atan2(shape->cone.radius, shape->cone.height));
	shape->cone.costheta_squared = cos_theta * cos_theta;
	shape->cone.direction = vec_normalize(shape->cone.direction);
}

static void	set_shapes_data(t_scene *scene)
{
	t_shape		*s;
	t_list		*node;

	node = scene->shapes;
	while (node)
	{
		s = (t_shape *)node->content;
		set_bounding_box(s);
		if (s->e_type == TRIANGLE)
			s->triangle.normal
				= vec_normalize(vec_cross(vec_sub(s->triangle.vertices[1],
							s->triangle.vertices[0]),
						vec_sub(s->triangle.vertices[2],
							s->triangle.vertices[0])));
		else if (s->e_type == SPHERE)
			s->sphere.squared_radius = s->sphere.radius * s->sphere.radius;
		else if (s->e_type == PLANE)
			s->plane.normal = vec_normalize(s->plane.normal);
		else if (s->e_type == CYLINDER)
			set_cylinder_data(s);
		else if (s->e_type == CONE)
			set_cone_data(s);
		node = node->next;
	}
}

static void	fill_octree(t_octree *node, t_list *shapes,
	uint8_t depth, t_vector box_top, t_vector box_bottom)
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
	center = (t_vector){(box_top.x + box_bottom.x) / 2,
		(box_top.y + box_bottom.y) / 2, (box_top.z + box_bottom.z) / 2};
	size = (t_vector){(box_top.x - box_bottom.x) / 2,
		(box_top.y - box_bottom.y) / 2, (box_top.z - box_bottom.z) / 2};
	i = -1;
	while (++i < 8)
	{
		new_box_top = center;
		new_box_bottom = center;
		new_box_top.x += (i & 1) * size.x;
		new_box_bottom.x += !(i & 1) * -size.x;
		new_box_top.y += (i & 2) * size.y;
		new_box_bottom.y += !(i & 2) * -size.y;
		new_box_top.z += (i & 4) * size.z;
		new_box_bottom.z += !(i & 4) * -size.z;
		shapes_inside_box = get_shapes_inside_box(shapes,
				new_box_top, new_box_bottom);
		if (!shapes_inside_box)
		{
			node->children[i] = NULL;
			continue ;
		}
		node->children[i] = (t_octree *)calloc_p(1, sizeof(t_octree));
		node->n_shapes += ft_lstsize(shapes_inside_box);
		fill_octree(node->children[i],
			shapes_inside_box, depth - 1, new_box_top, new_box_bottom);
	}
}

static t_list	*get_shapes_inside_box(t_list *shapes, t_point box_top, t_point box_bottom)
{
	t_list	*inside_shapes;
	t_shape	*current_shape;

	inside_shapes = NULL;
	while (shapes)
	{
		current_shape = (t_shape *)shapes->content;
		if (boxes_overlap(box_top, box_bottom,
				current_shape->bb_max, current_shape->bb_min))
			ft_lstadd_front(&inside_shapes, ft_lstnew(current_shape));
		shapes = shapes->next;
	}
	return (inside_shapes);
}

static void	set_world_extremes(t_scene *scene)
{
	t_list		*shapes;
	t_shape		*shape;
	t_point		world_max;
	t_point		world_min;

	world_max = (t_point){0, 0, 0};
	world_min = (t_point){0, 0, 0};
	shapes = scene->shapes;
	if (!shapes)
		return ;
	shape = (t_shape *)shapes->content;
	while (shapes)
	{
		shape = shapes->content;
		world_max = (t_point){fmax(world_max.x, shape->bb_max.x),
			fmax(world_max.y, shape->bb_max.y),
			fmax(world_max.z, shape->bb_max.z)};
		world_min = (t_point){fmin(world_min.x, shape->bb_min.x),
			fmin(world_min.y, shape->bb_min.y),
			fmin(world_min.z, shape->bb_min.z)};
		shapes = shapes->next;
	}
	scene->world_min = world_min;
	scene->world_max = world_max;
}

static void	set_bounding_box(t_shape *shape)
{
	static void (*const	get_bb_funcs[])(t_shape *)
		= {&set_bb_sphere, &set_bb_cylinder,
		&set_bb_triangle, &set_bb_cone, &set_bb_plane};
	const uint8_t		n_shapes
		= sizeof(get_bb_funcs) / sizeof(get_bb_funcs[0]);
	uint8_t				i;

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

static void	set_bb_sphere(t_shape *shape)
{
	shape->bb_min = (t_point){shape->sphere.center.x - shape->sphere.radius,
		shape->sphere.center.y - shape->sphere.radius,
		shape->sphere.center.z - shape->sphere.radius};
	shape->bb_max = (t_point){shape->sphere.center.x + shape->sphere.radius,
		shape->sphere.center.y + shape->sphere.radius,
		shape->sphere.center.z + shape->sphere.radius};
}

static void set_bb_plane(t_shape *shape)
{
	t_vector			u;
	t_vector			v;
	t_vector			r;
	static const double	size = WORLD_SIZE / 2;
	t_vector			size_by_v;
	t_point				size_by_u;
	t_point				bb_min;
	t_point				bb_max;
	t_vector			normal_by_thickness;

	r = (t_vector){1, 0, 0};
	if (fabs(vec_dot(r, shape->plane.normal)) > 0.99f)
		r = (t_vector){0, 1, 0};
	u = vec_cross(r, shape->plane.normal);
	v = vec_cross(u, shape->plane.normal);
	u = vec_normalize(u);
	v = vec_normalize(v);
	size_by_v.x = size * fabs(v.x);
	size_by_v.y = size * fabs(v.y);
	size_by_v.z = size * fabs(v.z);
	size_by_u.x = size * fabs(u.x);
	size_by_u.y = size * fabs(u.y);
	size_by_u.z = size * fabs(u.z);
	bb_min.x = -size_by_u.x - size_by_v.x;
	bb_min.y = -size_by_u.y - size_by_v.y;
	bb_min.z = -size_by_u.z - size_by_v.z;
	bb_max.x = size_by_u.x + size_by_v.x;
	bb_max.y = size_by_u.y + size_by_v.y;
	bb_max.z = size_by_u.z + size_by_v.z;
	normal_by_thickness.x = shape->plane.normal.x * EPSILON;
	normal_by_thickness.y = shape->plane.normal.y * EPSILON;
	normal_by_thickness.z = shape->plane.normal.z * EPSILON;
	bb_min.x -= normal_by_thickness.x;
	bb_min.y -= normal_by_thickness.y;
	bb_min.z -= normal_by_thickness.z;
	bb_max.x += normal_by_thickness.x;
	bb_max.y += normal_by_thickness.y;
	bb_max.z += normal_by_thickness.z;
	shape->bb_max = bb_max;
	shape->bb_min = bb_min;
}

static void	set_bb_cylinder(t_shape *shape)
{
	const t_vector	orientation = shape->cylinder.direction;
	const t_vector	center = shape->cylinder.center;
	const double	r = shape->cylinder.radius;
	const double	half_height = shape->cylinder.half_height;
	t_vector		perp1;
	t_vector		perp2;
	t_vector		axis_max;
	t_vector		axis_min;
	t_vector		radius_extents;
	t_point			bb_min;
	t_point			bb_max;

	if (fabs(orientation.x) < fabs(orientation.y)
		|| fabs(orientation.x) < fabs(orientation.z))
		perp1 = (t_vector){0, -orientation.z, orientation.y};
	else
		perp1 = (t_vector){-orientation.y, orientation.x, 0};
	perp1 = vec_normalize(perp1);
	perp2 = vec_cross(orientation, perp1);
	axis_max.x = center.x + orientation.x * half_height;
	axis_max.y = center.y + orientation.y * half_height;
	axis_max.z = center.z + orientation.z * half_height;
	axis_min.x = center.x - orientation.x * half_height;
	axis_min.y = center.y - orientation.y * half_height;
	axis_min.z = center.z - orientation.z * half_height;
	radius_extents.x = r * sqrt(perp1.x * perp1.x + perp2.x * perp2.x);
	radius_extents.y = r * sqrt(perp1.y * perp1.y + perp2.y * perp2.y);
	radius_extents.z = r * sqrt(perp1.z * perp1.z + perp2.z * perp2.z);
	bb_min.x = fmin(axis_min.x, axis_max.x) - radius_extents.x;
	bb_min.y = fmin(axis_min.y, axis_max.y) - radius_extents.y;
	bb_min.z = fmin(axis_min.z, axis_max.z) - radius_extents.z;
	bb_max.x = fmax(axis_min.x, axis_max.x) + radius_extents.x;
	bb_max.y = fmax(axis_min.y, axis_max.y) + radius_extents.y;
	bb_max.z = fmax(axis_min.z, axis_max.z) + radius_extents.z;
	shape->bb_min = (t_point)bb_min;
	shape->bb_max = (t_point)bb_max;
}

static void	set_bb_cone(t_shape *shape)
{
	const t_vector	orientation = shape->cone.direction;
	const t_vector	b_center = shape->cone.base_center;
	const double	r = shape->cone.radius;
	const double	height = shape->cone.height;
	t_vector		perp1;
	t_vector		perp2;
	t_vector		tip;
	t_vector		r_ext;
	t_vector		bb_min;
	t_vector		bb_max;

	tip.x = b_center.x + orientation.x * height;
	tip.y = b_center.y + orientation.y * height;
	tip.z = b_center.z + orientation.z * height;
	if (fabs(orientation.x) < fabs(orientation.y)
		|| fabs(orientation.x) < fabs(orientation.z))
		perp1 = (t_vector){0, -orientation.z, orientation.y};
	else
		perp1 = (t_vector){-orientation.y, orientation.x, 0};
	perp1 = vec_normalize(perp1);
	perp2 = vec_cross(orientation, perp1);
	r_ext.x = r * sqrt(perp1.x * perp1.x + perp2.x * perp2.x);
	r_ext.y = r * sqrt(perp1.y * perp1.y + perp2.y * perp2.y);
	r_ext.z = r * sqrt(perp1.z * perp1.z + perp2.z * perp2.z);
	bb_min.x = fmin(fmin(b_center.x, tip.x) - r_ext.x, b_center.x - r);
	bb_min.y = fmin(fmin(b_center.y, tip.y) - r_ext.y, b_center.y - r);
	bb_min.z = fmin(fmin(b_center.z, tip.z) - r_ext.z, b_center.z - r);
	bb_max.x = fmax(fmax(b_center.x, tip.x) + r_ext.x, b_center.x + r);
	bb_max.y = fmax(fmax(b_center.y, tip.y) + r_ext.y, b_center.y + r);
	bb_max.z = fmax(fmax(b_center.z, tip.z) + r_ext.z, b_center.z + r);
	shape->bb_min = (t_point)bb_min;
	shape->bb_max = (t_point)bb_max;
}

static void	set_bb_triangle(t_shape *shape)
{
	const t_vector	v0 = shape->triangle.vertices[0];
	const t_vector	v1 = shape->triangle.vertices[1];
	const t_vector	v2 = shape->triangle.vertices[2];
	t_vector		min;
	t_vector		max;

	min = v0;
	max = v0;
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
