# ft_irc
<br>
<p align="center">
  <img src=https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg alt="C++ Logo" width="125">
</p>

### Skills

- Network programming with **BSD sockets**
- Multiplexing with `poll` (or `select`, `kqueue`, `epoll`)
- **Non-blocking** I/O on file descriptors
- TCP/IP
- Parsing and handling **IRC protocol** messages
- Client session state (registration: `PASS` / `NICK` / `USER`)
- Users, channels, and operator privileges
- Channel modes (`+i`, `+t`, `+k`, `+o`, `+l`)
- Robust error handling & resource management (C++98)

<br>

## About

`ft_irc` is a **C++98 IRC server** (`ircserv`) that accepts multiple simultaneous clients over TCP.  
It implements the essentials of IRC so a standard client can connect and behave as on a classic server.

Supported features:
- Authentication with **password**
- **Nickname** and **username** registration
- **Channel** creation, joining, and parting
- **Private** and **channel** messaging
- **Operators** and moderation commands:
  - `KICK` – eject a client from a channel
  - `INVITE` – invite a client to a channel
  - `TOPIC` – set or view channel topic
  - `MODE` – set channel modes:
    - `+i`: Invite-only
    - `+t`: Topic restricted to operators
    - `+k`: Channel password
    - `+o`: Give/take operator privilege
    - `+l`: User limit

<br>

## Usage

**1. Clone the repository**
```bash
git clone git@github.com:maudiffr/ft_irc.git
cd ft_irc
```

**2. Build**
```bash
make
```

**3. Run the server**
```bash
./ircserv <port> <password>
# <port>     : e.g. 6667
# <password> : clients must send PASS with this value
```

**4. Connect with an IRC client (netcat)**
```bash
nc -C 127.0.0.1 6667
PASS <password>
NICK JohnDoe
USER johndoe 0 * :John Doe
JOIN #channel
PRIVMSG #channel :Hello from ft_irc!
```

<br>

## Example Commands

```irc
NICK JohnDoe
USER johndoe 0 * :John Doe
JOIN #channel
PRIVMSG #channel :Hello everyone!
PRIVMSG Alice :Hey Alice!
TOPIC #channel :New topic here
KICK #channel Bob :Spamming
```

### &#128213; This project was completed as a team of two students at 42.