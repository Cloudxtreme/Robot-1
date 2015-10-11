#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// percentF:percentB:percentR:percentL
// 000:000:000:000
// 15 Byte

int parseCmd( char* cmdString, int* pForw, int* pBack, int* pRight, int* pLeft)
{
  int rc = -1;

  if( cmdString != NULL )
  {
    rc = sscanf(cmdString, "%d:%d:%d:%d", pForw, pBack, pRight, pLeft);
  }

  return(rc);

}

main()
{
  int Forw, Back, Right, Left;
  int rc;

  rc = parseCmd("010:000:010:000", &Forw, &Back, &Right, &Left);
  fprintf(stderr, "Got %d args: %d, %d, %d, %d\n", rc, Forw, Back, Right, Left);
}

