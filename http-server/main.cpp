#include "server.h"

void main()
{
	HttpServer h(8080);
	h.Start();
}
