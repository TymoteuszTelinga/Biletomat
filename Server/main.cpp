
#include "Server.h"

int main()
{
    TicketServer server;
    server.LoadData("tickets.txt");
    server.Run();

    return 0;
}