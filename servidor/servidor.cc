// $ ./servidor 0.0.0.0 2222

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

const int MAX_USUARIOS = 3;
const int TAM_BUF_HILO = 128;

const int TAM_CELDA = 50;
const int TAM_LIENZO = 8;
//
const int COL_LIENZO = 5;

int celdas[TAM_LIENZO][TAM_LIENZO];

bool aceptaConexiones = true;
int numUsuarios = 0;
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
	
	bool conexion = true;

	while (conexion) {
		char bufferR[TAM_BUF_HILO];
		char bufferS[TAM_BUF_HILO] = "sus"; /////////////////////tmp/////////////////////
		// /!\ NO RECIBIR MÁS DE (TAM_BUF_HILO - 1)
		ssize_t rbytes = recv(cliente_sd, bufferR, 1, 0);
		bufferR[TAM_BUF_HILO - 1]='\0'; // ...
		if (rbytes > 0) {
			bufferR[rbytes]='\0'; // ¡indicar finalización de cadena!
			//////////////////////////////////////////bufferS[TAM_BUF_HILO - 1]='\0'; // ...
			ssize_t sbytes = send(cliente_sd, bufferS, rbytes, 0);
			if (sbytes < 0) {
				std::cout << "ERROR al ENVIAR desde el hilo " << thr << "\n";
				conexion = false;
			}
		}
		else if (rbytes == 0) {
			conexion = false;
		}
		else {
			std::cout << "ERROR al RECIBIR en el hilo " << thr << "\n";
			conexion = false;
		}
	}

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

    for (int i = 0; i < TAM_LIENZO; i++)
        for (int j = 0; j < TAM_LIENZO; j++)
            celdas[i][j] = COL_LIENZO;

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
		// ... // cerrar socket
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}

	//PONER EL SOCKET EN LISTEN (ACEPTAR CONEXIONES)
	int paco = listen(sd, 16);
	if (paco == -1) {
		perror("Error listen: ");
		// ... // cerrar socket
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}
	
	//ACEPTAR UNA NUEVA CONEXIÓN (PRIMER WHILE)
	while (aceptaConexiones) {
		///pthread_mutex_lock(&cerrojoHilos);
		///printf("ENTRA-%i\n", numUsuarios);
		///pthread_mutex_unlock(&cerrojoHilos);
		
		// socket cliente y su tamaño
		struct sockaddr_in client;
		socklen_t clientlen = sizeof(struct sockaddr_in);		

		//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
		// aceptar conexión
		int hl = buscaHiloLibre(); // BLOQUEANTE //
		if (hl != -1) {
			printf("(!) Hilo para la siguiente conexión: %i\n", hl);
			int client_sd = accept(sd, (struct sockaddr*) &client, &clientlen);
			if (client_sd == -1) {
				perror("Error accept: ");
				// ... // cerrar socket
				freeaddrinfo(result); // liberar memoria dinámica
				return -1;
			}

			// impresión por pantalla
			//printf("Conexión nueva\n");

			//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
			pthread_mutex_lock(&cerrojoHilos);
			numUsuarios++;
			hilosLibres[hl] = false;
			pthread_mutex_unlock(&cerrojoHilos);
			/**/
			usuarios[hl] = std::thread(trataConexion, client_sd, hl);
			usuarios[hl].detach();
		}
		else {
			printf("Imposible conectarse: número máximo de usuarios alcanzado\n");
		}

		///pthread_mutex_lock(&cerrojoHilos);
		///printf("SALE-%i\n", numUsuarios);
		///pthread_mutex_unlock(&cerrojoHilos);
	}

	// (i) no hay que esperar a que terminen los hilos de usuario (detach)
	for (int i = 0; i < MAX_USUARIOS; i++)
		usuarios[i].join();
	
	// ... // cerrar socket
	
	// liberar memoria dinámica
	freeaddrinfo(result);

	pthread_mutex_destroy(&cerrojoHilos);

	// éxito
	return 0;
}
