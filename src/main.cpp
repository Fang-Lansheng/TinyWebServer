#include "config/config.h"
#include "server/webserver.h"

using std::string;

int main(int argc, char* argv[]) {
    // Server config
    Config config;
    config.ParseArg(argc, argv);

    // Database information
    DatabaseInfo db_info("./DatabaseInfo.txt");
//    DatabaseInfo db_info(3006, "root", "password", "database_name");

    // Web server
    WebServer server(config, db_info);
    server.Start();

    return 0;
}
