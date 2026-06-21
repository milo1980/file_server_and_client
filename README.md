# file_server_and_client
Small file server and client on C.

Built with Eclipse version 2026-06 (4.40.0) and MinGW.

## Usage
### File server

```
> file_server.exe
Files available:
  file.txt
  file_server.exe
  file_server.o
  main.o
Listening on port 9000
Connected: 192.168.1.11
Client requested: file.txt
Sending file...
Done
Connected: 192.168.1.11
Client requested: file.out
Sending file...
Cannot open file
Done
Connected: 192.168.1.11
Client requested: file_server.o
Sending file...
Done
```

### File client
```
> file_client.exe file.txt
Get file 'file.txt' from 192.168.1.11:
Done
```

