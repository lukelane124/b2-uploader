#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "c_defs.h"

const char* UsageCopyWrite = "Copyright: Tommy Lane (L&L Operations) 2020";
const char* key = "085f4048015902475b4263661e3b5d080f1559002362407f443b742d260e15";

char secret[40];
char temp[5];

int main(int argc, char** argv, char** envp)
{
	char* inputString = argv[1];
	char *pos = key;
	for (int i = 0; i < strlen(inputString); i++)
	{
		printf("%02x", (char)(inputString[i] ^ (UsageCopyWrite[i%strlen(UsageCopyWrite)]) ));
	}
	printf("\n");

	memset((void*)secret, 0, sizeof(secret));
	LOG_DEBUG("decoding secret\n");
	for (size_t count = 0; count < sizeof(secret); count++) {
        sscanf(pos, "%2hhx", &secret[count]);
        secret[count] = (secret[count] ^ (UsageCopyWrite[(count)%strlen(UsageCopyWrite)]));
        pos += 2;
        if (!*pos)
        {
        	break;
        }
    }
	printf("secret: %s\n", secret);


}