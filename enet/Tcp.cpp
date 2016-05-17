/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/debug.h>
#include <enet/Tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <etk/stdTools.h>


enet::Tcp::Tcp() :
  m_socketId(-1),
  m_socketIdClient(-1),
  m_host("127.0.0.1"),
  m_port(23191),
  m_server(false),
  m_status(status::unlink) {
	
}

enet::Tcp::~Tcp() {
	unlink();
}

void enet::Tcp::setIpV4(uint8_t _fist, uint8_t _second, uint8_t _third, uint8_t _quatro) {
	std::string tmpname;
	tmpname  = etk::to_string(_fist);
	tmpname += ".";
	tmpname += etk::to_string(_second);
	tmpname += ".";
	tmpname += etk::to_string(_third);
	tmpname += ".";
	tmpname += etk::to_string(_quatro);
	setHostNane(tmpname);
}

void enet::Tcp::setHostNane(const std::string& _name) {
	if (_name == m_host) {
		return;
	}
	if (m_status == status::link) {
		ENET_ERROR("Can not change parameter while connection is started");
		return;
	}
	m_host = _name;
}

void enet::Tcp::setPort(uint16_t _port) {
	if (_port == m_port) {
		return;
	}
	if (m_status == status::link) {
		ENET_ERROR("Can not change parameter while connection is started");
		return;
	}
	m_port = _port;
}

void enet::Tcp::setServer(bool _status) {
	if (_status == m_server) {
		return;
	}
	if (m_status == status::link) {
		ENET_ERROR("Can not change parameter while connection is started");
		return;
	}
	m_server = _status;
}

bool enet::Tcp::link() {
	if (m_status == status::link) {
		ENET_ERROR("Connection is already started");
		return false;
	}
	ENET_INFO("Start connection on " << m_host << ":" << m_port);
	if (m_server == false) {
		#define MAX_TEST_TIME  (5)
		for(int32_t iii=0; iii<MAX_TEST_TIME ;iii++) {
			// open in Socket normal mode
			m_socketIdClient = socket(AF_INET, SOCK_STREAM, 0);
			if (m_socketIdClient < 0) {
				ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
				usleep(200000);
				continue;
			}
			ENET_INFO("Try connect on socket ... (" << iii+1 << "/" << MAX_TEST_TIME << ")");
			struct sockaddr_in servAddr;
			struct hostent* server = gethostbyname(m_host.c_str());
			if (server == NULL) {
				ENET_ERROR("ERROR, no such host : " << m_host);
				usleep(200000);
				continue;
			}
			bzero((char *) &servAddr, sizeof(servAddr));
			servAddr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);
			servAddr.sin_port = htons(m_port);
			ENET_INFO("Start connexion ...");
			if (connect(m_socketIdClient, (struct sockaddr *)&servAddr,sizeof(servAddr)) != 0) {
				if(errno != EINPROGRESS) {
					if(    errno != ENOENT
					    && errno != EAGAIN
					    && errno != ECONNREFUSED) {
						ENET_ERROR("ERROR connecting on : errno=" << errno << "," << strerror(errno));
					}
					close(m_socketIdClient);
					m_socketIdClient = -1;
				}
				ENET_ERROR("ERROR connecting, maybe retry ... errno=" << errno << "," << strerror(errno));
				usleep(500000);
				continue;
			}
			// if we are here ==> then the connextion is done corectly ...
			break;
		}
		if (m_socketIdClient<0) {
			ENET_ERROR("ERROR connecting ... (after all try)");
			return false;
		} else {
			m_status = status::link;
			ENET_DEBUG("Connection done");
		}
	} else {
		// open in Socket normal mode
		m_socketId = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socketId < 0) {
			ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
			return false;
		}
		// set the reuse of the socket if previously opened :
		int sockOpt = 1;
		if(setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(int)) != 0) {
			ENET_ERROR("ERROR while configuring socket re-use : errno=" << errno << "," << strerror(errno));
			return false;
		}
		// clear all
		struct sockaddr_in servAddr;
		bzero((char *) &servAddr, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = INADDR_ANY;
		servAddr.sin_port = htons(m_port);
		ENET_INFO("Start binding Socket ... (can take some time ...)");
		if (bind(m_socketId, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
			ENET_ERROR("ERROR on binding errno=" << errno << "," << strerror(errno));
			close(m_socketId);
			m_socketId = -1;
			return false;
		}
		ENET_INFO("End binding Socket ... (start listen)");
		listen(m_socketId,1); // 1 is for the number of connection at the same time ...
		ENET_INFO("End listen Socket ... (start accept)");
		struct sockaddr_in clientAddr;
		socklen_t clilen = sizeof(clientAddr);
		m_socketIdClient = accept(m_socketId, (struct sockaddr *) &clientAddr, &clilen);
		if (m_socketIdClient < 0) {
			ENET_ERROR("ERROR on accept errno=" << errno << "," << strerror(errno));
			close(m_socketId);
			m_socketId = -1;
			return false;
		} else {
			m_status = status::link;
			ENET_DEBUG("Connection done");
		}
	}
	#if 1
		//Initialize the pollfd structure
		memset(m_fds, 0 , sizeof(m_fds));
		//Set up the initial listening socket
		m_fds[0].fd = m_socketIdClient;
		m_fds[0].events = POLLIN | POLLERR;
	#endif
	ENET_INFO("End configuring Socket ...");
	return true;
}


bool enet::Tcp::unlink() {
	if (m_socketIdClient >= 0) {
		ENET_INFO(" close client socket");
		close(m_socketIdClient);
		m_socketIdClient = -1;
	}
	if (m_socketId >= 0) {
		ENET_INFO(" close server socket");
		close(m_socketId);
		m_socketId = -1;
	}
	m_status = status::unlink;
	return true;
}


int32_t enet::Tcp::read(void* _data, int32_t _maxLen) {
	ENET_VERBOSE("read [START]");
	if (m_status != status::link) {
		ENET_ERROR("Can not read on unlink connection");
		return -1;
	}
	int32_t size = -1;
	#if 0
		size = ::read(m_socketIdClient, _data, _maxLen);
		if (    size != 0
		     && errno == 2) {
			// simply the socket en empty
		} else if (errno != 0) {
			ENET_ERROR("PB when reading data on the FD : request=" << _maxLen << " have=" << size << ", erno=" << errno << "," << strerror(errno));
			m_status = status::error;
			return -1;
		}
	#else
		#ifndef SDFGSDFGSDFGSDFGSDFGSDFG
			int nfds = 1;
			// Initialize the timeout to 3 minutes. If no activity after 3 minutes this program will end. timeout value is based on milliseconds.
			int timeout = (3 * 60 * 1000);
			// Call poll() and wait 3 minutes for it to complete.
			ENET_VERBOSE("Waiting on poll()...");
			int rc = poll(m_fds, nfds, timeout);
			// Check to see if the poll call failed.
			if (rc < 0) {
				ENET_ERROR("	poll() failed");
				return-1;
			}
			// Check to see if the 3 minute time out expired.
			if (rc == 0) {
				ENET_ERROR("	poll() timed out.	End program.\n");
				return -1;
			}
			bool closeConn = false;
			// Receive all incoming data on this socket before we loop back and call poll again.
			// Receive data on this connection until the recv fails with EWOULDBLOCK.
			// If any other failure occurs, we will close the connection.
			rc = recv(m_fds[0].fd, _data, _maxLen, 0);
			if (rc < 0) {
				if (errno != EWOULDBLOCK) {
					ENET_ERROR("	recv() failed");
					closeConn = true;
				}
			}
			// Check to see if the connection has been closed by the client
			if (rc == 0) {
				ENET_ERROR("	Connection closed");
				closeConn = true;
			}
			if (closeConn == false) {
				// Data was received
				size = rc;
				ENET_VERBOSE("    " << size << " bytes received");
			} else {
				// If the close_conn flag was turned on, we need to clean up this active connection.
				// This clean up process includes removing the descriptor.
				ENET_ERROR("	Set status at remote close ...");
				m_status = status::linkRemoteClose;
			}
		#else
		//Initialize the pollfd structure
		memset(m_fds, 0 , sizeof(m_fds));
		//Set up the initial listening socket
		m_fds[0].fd = m_socketId;
		m_fds[0].events = POLLIN;
		int nfds = 1
		// Initialize the timeout to 3 minutes. If no activity after 3 minutes this program will end. timeout value is based on milliseconds.
		int timeout = (3 * 60 * 1000);
		//Loop waiting for incoming connects or for incoming data on any of the connected sockets.
		do {
			// Call poll() and wait 3 minutes for it to complete.
			ENET_INFO("Waiting on poll()...");
			int rc = poll(m_fds, nfds, timeout);
			// Check to see if the poll call failed.
			if (rc < 0) {
				ENET_ERROR("	poll() failed");
				break;
			}
			// Check to see if the 3 minute time out expired.
			if (rc == 0) {
				ENET_ERROR("	poll() timed out.	End program.\n");
				break;
			}
			// One or more descriptors are readable. Need to determine which ones they are.
			int current_size = nfds;
			for (int32_t iii iii=0; iii<current_size; ++iii) {
				// Loop through to find the descriptors that returned POLLIN and determine whether it's the listening or the active connection.
				if(m_fds[iii].revents == 0) {
					continue;
				}
				// If revents is not POLLIN, it's an unexpected result, log and end the server.
				if(m_fds[iii].revents != POLLIN) {
					printf("	Error! revents = %d\n", m_fds[iii].revents);
					bool end_server = true; // TODO: dsfsdfsdf
					break;
				}
				if (m_fds[iii].fd == m_socketId) {
					// Listening descriptor is readable.
					ENET_INFO("	Listening socket is readable");
					// Accept all incoming connections that are queued up on the listening socket before we loop back and call poll again.
					do {
						// Accept each incoming connection. If accept fails with EWOULDBLOCK, then we have accepted all of them.
						// Any other failure on accept will cause us to end the server.
						new_sd = accept(listen_sd, nullptr, nullptr);
						if (new_sd < 0) {
							if (errno != EWOULDBLOCK) {
								ENET_ERROR("	accept() failed");
								end_server = TRUE;
							}
							break;
						}
						// Add the new incoming connection to the pollfd structure
						ENET_ERROR("	New incoming connection - %d", new_sd);
						m_fds[nfds].fd = new_sd;
						m_fds[nfds].events = POLLIN;
						nfds++;
						// Loop back up and accept another incoming connection
					} while (new_sd != -1);
				} else {
					// This is not the listening socket, therefore an existing connection must be readable
					ENET_INFO("	Descriptor %d is readable", fds[iii].fd);
					close_conn = FALSE;
					// Receive all incoming data on this socket before we loop back and call poll again.
					do {
						// Receive data on this connection until the recv fails with EWOULDBLOCK.
						// If any other failure occurs, we will close the connection.
						rc = recv(m_fds[iii].fd, buffer, sizeof(buffer), 0);
						if (rc < 0) {
							if (errno != EWOULDBLOCK) {
								perror("	recv() failed");
								close_conn = TRUE;
							}
							break;
						}
						// Check to see if the connection has been closed by the client
						if (rc == 0) {
							ENET_ERROR("	Connection closed");
							close_conn = TRUE;
							break;
						}
						// Data was received
						len = rc;
						ENET_INFO("	%d bytes received", len);
						// Echo the data back to the client
						rc = send(m_fds[i].fd, buffer, len, 0);
						if (rc < 0) {
							ENET_ERRO("	send() failed");
							close_conn = TRUE;
							break;
						}
					} while(TRUE);
					// If the close_conn flag was turned on, we need to clean up this active connection.
					// This clean up process includes removing the descriptor.
					if (close_conn) {
						close(m_fds[i].fd);
						m_fds[i].fd = -1;
						compress_array = TRUE;
					}
				}
			}
			// If the compress_array flag was turned on, we need to squeeze together the array and decrement the number of file descriptors.
			// We do not need to move back the events and revents fields because the events will always be POLLIN in this case, and revents is output.
			if (compress_array) {
				compress_array = FALSE;
				for (int32_t iii=0; iii<nfds; ++iii) {
					if (m_fds[i].fd == -1) {
						for(int32_t jjj = iii; jjj < nfds; ++jjj) {
							m_fds[jjj].fd = fds[jjj+1].fd;
						}
						nfds--;
					}
				}
			}
		} while (end_server == FALSE);
		// Clean up all of the sockets that are open
		for (int32_t iii=0; iii<nfds; ++iii) {
			if(m_fds[iii].fd >= 0) {
				close(m_fds[iii].fd);
			}
		}
		#endif
	#endif
	ENET_VERBOSE("read [STOP]");
	return size;
}


int32_t enet::Tcp::write(const void* _data, int32_t _len) {
	if (m_status != status::link) {
		ENET_ERROR("Can not write on unlink connection");
		return -1;
	}
	int32_t size = ::write(m_socketIdClient, _data, _len);
	if (    size != _len
	     && errno != 0) {
		ENET_ERROR("PB when writing data on the FD : request=" << _len << " have=" << size << ", erno=" << errno << "," << strerror(errno));
		m_status = status::error;
		return -1;
	}
	return size;
}
