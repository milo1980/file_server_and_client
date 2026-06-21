/*
 * main.c
 *
 *  Created on: 19/06/2026
 *      Author: Milorad
 */

/* file_server_win.c */

#include <stdio.h>
#include "file_server.h"

int main(void)
{
  int rc;

  FileServer_ListFiles();
  rc = FileServer_Open();
  if (rc == 0)
  {
    fflush(stdout);
    FileServer_Handler();
  }

  return 0;
}
