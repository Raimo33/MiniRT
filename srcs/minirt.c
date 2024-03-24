/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 17:33:18 by craimond          #+#    #+#             */
/*   Updated: 2024/03/24 19:51:52 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

int	main(int argc, char **argv)
{
	t_mlx_data	mlx_data;
	t_scene		scene;
	int			fd;

	check_args(argc, argv);
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		ft_quit(4, NULL);
	init_scene(&scene);
	parse_scene(fd, &scene);
	init_grid(&scene.grid, scene);
	put_objects_in_grid(&scene.grid, scene);
	init_window(&mlx_data);
	init_hooks(&mlx_data, scene);
	render(mlx_data, scene);
	mlx_loop(mlx_data.mlx);
}
