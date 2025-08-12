/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 16:06:40 by fquercy           #+#    #+#             */
/*   Updated: 2024/09/10 16:06:40 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// Constructors //
Client::Client() : passChecked(false), isRegistered(false) {}

// Getters //
int Client::getFd() const { return this->fd; }
const std::string &Client::getIpAddr() const { return this->ipAddr; }
bool Client::getPassChecked() const { return this->passChecked; }
bool Client::getIsRegistered() const { return this->isRegistered; }
const std::string &Client::getNickname() const { return this->nickname; }
const std::string &Client::getUsername() const { return this->username; }
const std::string &Client::getHostname() const { return this->hostname; }
const std::string &Client::getServername() const { return this->servername; }
const std::string &Client::getRealname() const { return this->realname; }

// Setters //
void Client::setFd(int fd) { this->fd = fd; }
void Client::setIpAddr(std::string ipAddr) { this->ipAddr = ipAddr; }
void Client::setPassChecked(bool isChecked) { this->passChecked = isChecked; }
void Client::setIsRegistered(bool isRegistered) { this->isRegistered = isRegistered; }
void Client::setNickname(std::string name) { this->nickname = name; }
void Client::setUsername(std::string name) { this->username = name; }
void Client::setHostname(std::string name) { this->hostname = name; }
void Client::setServername(std::string name) { this->servername = name; }
void Client::setRealname(std::string name) { this->realname = name; }
