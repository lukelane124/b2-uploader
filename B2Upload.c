#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <curl/curl.h>
#define GLOBAL_BUFF_SIZE 4096
char BUFFER[GLOBAL_BUFF_SIZE];
size_t BUFFEROffset = 0;

#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
//#define LOG_BASE(...) LOG_ERROR()

const char* B2UploadKeyId = "00148165f5a47f20000000009";
const char* B2UploadSecret = "K001s0e//xC2qV0q/Y8nFBh3bwTbVkg";
const char* B2apiNameVerURL = "/b2api/v5/"

const char* INJEST_BUCKET = "com-km4lvw-injest";

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

char* getDynamicStringCopy(char* ntString)
{
	char* ret = NULL;
	size_t len = strlen(ntString) + 1;
	ret = malloc(len);
	if (ret != NULL)
	{
		memset((void*) ret, 0, len);
		snprintf(ret, len, "%s",ntString);
	}
	return ret;
}

int main(int argc, char** argv, char** envp)
{
	CURL* curl;
	CURLcode response = 0;
	struct curl_slist *chunk = NULL;
	char* fullAuthString = NULL;
	char* authTok = NULL;
	char* apiUrl = NULL;
	char* postUrlResponse = NULL;
	char* postUrl = NULL;
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
			LOG_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(response));
		}
		else
		{
			fullAuthString = getDynamicStringCopy(BUFFER);
			LOG_ERROR("fullAuthString: %s", fullAuthString);
			if (fullAuthString != NULL)
			{
				cp = strstr(BUFFER, "authorizationToken");
				if (cp != NULL)
				{	
					cp = strstr(cp, ":");
					cp++;
					cp = strtok(cp, "\"");
					cp = strtok(NULL, "\"");
					if (cp != NULL)
					{
						//sscanf(cp, "%s", BUFFER);
						snprintf(BUFFER, sizeof(BUFFER), "%s", cp);
						printf("authorizationToken: \n%s\n", BUFFER);
						authTok = getDynamicStringCopy(BUFFER);
						if (authTok != NULL)
						{
							//fullAuthString valid, and authTok valid.
							memset((void*) BUFFER, 0, sizeof(BUFFER));
							memcpy((void*) BUFFER, fullAuthString, strlen(fullAuthString));

							//Get api url from authString.
							cp = strstr(BUFFER, "apiUrl");
							if (cp != NULL)
							{	
								cp = strstr(cp, ":");
								cp++;
								cp = strtok(cp, "\"");
								cp = strtok(NULL, "\"");
								if (cp != NULL)
								{
									//sscanf(cp, "%s", BUFFER);
									snprintf(BUFFER, sizeof(BUFFER), "%s", cp);
									printf("apiUrl: \n%s\n", BUFFER);
									apiUrl = getDynamicStringCopy(BUFFER);
									if (apiUrl != NULL)
									{
										curl_easy_setopt(curl, CURLOPT_USERPWD, NULL);

										snprintf(BUFFER, sizeof(BUFFER), "%s%s/%s", apiUrl, B2apiNameVerURL, "b2_get_upload_url");
										curl_easy_setopt(curl, CURLOPT_URL, BUFFER);
										memset((void*) BUFFER, 0, sizeof(BUFFER));

										chunk = curl_slist_append(chunk, "Accept:");

										snprintf(BUFFER, sizeof(BUFFER) "%s: %s", "Authorization", authTok);
										chunk = curl_slist_append(chunk, BUFFER);
										curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

										memset((void*) BUFFER, 0, sizeof(BUFFER));
										response = curl_easy_perform();

										curl_slist_free_all(chunk);
										chunk = NULL;
										if (response != CURLE_OK)
										{
											LOG_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(response));
										}
										else
										{
											//Got back valid response.
											long responseCode;
											curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
											if (responseCode < 300 && responseCode > 199)
											{
												//Valid url recieved.
												postUrlResponse = getDynamicStringCopy(BUFFER);
												if (postUrlResponse != NULL)
												{
													memset((void*) BUFFER, 0, sizeof(BUFFER));

													
													
													free(postUrlResponse);
												}
											}
										}

										free(apiUrl);
									}
								}
							}
							free(authTok);
							authTok = NULL;
						}
					}
				}
				free(fullAuthString);
				fullAuthString = NULL;
			}
		}

		LOG_ERROR("cleaning up.\n");
		curl_easy_cleanup(curl);
	}
	/* code */
	return 0;
}