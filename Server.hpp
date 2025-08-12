/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fquercy <fquercy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 16:06:14 by fquercy           #+#    #+#             */
/*   Updated: 2024/09/10 16:06:14 by fquercy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Server_hpp
#define Server_hpp

#include "constants.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <csignal>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "Channel.hpp"
#include "Client.hpp"

class Server {
	
public:
	
	// Signals //
	static void handleSig(int sig);
	
	// Init //
	void run(int port, std::string password);
	
	// Fd //
	void closeFds();
	
	// Getters //
	int getPort() const;
	const std::string &getPassword() const;
	int getClientIndex(int fd);
	void getClientData(int fd);
	Client *getClientByFd(int cfd);
	Client *getClientByName(std::string &name);
	Channel *getChannelByName(std::string &name);
	std::string getModeMsg(Client &client, Channel &channel, std::string mode, std::string param);
	
private:
	// Attributs //
	std::time_t brutDatetime;
	std::string datetime;
	std::string ipAddr;
	int port;
	std::string password;
	int fd;
	static bool sig;
	std::vector<Channel> channels;
	std::vector<Client> clients;
	std::vector<struct pollfd> pfds;
	
	// Init //
	void init();
	
	// Channels //
	void createChannel(std::string name, std::string pass, int cfd);
	void deleteChannel(Channel &channel);
	
	// Clients //
	void AuthClient(std::vector<std::string> cmd, int cfd);
	bool checkRegistered(Client &client, int mark);
	void deleteClient(int cfd);
	void acceptClient();
	
	// Commands //
	void checkPassCmd(std::vector<std::string> cmd, int cfd);
	void checkNickCmd(std::vector<std::string> cmd, int cfd);
	void checkUserCmd(std::vector<std::string> cmd, int cfd);
	void joinCmd(std::vector<std::string> cmd, int cfd);
	void partCmd(std::vector<std::string> cmd, int cfd);
	void privmsgCmd(std::vector<std::string> cmd, int cfd);
	void quitCmd(std::vector<std::string> cmd, int cfd);
	void kickCmd(std::vector<std::string> cmd, int cfd);
	void inviteCmd(std::vector<std::string> cmd, int cfd);
	void topicCmd(std::vector<std::string> cmd, int cfd);
	void modeCmd(std::vector<std::string> cmd, int cfd);
	
	// Mode Commands //
	void onlyChannelCmd(Client &client, Channel &channel);
	bool opModeCmd(Client &client, Channel &channel, std::vector<std::string> cmd, std::string mode, std::string &param);
	bool limitModeCmd(Client &client, Channel &channel, std::vector<std::string> cmd, std::string mode, std::string &param);
	
	// Utils //
	bool isGoodNickname(std::string name);
	bool nicknameAlreadyUsed(std::string name);
	std::vector<std::vector<std::string> > parseBuff(std::string msg);
	std::map<std::string, std::string> parseJoinCmd(std::vector<std::string> cmd);
	bool parseModeCmd(Client &client, std::vector<std::string> cmd, std::map<std::string, std::string> &modes);
	std::string chanClientsToStr(Channel &channel);
	void sendChannelsList(int cfd);
	
	// Errors //
	void errNotRegistered(Client &client);
	void errNeedMoreParams(Client &client, std::vector<std::string> &cmd, std::string str);
	bool errInviteOnlyChan(Channel *channel, int cfd);
	bool errChannelIsFull(Channel *channel, int cfd);
	bool errBadChanMask(std::string &nameChan, int cfd);
	bool errBadChannelKey(Channel *channel, std::string &passChan, int cfd);
	bool errNoSuchChannel(Client &client, std::string &channel);
	bool errNotOnChannel(Client &client, std::string &channel);
	bool errNoSuchNick(Client &client, std::string &nick);
	bool errChanOprivsNeeded(Client &client, Channel &channel);
	bool errUserNotInChannel(Client &client, Channel &channel, std::string &nick);
	bool errUserOnChannel(Client &client, Client &user, Channel &channel);
	
};

#endif
