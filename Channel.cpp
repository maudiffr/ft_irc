/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 10:25:14 by fquercy           #+#    #+#             */
/*   Updated: 2024/09/24 10:25:14 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// Constructors //
Channel::Channel() {
	
	this->name = "";
	this->password = "";
	this->label = "";
	this->topic = "";
	this->limit = 0;
	this->inviteOnly = false;
	this->topicOpOnly = true;
	
}

// Getters //
std::string Channel::getName() { return this->name; }
std::string Channel::getPassword() { return this->password; }
std::string Channel::getLabel() { return this->label; }
std::string Channel::getTopic() { return this->topic; }
int Channel::getLimit() { return this->limit; }
bool Channel::getInviteOnly() { return this->inviteOnly; }
bool Channel::getTopicOpOnly() { return this->topicOpOnly; }
std::vector<int> Channel::getClients() { return this->clients; }
std::vector<int> Channel::getOperators() { return this->operators; }
std::vector<int> Channel::getInvitedClients() { return this->invitedClients; }

// Setters //
void Channel::setName(std::string name) { this->name = name; }
void Channel::setPassword(std::string pass) { this->password = pass; }
void Channel::setLabel(std::string label) { this->label = label; }
void Channel::setTopic(std::string topic) { this->topic = topic; }
void Channel::setLimit(int limit) { this->limit = limit; }
void Channel::setInviteOnly(bool tag) { this->inviteOnly = tag; }
void Channel::setTopicOpOnly(bool tag) { this->topicOpOnly = tag; }
void Channel::addClient(int cfd) { this->clients.push_back(cfd); }
void Channel::removeClient(int cfd) {
	
	for (std::vector<int>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		if (*it == cfd) {
			this->clients.erase(it);
			return;
		}
	}
	
}
void Channel::addOperator(int cfd) { this->operators.push_back(cfd); }
void Channel::removeOperator(int cfd) {
	
	for (std::vector<int>::iterator it = this->operators.begin(); it != this->operators.end(); ++it) {
		if (*it == cfd) {
			this->operators.erase(it);
			return;
		}
	}
	
}
void Channel::addInvitedClient(int cfd) { this->invitedClients.push_back(cfd); }
void Channel::removeInvitedClient(int cfd) {
	
	for (std::vector<int>::iterator it = this->operators.begin(); it != this->operators.end(); ++it) {
		if (*it == cfd) {
			this->operators.erase(it);
			return;
		}
	}
	
}
