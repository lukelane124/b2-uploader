#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "sha1.h"
uint16_t p = 0;
char* calcSha1Sum(char* filepath)
{
	char* ret = NULL;
	SHA1_CTX sha_context;
	unsigned char digest[20];
	FILE* inFile = NULL;
	size_t readOffset = 0;
	size_t status = 0;
	//feof();
	uint8_t rawData[65536];
	//printf("running sha1 sum.\n");
	//printf("filepath: %s\n", filepath);
	SHA1Init(&sha_context);
	//printf("sha1 init.\n");
	inFile = fopen(filepath, "rb");
	//fprintf(stderr, "debug here: %d\n", p++);
	if (inFile != NULL)
	{
		//fprintf(stderr, "debug here: %d\n", p++);
		while(feof(inFile) == 0)
		{
			//fprintf(stderr, "debug here: %d\n", p++);
			//fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
			memset((void*) rawData, 0, sizeof(rawData));
			status = fread(rawData, 1, sizeof(rawData), inFile);
			if (status < sizeof(rawData) && feof(inFile) == 0)
			{
				fprintf(stderr, ">>>>>Unknown file error occurred during fread on %s\n", filepath);
			}
			else
			{
				//void SHA1Update(SHA1_CTX* context, const unsigned char* data, u_int32_t len)
				SHA1Update(&sha_context, rawData, status);
			}
		}
	}
	else
	{
		fprintf(stderr, "fileError: %s\n", strerror(errno));
	}
	//void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
	SHA1Final(digest, &sha_context);
	ret = malloc(41);
	if (ret != NULL)
	{
		for (status = 0; status < 20; status++)
		{
			sprintf(&ret[status*2], "%02x", (int)digest[status]);
		}
		printf("sha1Sum: %*s\n", 41, ret);
	}

	
	return ret;
}
