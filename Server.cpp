/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 16:06:37 by fquercy           #+#    #+#             */
/*   Updated: 2024/09/10 16:06:37 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// Signals //
bool Server::sig = false;
void Server::handleSig(int sig) {
	
	(void)sig;
	Server::sig = true;
	
}

// Inits //
void Server::init() {
	
	struct sockaddr_in addr;
	struct pollfd pfd;
	int i = 1;
	char buffer[256];
	
	std::time_t now = std::time(0);
	std::tm* localTime = std::localtime(&now);
	std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localTime);
	this->brutDatetime = now;
	this->datetime = buffer;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->getPort());
	addr.sin_addr.s_addr = INADDR_ANY;
	
	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
		throw (std::runtime_error("Creation socket failed!"));
	if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1)
		throw (std::runtime_error("Set option to socket failed!"));
	if (fcntl(this->fd, F_SETFL, O_NONBLOCK) == -1)
		throw (std::runtime_error("Non block option on socket failed!"));
	if (bind(this->fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw (std::runtime_error("Binding socket failed!"));
	if (listen(this->fd, SOMAXCONN) == -1)
		throw (std::runtime_error("Listen socket failed!"));
	
	pfd.fd = this->fd;
	pfd.events = POLLIN;
	this->pfds.push_back(pfd);
	std::cout << "ircserv opened on port " << ntohs(addr.sin_port) << "!" << std::endl;
}
void Server::run(int port, std::string password) {
	
	this->port = port;
	this->password = password;
	
	this->init();
	while (Server::sig == false) {
		std::cout << "\r                              " << std::flush;
		if (this->clients.size() <= 0)
			std::cout << "\rWaiting connection..." << std::flush;
		else
			std::cout << "\rClients connected: " << this->clients.size() << std::flush;
		if ((poll(this->pfds.data(), static_cast<nfds_t>(this->pfds.size()), -1) < 0) && Server::sig == false)
			throw (std::runtime_error("poll error!"));
		if (this->pfds[0].revents & POLLIN)
			acceptClient();
		for (size_t i = 1; i < this->pfds.size(); i++) {
			if (this->pfds[i].revents & POLLIN) {
				this->getClientData(this->pfds[i].fd);
			}
		}
		
	}
	this->closeFds();
	
}

// Fd //
void Server::closeFds() {
	
	for (size_t i = 0; i < this->clients.size(); i++)
		close(this->clients[i].getFd());
	if (this->fd != -1)
		close(this->fd);
	
}

// Getters //
int Server::getPort() const { return (this->port); }
const std::string &Server::getPassword() const { return (this->password); }
int Server::getClientIndex(int fd) {
	
	int res = 0;
	
	for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i].getFd() == fd) {
			res = static_cast<int>(i);
			break;
		}
	}
	return (res);
	
}
void Server::getClientData(int cfd) {
	
	ssize_t r;
	char buff[1024];
	std::vector<std::vector<std::string> > cmds;
	
	r = recv(cfd, buff, sizeof(buff) - 1, 0);
	if (r <= 0) {
		deleteClient(cfd);
	} else {
		buff[r] = '\0';
		cmds = parseBuff(buff);
		Client &client = *getClientByFd(cfd);
		for (size_t i = 0; i < cmds.size(); i++) {
			std::vector<std::string> cmd = cmds[i];
			if (cmd.front() == PASS || cmd.front() == NICK || cmd.front() == USER) {
				AuthClient(cmd, cfd);
				continue;
			}
			if (client.getIsRegistered()) {
				if (cmd.front() == JOIN)
					joinCmd(cmd, cfd);
				else if (cmd.front() == PART)
					partCmd(cmd, cfd);
				else if (cmd.front() == PRIVMSG)
					privmsgCmd(cmd, cfd);
				else if (cmd.front() == QUIT)
					quitCmd(cmd, cfd);
				else if (cmd.front() == KICK)
					kickCmd(cmd, cfd);
				else if (cmd.front() == INVITE)
					inviteCmd(cmd, cfd);
				else if (cmd.front() == TOPIC)
					topicCmd(cmd, cfd);
				else if (cmd.front() == MODE)
					modeCmd(cmd, cfd);
				else if (cmd.front() == LIST)
					sendChannelsList(cfd);
			} else
				errNotRegistered(client);
		}
	}
	
}
Client *Server::getClientByFd(int cfd) {
	
	Client *res = NULL;
	for (size_t i = 0; i < this->clients.size(); i++) {
		if (this->clients[i].getFd() == cfd) {
			res = &this->clients[i];
			return (res);
		}
	}
	return (res);
	
}
Client *Server::getClientByName(std::string &name) {
	
	Client *res = NULL;
	for (size_t i = 0; i < this->clients.size(); i++) {
		if (this->clients[i].getNickname() == name) {
			res = &this->clients[i];
			return (res);
		}
	}
	return (res);
	
}
Channel *Server::getChannelByName(std::string &name) {
	
	Channel *res = NULL;
	for (size_t i = 0; i < this->channels.size(); i++) {
		if (this->channels[i].getName() == name) {
			res = &this->channels[i];
			return (res);
		}
	}
	return (res);
	
}
std::string Server::getModeMsg(Client &client, Channel &channel, std::string mode, std::string param) {
	
	std::string msg;
	if (param == "") {
		msg = ":" + client.getNickname() + " MODE " + channel.getName() + " " + mode + CRLF;
	} else {
		msg = ":" + client.getNickname() + " MODE " + channel.getName() + " " + mode + " " + param + CRLF;
	}
	return (msg);
	
}

// Channels //
void Server::createChannel(std::string name, std::string pass, int cfd) {
	
	Channel channel;
	Client *client = getClientByFd(cfd);
	
	channel.setName(name);
	channel.setPassword(pass);
	channel.addClient(cfd);
	channel.addOperator(cfd);
	this->channels.push_back(channel);
	
	std::string joinMsg = ":" + client->getNickname() + " JOIN " + channel.getName() + CRLF;
	send(cfd, joinMsg.c_str(), joinMsg.length(), 0);
	std::string lstNamesMsg = ":" + std::string(NAME_SERV) + " 353 " + client->getNickname() + " = " + channel.getName() + " :" + chanClientsToStr(channel) + CRLF;
	send(cfd, lstNamesMsg.c_str(), lstNamesMsg.length(), 0);
	std::string endNamesMsg = ":" + std::string(NAME_SERV) + " 366 " + client->getNickname() + " " + channel.getName() + " :End of /NAMES list" + CRLF;
	send(cfd, endNamesMsg.c_str(), endNamesMsg.length(), 0);
	
}
void Server::deleteChannel(Channel &channel) {
	
	for (std::vector<Channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
		if (it->getName() == channel.getName()) {
			this->channels.erase(it);
			return;
		}
	}
	
}

// Clients //
void Server::deleteClient(int cfd) {
	
	for (std::vector<Channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
		it->removeClient(cfd);
		it->removeOperator(cfd);
		it->removeInvitedClient(cfd);
		if (it->getClients().size() == 0) {
			deleteChannel(*it);
			it--;
		}
	}
	
	for (size_t i = 0; i < this->pfds.size(); i++) {
		if (this->pfds[i].fd == cfd) {
			this->pfds.erase(this->pfds.begin() + i);
			break;
		}
	}
	
	for (size_t i = 0; i < this->clients.size(); i++) {
		if (this->clients[i].getFd() == cfd) {
			clients.erase(this->clients.begin() + i);
			break;
		}
	}
	close(cfd);
	
}
void Server::acceptClient() {
	
	Client client;
	int tmpFd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	struct pollfd pfd;
	
	tmpFd = accept(this->fd, (sockaddr *)&addr, &len);
	if (tmpFd == -1) {
		std::cerr << "Accept client failed!" << std::endl;
		return;
	}
	if (fcntl(tmpFd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "set non-block to client failed!" << std::endl;
		return;
	}
	
	pfd.fd = tmpFd;
	pfd.events = POLLIN;
	
	client.setFd(tmpFd);
	client.setIpAddr(inet_ntoa((addr.sin_addr)));
	this->clients.push_back(client);
	this->pfds.push_back(pfd);
	
}
void Server::AuthClient(std::vector<std::string> cmd, int cfd) {
	
	if (cmd.front() == "PASS")
		this->checkPassCmd(cmd, cfd);
	else if (cmd.front() == "NICK")
		this->checkNickCmd(cmd, cfd);
	else if (cmd.front() == "USER")
		this->checkUserCmd(cmd, cfd);
	
}
bool Server::checkRegistered(Client &client, int mark) {
	
	if (!client.getPassChecked()) {
		errNotRegistered(client);
		return (false);
	}
	if (mark == 1) {
		if (client.getNickname() == "" || client.getUsername() == "" || client.getHostname() == "" || client.getServername() == "" || client.getRealname() == "")
			return (false);
	}
	return (true);
}

// Commands //
void Server::checkPassCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = this->clients[this->getClientIndex(cfd)];
	
	if (cmd.size() == 1) {
		// Send ERR_NEEDMOREPARAMS
		std::string res = ":" + std::string(NAME_SERV) + " 461 PASS :Not enough parameters" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
		return;
	}
	if (cmd[1] == this->password && !client.getPassChecked())
		// Good password!
		client.setPassChecked(true);
	else if (client.getPassChecked()) {
		// Send ERR_ALREADYREGISTRED
		std::string res = ":" + std::string(NAME_SERV) + " 462 :Unauthorized command (already registered)" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
	} else {
		// Send ERR_PASSWDMISMATCH
		std::string res = ":" + std::string(NAME_SERV) + " 464 :Password incorrect" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
		deleteClient(cfd);
	}
	
}
void Server::checkNickCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = this->clients[this->getClientIndex(cfd)];
	
	if (!this->checkRegistered(client, 0))
		return;
	if (cmd.size() == 1) {
		// Send ERR_NEEDMOREPARAMS
		std::string res = ":" + std::string(NAME_SERV) + " 461 NICK :Not enough parameters" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
		return;
	}
	if (cmd[1].length() == 0) {
		// Send ERR_NONICKNAMEGIVEN
		std::string res = ":" + std::string(NAME_SERV) + " 431 :No nickname given" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
		deleteClient(cfd);
	} else if (!isGoodNickname(cmd[1])) {
		// Send ERR_ERRONEUSNICKNAME
		std::string res = ":" + std::string(NAME_SERV) + " 432 " + cmd[1] + " :Erroneous nickname" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
	} else if (nicknameAlreadyUsed(cmd[1])) {
		// Send ERR_NICKNAMEINUSE
		std::string res = ":" + std::string(NAME_SERV) + " 433 " + cmd[1] + " :Nickname is already in use" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
	} else {
		if (client.getIsRegistered()) {
			std::string res = ":" + client.getNickname() + " NICK :" + cmd[1] + CRLF;
			for (size_t i = 0; i < this->clients.size(); i++) {
				if (this->clients[i].getIsRegistered())
					send(this->clients[i].getFd(), res.c_str(), res.length(), 0);
			}
		}
		client.setNickname(cmd[1]);
	}
	
}
void Server::checkUserCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = this->clients[this->getClientIndex(cfd)];
	
	if (!this->checkRegistered(client, 0))
		return;
	if (client.getNickname().length() == 0) {
		// Send ERR_NOTREGISTERED
		std::string res = ":" + std::string(NAME_SERV) + " 451 :You have not registered" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
		return ;
	}
	if (cmd.size() < 5) {
		// Send ERR_NEEDMOREPARAMS
		std::string res = ":" + std::string(NAME_SERV) + " 461 USER :Not enough parameters" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
		return;
	}
	for (size_t i = 0; i < cmd.size(); i++) {
		if (cmd[i].length() == 0) {
			// Send ERR_NEEDMOREPARAMS
			std::string res = ":" + std::string(NAME_SERV) + " 461 USER :Not enough parameters" + CRLF;
			send(cfd, res.c_str(), res.length(), 0);
			return;
		}
	}
	if (client.getUsername().length() > 0) {
		// Send ERR_ALREADYREGISTRED
		std::string res = ":" + std::string(NAME_SERV) + " 462 :Unauthorized command (already registered)" + CRLF;
		send(cfd, res.c_str(), res.length(), 0);
	} else {
		client.setUsername(cmd[1]);
		client.setHostname(client.getIpAddr());
		client.setServername(NAME_SERV);
		client.setRealname(cmd[4]);
		if (!client.getIsRegistered()) {
			std::string res = ":" + std::string(NAME_SERV) + " 001 " + client.getNickname() + " :Welcome to the " + NAME_SERV + " Network, " + client.getNickname() + CRLF;
			send(cfd, res.c_str(), res.length(), 0);
			res = ":" + std::string(NAME_SERV) + " 002 " + client.getNickname() + " :Your host is " + client.getIpAddr() + ", running version 1.0" + CRLF;
			send(cfd, res.c_str(), res.length(), 0);
			res = ":" + std::string(NAME_SERV) + " 003 " + client.getNickname() + " :This server was created " + this->datetime + CRLF;
			send(cfd, res.c_str(), res.length(), 0);
			res = ":" + std::string(NAME_SERV) + " 004 " + client.getNickname() + " " + NAME_SERV + " 1.0 itkol" + CRLF;
			send(cfd, res.c_str(), res.length(), 0);
			client.setIsRegistered(true);
		}
	}
	 
}
void Server::joinCmd(std::vector<std::string> cmd, int cfd) {
	
	Client *client = getClientByFd(cfd);
	
	if (cmd.size() == 1)
		return (errNeedMoreParams(*client, cmd, "Not enough parameters"));
	std::map<std::string, std::string> channels = parseJoinCmd(cmd);
	for (std::map<std::string, std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
		std::string nameChan = it->first;
		std::string passChan = it->second;
		
		if (errBadChanMask(nameChan, cfd))
			continue;
		Channel *channel = getChannelByName(nameChan);
		if (!channel) {
			createChannel(nameChan, passChan, cfd);
			continue;
		}
		if (errInviteOnlyChan(channel, cfd))
			continue;
		if (errChannelIsFull(channel, cfd))
			continue;
		if (errBadChannelKey(channel, passChan, cfd))
			continue;
		channel->addClient(cfd);
		for (size_t i = 0; i < channel->getClients().size(); i++) {
			std::string joinMsg = ":" + client->getNickname() + " JOIN " + channel->getName() + CRLF;
			send(channel->getClients()[i], joinMsg.c_str(), joinMsg.length(), 0);
		}
		if (channel->getTopic().length() > 0) {
			std::string topicMsg = ":" + std::string(NAME_SERV) + " 332 " + client->getNickname() + " " + channel->getName() + " :" + channel->getTopic() + CRLF;
			send(cfd, topicMsg.c_str(), topicMsg.length(), 0);
		}
		std::string lstNamesMsg = ":" + std::string(NAME_SERV) + " 353 " + client->getNickname() + " = " + channel->getName() + " :" + chanClientsToStr(*channel) + CRLF;
		send(cfd, lstNamesMsg.c_str(), lstNamesMsg.length(), 0);
		std::string endNamesMsg = ":" + std::string(NAME_SERV) + " 366 " + client->getNickname() + " " + channel->getName() + " :End of /NAMES list" + CRLF;
		send(cfd, endNamesMsg.c_str(), endNamesMsg.length(), 0);
	}
	
}
void Server::partCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::stringstream sstream;
	std::string tmp;
	std::vector<std::string> channels;
	
	if (cmd.size() == 1)
		return (errNeedMoreParams(client, cmd, "Not enough parameters"));
	sstream.str(cmd[1]);
	while (std::getline(sstream, tmp, ','))
		channels.push_back(tmp);
	for (size_t i = 0; i < channels.size(); i++) {
		if (errNoSuchChannel(client, channels[i]))
			continue;
		if (errNotOnChannel(client, channels[i]))
			continue;
		Channel &channel = *getChannelByName(channels[i]);
		for (size_t j = 0; j < channel.getClients().size(); j++) {
			std::string msg;
			if (cmd.size() >= 3) {
				msg = ":" + client.getNickname() + " PART " + channels[i] + " " + cmd[2] + CRLF;
			} else {
				msg = ":" + client.getNickname() + " PART " + channels[i] + CRLF;
			}
			send(channel.getClients()[j], msg.c_str(), msg.length(), 0);
		}
		channel.removeClient(cfd);
		channel.removeOperator(cfd);
		if (channel.getClients().empty())
			deleteChannel(channel);
	}
	
}
void Server::privmsgCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::stringstream sstream;
	std::string tmp;
	std::vector<std::string> targets;
	
	if (cmd.size() == 1)
		return (errNeedMoreParams(client, cmd, "Not enough parameters"));
	sstream.str(cmd[1]);
	while (std::getline(sstream, tmp, ','))
		targets.push_back(tmp);
	for (size_t i = 0; i < targets.size(); i++) {
		bool isChannel = (targets[i][0] == '#' || targets[i][0] == '&');
		if (isChannel) {
			if (errNoSuchChannel(client, targets[i]))
				continue;
			if (errNotOnChannel(client, targets[i]))
				continue;
		} else {
			if (errNoSuchNick(client, targets[i]))
				continue;
		}
		if (cmd.size() < 3 || cmd[2] == "") {
			std::string msg = ":" + std::string(NAME_SERV) + " 412 " + client.getNickname() + " :No text to send" + CRLF;
			send(client.getFd(), msg.c_str(), msg.length(), 0);
		}
		std::string msg = ":" + client.getNickname() + " PRIVMSG " + targets[i] + " :" + cmd[2] + CRLF;
		if (isChannel) {
			Channel &channel = *getChannelByName(targets[i]);
			for (size_t j = 0; j < channel.getClients().size(); j++) {
				if (channel.getClients()[j] != cfd)
					send(channel.getClients()[j], msg.c_str(), msg.length(), 0);
			}
		} else {
			for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
				if (it->getNickname() == targets[i] && it->getNickname() != client.getNickname()) {
					send(it->getFd(), msg.c_str(), msg.length(), 0);
					break;
				}
			}
		}
	}
	
}
void Server::quitCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::string msg;
	
	if (cmd.size() == 1)
		msg = ":" + client.getNickname() + " QUIT " + ":Goodbye!" + CRLF;
	else
		msg = ":" + client.getNickname() + " QUIT " + ":" + cmd[1] + CRLF;
	for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		if (it->getIsRegistered())
			send(it->getFd(), msg.c_str(), msg.length(), 0);
	}
	deleteClient(cfd);
	
}
void Server::kickCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::vector<std::string> chanNames;
	std::vector<std::string> cliNames;
	std::stringstream sstream;
	std::string tmp;
	
	if (cmd.size() < 3)
		return (errNeedMoreParams(client, cmd, "Not enough parameters"));
	sstream.str(cmd[1]);
	while (std::getline(sstream, tmp, ','))
		chanNames.push_back(tmp);
	sstream.clear();
	sstream.str(cmd[2]);
	while (std::getline(sstream, tmp, ','))
		cliNames.push_back(tmp);
	for (std::vector<std::string>::iterator chanIt = chanNames.begin(); chanIt != chanNames.end(); ++chanIt) {
		if (errNoSuchChannel(client, *chanIt))
			continue;
		if (errNotOnChannel(client, *chanIt))
			continue;
		Channel &channel = *getChannelByName(*chanIt);
		if (errChanOprivsNeeded(client, channel))
			continue;
		for (std::vector<std::string>::iterator cliIt = cliNames.begin(); cliIt != cliNames.end(); ++cliIt) {
			if (errUserNotInChannel(client, channel, *cliIt))
				continue;
			Client &user = *getClientByName(*cliIt);
			std::string msg;
			if (cmd.size() >= 4)
				msg = ":" + client.getNickname() + " KICK " + channel.getName() + " " + user.getNickname() + " :" + cmd[3] + CRLF;
			else
				msg = ":" + client.getNickname() + " KICK " + channel.getName() + " " + user.getNickname() + " :Kicked of channel" + CRLF;
			for (size_t i = 0; i < channel.getClients().size(); i++)
				send(channel.getClients()[i], msg.c_str(), msg.length(), 0);
			channel.removeClient(user.getFd());
			channel.removeOperator(user.getFd());
			channel.removeInvitedClient(user.getFd());
			if (channel.getClients().size() == 0) {
				deleteChannel(channel);
				break;
			}
		}
	}
	
}
void Server::inviteCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	
	if (cmd.size() < 3)
		return (errNeedMoreParams(client, cmd, "Not enough parameters"));
	if (errNoSuchNick(client, cmd[1]))
		return;
	if (errNoSuchChannel(client, cmd[2]))
		return;
	if (errNotOnChannel(client, cmd[2]))
		return;
	Channel &channel = *getChannelByName(cmd[2]);
	if (errChanOprivsNeeded(client, channel))
		return;
	Client &user = *getClientByName(cmd[1]);
	if (errUserOnChannel(client, user, channel))
		return;
	std::string msg = ":" + std::string(NAME_SERV) + " 341 " + client.getNickname() + " " + user.getNickname() + " " + channel.getName() + CRLF;
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	msg = ":" + client.getNickname() + " INVITE " + user.getNickname() + " " + channel.getName() + CRLF;
	std::vector<int> chanClients = channel.getClients();
	std::vector<int> chanInvitedClients = channel.getInvitedClients();
	send(user.getFd(), msg.c_str(), msg.length(), 0);
	for (std::vector<int>::iterator it = chanInvitedClients.begin(); it != chanInvitedClients.end(); ++it) {
		if (*it == user.getFd())
			return;
	}
	channel.addInvitedClient(user.getFd());
	
}
void Server::topicCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::string msg;
	
	if (cmd.size() < 2)
		return (errNeedMoreParams(client, cmd, "Not enough parameters"));
	if (errNoSuchChannel(client, cmd[1]))
		return;
	if (errNotOnChannel(client, cmd[1]))
		return;
	Channel &channel = *getChannelByName(cmd[1]);
	if (channel.getTopicOpOnly()) {
		if (errChanOprivsNeeded(client, channel))
			return;
	}
	if (cmd.size() == 2 || cmd[2] == "") {
		msg = ":" + std::string(NAME_SERV) + " 331 " + client.getNickname() + " " + channel.getName() + " :No topic is set" + CRLF;
		send(client.getFd(), msg.c_str(), msg.length(), 0);
		channel.setTopic("");
	} else {
		msg = ":" + std::string(NAME_SERV) + " 332 " + client.getNickname() + " " + channel.getName() + " :" + cmd[2] + CRLF;
		send(client.getFd(), msg.c_str(), msg.length(), 0);
		channel.setTopic(cmd[2]);
	}
	msg = ":" + client.getNickname() + " TOPIC " + channel.getName() + " :" + channel.getTopic() + CRLF;
	std::vector<int> chanClients = channel.getClients();
	for (std::vector<int>::iterator it = chanClients.begin(); it != chanClients.end(); ++it)
		send(*it, msg.c_str(), msg.length(), 0);
	
}
void Server::modeCmd(std::vector<std::string> cmd, int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::map<std::string, std::string> modes;
	
	if (cmd.size() == 1)
		return (errNeedMoreParams(client, cmd, "Not enough parameters"));
	if (errNoSuchChannel(client, cmd[1]))
		return;
	if (errNotOnChannel(client, cmd[1]))
		return;
	Channel &channel = *getChannelByName(cmd[1]);
	if (errChanOprivsNeeded(client, channel))
		return;
	if (cmd.size() == 2)
		onlyChannelCmd(client, channel);
	if (!parseModeCmd(client, cmd, modes))
		return;
	for (std::map<std::string, std::string>::iterator it = modes.begin(); it != modes.end(); ++it) {
		std::string mode = it->first;
		std::string param = it->second;
		std::string msg;
		std::vector<int> chanClients = channel.getClients();
		
		if (mode[0] == '+') {
			if (mode[1] == 'i')
				channel.setInviteOnly(true);
			else if (mode[1] == 't')
				channel.setTopicOpOnly(true);
			else if (mode[1] == 'k') {
				if (param != "")
					channel.setPassword(param);
				else {
					errNeedMoreParams(client, cmd, "Password not given");
					continue;
				}
			}
			else if (mode[1] == 'o') {
				if (!opModeCmd(client, channel, cmd, mode, param))
					continue;
			} else if (mode[1] == 'l') {
				if (!limitModeCmd(client, channel, cmd, mode, param))
					continue;
			}
		} else if (mode[0] == '-') {
			if (mode[1] == 'i')
				channel.setInviteOnly(false);
			else if (mode[1] == 't')
				channel.setTopicOpOnly(false);
			else if (mode[1] == 'k')
				channel.setPassword("");
			else if (mode[1] == 'o') {
				if (!opModeCmd(client, channel, cmd, mode, param))
					continue;
			} else if (mode[1] == 'l') {
				if (!limitModeCmd(client, channel, cmd, mode, param))
					continue;
			}
		}
		if (mode == "-o" && param == "")
			msg = getModeMsg(client, channel, mode, client.getNickname());
		else
			msg = getModeMsg(client, channel, mode, param);
		for (std::vector<int>::iterator chanIt = chanClients.begin(); chanIt != chanClients.end(); ++chanIt)
			send(*chanIt, msg.c_str(), msg.length(), 0);
	}
	
}

// Mode Commands //
void Server::onlyChannelCmd(Client &client, Channel &channel) {
	
	std::stringstream sstream;
	std::string modestr = " +";
	
	if (channel.getInviteOnly())
		modestr += 'i';
	if (channel.getTopicOpOnly())
		modestr += 't';
	if (channel.getPassword() != "")
		modestr += 'k';
	if (channel.getLimit() > 0)
		modestr += 'l';
	if (channel.getPassword() != "") {
		modestr += ' ';
		modestr += channel.getPassword();
	}
	if (channel.getLimit() > 0) {
		modestr += ' ';
		modestr += channel.getLimit();
	}
	std::string chanmodeis = ":" + std::string(NAME_SERV) + " 324 " + client.getNickname() + " " + channel.getName() + modestr + CRLF;
	sstream << ":" << NAME_SERV << " 329 " << client.getNickname() << " " << channel.getName() << " " << this->brutDatetime << CRLF;
	send(client.getFd(), chanmodeis.c_str(), chanmodeis.length(), 0);
	std::string creationtime = sstream.str();
	send(client.getFd(), creationtime.c_str(), creationtime.length(), 0);
	
}
bool Server::opModeCmd(Client &client, Channel &channel, std::vector<std::string> cmd, std::string mode, std::string &param) {
	
	bool isOp = false;
	
	if (param != "") {
		if (errNoSuchNick(client, param))
			return (false);
		if (errUserNotInChannel(client, channel, param))
			return (false);
	}
	Client *user = getClientByName(param);
	if (mode[0] == '+') {
		if (param == "")
			return (errNeedMoreParams(client, cmd, "+o mode needs parameter"), false);
		std::vector<int> operators = channel.getOperators();
		for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); ++it) {
			if (*it == user->getFd())
				isOp = true;
		}
		if (!isOp)
			channel.addOperator(user->getFd());
	} else {
		if (param != "")
			channel.removeOperator(user->getFd());
		else
			channel.removeOperator(client.getFd());
	}
	return (true);
	
}
bool Server::limitModeCmd(Client &client, Channel &channel, std::vector<std::string> cmd, std::string mode, std::string &param) {
	
	std::stringstream sstream;
	int nbr;
	
	if (mode[0] == '-') {
		channel.setLimit(0);
		return (true);
	}
	if (param == "")
		return (errNeedMoreParams(client, cmd, "+l mode needs parameter"), false);
	for (std::string::iterator it = param.begin(); it != param.end(); ++it) {
		if (!(*it >= '0' && *it <= '9'))
			return (errNeedMoreParams(client, cmd, "Parameter must be a positive number"), false);
	}
	sstream.str(param);
	sstream >> nbr;
	channel.setLimit(nbr);
	return (true);
	
}

// Utils //
bool Server::isGoodNickname(std::string name) {
	
	if (name.length() > 9)
		return (false);
	for (size_t i = 0; i < name.length(); i++) {
		if (i == 0 && ((name[i] >= '0' && name[i] <= '9') || name[i] == '-' || name[i] == '_'))
			return (false);
		if (name[i] >= 'a' && name[i] <= 'z')
			continue;
		else if (name[i] >= 'A' && name[i] <= 'Z')
			continue;
		else if (name[i] >= '0' && name[i] <= '9')
			continue;
		else if (name[i] == '[' || name[i] == ']')
			continue;
		else if (name[i] == '{' || name[i] == '}')
			continue;
		else if (name[i] == '-' || name[i] == '_')
			continue;
		else if (name[i] == '|' || name[i] == '\\')
			continue;
		else if (name[i] == '`' || name[i] == '^')
			continue;
		else
			return (false);
	}
	return (true);
	
}
bool Server::nicknameAlreadyUsed(std::string name) {
	
	for (size_t i = 0; i < clients.size(); i++) {
		if (this->clients[i].getNickname() == name)
			return (true);
	}
	return (false);
	
}
std::vector<std::vector<std::string> > Server::parseBuff(std::string msg) {
	
	std::vector<std::vector<std::string> > res;
	std::stringstream sstream(msg);
	std::string tmp;

	// Utiliser stringstream pour séparer les commandes avec "\r\n"
	while (std::getline(sstream, tmp, '\n')) {
		// Retirer le "\r" final s'il existe
		if (!tmp.empty() && tmp[tmp.size() - 1] == '\r') {
			tmp.erase(tmp.size() - 1);
		}
		std::vector<std::string> parts;
		std::stringstream lineStream(tmp);
		std::string part;
		// Vérifier s'il y a un ':' dans la ligne
		std::string prefix;
		std::getline(lineStream, prefix, ':');  // Extraire la partie avant ':'
		std::stringstream preColonStream(prefix);
		// Séparer les parties avant le ':' avec un espace
		while (preColonStream >> part) {
			parts.push_back(part);
		}
		// Si la ligne contenait un ':', ajouter la partie après ':' comme un seul élément
		if (lineStream) {
			std::string postColon;
			std::getline(lineStream, postColon);  // Récupérer tout ce qui suit ':'
			if (!postColon.empty()) {
				parts.push_back(postColon);  // Ajouter la partie après ':' en une seule chaîne
			}
		}
		// Ajouter la commande au vecteur principal
		if (!parts.empty()) {
			res.push_back(parts);
		}
	}
	return (res);
	
}
std::map<std::string, std::string> Server::parseJoinCmd(std::vector<std::string> cmd) {
	
	std::map<std::string, std::string> res;
	std::stringstream sstream;
	std::vector<std::string> keys;
	std::vector<std::string> values;
	std::string tmp;
	
	sstream.str(cmd[1]);
	while (std::getline(sstream, tmp, ','))
		keys.push_back(tmp);
	for (size_t i = 0; i < keys.size(); i++)
		res[keys[i]] = "";
	if (cmd.size() == 3) {
		sstream.clear();
		sstream.str(cmd[2]);
		while (std::getline(sstream, tmp, ','))
			values.push_back(tmp);
		for (size_t i = 0; i < keys.size(); i++) {
			if (i < values.size())
				res[keys[i]] = values[i];
		}
	}
	return (res);
	
}
bool Server::parseModeCmd(Client &client, std::vector<std::string> cmd, std::map<std::string, std::string> &modes) {
	
	std::string tmp;
	bool params = false;
	
	for (std::vector<std::string>::iterator it = cmd.begin() + 2; it != cmd.end(); ++it) {
		if ((*it)[0] != '+' && (*it)[0] != '-')
			params = true;
		if (((*it)[0] == '+' || (*it)[0] == '-') && params)
			return (errNeedMoreParams(client, cmd, "Parameters placed before modes"), false);
		if ((*it)[0] == '+' || (*it)[0] == '-') {
			for (std::string::iterator strIt = it->begin() + 1; strIt != it->end(); ++strIt) {
				tmp = (*it)[0];
				if (*strIt != 'i' && *strIt != 't' && *strIt != 'k' && *strIt != 'o' && *strIt != 'l') {
					std::string msg = ":" + std::string(NAME_SERV) + " 472 " + client.getNickname() + " " + *strIt + " :is unknown mode char to me" + CRLF;
					send(client.getFd(), msg.c_str(), msg.length(), 0);
				} else {
					tmp += *strIt;
					modes[tmp] = "";
				}
			}
		} else {
			for (std::map<std::string, std::string>::iterator mapIt = modes.begin(); mapIt != modes.end(); ++mapIt) {
				std::string key = mapIt->first;
				if ((key == "+k" || key == "+o" || key == "-o" || key == "+l") && mapIt->second == "") {
					modes[key] = *it;
					break;
				}
			}
		}
	}
	return (true);
	
}
std::string Server::chanClientsToStr(Channel &channel) {
	
	std::string res;
	
	if (channel.getClients().empty())
		return ("");
	for (size_t i = 0; i < channel.getClients().size(); i++) {
		Client &client = *getClientByFd(channel.getClients()[i]);
		bool isOp = false;
		for (size_t j = 0; j < channel.getOperators().size(); j++) {
			if (channel.getOperators()[j] == client.getFd()) {
				isOp = true;
				break;
			}
		}
		if (isOp) {
			res += "@";
			res += client.getNickname();
			res += " ";
		} else {
			res += client.getNickname();
			res += " ";
		}
	}
	res.resize(res.length() - 1);
	return (res);
	
}
void Server::sendChannelsList(int cfd) {
	
	Client &client = *getClientByFd(cfd);
	std::stringstream sstream;
	std::string msg;
	
	sstream << ":" << NAME_SERV << " 321 " << client.getNickname() << " Channel :Users  Name" << CRLF;
	msg = sstream.str();
	sstream.str("");
	sstream.clear();
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	for (std::vector<Channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
		sstream << ":" << NAME_SERV << " 322 " << client.getNickname() << " " << it->getName() << " " << it->getClients().size() << " :" << it->getTopic() << CRLF;
		msg = sstream.str();
		sstream.str("");
		sstream.clear();
		send(client.getFd(), msg.c_str(), msg.length(), 0);
	}
	sstream << ":" << NAME_SERV << " 323 " << client.getNickname() << " :End of /LIST" << CRLF;
	msg = sstream.str();
	sstream.str("");
	sstream.clear();
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	
}

// Errors //
void Server::errNotRegistered(Client &client) {
	
	std::string res = ":" + std::string(NAME_SERV) + " 451 :You have not registered" + CRLF;
	send(client.getFd(), res.c_str(), res.length(), 0);
	
}
void Server::errNeedMoreParams(Client &client, std::vector<std::string> &cmd, std::string str) {
	
	std::string msg = ":" + std::string(NAME_SERV) + " 461 " + client.getNickname() + " " + cmd.front() + " :" + str + CRLF;
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	
}
bool Server::errInviteOnlyChan(Channel *channel, int cfd) {
	
	bool isInvited = false;
	
	if (channel->getInviteOnly()) {
		for (size_t i = 0; i < channel->getInvitedClients().size(); i++) {
			if (channel->getInvitedClients()[i] == cfd) {
				isInvited = true;
				break;
			}
		}
		if (!isInvited) {
			std::string msg = ":" + std::string(NAME_SERV) + " 473 " + channel->getName() + " :Cannot join channel (+i)" + CRLF;
			send(cfd, msg.c_str(), msg.length(), 0);
			return (true);
		}
	}
	return (false);
	
}
bool Server::errChannelIsFull(Channel *channel, int cfd) {
	
	if (channel->getLimit() <= 0)
		return (false);
	if ((int)channel->getClients().size() >= channel->getLimit()) {
		std::string msg = ":" + std::string(NAME_SERV) + " 471 " + channel->getName() + " :Cannot join channel (+l)" + CRLF;
		send(cfd, msg.c_str(), msg.length(), 0);
		return (true);
	}
	return (false);
	
}
bool Server::errBadChanMask(std::string &nameChan, int cfd) {
	
	bool error = false;

	if (nameChan.length() == 0 || nameChan.length() == 1 || nameChan.length() >= 50)
		error = true;
	if (nameChan[0] != '#' && nameChan[0] != '&')
		error = true;
	for (size_t i = 0; i < nameChan.length(); i++) {
		if (nameChan[i] == ':' || nameChan[i] == ',' || (nameChan[i] >= 0 && nameChan[i] <= 32))
			error = true;
	}
	if (error) {
		std::string msg = ":" + std::string(NAME_SERV) + " 476 " + nameChan + " :Bad Channel Mask" + CRLF;
		send(cfd, msg.c_str(), msg.length(), 0);
		return (error);
	}
	return (false);
	
}
bool Server::errBadChannelKey(Channel *channel, std::string &passChan, int cfd) {
	
	if (channel->getPassword().length() > 0) {
		if (channel->getPassword() != passChan) {
			std::string msg = ":" + std::string(NAME_SERV) + " 475 " + channel->getName() + " :Cannot join channel (+k)" + CRLF;
			send(cfd, msg.c_str(), msg.length(), 0);
			return (true);
		}
	}
	return (false);
	
}
bool Server::errNoSuchChannel(Client &client, std::string &channel) {
	
	if (!getChannelByName(channel)) {
		std::string msg = ":" + std::string(NAME_SERV) + " 403 " + client.getNickname() + " " + channel + " :No such channel" + CRLF;
		send(client.getFd(), msg.c_str(), msg.length(), 0);
		return (true);
	}
	return (false);
	
}
bool Server::errNotOnChannel(Client &client, std::string &channel) {
	
	Channel &chan = *getChannelByName(channel);
	std::vector<int> chanClients = chan.getClients();
	
	for (size_t i = 0; i < chanClients.size(); i++) {
		Client &chanClient = *getClientByFd(chanClients[i]);
		if (chanClient.getNickname() == client.getNickname())
			return (false);
	}
	std::string msg = ":" + std::string(NAME_SERV) + " 442 " + client.getNickname() + " " + channel + " :You're not on that channel" + CRLF;
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	return (true);
	
}
bool Server::errNoSuchNick(Client &client, std::string &nick) {
	
	bool found = false;
	
	for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		if (it->getNickname() == nick && it->getIsRegistered()) {
			found = true;
			break;
		}
	}
	if (!found) {
		std::string msg = ":" + std::string(NAME_SERV) + " 401 " + client.getNickname() + " " + nick + " :No such nick" + CRLF;
		send(client.getFd(), msg.c_str(), msg.length(), 0);
		return (true);
	}
	return (false);
	
}
bool Server::errChanOprivsNeeded(Client &client, Channel &channel) {
	
	std::vector<int> operators = channel.getOperators();
	
	for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); ++it) {
		if (*it == client.getFd())
			return (false);
	}
	std::string msg = ":" + std::string(NAME_SERV) + " 482 " + client.getNickname() + " " + channel.getName() + " :You're not channel operator" + CRLF;
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	return (true);
	
}
bool Server::errUserNotInChannel(Client &client, Channel &channel, std::string &nick) {
	
	std::vector<int> chanClients = channel.getClients();
	Client *user = getClientByName(nick);
	
	if (user) {
		for (std::vector<int>::iterator it = chanClients.begin(); it != chanClients.end(); ++it) {
			if (*it == user->getFd())
				return (false);
		}
	}
	std::string msg = ":" + std::string(NAME_SERV) + " 441 " + client.getNickname() + " " + nick + " " + channel.getName() + " :They aren't on that channel" + CRLF;
	send(client.getFd(), msg.c_str(), msg.length(), 0);
	return (true);
	
}
bool Server::errUserOnChannel(Client &client, Client &user, Channel &channel) {
	
	std::vector<int> chanClients = channel.getClients();
	
	for (std::vector<int>::iterator it = chanClients.begin(); it != chanClients.end(); ++it) {
		if (*it == user.getFd()) {
			std::string msg = ":" + std::string(NAME_SERV) + " 443 " + client.getNickname() + " " + user.getNickname() + " " + channel.getName() + " :is already on channel" + CRLF;
			send(client.getFd(), msg.c_str(), msg.length(), 0);
			return (true);
		}
	}
	return (false);
	
}
