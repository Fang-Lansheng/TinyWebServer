# TinyWebServer
A tiny HTTP server implemented by epoll in Linux.

## 1. Features
![main_functions.png](https://s2.loli.net/2022/04/17/dAgxzLZhwIrpiba.png)

## 2. Quickstart

### 2.1 Requirements
- Server:
  - OS: Linux (sorry, but *epoll* is a Linux-specific API).
  - Database: MySQL >= 5.2.6
- Client:
  - Mainstream web browsers: Chrome, Safari, Firefox, etc.

### 2.2 MySQL Configuration
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
INSERT INTO user(username, password) VALUES('user1', 'password1');
```

3. Store your information in `DatabaseInfo.txt`.
```text
port        3006
user        mysql_username
passwd      mysql_password
database    web_server
```
</details>

### 2.3 Build 
```shell
cd build
make
```
This might take a few seconds.

### 2.4 Run
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

## 3. Test
### 3.1 Unit Testing

<details>
<summary>Click to expand.</summary>

- Build

```shell
cd test
make
```

- Test (*Hit Enter to exit*)
```shell
cd ..  # /path/to/your/project
./test/unit_test
```

- Check the log files `logs/test_log_1/{DATE}.log`,
  `logs/test_log_2/{DATE}.log` and `logs/test_thread_pool/{DATE}.log`.

</details>

### 3.2 Stress Testing
- Build
```shell
cd test/webbench-1.5/
make
```

- Run
```shell
cd ../..  # /path/to/your/project
./test/webbench -c 2000 -t 10 http://localhost:9006/
```

- My stress testing result:
  - Environment information:
    - OS: Ubuntu 20.04
    - Memory: 2 GB
    - CPU: Intel(R) Xeon(R) Platinum 8269CY CPU @ 2.50GHz (2 core)
  
  - Webbench output:
    ```text
    Webbench - Simple Web Benchmark 1.5
    Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.
    
    Benchmarking: GET http://localhost:9006/
    2500 clients, running 10 sec.
    
    Speed=180516 pages/min, 9421508 bytes/sec.
    Requests: 30086 succeed, 0 failed.
    ```



## 4. Further Reading 📖
- [Problems encountered](http://jifan.tech/projects/tiny-web-server/problems/) 
  when building and implementing this project.
- (Coming soon …) Detailed explanations of this project:
  - [Thread]()
  - [HTTP connection, parsing and response]()
  - [Timer]()
  - [Database manipulation and connection pool]()
- More about network programming:
  - [*The Linux Programming Interface*](https://man7.org/tlpi/) (Chapter 56-61, 63).

## Acknowledgements

- [qinguoyi/TinyWebServer: Linux下C++轻量级Web服务器](https://github.com/qinguoyi/TinyWebServer)
- [markparticle/WebServer: C++ Linux WebServer服务器](https://github.com/markparticle/WebServer)
