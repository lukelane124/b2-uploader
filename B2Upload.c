#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <curl/curl.h>
#define GLOBAL_BUFF_SIZE 4096
uint8_t BUFFER[GLOBAL_BUFF_SIZE];
size_t BUFFEROffset = 0;

const char* B2UploadKeyId = "00148165f5a47f20000000009";
const char* B2UploadSecret = "K001s0e//xC2qV0q/Y8nFBh3bwTbVkg";

const char* INJEST_BUCKET = "com-kml4vw-injest";

void setBuffer(uint8_t* data, size_t len)
{
	size_t outLen = 0;
	memset((void*) BUFFER, 0, sizeof(BUFFER));
	if (len >= sizeof(BUFFER))
	{
		outLen = sizeof(BUFFER) - 1;
	}
	else
	{
		outLen = len;
	}
	memcpy((void*) BUFFER, data, outLen);
}

static size_t curlFillBuffer(uint8_t* ptr, size_t size, size_t nmemb, void* stream)
{
	size_t val = 0;
	if ((size*nmemb) >= sizeof(BUFFER) - BUFFEROffset - 1)
	{
		val = sizeof(BUFFER) - BUFFEROffset - 1;
	}
	else
	{
		val = size*nmemb;
	}
	memcpy((void*) &BUFFER[BUFFEROffset], ptr, val);
	BUFFEROffset += val;
	//printf("%*s", size*nmemb, ptr);
	return size*nmemb;
}

int main(int argc, char** argv, char** envp)
{
	CURL* curl;
	CURLcode response = 0;
	char* cp;
	memset((void*) BUFFER, 0, sizeof(BUFFER));
	snprintf(BUFFER, sizeof(BUFFER), "%s:%s", B2UploadKeyId, B2UploadSecret);
	curl = curl_easy_init();
	if (curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_USERPWD, BUFFER);

		curl_easy_setopt(curl, CURLOPT_URL, 
			"https://api.backblazeb2.com/b2api/v2/b2_authorize_account");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFillBuffer);
		BUFFEROffset = 0;
		response = curl_easy_perform(curl);
		if (response != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(response));
		}
		else
		{
			cp = strstr(BUFFER, "authorizationToken");
			if (cp != NULL)
			{	
				cp = strstr(cp, ":");
				cp++;
				cp = strtok(cp, "\"");
				cp = strtok(NULL, "\"");
				sscanf(cp, "%s", BUFFER);
				printf("authorizationToken: \n%s\n", BUFFER);

			}
		}
		printf("cleaning up.\n");
		curl_easy_cleanup(curl);
	}
	/* code */
	return 0;
}