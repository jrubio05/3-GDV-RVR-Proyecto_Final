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
//
#include "Message.h"	// mensajes de red

const int MAX_USUARIOS = 3;

const int TAM_CELDA = 50;
const int TAM_LIENZO = 8;
//
const int COL_LIENZO = 5;

int celdas[TAM_LIENZO][TAM_LIENZO];

bool aceptaConexiones = true;
int numUsuarios = 0;
std::thread usuarios[MAX_USUARIOS];
int hilosOcupados[MAX_USUARIOS];
pthread_mutex_t cerrojoHilos;

int buscaHiloLibre() { // BLOQUEANTE //
	pthread_mutex_lock(&cerrojoHilos);

	if (numUsuarios >= MAX_USUARIOS) {
		pthread_mutex_unlock(&cerrojoHilos);
		return -1;
	}
	
	int i = 0;
	while (i < MAX_USUARIOS && hilosOcupados[i] != -1)
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

int enviaRed(int sd, Message msg, bool& conexion, int thr)
{
    msg.to_bin();

	ssize_t sbytes = send(sd, msg.data(), Message::MESSAGE_SIZE, 0);
	if (sbytes < 0) {
		std::cout << "ERROR de servidor al ENVIAR desde el hilo " << thr << "\n";
		conexion = false;
        return -1;
	}

    return 0;
}

//TRATAR LA CONEXIÓN (SEGUNDO WHILE)
int trataConexion(int cliente_sd, int thr) {
	/*
	pthread_mutex_lock(&cerrojoHilos);
	numUsuarios++;
	hilosOcupados[thr] = cliente_sd;
	pthread_mutex_unlock(&cerrojoHilos);
	*/

	std::cout << "Hilo " << thr << " trabajando\n";
	
	bool conexion = true;

	while (conexion) {
		char bufferR[Message::MESSAGE_SIZE];

		ssize_t rbytes = recv(cliente_sd, bufferR, Message::MESSAGE_SIZE, 0);
		if (rbytes < 0) {
			std::cout << "ERROR al RECIBIR en el hilo " << thr << "\n";
			conexion = false;
			continue;
		}

		Message msg;
    	msg.from_bin(bufferR);

		Message resp;
		switch (msg.msgType)
		{
		case Message::SYNCREQ:
			if (msg.cellValue == 0) {
				// enviar respuesta al cliente en concreto
				resp = Message(Message::SYNCRES, TAM_LIENZO, TAM_CELDA, msg.cellValue);
				enviaRed(cliente_sd, resp, conexion, thr);
				std::cout << "[!] Enviadas propiedades del lienzo pedidas por hilo cliente " << thr << "\n";
			}
			else {
				// enviar el estado actual de las celdas del lienzo
				for (int i = 0; i < TAM_LIENZO; i++) {
					for (int j = 0; j < TAM_LIENZO; j++) {
						Message c = Message(Message::UPDATE, i, j, celdas[i][j]);
						enviaRed(cliente_sd, c, conexion, thr);
					}
				}
				Message f = Message(Message::SYNCRES, 0, 0, 1);
				enviaRed(cliente_sd, f, conexion, thr);
				std::cout << "[!] Enviadas celdas del lienzo pedidas por hilo cliente " << thr << "\n";
			}
			break;
		case Message::UPDATE:
			if (msg.xPos_canvasSize < TAM_LIENZO && msg.xPos_canvasSize >= 0 &&
      	      	msg.yPos_cellSize < TAM_LIENZO && msg.yPos_cellSize >= 0) {
				std::cout << "[!] Recibida celda del hilo cliente " << thr << ", reenviando a los clientes...\n";
				// para servidor:
        	    celdas[msg.xPos_canvasSize][msg.yPos_cellSize] = msg.cellValue;
				// para todos los clientes actuales:
				resp = Message(Message::UPDATE, msg.xPos_canvasSize, msg.yPos_cellSize, msg.cellValue);
				// reenviar actualización al cliente en concreto
				enviaRed(cliente_sd, resp, conexion, thr);
				// reenviar actualización al resto de clientes
				for (int ind = 0; ind < MAX_USUARIOS; ind++)
					if (hilosOcupados[ind] != -1 && hilosOcupados[ind] != cliente_sd)
						enviaRed(hilosOcupados[ind], resp, conexion, thr);
			}
    	    else {
    	        std::cout << "MENSAJE inválido: actualización ilegal\n";
    	    }
			break;
		case Message::SYNCRES: // ¿? //
    	    std::cout << "MENSAJE inválido: servidor recibe 'SYNCRES'\n";
    	    break;
    	case Message::EXIT:
			std::cout << "[!] Recibida señal de finalización por hilo cliente " << thr << ", reenviando a cliente...\n";
    	default:
    	    conexion = false;
			// reenviar señal de cierre al cliente en concreto
			resp = Message(Message::EXIT, 0, 0, 0);
            enviaRed(cliente_sd, resp, conexion, thr);
    	    break;
		}
	}

	// impresión por pantalla
	std::cout << "Hilo " << thr << " ha terminado\n";

	pthread_mutex_lock(&cerrojoHilos);
	hilosOcupados[thr] = -1;
	numUsuarios--;
	pthread_mutex_unlock(&cerrojoHilos);

	return 0;
}

int main(int argc, char** argv){

	if (TAM_CELDA < 0 || TAM_LIENZO < 0 || MAX_USUARIOS < 0)
		return -1;

	for (int i = 0; i < MAX_USUARIOS; i++)
		hilosOcupados[i] = -1;

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
	
	bool aux = true;
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
			aux = true;
			
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
			hilosOcupados[hl] = client_sd;
			pthread_mutex_unlock(&cerrojoHilos);
			/**/
			usuarios[hl] = std::thread(trataConexion, client_sd, hl);
			usuarios[hl].detach();
		}
		else {
			if (aux)
				printf("Imposible conectarse: número máximo de usuarios alcanzado...\n");
			aux = false;
		}

		///pthread_mutex_lock(&cerrojoHilos);
		///printf("SALE-%i\n", numUsuarios);
		///pthread_mutex_unlock(&cerrojoHilos);
	}

	////// (i) no hay que esperar a que terminen los hilos de usuario (detach)
	////for (int i = 0; i < MAX_USUARIOS; i++)
	////	usuarios[i].join();
	
	// ... // cerrar socket
	
	// liberar memoria dinámica
	freeaddrinfo(result);

	pthread_mutex_destroy(&cerrojoHilos);

	// éxito
	return 0;
}
