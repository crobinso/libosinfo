/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#include <config.h>

#include <stdlib.h>
#include <osinfo/osinfo.h>
#include <curl/curl.h>

#include "test-uris-common.h"

static size_t write_callback(char *ptr,
                             size_t size,
                             size_t nmemb,
                             void *userdata)
{
    abort();
}

void test_uri(OsinfoList *list, GetURLFunc get_url_func, GError **error)
{
    GList *el = NULL, *tmp;

    tmp = el = osinfo_list_get_elements(list);
    while (tmp) {
        const gchar *url = get_url_func(tmp->data);
        const gchar *debugstr;
        CURL *curl;
        CURLcode res;
        long response_code;

        if (url == NULL || g_str_equal(url, "")) {
            tmp = tmp->next;
            continue;
        }

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        if ((debugstr = g_getenv("LIBOSINFO_TEST_DEBUG"))) {
            int debug_level = atoi(debugstr);

            curl_easy_setopt(curl, CURLOPT_VERBOSE, debug_level > 0 ? 1L : 0L);
        }

        g_test_message("%s", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        g_test_message("res=%d, %s; code=%ld", res, curl_easy_strerror(res), response_code);

        if (res == CURLE_OPERATION_TIMEDOUT) {
            g_printerr("Ignoring network timeout failure for %s\n", url);
        } else {
            if (res != CURLE_OK) {
                g_printerr("Failed URI %s res=%d (%s) code=%ld\n",
                           url, res, curl_easy_strerror(res), response_code);
            }
            g_assert_cmpint(res, ==, CURLE_OK);
        }

        tmp = tmp->next;
        curl_easy_cleanup(curl);
    }

    g_list_free(el);
}


void test_uris(OsinfoOsList *oslist, const gchar *prefix, TestURLFunc test_func)
{
    GList *osel = NULL, *tmp;

    tmp = osel = osinfo_list_get_elements(OSINFO_LIST(oslist));

    while (tmp) {
        OsinfoOs *os = tmp->data;
        gchar *name = g_strdup_printf("%s/%s",
                                      prefix,
                                      osinfo_product_get_short_id(OSINFO_PRODUCT(os)));

        g_test_add_data_func(name, os, test_func);
        g_free(name);

        tmp = tmp->next;
    }

    g_list_free(osel);
}

