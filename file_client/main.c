/*
 * main.c
 *
 *  Created on: 21/06/2026
 *      Author: Milorad
 */

#include <stdio.h>
#include "file_client.h"

#define IP "192.168.1.11"


int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: %s <file name>\n", argv[0]);
    return 1;
  }

  char *pfname = argv[1];

  printf("Get file '%s' from %s:\n", pfname, IP);

  FileClient_GetFile(IP, pfname);

  return 0;
}
