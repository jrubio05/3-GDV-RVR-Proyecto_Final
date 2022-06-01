// $ ./cliente 127.0.0.1 2222

#include <iostream>     // cout
#include <sys/types.h>	// getaddrinfo
#include <sys/socket.h>	// getaddrinfo
#include <netdb.h>      // getaddrinfo
#include <string.h>     // memset
#include <stdio.h>      // perror
#include <unistd.h>     // sleep
#include <thread>       // std::thread
#include <mutex>		// mutex
//
#include "XLDisplay.h"  // GRÁFICOS
//
#include "Message.h"	// mensajes de red

int TAM_LIENZO;
int TAM_CELDA;
//
const int COL_LIENZO = 5;

bool salir = false;
bool sincronizado = false;
int posX = 0;
int posY = 0;

pthread_mutex_t cerrojoMundoJuego;

// vvv MÉTODOS DE DIBUJADO vvv //

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

void renderizaLienzo(int** &celdas)
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

// ^^^ MÉTODOS DE DIBUJADO ^^^ //

// vvv CLIENTE vvv //

int enviaRed(int sd, Message msg)
{
    //char bufferEnvio[Message::MESSAGE_SIZE];
    
    //Serializar el objeto
    msg.to_bin();

    //Enviar el objeto binario usando el socket sd
	ssize_t sbytes = send(sd, msg.data(), Message::MESSAGE_SIZE, 0);
	if (sbytes < 0) {
		std::cout << "ERROR de cliente al ENVIAR\n";
		salir = true;
        return -1;
	}

    return 0;
}

int recibeRed(int sd, int** &celdas)
{
    char bufferRecepcion[Message::MESSAGE_SIZE];

	ssize_t rbytes = recv(sd, bufferRecepcion, Message::MESSAGE_SIZE, 0);
    if (rbytes < 0) {
		std::cout << "ERROR de cliente al RECIBIR\n";
		salir = true;
        return -1;
	}

    Message msg;
    msg.from_bin(bufferRecepcion);

    switch (msg.msgType)
    {
    case Message::SYNCRES:
        if (!sincronizado) {
            if (msg.xPos_canvasSize >= 0 && msg.yPos_cellSize >= 0) 
            {
                TAM_CELDA = msg.yPos_cellSize;
                TAM_LIENZO = msg.xPos_canvasSize;
                /**/
                celdas = new int*[TAM_LIENZO];
                for(int i = 0; i < TAM_LIENZO; i++) {
                    celdas[i] = new int[TAM_LIENZO];
                    for(int j = 0; j < TAM_LIENZO; j++) {
                       celdas[i][j] = COL_LIENZO;
                    }
                }
                sincronizado = true;
                std::cout << "[!] Recibidos parámetros del lienzo del servidor\n";
            }
            else {
                std::cout << "MENSAJE inválido: sincronización ilegal\n";
                return -1;
            }
        }
        else {
            std::cout << "MENSAJE inválido: sincronización doble\n";
        }
        break;
    case Message::UPDATE:
        if (sincronizado && msg.xPos_canvasSize < TAM_LIENZO && msg.xPos_canvasSize >= 0 &&
            msg.yPos_cellSize < TAM_LIENZO && msg.yPos_cellSize >= 0) {
            std::cout << "[!] Recibida celda del servidor\n";
            celdas[msg.xPos_canvasSize][msg.yPos_cellSize] = msg.cellValue;
        }
        else {
            std::cout << "MENSAJE inválido: actualización ilegal\n";
        }
        break;
    case Message::SYNCREQ: // ¿? //
        std::cout << "MENSAJE inválido: cliente recibe 'SYNCREQ'\n";
        break;
    case Message::EXIT:
        std::cout << "[!] Recibida señal de finalización del servidor\n";
    default:
        salir = true;
        break;
    }

    return 0;
}

// ^^^ CLIENTE ^^^ //

// vvv MÉTODOS DE ENTRADA vvv //

void obtenEntrada(int sd)
{
    XLDisplay& dpy = XLDisplay::display();

    char k;

    do
    {
         k = dpy.wait_key();
    } while (k != 'q' && k != 'w' && k != 'a' && k != 's' && k != 'd' &&
        k != '0' && k != '1' && k != '2' && k != '3' && k != '4' && k != '5' && k != '6');
    
    Message msg;

    switch (k) {
        default:
        case 'q':
            msg = Message(Message::EXIT, 0, 0, 0);
            enviaRed(sd, msg);
            break;
        case '0':
            msg = Message(Message::UPDATE, posX, posY, 0);
            enviaRed(sd, msg);
            break;
        case '1':
            msg = Message(Message::UPDATE, posX, posY, 1);
            enviaRed(sd, msg);
            break;
        case '2':
            msg = Message(Message::UPDATE, posX, posY, 2);
            enviaRed(sd, msg);
            break;
        case '3':
            msg = Message(Message::UPDATE, posX, posY, 3);
            enviaRed(sd, msg);
            break;
        case '4':
            msg = Message(Message::UPDATE, posX, posY, 4);
            enviaRed(sd, msg);
            break;
        case '5':
            msg = Message(Message::UPDATE, posX, posY, 5);
            enviaRed(sd, msg);
            break;
        case '6':
            msg = Message(Message::UPDATE, posX, posY, 6);
            enviaRed(sd, msg);
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

// ^^^ MÉTODOS DE ENTRADA ^^^ //

// vvv CLIENTE vvv //

int trataRedBuclePpal(int sd, int** celdas)
{
    // bucle principal
    while (!salir) {
        recibeRed(sd, celdas);
        //
        pthread_mutex_lock(&cerrojoMundoJuego);
        renderizaLienzo(celdas);
        pthread_mutex_unlock(&cerrojoMundoJuego);
    }

    return 0;
}

int trataConexion(int sd)
{
    // celdas del lado cliente
    int** celdas;
    
    // sincro
    Message peticion = Message(Message::SYNCREQ, 0, 0, 0);
    enviaRed(sd, peticion);
    /**/
    int aux = 0;
    while (!sincronizado && aux == 0) {
        aux = recibeRed(sd, celdas);
    }
    if (aux == -1) {
        std::cout << "NO se pudo compartir el lienzo del servidor\n";
        return -1;    
    }

    int** celdasBis = celdas;
    // se trata la recepción de red durante el bucle principal aparte
    std::thread(trataRedBuclePpal, sd, std::move(celdasBis)).detach();

    // bucle principal (input (con posible envío) + render)
	XLDisplay::init(TAM_LIENZO * TAM_CELDA, TAM_LIENZO * TAM_CELDA, "Proyecto-Redes");
    while (!salir) {
        obtenEntrada(sd);
        //
        pthread_mutex_lock(&cerrojoMundoJuego);
        renderizaLienzo(celdas); // aunque solo va a cambiar la casilla seleccionada...
        pthread_mutex_unlock(&cerrojoMundoJuego);
    }

    return 0;
}

int main(int argc, char** argv){

    // cerrojo
    pthread_mutex_init(&cerrojoMundoJuego, NULL);

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
    trataConexion(sd);

	// ... // cerrar socket
	
	// liberar memoria dinámica
	freeaddrinfo(result);

    // cerrojo
    pthread_mutex_destroy(&cerrojoMundoJuego);

	// éxito
	return 0;
}

// ^^^ CLIENTE ^^^ //

/*
TENGO QUE BORRAR LA MEMORIA DINÁMICA DE LAS CELDAS **
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
QUE NO SE ME OLVIDE
*/
