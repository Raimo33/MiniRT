/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:25:57 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 15:45:57 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void	first_if(t_octree *node, t_list *shapes,
	t_vector box_top, t_vector box_bottom)
{
	node->children = NULL;
	node->shapes = shapes;
	node->box_top = box_top;
	node->box_bottom = box_bottom;
	node->n_shapes = ft_lstsize(shapes);
}

static void	repeat_eight_times(t_octree *n, t_list *shapes,
	uint8_t d, t_center_size center_size)
{
	uint8_t	i;
	t_point	nb_tb[2];
	t_list	*sib;

	i = -1;
	while (++i < 8)
	{
		nb_tb[0] = center_size.center;
		nb_tb[1] = center_size.center;
		nb_tb[0].x += (i & 1) * center_size.size.x;
		nb_tb[1].x += !(i & 1) * -center_size.size.x;
		nb_tb[0].y += (i & 2) * center_size.size.y;
		nb_tb[1].y += !(i & 2) * -center_size.size.y;
		nb_tb[0].z += (i & 4) * center_size.size.z;
		nb_tb[1].z += !(i & 4) * -center_size.size.z;
		sib = get_shapes_inside_box(shapes, nb_tb[0], nb_tb[1]);
		if (!sib)
		{
			n->children[i] = NULL;
			continue ;
		}
		n->children[i] = (t_octree *)calloc_p(1, sizeof(t_octree));
		n->n_shapes += ft_lstsize(sib);
		fill_octree(n->children[i], sib, d - 1, (t_boxes){nb_tb[0], nb_tb[1]});
	}
}

void	fill_octree(t_octree *node, t_list *shapes,
	uint8_t depth, t_boxes b)
{
	t_point		center;
	t_vector	size;

	if (depth == 0)
	{
		first_if(node, shapes, b.box_top, b.box_bottom);
		return ;
	}
	node->children = (t_octree **)calloc_p(8, sizeof(t_octree *));
	node->box_bottom = b.box_bottom;
	node->box_top = b.box_top;
	node->shapes = shapes;
	node->n_shapes = 0;
	center = (t_vector){(b.box_top.x + b.box_bottom.x) / 2,
		(b.box_top.y + b.box_bottom.y) / 2, (b.box_top.z + b.box_bottom.z) / 2};
	size = (t_vector){(b.box_top.x - b.box_bottom.x) / 2,
		(b.box_top.y - b.box_bottom.y) / 2, (b.box_top.z - b.box_bottom.z) / 2};
	repeat_eight_times(node, shapes, depth, (t_center_size){center, size});
}
