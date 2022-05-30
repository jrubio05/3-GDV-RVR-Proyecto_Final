#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Serializable.h"

/**
 *  Mensaje del protocolo de la aplicación de Chat
 */
class Message: public Serializable
{
public:
    static const size_t MESSAGE_SIZE = sizeof(int) * 3 + sizeof(uint8_t);

    enum MessageType
    {
        SYNCREQ = 0,
        SYNCRES = 1,
        UPDATE  = 2,
        EXIT    = 3
    };

    Message(){};

    Message(const uint8_t& type, const int& x, const int& y, const int& val) :
        msgType(type), xPos_canvasSize(x), yPos_cellSize(y), cellValue(val) {};

    void to_bin();

    int from_bin(char * bobj);

    uint8_t msgType;
    int xPos_canvasSize;
    int yPos_cellSize;
    int cellValue;
};
