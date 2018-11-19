/*
 * Copyright (C) 2009-2012, 2014 Red Hat, Inc.
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
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <stdlib.h>
#include <osinfo/osinfo.h>
#include <curl/curl.h>

static void test_tree(OsinfoTreeList *treelist, GError **error, CURL *curl)
{
    GList *treeel = NULL, *tmp;

    tmp = treeel = osinfo_list_get_elements(OSINFO_LIST(treelist));
    while (tmp) {
        OsinfoTree *tree = tmp->data;
        const gchar *url = osinfo_tree_get_url(tree);
        CURLcode res;
        long response_code;

        if (url == NULL || g_str_equal(url, "")) {
            tmp = tmp->next;
            continue;
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
    }

    g_list_free(treeel);
}

static void
test_uris(void)
{
    CURL *curl;
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    GError *error = NULL;
    OsinfoOsList *oslist = NULL;
    GList *osel = NULL, *tmp;
    const gchar *debugstr;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    if ((debugstr = g_getenv("LIBOSINFO_TEST_DEBUG"))) {
        int debug_level = atoi(debugstr);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, debug_level > 0 ? 1L : 0L);
    }

    g_assert_true(OSINFO_IS_LOADER(loader));
    g_assert_true(OSINFO_IS_DB(db));

    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);

    oslist = osinfo_db_get_os_list(db);
    tmp = osel = osinfo_list_get_elements(OSINFO_LIST(oslist));
    while (tmp) {
        OsinfoOs *os = tmp->data;
        OsinfoTreeList *treelist = osinfo_os_get_tree_list(os);

        test_tree(treelist, &error, curl);

        g_assert_no_error(error);

        g_object_unref(treelist);
        tmp = tmp->next;
    }

    curl_easy_cleanup(curl);

    g_list_free(osel);
    if (oslist)
        g_object_unref(oslist);

    g_object_unref(loader);
}



int
main(int argc, char *argv[])
{
    int ret;

    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();

    g_test_add_func("/treeuris/uris", test_uris);

    if (!g_getenv("LIBOSINFO_NETWORK_TESTS"))
        return 77; /* Skip */

    /* Upfront so we don't confuse valgrind */
    curl_global_init(CURL_GLOBAL_ALL);
    osinfo_entity_get_type();
    osinfo_db_get_type();
    osinfo_device_get_type();
    osinfo_platform_get_type();
    osinfo_os_get_type();
    osinfo_list_get_type();
    osinfo_devicelist_get_type();
    osinfo_platformlist_get_type();
    osinfo_oslist_get_type();
    osinfo_filter_get_type();

    ret = g_test_run();

    curl_global_cleanup();

    return ret;
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
