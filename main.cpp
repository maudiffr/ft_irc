/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 15:31:56 by fquercy           #+#    #+#             */
/*   Updated: 2024/07/28 15:31:56 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int ac, char **av) {
	
	Server serv;
	char *endptr;
	long port;
	
	if (ac != 3) {
		std::cerr << "./" << NAME_SERV << " <port> <password>" << std::endl;
		return (1);
	}
	signal(SIGINT, Server::handleSig);
	signal(SIGQUIT, Server::handleSig);
	try {
		port = std::strtol(av[1], &endptr, 10);
		if (*endptr != '\0' || port < 1 || port > 65535)
			throw (std::invalid_argument("Please enter valid port value!"));
		serv.run(std::atoi(av[1]), av[2]);
	} catch (const std::exception &e) {
		serv.closeFds();
		std::cerr << e.what() << std::endl;
	}
	std::cout << "\r                              " << std::flush;
	std::cout << "\r" << "Server closed!" << std::endl;
	return (0);
	
}
