#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "c_defs.h"

int returnCode = 0;

const char* UsageCopyWrite = "Copyright: Tommy Lane (L&L Operations) 2020";
char* programName = NULL;

char secret[40];
char temp[5];

void Usage(void)
{
	fprintf(stderr, "Usage:\n\t%s <secret key for obfuscation>\n\n\n", programName);
	returnCode = -1;
}

int main(int argc, char** argv, char** envp)
{
	programName = argv[0];
	char* inputString = argv[1];
	// char *pos = key;
	if (argc < 2)
	{
		Usage();
	}
	else
	{
		for (int i = 0; i < strlen(inputString); i++)
		{
			printf("%02x", (char)(inputString[i] ^ (UsageCopyWrite[i%strlen(UsageCopyWrite)]) ));
		}
		printf("\n");

		// memset((void*)secret, 0, sizeof(secret));
		// LOG_DEBUG("decoding secret\n");
		// for (size_t count = 0; count < sizeof(secret); count++) {
	 //        sscanf(pos, "%2hhx", &secret[count]);
	 //        secret[count] = (secret[count] ^ (UsageCopyWrite[(count)%strlen(UsageCopyWrite)]));
	 //        pos += 2;
	 //        if (!*pos)
	 //        {
	 //        	break;
	 //        }
	 //    }
		// printf("secret: %s\n", secret);
	}
	return returnCode;

}