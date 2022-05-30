//>./client 127.0.0.1 2222

#include <iostream>	// cout
#include <sys/types.h>	// getaddrinfo
#include <sys/socket.h>	// getaddrinfo
#include <netdb.h>	// getaddrinfo
#include <string.h>	// memset
#include <stdio.h>	// perror
#include <unistd.h>	// sleep
//
#include "XLDisplay.h" // GRÁFICOS

#define BUFFSIZE 128

////////////////////////////////////tmp/////////////////////
const int TAM_LIENZO = 8;
const int TAM_CELDA = 50;
////////////////////////////////////tmp/////////////////////

bool salir = false;
int celdas[TAM_LIENZO][TAM_LIENZO];
int posX = 0;
int posY = 0;

// MÉTODOS DE DIBUJADO //

void pintaCelda(XLDisplay& dpy, int oriX, int oriY, int color)
{
    switch(color) {
        default:
        case 0:
            dpy.set_color(XLDisplay::WHITE);
            break;
        case 1:
            dpy.set_color(XLDisplay::RED);
            break;
        case 2:
            dpy.set_color(XLDisplay::YELLOW);
            break;
        case 3:
            dpy.set_color(XLDisplay::GREEN);
            break;
        case 4:
            dpy.set_color(XLDisplay::BLUE);
            break;
        case 5:
            dpy.set_color(XLDisplay::BLACK);
            break;
        case 6:
            dpy.set_color(XLDisplay::BROWN);
            break;
    }

    for (int i = 0; i < TAM_CELDA; i++)
        dpy.line(oriX + i, oriY, oriX + i, oriY + TAM_CELDA);
}

void pintaSeleccionCelda(XLDisplay& dpy, int oriX, int oriY)
{
    dpy.set_color(XLDisplay::WHITE);
    
    // borde izdo.
    dpy.line(oriX, oriY, oriX, oriY + TAM_CELDA);
    dpy.line(oriX + 1, oriY, oriX + 1, oriY + TAM_CELDA);

    // borde dcho.
    dpy.line(oriX + TAM_CELDA, oriY, oriX + TAM_CELDA, oriY + TAM_CELDA);
    dpy.line(oriX + TAM_CELDA - 1, oriY, oriX + TAM_CELDA - 1, oriY + TAM_CELDA);

    // borde sup.
    dpy.line(oriX, oriY, oriX + TAM_CELDA, oriY);
    dpy.line(oriX, oriY + 1, oriX + TAM_CELDA, oriY + 1);

    //borde inf.
    dpy.line(oriX, oriY + TAM_CELDA, oriX + TAM_CELDA, oriY + TAM_CELDA);
    dpy.line(oriX, oriY + TAM_CELDA - 1, oriX + TAM_CELDA, oriY + TAM_CELDA - 1);
}

void renderizaLienzo()
{
    XLDisplay& dpy = XLDisplay::display();

    dpy.clear();

    dpy.flush();

    int celX = posX * TAM_CELDA;
    int celY = posY * TAM_CELDA;

    for (int i = 0; i < TAM_LIENZO; i++)
        for (int j = 0; j < TAM_LIENZO; j++)
            pintaCelda(dpy, i * TAM_CELDA, j * TAM_CELDA, celdas[i][j]);

    pintaSeleccionCelda(dpy, celX, celY);

    dpy.flush();
}

// MÉTODOS DE DIBUJADO //

// MÉTODOS DE ENTRADA //

void obtenEntrada()
{
    XLDisplay& dpy = XLDisplay::display();

    char k;

    do
    {
         k = dpy.wait_key();
    } while (k != 'q' && k != 'w' && k != 'a' && k != 's' && k != 'd' &&
        k != '0' && k != '1' && k != '2' && k != '3' && k != '4' && k != '5' && k != '6');
    
    switch (k) {
        default:
        case 'q':
            salir = true;
            break;
        case '0':
            celdas[posX][posY] = 0;
            break;
        case '1':
            celdas[posX][posY] = 1;
            break;
        case '2':
            celdas[posX][posY] = 2;
            break;
        case '3':
            celdas[posX][posY] = 3;
            break;
        case '4':
            celdas[posX][posY] = 4;
            break;
        case '5':
            celdas[posX][posY] = 5;
            break;
        case '6':
            celdas[posX][posY] = 6;
            break;
        case 'w':
            if (posY - 1 >= 0)
                posY--;
            break;
        case 'a':
            if (posX - 1 >= 0)
                posX--;
            break;
        case 's':
            if (posY + 1 <= TAM_LIENZO - 1)
                posY++;
            break;
        case 'd':
            if (posX + 1 <= TAM_LIENZO - 1)
                posX++;
            break;
    }
}

// MÉTODOS DE ENTRADA //

// CLIENTE //

int trataConexion(/*int cliente_sd, int thr*/)///////////////////
{
	XLDisplay::init(TAM_LIENZO * TAM_CELDA, TAM_LIENZO * TAM_CELDA, "Proyecto-Redes");

	///////tmp
    for (int i = 0; i < TAM_LIENZO; i++)
        for (int j = 0; j < TAM_LIENZO; j++)
            celdas[i][j] = 5;
	///////tmp

    while (!salir) {
        renderizaLienzo();
        obtenEntrada();
    }

    return 0;
}

int main(int argc, char** argv){
	// criterios
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; // Direcciones tipo IPv4
	hints.ai_socktype = SOCK_STREAM; // Flujo (socket tipo TCP)
	// lista de resultados
	struct addrinfo *result;
	
	// obtener resultados
	int luis = getaddrinfo(argv[1], argv[2], &hints, &result);
	// error?
	if (luis != 0) {
		std::cerr << "Error getaddrinfo: " << gai_strerror(luis) << "\n";
		return -1;
	}
	
	// apertura y conexión
	int sd = socket(result->ai_family, result->ai_socktype, 0);
	if (sd == -1) {
		perror("ERROR_EN_SOCKET"); /// o strerror(errno)
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}
	int juan = connect(sd, result->ai_addr, result->ai_addrlen);
	if (juan == -1) {
		perror("ERROR_EN_CONNECT");
		// ... // cerrar socket
		freeaddrinfo(result); // liberar memoria dinámica
		return -1;
	}
	
	// comunicación
    trataConexion();
	/*
	while (!salir) {
		
		char bufferUsuario[BUFFSIZE];
		char bufferRecepcion[BUFFSIZE];
		
		// recoger entrada
		std::cin.getline(bufferUsuario, BUFFSIZE);
		/////bufferUsuario[BUFFSIZE - 1] = '\0';
		/////printf("\t___%li\n", strlen(bufferUsuario));////////////////////////
		/////sleep(2);
		///// se traba al rebosar el búffer y no alcanzo a ver por qué
		if (strlen(bufferUsuario) > BUFFSIZE - 1) { // truncar
			bufferUsuario[BUFFSIZE - 1] = '\n';
		}
		else { // concatenar
			strncat(bufferUsuario, "\n", 2);
		}
		
		// control: cerrar cliente
		if (strcmp(bufferUsuario, "Q\n") == 0) {
			salir = true;
			continue;
		}
		
		// envío del mensaje
		ssize_t sbytes = send(sd, bufferUsuario, strlen(bufferUsuario), 0);
		if (sbytes == -1) {
			perror(NULL);
			// ... // cerrar socket
			freeaddrinfo(result); // liberar memoria dinámica
			return -1;
		}
		
		// recepción de la réplica
		int rbytes = recv(sd, bufferRecepcion, strlen(bufferUsuario), 0);
		if (rbytes == -1) {
			perror(NULL);
			// ... // cerrar socket
			freeaddrinfo(result); // liberar memoria dinámica
			return -1;
		}
		// ¡indicar finalización de cadena!
		bufferRecepcion[rbytes]='\0';
		
		// impresión por pantalla
		std::cout << bufferRecepcion;
	}
	*/

	// ... // cerrar socket
	
	// liberar memoria dinámica
	freeaddrinfo(result);

	// éxito
	return 0;
}

// CLIENTE //
