# TinyWebServer
A tiny HTTP server implemented by epoll in Linux.

## Features
![main_functions.png](https://s2.loli.net/2022/04/17/dAgxzLZhwIrpiba.png)

## Quickstart

### MySQL Configuration
<details>
  <summary>Click to expand.</summary>

1. Log in to your MySQL server.
```shell
mysql -u root -p
```
2. Create a new database and insert some records.
```mysql
# Create a new database 
CREATE DATABASE web_server;
USE web_server;

# Create a new table
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
) ENGINE=InnoDB;

# Insert an record
INSERT INTO user(username, password) VALUES('user1', 'password1')
```

3. Store your information in `DatabaseInfo.txt`.
```text
port        3006
user        mysql_username
passwd      mysql_password
database    web_server
```
</details>

### Build 
```shell
cd build
make
```
This might take a few seconds.

### Run
```shell
cd ..  # /path/to/your/project
./bin/server
```
<details>
<summary>Arguments</summary>

```shell
./bin/server [-p port] [-m trigger_mode] [-o opt_linger] [-d conn_pool_num] [-t thread_num] [-l open_log]
```

- `-p`: port to the server. Default: `9006`.
- `-m`: trigger mode. Default: `3` (ET mode).
    - `0`: LT + LT.
    - `1`: LT + ET.
    - `2`: ET + LT.
    - `3`: ET + ET.
- `-o`: Weather to use opt linger. Default: `1` (true).
    - `0`: false.
    - `1`: true.
- `-d`: Number of connections in database connection pool. Default: `12`.
- `-t`: Number of thread in thread pool. Default: `6`.
- `-l`: Weather to open log. Default: `1` (true).
    - `0`: false.
    - `1`: true.

For more details, see [src/config/config.h](src/config/config.h).

</details>

## Acknowledgement

- [qinguoyi/TinyWebServer: Linux下C++轻量级Web服务器](https://github.com/qinguoyi/TinyWebServer)
- [markparticle/WebServer: C++ Linux WebServer服务器](https://github.com/markparticle/WebServer)
