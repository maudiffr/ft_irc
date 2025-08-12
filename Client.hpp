/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 16:06:25 by fquercy           #+#    #+#             */
/*   Updated: 2024/09/10 16:06:25 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Client_hpp
#define Client_hpp

#include "constants.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <unistd.h>

class Client {
	
public:
	// Constructors //
	Client();
	
	// Getters //
	int getFd() const;
	const std::string &getIpAddr() const;
	bool getPassChecked() const;
	bool getIsRegistered() const;
	const std::string &getNickname() const;
	const std::string &getUsername() const;
	const std::string &getHostname() const;
	const std::string &getServername() const;
	const std::string &getRealname() const;
	
	// Setters //
	void setFd(int fd);
	void setIpAddr(std::string ipAddr);
	void setPassChecked(bool isChecked);
	void setIsRegistered(bool isRegistered);
	void setNickname(std::string name);
	void setUsername(std::string name);
	void setHostname(std::string name);
	void setServername(std::string name);
	void setRealname(std::string name);
	
	
private:
	// Attributs //
	int fd;
	std::string ipAddr;
	bool passChecked;
	bool isRegistered;
	std::string nickname;
	std::string username;
	std::string hostname;
	std::string servername;
	std::string realname;
	
};

#endif
