/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yioffe <yioffe@student.42lisboa.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 18:54:49 by yioffe            #+#    #+#             */
/*   Updated: 2024/09/12 10:59:52 by yioffe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/philo.h"

// TODO: remove printf and other not allowed functions
// TODO: remove printf for put up/down fork
// TODO: add limits (60 for time and 200 for philo)
// TODO: fix normi
// TODO: put back Makefile flags
// TODO: put all in philo dir

void philo_print(const char *message, t_philo *philo)
{
    t_config *config = philo->config;
    struct timeval current_time;
	unsigned long long timestamp_in_ms;

    gettimeofday(&current_time, NULL);
    timestamp_in_ms = (current_time.tv_sec * 1000) + (current_time.tv_usec / 1000);

    pthread_mutex_lock(&config->mutex_write);

    ft_putnbr_fd(timestamp_in_ms, STDOUT_FILENO);
    //write(STDOUT_FILENO, " Philosopher ", 13);

	write(STDOUT_FILENO, " ", 1);
    ft_putnbr_fd(philo->id, STDOUT_FILENO);
    write(STDOUT_FILENO, " ", 1);

	while(*message)
	{
    	write(STDOUT_FILENO, message, 1);
		message ++;
	}
    write(STDOUT_FILENO, "\n", 1);

    pthread_mutex_unlock(&config->mutex_write);
}

void philo_sleep(int duration_ms, t_config *config) {
    struct timeval start_time, current_time;
    int elapsed_time_ms = 0;

    gettimeofday(&start_time, NULL);

    while ((elapsed_time_ms < duration_ms) && !check_config_stop(config)) 
    {
        usleep(500);
        gettimeofday(&current_time, NULL);
        elapsed_time_ms = time_diff_in_ms(start_time, current_time);
    }
}

void philo_eat(t_philo *philo)
{
    t_config *config = philo->config;
	struct timeval current_time;

	if (check_config_stop(config))
		return ;
    pthread_mutex_lock(&philo->mutex_eating);
	gettimeofday(&current_time, NULL);
	gettimeofday(&philo->last_eat_time, NULL);
    philo_print("is eating", philo);
    philo_sleep(config->time_to_eat, config);
    increment_eat_counter(philo);
    pthread_mutex_unlock(&philo->mutex_eating);
}


void philo_take_forks_and_eat(t_philo *philo)
{
	t_config	*config;
	
	config = philo->config;

	if (check_config_stop(config))
		return ;
	if (philo->left_fork == philo->right_fork) 
	{
        pthread_mutex_lock(philo->left_fork);
		philo_print("has taken a fork", philo);
		philo_sleep(philo->config->time_to_die, philo->config);
        pthread_mutex_unlock(philo->left_fork);
		return ;
    }
	if (check_config_stop(config))
		return ;
    if (philo->id % 2 == 0) {
        pthread_mutex_lock(philo->left_fork);
        philo_print("has taken a fork", philo);
        if (check_config_stop(config)) 
		{
            pthread_mutex_unlock(philo->left_fork);
            return;
        }
        pthread_mutex_lock(philo->right_fork);
    } else {
        pthread_mutex_lock(philo->right_fork);
        philo_print("has taken a fork", philo);
        if (check_config_stop(config)) {
            pthread_mutex_unlock(philo->right_fork);
            return;
        }
        pthread_mutex_lock(philo->left_fork);
    }
    philo_print("has taken a fork", philo);
    if (!check_config_stop(config))
        philo_eat(philo);
    pthread_mutex_unlock(philo->right_fork);
    pthread_mutex_unlock(philo->left_fork);
}

int	main(int argc, char **argv)
{
	t_config	*config;
	t_philo		*philos;
	pthread_t	*threads;
	int			created_threads;
	pthread_t   monitor_thread;
	int			i;

	config = init_config(argc, argv);
	if (!config)
		return (EXIT_FAILURE);
	philos = initiate_philos(config);
	if (!philos)
		return (free(config), EXIT_FAILURE);
	created_threads = create_threads(&threads, philos, config);
	if (created_threads != EXIT_SUCCESS)
	{
		i = 0;
		if (created_threads > 0)
		{
			while (i < created_threads)
			{
				pthread_join(threads[i], NULL);
				i++;
			}
		}
		cleanup(philos, threads, config);
		return (EXIT_FAILURE);
	}
	if (pthread_create(&monitor_thread, NULL, monitor_routine, config) != 0)
    {
        cleanup(philos, threads, config);
        return (EXIT_FAILURE);
    }
	i = 0;
	while (i < config->number_of_philosophers)
	{
		pthread_join(threads[i], NULL);
		i++;
	}
	pthread_join(monitor_thread, NULL);
	cleanup(philos, threads, config);
	return (EXIT_SUCCESS);
}
