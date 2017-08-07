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
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <config.h>
#include <stdlib.h>
#include <osinfo/osinfo.h>
#include <check.h>
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

        g_print("%s\n", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        }

        fail_unless(res == CURLE_OK, "Failed HEAD (res=%d, %s; code=%ld) on %s", res, curl_easy_strerror(res), response_code, url);

        tmp = tmp->next;
    }

    g_list_free(treeel);
}

START_TEST(test_uris)
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

    fail_unless(OSINFO_IS_LOADER(loader), "Loader is not a LOADER");
    fail_unless(OSINFO_IS_DB(db), "Db is not a DB");

    osinfo_loader_process_default_path(loader, &error);
    fail_unless(error == NULL, error ? error->message : "none");

    oslist = osinfo_db_get_os_list(db);
    tmp = osel = osinfo_list_get_elements(OSINFO_LIST(oslist));
    while (tmp) {
        OsinfoOs *os = tmp->data;
        OsinfoTreeList *treelist = osinfo_os_get_tree_list(os);

        test_tree(treelist, &error, curl);

        fail_unless(error == NULL, error ? error->message : "none");

        g_object_unref(treelist);
        tmp = tmp->next;
    }

    curl_easy_cleanup(curl);

    g_list_free(osel);
    if (oslist)
        g_object_unref(oslist);

    g_object_unref(loader);
}
END_TEST



static Suite *
list_suite(void)
{
    Suite *s = suite_create("List");
    TCase *tc = tcase_create("Core");
    tcase_set_timeout(tc, 120);

    tcase_add_test(tc, test_uris);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = list_suite();
    SRunner *sr = srunner_create(s);

    /* Make sure we catch unexpected g_warning() */
    g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);

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

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    curl_global_cleanup();

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
