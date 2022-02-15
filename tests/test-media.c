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
 */

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

static void
test_loaded_installer_script(void)
{

    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoMedia *media;
    OsinfoMediaList *list;
    OsinfoInstallScriptList *scripts;
    gint list_len;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/media-installer-script");

    list = osinfo_os_get_media_list(os);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 1);

    media = OSINFO_MEDIA(osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(osinfo_media_supports_installer_script(media));

    scripts = osinfo_media_get_install_script_list(media);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(scripts)), ==, 1);

    g_object_unref(scripts);
    g_object_unref(list);
    g_object_unref(loader);
}

static OsinfoMedia *
test_create_media(const char *id,
                  const char *arch,
                  const char *volume,
                  const char *system,
                  const char *publisher,
                  const char *application,
                  guint64 size)
{
    OsinfoMedia *media = osinfo_media_new(id, arch);

    if (volume)
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_VOLUME_ID,
                                volume);
    if (system)
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_SYSTEM_ID,
                                system);
    if (publisher)
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_PUBLISHER_ID,
                                publisher);
    if (application)
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_APPLICATION_ID,
                                application);
    if (size != 0)
        osinfo_entity_set_param_int64(OSINFO_ENTITY(media),
                                      OSINFO_MEDIA_PROP_VOLUME_SIZE,
                                      size);

    return media;
}

static void
test_matching(void)
{
    g_autoptr(OsinfoMedia) unknown = test_create_media("https://libosinfo.org/test/",
                                                       "x86_64",
                                                       "Fedora 35",
                                                       "LINUX",
                                                       "Fedora",
                                                       "Fedora OS",
                                                       1234567);
    /* Match with several optional fields */
    g_autoptr(OsinfoMedia) reference1 = test_create_media("https://fedoraproject.org/fedora/35/media1",
                                                          "x86_64",
                                                          "Fedora 35",
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0);
    /* Mis-match on volume */
    g_autoptr(OsinfoMedia) reference2 = test_create_media("https://fedoraproject.org/fedora/34/media2",
                                                          "x86_64",
                                                          "Fedora 34",
                                                          "LINUX",
                                                          NULL,
                                                          NULL,
                                                          0);
    /* Match with all fields with some regexes */
    g_autoptr(OsinfoMedia) reference3 = test_create_media("https://fedoraproject.org/fedora/unknown/media3",
                                                          "x86_64",
                                                          "Fedora [0-9]+",
                                                          "LINUX",
                                                          "Fedora",
                                                          "Fedora OS",
                                                          0);
    /* Match including vol size */
    g_autoptr(OsinfoMedia) reference4 = test_create_media("https://fedoraproject.org/fedora/35/media4",
                                                          "x86_64",
                                                          "Fedora 35",
                                                          "LINUX",
                                                          NULL,
                                                          NULL,
                                                          1234567);
    /* Mis-match on vol size */
    g_autoptr(OsinfoMedia) reference5 = test_create_media("https://fedoraproject.org/fedora/35/media5",
                                                          "x86_64",
                                                          "Fedora 35",
                                                          "LINUX",
                                                          NULL,
                                                          NULL,
                                                          1234568);
    /* Mis-match on arch */
    g_autoptr(OsinfoMedia) reference6 = test_create_media("https://fedoraproject.org/fedora/35/media1",
                                                          "i686",
                                                          "Fedora 35",
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0);
    g_assert(osinfo_media_matches(unknown, reference1));
    g_assert(!osinfo_media_matches(unknown, reference2));
    g_assert(osinfo_media_matches(unknown, reference3));
    g_assert(osinfo_media_matches(unknown, reference4));
    g_assert(!osinfo_media_matches(unknown, reference5));
    g_assert(!osinfo_media_matches(unknown, reference6));
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/media/basic", test_basic);
    g_test_add_func("/media/loaded/attributes", test_loaded_attributes);
    g_test_add_func("/media/loaded/no-installer", test_loaded_no_installer);
    g_test_add_func("/media/loaded/installer-script", test_loaded_installer_script);
    g_test_add_func("/media/matching", test_matching);

    /* Upfront so we don't confuse valgrind */
    osinfo_media_get_type();

    return g_test_run();
}
