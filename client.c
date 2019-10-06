#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "login.h"
#include "icclient/product.h"
#include "icclient/catalog.h"
#include "icclient/ord.h"
#include "icclient/client.h"

typedef struct icclient_product icclient_product;
typedef struct icclient_catalog icclient_catalog;
typedef struct icclient_ord_item icclient_ord_item;
typedef struct icclient_ord_order icclient_ord_order;

CURL *curl = NULL;
char *server_url = NULL;

bool icclient_init(const char *url, const char *certificate)
{
	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
		if (certificate)
			curl_easy_setopt(curl, CURLOPT_CAINFO, certificate);
		size_t length = strlen(url);
		bool append = !(bool)(url[length - 1] == '/');
		server_url = malloc(length + (size_t)append + 1);
		strcpy(server_url, url);
		if (append)
			strcat(server_url, "/");
	}

	return (bool)curl;
}

void icclient_allproducts(size_t (*handler)(void *, size_t, size_t, void *)
		, icclient_catalog **catalogptr)
{
	(*catalogptr) = malloc(sizeof(icclient_catalog));
	request(handler, (void *)catalogptr, NULL, "%s", "All-Products");
}

static int prodcmp(const void *product1, const void *product2)
{
	return strcmp(((icclient_product *)product1)->sku
			, ((icclient_product *)product2)->sku);
}

static int itemcmp(const void *item1, const void *item2)
{
	return strcmp(((icclient_ord_item *)item1)->product->sku
			, ((icclient_ord_item *)item2)->product->sku);
}

void icclient_order(icclient_ord_order **orderptr, const char *sku
		, icclient_catalog *catalog)
{
	if (!*orderptr) {
		(*orderptr) = malloc(sizeof(icclient_ord_order));
		(*orderptr)->subtotal = .0;
		(*orderptr)->shipping = .0;
		(*orderptr)->subtotal = .0;
		(*orderptr)->nitems = 0;
	}
	icclient_ord_order *order = *orderptr;

	qsort(catalog->products, catalog->length, sizeof(icclient_product)
			, prodcmp);
	icclient_product *product = bsearch(sku, catalog->products
		, catalog->length, sizeof(icclient_product), prodcmp);

	qsort(order->items, order->nitems, sizeof(icclient_ord_item), itemcmp);
	icclient_ord_item *item = bsearch(sku, order->items, order->nitems
			, sizeof(icclient_ord_item), itemcmp);

	if (item)
		item->quantity++;
	else {
		item = malloc(sizeof(icclient_ord_item));
		item->product = product;
		item->quantity = 1;
		*orderptr = realloc(*orderptr, sizeof(*orderptr)
				+ sizeof(icclient_ord_item));
		(*orderptr)->items[(*orderptr)->nitems++] = item;
		order = *orderptr;
	}

	order->subtotal += item->product->price;
	order->total_cost += item->product->price;

	request(NULL, NULL, NULL, "%s%s", "order?mv_arg=", sku);
}

void icclient_newaccount(const char *username, const char *password
		, const char *verify, const char *successpage, const char *nextpage
		, const char *failpage)
{
	login(username, password, verify, "NewAccount", successpage, nextpage
			, failpage);
}

void icclient_login(const char *username, const char *password
		, const char *successpage, const char *nextpage
		, const char *failpage)
{
	login(username, password, NULL, "Login", successpage, nextpage, failpage);
}

void icclient_logout()
{
	request(NULL, NULL, NULL, "%s", "logout");
}

void icclient_page(const char *path, size_t (*handler)(void *, size_t, size_t
			, void *), void **dataptr)
{
	request(handler, (void *)dataptr, NULL, "%s", path);
}

void icclient_freeproduct(icclient_product *product)
{
	if (product->image)
		free(product->image);
	if (product->comment)
		free(product->comment);
	if (product->description)
		free(product->description);
	free(product->sku);
	free(product);
	product = NULL;
}

void icclient_freecatalog(icclient_catalog *catalog)
{
	for (size_t i = 0; i < catalog->length; i++)
		icclient_freeproduct(catalog->products[i]);
	free(catalog);
	catalog = NULL;
}

void icclient_cleanup()
{
	if (curl) {
		free(server_url);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}
