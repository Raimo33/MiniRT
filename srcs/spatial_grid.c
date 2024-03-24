/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spatial_grid.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 16:26:26 by craimond          #+#    #+#             */
/*   Updated: 2024/03/24 19:43:08 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

static void	set_world_extremes(t_grid *grid, t_scene scene);
static void	put_object_in_cells(t_grid *grid, const t_shape shape);

void	init_grid(t_grid *grid, const t_scene scene)
{
	t_vector	world_size;
	uint64_t	n_cells;

	//TODO calculate_optimal_cell_size();
	set_world_extremes(grid, scene);
	//distanza tra i due punti estremi sugli assi
	world_size.x = fabs(grid->world_max.x - grid->world_min.x);
	world_size.y = fabs(grid->world_max.y - grid->world_min.y);
	world_size.z = fabs(grid->world_max.z - grid->world_min.z);
	//numero di celle per ogni asse
	grid->n_cells.x = world_size.x / (float)CELL_SIZE_X + 1;
	grid->n_cells.y = world_size.y / (float)CELL_SIZE_Y + 1;
	grid->n_cells.z = world_size.z / (float)CELL_SIZE_Z + 1;

	n_cells = grid->n_cells.x * grid->n_cells.y * grid->n_cells.z;
	grid->cells = ft_calloc(n_cells + 1, sizeof(t_list *)); //ogni cella ha una lista di oggetti che la occupano
}

void	put_objects_in_grid(t_grid *grid, const t_scene scene)
{
	t_list		*shapes;
	t_shape		*shape;

	shapes = scene.shapes;
	while (shapes)
	{
		shape = shapes->content;
		put_object_in_cell(grid, *shape);
		shapes = shapes->next;
	}
}

static void	set_world_extremes(t_grid *grid, const t_scene scene)
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
	//TODO considerare
	grid->world_min = world_min;
	grid->world_max = world_max;
}

//gets the index of the cell in which the object is located
static void	put_object_in_cells(t_grid *grid, const t_shape shape)
{
	t_uint3		min;
	t_uint3		max;
	uint64_t	cell_index;
	uint16_t	x;
	uint16_t	y;
	uint16_t	z;

	//calcola il range di celle in cui l'oggetto si trova
	min.x = floor((shape.min.x - grid->world_min.x) / CELL_SIZE_X);
	max.x = floor((shape.max.x - grid->world_min.x) / CELL_SIZE_X);
	min.y = floor((shape.min.y - grid->world_min.y) / CELL_SIZE_Y);
	max.y = floor((shape.max.y - grid->world_min.y) / CELL_SIZE_Y);
	min.z = floor((shape.min.z - grid->world_min.z) / CELL_SIZE_Z);
	max.z = floor((shape.max.z - grid->world_min.z) / CELL_SIZE_Z);

	//clamp dei valori per evitare out of bounds
	min.x = fmax(min.x, 0);
	min.y = fmax(min.y, 0);
	min.z = fmax(min.z, 0);
	max.x = fmin(max.x, grid->n_cells.x - 1);
	max.y = fmin(max.y, grid->n_cells.y - 1);
	max.z = fmin(max.z, grid->n_cells.z - 1);

	x = min.x;
	while (x <= max.x)
	{
		y = min.y;
		while (y <= max.y)
		{
			z = min.z;
			while (z <= max.z)
			{
				//aggiunge l'oggetto alla lista di oggetti che occupano una cella
				//un oggetto pue' essere in piu' celle contemporaneamente
				cell_index = x + y * grid->n_cells.x + z * grid->n_cells.x * grid->n_cells.y;
				ft_lstadd_front(&grid->cells[cell_index], ft_lstnew(shape));
				z++;
			}
			y++;
		}
		x++;
	}
}