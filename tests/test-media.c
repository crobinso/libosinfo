/*
 * Copyright (C) 2018 Red Hat, Inc.
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
 *   Fabiano Fidêncio <fidencio@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>



static void
test_basic(void)
{
    OsinfoMedia *media = osinfo_media_new("foo", "x86_64");

    g_assert_true(OSINFO_IS_MEDIA(media));
    g_assert_cmpstr((const char *)osinfo_entity_get_id(OSINFO_ENTITY(media)), ==, "foo");
    g_assert_cmpstr((const char *)osinfo_media_get_architecture(media), ==, "x86_64");

    g_object_unref(media);
}


static void
test_loaded_attributes(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoMediaList *list;
    gint list_len, i;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/media");

    list = osinfo_os_get_media_list(os);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 2);

    for (i = 0; i < list_len; i++) {
        OsinfoMedia *media = OSINFO_MEDIA(osinfo_list_get_nth(OSINFO_LIST(list), i));

        if (g_str_equal(osinfo_media_get_architecture(media), "x86_64")) {
            /* non-default values */
            g_assert_true(osinfo_media_get_live(media));
            g_assert_cmpint(osinfo_media_get_installer_reboots(media), ==, 3);
            g_assert_false(osinfo_media_get_eject_after_install(media));
            g_assert_false(osinfo_media_supports_installer_script(media));
        } else if (g_str_equal(osinfo_media_get_architecture(media), "x86")) {
            /* default-values */
            g_assert_false(osinfo_media_get_live(media));
            g_assert_cmpint(osinfo_media_get_installer_reboots(media), ==, 1);
            g_assert_true(osinfo_media_get_eject_after_install(media));
            g_assert_true(osinfo_media_supports_installer_script(media));
        }
    }

    g_object_unref(list);
    g_object_unref(loader);
}


static void
test_loaded_no_installer(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoMediaList *list;
    gint list_len, i;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/media-no-installer");

    list = osinfo_os_get_media_list(os);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 2);

    for (i = 0; i < list_len; i++) {
        OsinfoMedia *media = OSINFO_MEDIA(osinfo_list_get_nth(OSINFO_LIST(list), i));

        g_assert_false(osinfo_media_supports_installer_script(media));
    }

    g_object_unref(list);
    g_object_unref(loader);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/media/basic", test_basic);
    g_test_add_func("/media/loaded/attributes", test_loaded_attributes);
    g_test_add_func("/media/loaded/no-installer", test_loaded_no_installer);

    /* Upfront so we don't confuse valgrind */
    osinfo_media_get_type();

    return g_test_run();
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
