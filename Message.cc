#include "Message.h"

#include <stdio.h>

void Message::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos en el buffer _data
    char* tmp = _data;
    memcpy(tmp, &msgType, sizeof(uint8_t));
    tmp += sizeof(uint8_t);
    memcpy(tmp, &xPos_canvasSize, sizeof(int));
    tmp += sizeof(int);
    memcpy(tmp, &yPos_cellSize, sizeof(int));
    tmp += sizeof(int);
    memcpy(tmp, &cellValue, sizeof(int));
}

int Message::from_bin(char* bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    memcpy(&msgType, bobj, sizeof(uint8_t));
    bobj += sizeof(uint8_t);
    memcpy(&xPos_canvasSize, bobj, sizeof(int));
    bobj += sizeof(int);
    memcpy(&yPos_cellSize, bobj, sizeof(int));
    bobj += sizeof(int);
    memcpy(&cellValue, bobj, sizeof(int));

    return 0;
}
