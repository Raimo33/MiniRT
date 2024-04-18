/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 17:33:18 by craimond          #+#    #+#             */
/*   Updated: 2024/04/18 17:34:32 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

int	main(int argc, char **argv)
{
	t_mlx_data		mlx_data;
	t_scene			scene;
	int				fd;
	double			rendering_time;
	struct timeval	start_time;
	struct timeval	end_time;

	check_args(argc, argv);
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		ft_quit(4, "failed to open scene: no such file or directory\n");
	init_scene(&scene);
	ft_putstr_fd("Parsing scene...", 1);
	parse_scene(fd, &scene);
	setup_scene(&scene);
	init_window(&mlx_data, &scene);
	init_textures(&scene, &mlx_data);
	init_hooks(&mlx_data, &scene);
	gettimeofday(&start_time, NULL);
	render_scene(&mlx_data, &scene);
	gettimeofday(&end_time, NULL);
	rendering_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
	rendering_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
	printf("\nRendered scene in %.2f s\n", rendering_time / 1000.0);
	mlx_loop(mlx_data.mlx);
	ft_quit(0, "Minilibx loop ended abruptly\n");
}
