/// mitcp --- ahora se trata una conexión en lugar de mensajes

/*
>./echo_server 0.0.0.0 2222
> nc 127.0.0.1 2222
*/

/*
las 4 secciones que SIEMPRE van a estar:
	* INICIALIZAR SOCKET Y ADDR
	* PONER EL SOCKET EN LISTEN (ACEPTAR CONEXIONES)
	* ACEPTAR UNA NUEVA CONEXIÓN (PRIMER WHILE)
	* TRATAR LA CONEXIÓN (SEGUNDO WHILE)
*/

#include <iostream>		// cout
#include <sys/types.h>	// getaddrinfo
#include <sys/socket.h>	// getaddrinfo
#include <netdb.h>		// getaddrinfo
#include <string.h>		// memset
#include <stdio.h>		// perror
#include <thread>       // std::thread
#include <unistd.h>		// sleep
#include <mutex>		// mutex
///////////////////////////////////////////#include <vector>       // std::vector

const int MAX_USUARIOS = 3;

bool salir = false;
int numUsuarios = 0;
///////////////std::vector<std::thread> usuarios;
std::thread usuarios[MAX_USUARIOS];
bool hilosLibres[MAX_USUARIOS];
pthread_mutex_t cerrojoHilos;

int buscaHiloLibre() { // BLOQUEANTE //
	pthread_mutex_lock(&cerrojoHilos);

	if (numUsuarios >= MAX_USUARIOS) {
		pthread_mutex_unlock(&cerrojoHilos);
		return -1;
	}
	
	int i = 0;
	while (i < MAX_USUARIOS && !hilosLibres[i])
		i++;
	if (i == MAX_USUARIOS) {
		pthread_mutex_unlock(&cerrojoHilos);
		return -1;
	}
	else {
		pthread_mutex_unlock(&cerrojoHilos);
		return i;
	}
}

//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
int trataConexion(int cliente_sd, int thr) {
	/*
	pthread_mutex_lock(&cerrojoHilos);
	numUsuarios++;
	hilosLibres[thr] = false;
	pthread_mutex_unlock(&cerrojoHilos);
	*/

	std::cout << "Hilo " << thr << " trabajando\n";
	
	sleep(5);
	
	/*
	while (conexion) {
		//ssize_t c = recv(client_sd, &(buffer[bytes]), 1, 0); // ¡se usa client_sd!
		//ssize_t sbytes = send(client_sd, buffer, bytes, 0); // ¡se usa client_sd!
	}

	int conn = 1;
		while (conn) {
			ssize_t c = 1;
			int bytes = 0;
			int finCadena = 0;
			// se reciben hasta 127 bytes y luego se termina el búffer con \n
			while (c > 0 && bytes < 127 && !finCadena) {
				c = recv(client_sd, &(buffer[bytes]), 1, 0); // ¡se usa client_sd!
				if (buffer[bytes] == '\n')
					finCadena = 1;
				bytes++;
			}
			if (c < 0) { // error
				perror(NULL);
				conn = 0;
				continue;
			}
			else if (c == 0) { // cierre ordenado
				conn = 0;
				continue;
			}
		
			// ¡indicar finalización de cadena!
			buffer[bytes]='\0';
		
			// reenvío del mensaje
			ssize_t sbytes = send(client_sd, buffer, bytes, 0); // ¡se usa client_sd, no del propio descriptor sd!
			if (sbytes == -1) {
				perror(NULL);
				conn = 0;
			}
		}
	*/

	// impresión por pantalla
	std::cout << "Hilo " << thr << " ha terminado\n";

	pthread_mutex_lock(&cerrojoHilos);
	hilosLibres[thr] = true;
	numUsuarios--;
	pthread_mutex_unlock(&cerrojoHilos);

	return 0;
}

int main(int argc, char** argv){

	for (int i = 0; i < MAX_USUARIOS; i++)
		hilosLibres[i] = true;

	pthread_mutex_init(&cerrojoHilos, NULL);

	// criterios
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE; // Escuchar en 0.0.0.0
	hints.ai_family = AF_INET; // Dir. IPv4
	hints.ai_socktype = SOCK_STREAM; // Flujo TCP
	// lista de resultados
	struct addrinfo *result;
	
	// obtener resultados
	int luis = getaddrinfo(argv[1], argv[2], &hints, &result);
	// error?
	if (luis != 0) {
		std::cerr << "Error getaddrinfo: " << gai_strerror(luis) << "\n";
		return -1;
	}
	
	//INICIALIZAR SOCKET Y ADDR
	int sd = socket(result->ai_family, result->ai_socktype, 0);
	if (sd == -1) {
		perror("Error socket: ");
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}
	int juan = bind(sd, result->ai_addr, result->ai_addrlen);
	if (juan == -1) {
		perror("Error bind: ");
		free(&sd); // cerrar socket
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}

	//PONER EL SOCKET EN LISTEN (ACEPTAR CONEXIONES)
	int paco = listen(sd, 16);
	if (paco == -1) {
		perror("Error listen: ");
		free(&sd); // cerrar socket
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}
	
	//ACEPTAR UNA NUEVA CONEXIÓN (PRIMER WHILE)
	while (!salir) {
		//////////////////////char buffer[128];
		//////////////////////char host[NI_MAXHOST];
		//////////////////////char serv[NI_MAXSERV];

		pthread_mutex_lock(&cerrojoHilos);
		printf("ENTRA-%i\n", numUsuarios);
		pthread_mutex_unlock(&cerrojoHilos);
		
		// socket cliente y su tamaño
		struct sockaddr_in client;
		socklen_t clientlen = sizeof(struct sockaddr_in);		

		//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
		// aceptar conexión
		int hl = buscaHiloLibre(); // BLOQUEANTE //
		if (hl != -1) {
			printf("(!) Adjudicado %i\n", hl);
			int client_sd = accept(sd, (struct sockaddr*) &client, &clientlen);
			if (client_sd == -1) {
				perror("Error accept: ");
				free(&sd); // cerrar socket
				freeaddrinfo(result); // liberar memoria dinámica
				return -1;
			}

			// impresión por pantalla
			printf("Conexión nueva\n");

			//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
			pthread_mutex_lock(&cerrojoHilos);
			numUsuarios++;
			hilosLibres[hl] = false;
			pthread_mutex_unlock(&cerrojoHilos);
			usuarios[hl] = std::thread(trataConexion, client_sd, hl);
			usuarios[hl].detach();
		}
		else {
			printf("Imposible conectarse: número máximo de usuarios alcanzado\n");
		}
		
		/*
		int pepe = getnameinfo(
			(struct sockaddr*) &client, clientlen,
			host, NI_MAXHOST,
			serv, NI_MAXSERV,
			NI_NUMERICHOST | NI_NUMERICSERV);
		if (pepe != 0) { // error?
			std::cerr << "Error getnameinfo: " << gai_strerror(pepe) << "\n";
			free(&sd); // cerrar socket
			freeaddrinfo(result); // liberar memoria dinámica
			return -1;
		}

		// impresión por pantalla
		printf("Conexión desde %s:%s\n",host, serv);
		*/

	//....

		/*
		// impresión por pantalla
		printf("Conexión %s:%s terminada\n",host, serv);
		*/

		pthread_mutex_lock(&cerrojoHilos);
		printf("SALE-%i\n", numUsuarios);
		pthread_mutex_unlock(&cerrojoHilos);
	}

	// (i) no hay que esperar a que terminen los hilos de usuario (detach)
	for (int i = 0; i < MAX_USUARIOS; i++)
		usuarios[i].join();
	
	// cerrar socket
	free(&sd);
	
	// liberar memoria dinámica
	freeaddrinfo(result);

	pthread_mutex_destroy(&cerrojoHilos);

	// éxito
	return 0;
}

/*
std::thread first (foo);     // spawn new thread that calls foo()
  std::thread second (bar,0);  // spawn new thread that calls bar(0)

  std::cout << "main, foo and bar now execute concurrently...\n";

*/

/*
namespace {
  std::vector<std::thread> workers;

  int total = 4;
  int arr[4] = {0};

  void each_thread_does(int i) {
    arr[i] += 2;
  }
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < 8; ++i) { // for 8 iterations,
    for (int j = 0; j < 4; ++j) {
      workers.push_back(std::thread(each_thread_does, j));
    }
    for (std::thread &t: workers) {
      if (t.joinable()) {
        t.join();
      }
    }
    arr[4] = std::min_element(arr, arr+4);
  }
  return 0;
}
*/

/*
Inicializar un mutex:
	int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
Destruir un mutex:
	int pthread_mutex_destroy(pthread_mutex_t*mutex);
Obtener el mutex o bloquear al hilo si el mutex lo tiene otro hilo:
	int pthread_mutex_lock(pthread_mutex_t *mutex);
Liberar el mutex:
	int pthread_mutex_unlock(pthread_mutex_t *mutex);
*/


/*

		//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
		// aceptar conexión
		int hl = buscaHiloLibre(); // BLOQUEANTE //
		if (hl != -1) {
			printf("(!) Adjudicado %i\n", hl);
			int client_sd = accept(sd, (struct sockaddr*) &client, &clientlen);
			if (client_sd == -1) {
				perror("Error accept: ");
				free(&sd); // cerrar socket
				freeaddrinfo(result); // liberar memoria dinámica
				return -1;
			}

			// impresión por pantalla
			printf("Conexión nueva\n");

			//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
			//////usuarios.push_back(std::thread(trataConexion, 0));
			//////for (int i = 0; i < MAX_USUARIOS; i++)
			usuarios[hl] = std::thread(trataConexion, client_sd, hl);
			usuarios[hl].detach();
		}
		else {
			printf("Imposible conectarse: número máximo de usuarios alcanzado\n");
		}
*/