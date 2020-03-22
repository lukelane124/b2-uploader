#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <curl/curl.h>
#define GLOBAL_BUFF_SIZE 4096
char BUFFER[GLOBAL_BUFF_SIZE];
size_t BUFFEROffset = 0;

//#define LOG_ERROR(...) do {char buf[128]; snprintf(buf, sizeof(buf), "[tll-error]:%s; %s", __FUNCTION__, STRING); fprintf(stderr, buf, __VA_ARGS__);}while(false)
#define _LOG_FUNC(FILE, HEADING, FORMATSTRING, FUNCP, ...) fprintf(FILE, "%s: %s; " FORMATSTRING, HEADING, FUNCP, ## __VA_ARGS__)
#define VA_ARGS(...) , ##__VA_ARGS__
#define LOG_ERROR(string, ...)  fprintf(stderr, "%s: %s:%d; " string, "[tll-error]", __FUNCTION__, __LINE__ VA_ARGS(__VA_ARGS__))
//_LOG_FUNC(stderr, "[tll-error]", string, __FUNCTION__, __VA_ARGS__)

//#define LOG_BASE(...) LOG_ERROR()

const char* B2UploadKeyId = "00148165f5a47f20000000009";
const char* B2UploadSecret = "K001s0e//xC2qV0q/Y8nFBh3bwTbVkg";
const char* B2apiNameVerURL = "/b2api/v2/";

const char* INJEST_BUCKET = "com-km4lvw-injest";

typedef struct 
{
	void* array;
	size_t index;
}dynamic_buffer_t;

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

static size_t curlFillBufferDynamic(uint8_t* ptr, size_t size, size_t nmemb, void* stream)
{
	size_t val = 0;
	void* allocStatus = NULL;
	uint8_t* u8p;
	dynamic_buffer_t* dBuffer = NULL;
	if (stream != NULL)
	{
		dBuffer = (dynamic_buffer_t*) stream;
		allocStatus = realloc(dBuffer->array, (dBuffer->index + (size * nmemb) + 1));
		if (allocStatus != NULL)
		{
			dBuffer->array = allocStatus;
			memcpy((void*) &dBuffer->array[dBuffer->index], ptr, nmemb * size);
			dBuffer->index += (nmemb * size);
			// u8p = (uint8_t*) &dBuffer->array[dBuffer->index];
			// u8p = 0;
			val = (nmemb * size);
		}
	}
	//LOG_ERROR("returning successfully.\n");
	return val;
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

char* getValueFromResp(char* key, char* response)
{
	char* ret = NULL;
	char* cp = NULL;
	char* cp1 = NULL;
	size_t valueLen = 0;
	//LOG_ERROR("key: %s\n", key);
	cp = strstr(response, key);
	if (cp != NULL)
	{
		// LOG_ERROR("KEY CONTAINED IN REPONSE.\n");
		cp = strstr(cp, ":");
		if (cp != NULL)
		{
			cp = strstr(cp, "\"");
			cp++;
			if (cp != NULL)
			{
				cp1 = strstr(cp, "\"");

				if (cp1 != NULL)
				{
					// LOG_ERROR("Value: \n\n\n\n%*s\n\n\n\n\n", cp1-cp, cp);
					valueLen = cp1 - cp + 1;
					ret = malloc(valueLen);
					if (ret != NULL)
					{
						memset((void*) ret, 0, valueLen);
						snprintf(ret, valueLen, "%*s", valueLen-1, cp);
					}
				}
			}
		}
	}
	return ret;
}


dynamic_buffer_t* makeCurlGetReq(char* url, CURLcode* resp_p, char* authString)
{
	dynamic_buffer_t* ret = NULL;
	CURL* curl;
	CURLcode response = 0;
	uint8_t* u8p;
	if (url != NULL)
	{
		ret = malloc(sizeof(dynamic_buffer_t));
		if (ret != NULL)
		{
			memset((void*) ret, 0, sizeof(dynamic_buffer_t));
			ret->array = malloc(1);
			if (ret->array != NULL)
			{
				curl = curl_easy_init();
				if (curl != NULL)
				{
					curl_easy_setopt(curl, CURLOPT_URL, url);
					if (authString != NULL)
					{
						curl_easy_setopt(curl, CURLOPT_USERPWD, authString);
					}
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFillBufferDynamic);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, ret);
					response = curl_easy_perform(curl);
					if (resp_p != NULL)
					{
						*resp_p = response;
					}
					LOG_ERROR("cleaning up.\n");
					curl_easy_cleanup(curl);
				}
				else
				{
					LOG_ERROR("curl was null");
					free(ret->array);
					free(ret);
					ret = NULL;
				}
			}
			else
			{
				free(ret);
				ret = NULL;
			}
		}
	}
	// LOG_ERROR("ret->array: %s\n", ret->array);
	return ret;
}


dynamic_buffer_t* makeCurlPostReq(CURL** curl_p, CURLcode* resp_p, char* data, struct curl_slist* chunkList)
{
	dynamic_buffer_t* ret = NULL;
	CURL* curl;
	CURLcode response = 0;
	// LOG_ERROR("ret->array: %s\n", ret->array);



	if ((curl_p != NULL) && (*curl_p != NULL) && (data != NULL) )
	{
		ret = malloc(sizeof(dynamic_buffer_t));
		if (ret != NULL)
		{
			memset((void*) ret, 0, sizeof(dynamic_buffer_t));
			ret->array = malloc(1);
			if (ret->array != NULL)
			{
				//curl = curl_easy_init();
				curl = *curl_p;
				if (curl != NULL)
				{
					//curl_easy_setopt(curl, CURLOPT_URL, url);
					if (chunkList != NULL)
					{
						curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunkList);
					}
					//Don't need to check null.
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFillBufferDynamic);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, ret);
					response = curl_easy_perform(curl);
					if (resp_p != NULL)
					{
						*resp_p = response;
					}
					//LOG_ERROR("cleaning up.\n");
					//Not mine to clean.
					//curl_easy_cleanup(curl);
				}
				else
				{
					LOG_ERROR("curl was null");
					free(ret->array);
					free(ret);
					ret = NULL;
				}
			}
			else
			{
				free(ret);
				ret = NULL;
			}
		}
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
	char* bucketId = NULL;
	char* apiUrl = NULL;
	char* postUrlResponse = NULL;
	char* postUrl = NULL;
	char* postDataString = NULL;
	char* postAuthTok = NULL;
	char* cp;
	memset((void*) BUFFER, 0, sizeof(BUFFER));
	snprintf(BUFFER, sizeof(BUFFER), "%s:%s", B2UploadKeyId, B2UploadSecret);
	curl = curl_easy_init();
	if (curl != NULL)
	{
		dynamic_buffer_t* dArray = makeCurlGetReq("https://api.backblazeb2.com/b2api/v2/b2_authorize_account",
												&response, BUFFER);
		if (response != CURLE_OK)
		{
			LOG_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(response));
		}
		else
		{
			if (dArray != NULL && dArray->array != NULL && dArray->index > 0)
			fullAuthString = (char*) dArray->array;
			free(dArray);
			dArray = NULL;
			if (fullAuthString != NULL)
			{
				LOG_ERROR("fullAuthString: %s", fullAuthString);
				authTok = getValueFromResp("authorizationToken", fullAuthString);
								
				if (authTok != NULL)
				{
					//fullAuthString valid, and authTok valid.
					printf("authorizationToken: \n%s\n", authTok);
					LOG_ERROR("fullAuthString: %s\n", fullAuthString);
					bucketId = getValueFromResp("bucketId", fullAuthString);
					if (bucketId != NULL)
					{
						LOG_ERROR("bucketId: %s\n", bucketId);
						apiUrl = getValueFromResp("apiUrl", fullAuthString);
					
						if (apiUrl != NULL)
						{
							LOG_ERROR("apiUrl: \n%s\n", apiUrl);
							curl_easy_setopt(curl, CURLOPT_USERPWD, NULL);
							//curl_easy_reset(curl);

							snprintf(BUFFER, sizeof(BUFFER), "%s%s%s", apiUrl, B2apiNameVerURL, "b2_get_upload_url");
							LOG_ERROR("get upload url: %s\n", BUFFER);
							curl_easy_setopt(curl, CURLOPT_URL, BUFFER);
							memset((void*) BUFFER, 0, sizeof(BUFFER));

							chunk = curl_slist_append(chunk, "Accept:");

							snprintf(BUFFER, sizeof(BUFFER), "%s: %s", "Authorization", authTok);
							LOG_ERROR("auth header: %s\n", BUFFER);
							chunk = curl_slist_append(chunk, BUFFER);
							//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

							snprintf(BUFFER, sizeof(BUFFER), "{\"bucketId\": \"%s\"}", bucketId);
							postDataString = getDynamicStringCopy(BUFFER);
							if (postDataString != NULL)
							{
								//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postDataString);
								memset((void*) BUFFER, 0, sizeof(BUFFER));
								LOG_ERROR("postDataString: %s\n", postDataString);

								//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); 
								//dynamic_buffer_t* makeCurlPostReq(CURL** curl_p, CURLcode* resp_p, char* data, struct curl_slist* chunkList)
								dArray = makeCurlPostReq(&curl, &response, postDataString, chunk);
								curl_slist_free_all(chunk);
								chunk = NULL;
								if (dArray != NULL)
								{

									//response = curl_easy_perform(curl);
									if (response != CURLE_OK)
									{
										LOG_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(response));
									}
									else
									{
										//Got back valid response
										long responseCode;
										LOG_ERROR("curl call was ok\n");
										curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

										LOG_ERROR("response code; Code: %d\n", responseCode);
										if (responseCode < 300 && responseCode > 199)
										{
											//Valid url recieved.
											postUrlResponse = dArray->array;
											free(dArray);
											if (postUrlResponse != NULL)
											{
												memset((void*) BUFFER, 0, sizeof(BUFFER));
												LOG_ERROR("postUrlResponse: %s\n", postUrlResponse);
												postUrl = getValueFromResp("uploadUrl", postUrlResponse);
												if (postUrl != NULL)
												{
													LOG_ERROR("postUrl: %s\n", postUrl);
													postAuthTok = getValueFromResp("authorizationToken", postUrlResponse);
												}


												free(postUrlResponse);
											}
										}
										else
										{
											LOG_ERROR("buffer after failed get upload url: %s\n", BUFFER);
										}
									}
								}
								free(postDataString);
							}

							free(apiUrl);
							apiUrl = NULL;
						}
						//Removed as reused in final upload call.
						//free(bucketId);
					}
				
					free(authTok);
					authTok = NULL;
					
				}
				free(fullAuthString);
				fullAuthString = NULL;
			}
		}
		if (postAuthTok != NULL)
		{
			//Means everything has completed and I have what I need to perform upload.

			curl_easy_reset(curl);
			curl_easy_setopt(curl, CURLOPT_URL, postUrl);
			chunk = curl_slist_append(chunk, "Accept:");
			memset((void*) BUFFER, 0, sizeof(BUFFER));
			//snprintf(BUFFER, sizeof(BUFFER), "%s");

			free(postAuthTok);
			postAuthTok = NULL;
			free(bucketId);
			bucketId = NULL;
		}
		if (postUrl != NULL)
		{
			free(postUrl);
			postUrl = NULL;
		}

		LOG_ERROR("cleaning up.\n");
		curl_easy_cleanup(curl);
	}
	/* code */
	return 0;
}