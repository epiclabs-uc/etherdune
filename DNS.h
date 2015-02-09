#ifndef __DNS__
#define __DNS__

#include "NetworkService.h"

class DNS : public NetworkService
{
public:

	IPAddress resolve(const char* name);
	//único slot para una única pregunta
	//queda libre una vez retornado el valor o pasado un tiempo
	//¿calcular checksum de la cadena a modo de recordar el identificador de query DNS?

};

#endif