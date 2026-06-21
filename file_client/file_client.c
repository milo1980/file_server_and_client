/*
 * file_client.c
 *
 *  Created on: 21/06/2026
 *      Author: Milorad
 */
#include <stdio.h>
#include <winsock2.h>

#define BUFFER_SIZE 4096

static char buffer[BUFFER_SIZE];

void FileClient_GetFile(char *ip, char *fname)
{
  WSADATA wsa;
  int bytes;
  struct sockaddr_in server;
  SOCKET s;
  int rc;
  FILE *fp;

  WSAStartup(MAKEWORD(2, 2), &wsa);

  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  server.sin_family = AF_INET;
  server.sin_port = htons(9000);
  server.sin_addr.s_addr = inet_addr(ip);

  rc = connect(s, (struct sockaddr*) &server, sizeof(server));
  if (rc != 0)
  {
    printf("\tConnect failed\n");
    WSACleanup();
    return;
  }

  rc = send(s, fname, (int) strlen(fname), 0);
  if (rc <= 0)
  {
    printf("\tSend failed\n");
    WSACleanup();
    return;
  }

  bytes = recv(s, buffer, sizeof(buffer), 0);
  if (bytes > 0)
  {
    fp = fopen(fname, "wb");

    while (bytes > 0)
    {
      fwrite(buffer, 1, bytes, fp);
      bytes = recv(s, buffer, sizeof(buffer), 0);
    }

    fclose(fp);
  }

  closesocket(s);
  WSACleanup();

  printf("Done\n");
}
