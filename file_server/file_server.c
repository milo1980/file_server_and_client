/*
 * file_server.c
 *
 *  Created on: 21/06/2026
 *      Author: Milorad
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <winsock2.h>
#include <ws2tcpip.h>


#define SERVER_PORT     9000
#define MAX_CLIENTS     16
#define BUFFER_SIZE     4096
#define MAX_FILENAME    256
#define MAX_FILES       100


typedef struct
{
  char name[MAX_FILENAME];
  uint16_t version;
}file_entry_t;


static SOCKET server;
static WSADATA wsa;
static BOOL reuse = TRUE;
static struct sockaddr_in addr;
static SOCKET clients[MAX_CLIENTS];
static file_entry_t file_table[MAX_FILES] = {0};


static int fileserver_send(SOCKET client, const char *filename)
{
  char buffer[BUFFER_SIZE];
  size_t bytes;
  size_t total;
  int n;

  FILE *fp = fopen(filename, "rb");

  if (fp == NULL)
  {
    printf("Cannot open file\n");
    return -1;
  }

  bytes = fread(buffer, 1, BUFFER_SIZE, fp);
  while (bytes > 0)
  {
    total = 0;

    while (total < bytes)
    {
      n = send(client, buffer + total, (int) (bytes - total), 0);

      if (n <= 0)
      {
        fclose(fp);
        return -1;
      }

      total += n;
    }

    bytes = fread(buffer, 1, BUFFER_SIZE, fp);
  }

  fclose(fp);

  return 0;
}

void FileServer_ListFiles(void)
{
  WIN32_FIND_DATA fd;
  HANDLE h = FindFirstFile("*.*", &fd);
  uint16_t file_index;

  if (h != INVALID_HANDLE_VALUE)
  {
    printf("Files available:\n");

    file_index = 0;
    do
    {
      if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        printf("  %s\n", fd.cFileName);
        strcpy(file_table[file_index].name, fd.cFileName);
        file_table[file_index].version = 1;
        file_index++;
      }
    }
    while (FindNextFile(h, &fd));

    FindClose(h);
  }
}

int FileServer_Open(void)
{
  int rc;

  rc = WSAStartup(MAKEWORD(2, 2), &wsa);
  if (rc != 0)
  {
    printf("WSAStartup failed: %d\n", rc);
    return 1;
  }

  server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server == INVALID_SOCKET)
  {
    printf("Socket failed, WSA error := %d\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse, sizeof(reuse));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(SERVER_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;

  rc = bind(server, (struct sockaddr*) &addr, sizeof(addr));
  if (rc == SOCKET_ERROR)
  {
    printf("Bind failed, WSA error := %d\n", WSAGetLastError());
    WSACleanup();
    closesocket(server);
    return 1;
  }

  rc = listen(server, 5);
  if (rc == SOCKET_ERROR)
  {
    printf("Listen failed, WSA error := %d\n", WSAGetLastError());
    WSACleanup();
    closesocket(server);
    return 1;
  }

  printf("Listening on port %d\n", SERVER_PORT);

  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    clients[i] = INVALID_SOCKET;
  }

  return 0;
}

void FileServer_Handler(void)
{
  while (1)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server, &readfds);
    SOCKET maxfd = server;

    /*One thread, many clients*/

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i] != INVALID_SOCKET)
      {
        FD_SET(clients[i], &readfds);
        if (clients[i] > maxfd)
        {
          maxfd = clients[i];
        }
      }
    }

    int activity = select((int) maxfd + 1, &readfds, NULL, NULL, NULL);

    if (activity == SOCKET_ERROR)
    {
      printf("select failed\n");
      break;
    }

    /* New client */

    if (FD_ISSET(server, &readfds))
    {
      struct sockaddr_in client_addr;
      int len = sizeof(client_addr);

      SOCKET new_client = accept(server, (struct sockaddr*) &client_addr, &len);

      if (new_client != INVALID_SOCKET)
      {
        char *ip;
        ip = inet_ntoa(client_addr.sin_addr);

        printf("Connected: %s\n", ip);
        fflush(stdout);

        int inserted = 0;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
          if (clients[i] == INVALID_SOCKET)
          {
            clients[i] = new_client;
            inserted = 1;
            break;
          }
        }

        if (!inserted)
        {
          printf("Too many clients\n");
          fflush(stdout);
          closesocket(new_client);
        }
      }
    }

    /* Serve clients */

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i] != INVALID_SOCKET)
      {
        if (FD_ISSET(clients[i], &readfds))
        {
          char filename[MAX_FILENAME];
          int n = recv(clients[i], filename, sizeof(filename)-1, 0);

          if (n > 0)
          {
              filename[n] = '\0';

              printf("Client requested: %s\n", filename);
              printf("Sending file...\n");
              fflush(stdout);

              fileserver_send(clients[i], filename);
          }

          closesocket(clients[i]);
          clients[i] = INVALID_SOCKET;

          printf("Done\n");
          fflush(stdout);
        }
      }
    }
  }

  closesocket(server);
  WSACleanup();

}
