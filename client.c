#include <stdlib.h>
#include <string.h>
#ifndef __EMSCRIPTEN__
#include <curl/curl.h>
#endif
#include "request.h"
#include "icclient.h"

char *image_dir;
#ifdef __EMSCRIPTEN__
emscripten_fetch_attr_t attr;
#else
char *sampleurl;
CURL *curl;
#endif

extern void handle_results(icclient_response *);

void icclient_init(const char *url, const char *dir, const char *certificate)
{
	image_dir = malloc(strlen(dir) + 1);
	strcpy(image_dir, dir);
#ifdef __EMSCRIPTEN__
	emscripten_fetch_attr_init(&attr);
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
#else
	size_t length = strlen(url);
	size_t append = url[length - 1] != '/';
	sampleurl = malloc(length + append + 1);
	strcpy(sampleurl, url);
	if (append)
		strcat(sampleurl, "/");
	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	if (certificate)
		curl_easy_setopt(curl, CURLOPT_CAINFO, certificate);
#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
#endif
}

void icclient_catalog(const char *prod_group, void (*handler)(icclient_response *), void (*callback)(struct icclient_catalog *))
{
	char nonspaced[strlen(prod_group) + 1];
	strcpy(nonspaced, prod_group);
	char *space = NULL;
	while ((space = strchr(nonspaced, ' ')))
		*space = '-';
	request(handler ? handler : handle_results, (void (*)(void *))callback, NULL, "%s", nonspaced);
}

void icclient_product(const char *sku, void (*handler)(icclient_response *), void (*callback)(struct icclient_product *))
{
	request(handler, (void (*)(void *))callback, NULL, "%s", sku);
}

void icclient_page(const char *path, void (*handler)(icclient_response *))
{
	request(handler, NULL, NULL, "%s", path);
}

void icclient_free_product(struct icclient_product *product)
{
	if (product->crosssell)
		for (size_t i = 0; i < product->crosssell->length; i++)
			free(product->crosssell->skus[i]);
	if (product->author)
		free(product->author);
	if (product->prod_group)
		free(product->prod_group);
	if (product->image)
		free(product->image);
	if (product->thumb)
		free(product->thumb);
	if (product->comment)
		free(product->comment);
	if (product->description)
		free(product->description);
	free(product->sku);
	free(product);
}

void icclient_free_catalog(struct icclient_catalog *catalog)
{
	for (size_t i = 0; i < catalog->length; i++)
		icclient_free_product(catalog->products[i]);
	free(catalog);
}

void icclient_free_response(icclient_response *response)
{
	if (response->userData)
		free(response->userData);
#ifdef __EMSCRIPTEN__
	emscripten_fetch_close(response);
#else
	free(response->data);
	free(response);
#endif
}

void icclient_cleanup()
{
	free(image_dir);
#ifndef __EMSCRIPTEN__
	free(sampleurl);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
#endif
}
