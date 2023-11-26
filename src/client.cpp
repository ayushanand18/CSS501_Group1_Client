#include "client.hpp"

int main()
{
    rpc::client client("127.0.0.1", 8080);
    FSS_Client::Client instance(client);
    instance.init();
}