/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 10:23:38 by fquercy           #+#    #+#             */
/*   Updated: 2024/09/24 10:23:38 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Channel_hpp
#define Channel_hpp

#include "constants.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include "Client.hpp"

class Channel {
	
public:
	Channel();
	
	// Getters //
	std::string getName();
	std::string getPassword();
	std::string getLabel();
	std::string getTopic();
	int getLimit();
	bool getInviteOnly();
	bool getTopicOpOnly();
	std::vector<int> getClients();
	std::vector<int> getOperators();
	std::vector<int> getInvitedClients();
	
	// Setters //
	void setName(std::string name);
	void setPassword(std::string pass);
	void setLabel(std::string label);
	void setTopic(std::string topic);
	void setLimit(int limit);
	void setInviteOnly(bool tag);
	void setTopicOpOnly(bool tag);
	void addClient(int cfd);
	void removeClient(int cfd);
	void addOperator(int cfd);
	void removeOperator(int cfd);
	void addInvitedClient(int cfd);
	void removeInvitedClient(int cfd);
	
private:
	// Attributs //
	std::string name;
	std::string password;
	std::string label;
	std::string topic;
	int limit;
	bool inviteOnly;
	bool topicOpOnly;
	std::vector<int> clients;
	std::vector<int> operators;
	std::vector<int> invitedClients;
	
};

#endif
