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

#include <osinfo/osinfo.h>

#define ID "foo:0"
#define ARCH "x86_64"
#define TYPE "efi"

static void
test_basic(void)
{
    OsinfoFirmware *firmware = osinfo_firmware_new(ID, ARCH, TYPE);

    g_assert_true(OSINFO_IS_FIRMWARE(firmware));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(firmware)), ==, ID);
    g_assert_cmpstr(osinfo_firmware_get_architecture(firmware), ==, ARCH);
    g_assert_cmpstr(osinfo_firmware_get_firmware_type(firmware), ==, TYPE);
}

static void
test_loaded(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoFilter *filter;
    OsinfoFirmwareList *list;
    gint list_len;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/db/firmware");

    list = osinfo_os_get_firmware_list(os, NULL);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 3);
    g_object_unref(list);

    filter = osinfo_filter_new();
    osinfo_filter_add_constraint(filter,
                                 OSINFO_FIRMWARE_PROP_ARCHITECTURE, "i686");
    list = osinfo_os_get_firmware_list(os, filter);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 1);
    g_object_unref(list);

    g_object_unref(loader);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/firmware/basic", test_basic);
    g_test_add_func("/firmware/loaded/basic", test_loaded);

    /* Upfront so we don't confuse valgrind */
    osinfo_firmware_get_type();
    osinfo_firmwarelist_get_type();
    osinfo_db_get_type();
    osinfo_filter_get_type();
    osinfo_list_get_type();
    osinfo_loader_get_type();
    osinfo_os_get_type();

    return g_test_run();
}
