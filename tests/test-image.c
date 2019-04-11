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
 *   Fabiano Fidênciog <fidencio@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>

#define ID "foo"
#define ARCH "x86_64"
#define FORMAT "qcow2"
#define URL "https://foo.bar/foobar.img"

static void
test_basic(void)
{
    OsinfoImage *image = osinfo_image_new(ID, ARCH, FORMAT);
    osinfo_entity_set_param(OSINFO_ENTITY(image),
                            OSINFO_IMAGE_PROP_URL,
                            URL);

    g_assert_true(OSINFO_IS_IMAGE(image));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(image)), ==, ID);
    g_assert_cmpstr(osinfo_image_get_architecture(image), ==, ARCH);
    g_assert_cmpstr(osinfo_image_get_format(image), ==, FORMAT);
    g_assert_cmpstr(osinfo_image_get_url(image), ==, URL);

    g_assert_false(osinfo_image_get_cloud_init(image));
    osinfo_entity_set_param_boolean(OSINFO_ENTITY(image),
                                    OSINFO_IMAGE_PROP_CLOUD_INIT,
                                    TRUE);
    g_assert_true(osinfo_image_get_cloud_init(image));

    g_object_unref(image);
}


static void
test_loaded(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoImageList *list;
    gint list_len, i;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/db/image");

    list = osinfo_os_get_image_list(os);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 2);

    for (i = 0; i < list_len; i++) {
        OsinfoImage *image = OSINFO_IMAGE(osinfo_list_get_nth(OSINFO_LIST(list), i));
        OsinfoOsVariantList *variant_list;
        OsinfoOsVariant *variant;

        variant_list = osinfo_image_get_os_variants(image);
        g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(variant_list)), ==, 1);

        variant = OSINFO_OS_VARIANT(osinfo_list_get_nth(OSINFO_LIST(variant_list), 0));

        if (g_str_equal(osinfo_image_get_architecture(image), "i686")) {
            g_assert_cmpstr(osinfo_image_get_url(image), ==, "http://libosinfo.org/db/image.vmdk");
            g_assert_cmpstr(osinfo_image_get_format(image), ==, "vmdk");
            g_assert_false(osinfo_image_get_cloud_init(image));
            g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(variant)), ==, "bar");
            g_assert_cmpstr(osinfo_os_variant_get_name(variant), ==, "Image Bar");
        } else {
            g_assert_cmpstr(osinfo_image_get_url(image), ==, "http://libosinfo.org/db/image.qcow2");
            g_assert_cmpstr(osinfo_image_get_format(image), ==, "qcow2");
            g_assert_true(osinfo_image_get_cloud_init(image));
            g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(variant)), ==, "foo");
            g_assert_cmpstr(osinfo_os_variant_get_name(variant), ==, "Image Foo");
        }
    }

    g_object_unref(list);
    g_object_unref(loader);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/image/basic", test_basic);
    g_test_add_func("/image/loaded", test_loaded);

    /* Upfront so we don't confuse valgrind */
    osinfo_db_get_type();
    osinfo_image_get_type();
    osinfo_imagelist_get_type();
    osinfo_loader_get_type();
    osinfo_os_get_type();

    return g_test_run();
}
