# Simple TFTP implementation

This repository contains a simple TFTP implementation
([RFC1350](https://tools.ietf.org/html/rfc1350)), made as a project for the
Course in Newtworking @ University of Pisa.

The [project assignment](http://www2.ing.unipi.it/c.vallati/files/reti_2018/Progetto2018.pdf) requires to:
 1. handle only read requests from client to server (download)
 2. assume that the connection is reliable (no packets can be lost or altered,
 no retransmission)
 3. handle only File Not Found and Illegal TFTP operation errors

The server can be started with the following syntax:
```
$ ./tftp_server <listening_port> <files_directory>
```

The server is implemented as multi-process, with each new process handling a new
"connection".

Example:
```
$ path/to/tftp_server 9999 test/
```

The client can be started with the following syntax:
```
$ ./tftp_client <server_IP_address> <server_port>
```

The client should also support the following operations:
 - `!help`: prints an help message.
 - `!mode {txt|bin}`: change prefered transfer mode to netascii or octet.
 - `!get <filename> <local_filename>`: download `<filename>` from server and 
 save it to `<local_filename>`.
 - `!quit`: exit client

Example of client operation:
```
$ path/to/tftp_client 127.0.0.1 9999
> !mode txt
...
> !get test.txt my_test.txt
...
> !quit
```
