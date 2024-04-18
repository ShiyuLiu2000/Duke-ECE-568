#include "server.hpp"

// implement Server class

Server::Server(const char * port) :
    port(port), socket_fd(-1), running(true) { // , C(nullptr) {
      // C = connectDatabase();
}

Server::~Server() {
  running = false;
  if (socket_fd != -1) {
    close(socket_fd);
  }
  cout << "Closing server." << endl;
  // if (C) {
  //   C->disconnect();
  //   delete C;
  // }
}

// here I implement on my own code from ECE650 project3 for the hot potato

void Server::createHost(const char * port) {
  const char * hostname = NULL;
  int status;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }

  // connect SOCKET
  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }

  // bind
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (status == -1) {
    cerr << "Error: setsockopt" << endl;
    ::exit(EXIT_FAILURE);
  }
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket: " << strerror(errno) << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }

  // start listen
  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }
  freeaddrinfo(host_info_list);
}

void Server::createClient(const char * hostname, const char * port) {
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  int status;

  // hostname = NULL;
  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }

  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket: " << strerror(errno) << endl;
    cerr << "Status: " << status << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    ::exit(EXIT_FAILURE);
  }
  freeaddrinfo(host_info_list);
}

int Server::acceptConnection() {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    ::exit(EXIT_FAILURE);
  }
  return client_connection_fd;
}

void Server::runServer(connection * C) {
    cout << "Start running server." << endl;
    createHost(this->port);
    while (running) {
        int client_fd = acceptConnection();
        if (client_fd < 0) {
            cerr << "Failed to accept client connection" << endl;
            continue;
        }

        client_fd_c * passToHandle = new client_fd_c;
        passToHandle->client_fd = client_fd;
        passToHandle->C = C;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handleClient, (void*) passToHandle) != 0) {
            cerr << "Failed to create thread for new client" << endl;
            close(client_fd);
            delete passToHandle;
        }
    }
}


void * Server::handleClient(void * arg) {
    // cout << "Start handling request from client." << endl;
    client_fd_c * passToHandle = static_cast<client_fd_c*>(arg);
    int client_fd = passToHandle->client_fd;
    connection* C = passToHandle->C;

    char buffer[1024] = {0};
    string xmlRequest, xmlResponse;

    // listen and read
    ssize_t bytes_read = read(client_fd, buffer, 1023);
    if (bytes_read < 0) {
        cerr << "Error reading from client" << endl;
        close(client_fd);
        return NULL;
    }
    xmlRequest = string(buffer);

    xmlResponse = handle_client_request(xmlRequest, C);
    send(client_fd, xmlResponse.c_str(), xmlResponse.length(), 0);

    close(client_fd);
    delete (int*)arg;
    return NULL;
}