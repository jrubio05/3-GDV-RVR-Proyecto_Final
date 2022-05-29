#include "XLDisplay.h"
#include <unistd.h>

const int TAM_CELDA = 50;
const int TAM_LIENZO = 8;
const int COL_LIENZO = 5;

bool salir = false;
int celdas[TAM_LIENZO][TAM_LIENZO];
int posX = 0;
int posY = 0;

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

void pintaLienzo(XLDisplay& dpy, int color)
{
    for (int i = 0; i < TAM_LIENZO; i++)
        for (int j = 0; j < TAM_LIENZO; j++)
            pintaCelda(dpy, i * TAM_CELDA, j * TAM_CELDA, color);
}

void draw()
{
    XLDisplay& dpy = XLDisplay::display();

    dpy.clear();

    dpy.flush();

    int celX = posX * TAM_CELDA;
    int celY = posY * TAM_CELDA;

    /////*pintaLienzo(dpy, 5);*/

    for (int i = 0; i < TAM_LIENZO; i++)
        for (int j = 0; j < TAM_LIENZO; j++)
            pintaCelda(dpy, i * TAM_CELDA, j * TAM_CELDA, celdas[i][j]);

    pintaSeleccionCelda(dpy, celX, celY);

    dpy.flush();

    /*
    // formas de ejemplo
    dpy.set_color(XLDisplay::RED);
    dpy.point(50,50);
    dpy.line(20,20,100,50);
    dpy.set_color(XLDisplay::BROWN);
    dpy.circle(45,45,15);
    dpy.set_color(XLDisplay::BLUE);
    dpy.rectangle(60,60,30,15);
    XPoint pts[] = {{100,100},{130,130},{100,130},{100,100}};
    dpy.set_color(XLDisplay::YELLOW);
    dpy.lines(pts, 4);
    dpy.set_color(XLDisplay::GREEN);
    dpy.text(10, 80, "Hola, mundo - BIENVENIDO");
    dpy.flush();
    */
}

void get_input()
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

void wait(int t)
{
    ////dpy.clear();

    ////dpy.flush();

    sleep(t);
}

int main()
{
    XLDisplay::init(TAM_LIENZO * TAM_CELDA, TAM_LIENZO * TAM_CELDA, "Proyecto-Redes");

    for (int i = 0; i < TAM_LIENZO; i++)
        for (int j = 0; j < TAM_LIENZO; j++)
            celdas[i][j] = COL_LIENZO;

    while (!salir) {
        draw();
        get_input();
    }

    return 0;
}
