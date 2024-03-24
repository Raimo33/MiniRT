/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spatial_grid.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 16:26:26 by craimond          #+#    #+#             */
/*   Updated: 2024/03/24 16:59:13 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

static void	set_world_extremes(t_grid *grid, t_scene scene)
{
	t_list		*shapes;
	t_shape		*shape;
	t_vector	world_min;
	t_vector	world_max;

	shapes = scene.shapes;
	shape = shapes->content;
	world_min = shape->center;
	world_max = shape->center;
	while (shapes)
	{
		shape = shapes->content;
		world_min = vec_min(world_min, shape->center);
		world_max = vec_max(world_max, shape->center);
		shapes = shapes->next;
	}
	grid->world_min = world_min;
	grid->world_max = world_max;
}

void	init_grid(t_grid *grid, t_scene scene)
{
	t_vector	world_size;

	set_world_extremes(grid, scene);
	//distanza tra i due punti estremi sugli assi
	world_size.x = fabs(grid->world_max.x - grid->world_min.x);
	world_size.y = fabs(grid->world_max.y - grid->world_min.y);
	world_size.z = fabs(grid->world_max.z - grid->world_min.z);
	//numero di celle per ogni asse
	grid->n_cells_x = world_size.x / (float)CELL_SIZE + 1;
	grid->n_cells_y = world_size.y / (float)CELL_SIZE + 1;
	grid->n_cells_z = world_size.z / (float)CELL_SIZE + 1;

	
}