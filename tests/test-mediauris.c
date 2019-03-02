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

#include "test-uris-common.h"

static void
test_mediauris(gconstpointer data)
{
    OsinfoOs *os = OSINFO_OS(data);
    GError *error = NULL;

    OsinfoMediaList *medialist = osinfo_os_get_media_list(os);

    test_uri(OSINFO_LIST(medialist), (GetURLFunc) osinfo_media_get_url, &error);

    g_assert_no_error(error);

    g_object_unref(medialist);
}

int
main(int argc, char *argv[])
{
    int ret;
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    OsinfoOsList *oslist = NULL;
    GError *error = NULL;

    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();

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


    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);

    oslist = osinfo_db_get_os_list(db);
    test_uris(oslist, "/mediauris/uris", test_mediauris);

    ret = g_test_run();

    if (oslist)
        g_object_unref(oslist);

    g_object_unref(loader);

    curl_global_cleanup();

    return ret;
}
