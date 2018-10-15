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

#include <osinfo/osinfo.h>



static void
test_basic(void)
{
    OsinfoOs *os = osinfo_os_new("pony");

    g_assert_true(OSINFO_IS_OS(os));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(os)), ==, "pony");

    g_object_unref(os);
}

static void
test_devices(void)
{
    OsinfoOs *os = osinfo_os_new("awesome");
    OsinfoDevice *dev1 = osinfo_device_new("e1000");
    OsinfoDevice *dev2 = osinfo_device_new("rtl8139");

    OsinfoDeviceLink *link1 = osinfo_os_add_device(os, dev1);
    osinfo_entity_add_param(OSINFO_ENTITY(link1), "device", "pci-e1000");
    OsinfoDeviceLink *link2 = osinfo_os_add_device(os, dev2);
    osinfo_entity_add_param(OSINFO_ENTITY(link2), "device", "pci-8139");

    OsinfoDeviceList *devices = osinfo_os_get_devices(os, NULL);

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(devices)), ==, 2);
    g_assert_true(osinfo_list_get_nth(OSINFO_LIST(devices), 0) == OSINFO_ENTITY(dev1));
    g_assert_true(osinfo_list_get_nth(OSINFO_LIST(devices), 1) == OSINFO_ENTITY(dev2));

    g_object_unref(devices);
    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(os);
}


static void
test_loader(void)
{
    OsinfoLoader *loader;
    OsinfoDb *db;
    OsinfoOs *os;
    GError *error = NULL;
    const char *str;

    loader = osinfo_loader_new();
    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/test1");
    g_assert_nonnull(os);
    str = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "test1");
    str = osinfo_product_get_name(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "Test 1");
    str = osinfo_product_get_version(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "unknown");
    str = osinfo_product_get_vendor(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "libosinfo.org");
    str = osinfo_os_get_family(os);
    g_assert_cmpstr(str, ==, "test");
    g_assert_cmpint(osinfo_os_get_release_status(os), ==, OSINFO_RELEASE_STATUS_PRERELEASE);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/test2");
    g_assert_nonnull(os);
    str = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "test2");
    str = osinfo_product_get_name(OSINFO_PRODUCT(os));
    g_assert_null(str);
    str = osinfo_product_get_version(OSINFO_PRODUCT(os));
    g_assert_null(str);
    str = osinfo_product_get_vendor(OSINFO_PRODUCT(os));
    g_assert_null(str);
    str = osinfo_os_get_family(os);
    g_assert_null(str);
    g_assert_cmpint(osinfo_os_get_release_status(os), ==, OSINFO_RELEASE_STATUS_RELEASED);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/test3");
    g_assert_nonnull(os);
    str = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "test3");
    g_assert_cmpint(osinfo_os_get_release_status(os), ==, OSINFO_RELEASE_STATUS_RELEASED);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/test4");
    g_assert_nonnull(os);
    str = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "test4");
    g_assert_cmpint(osinfo_os_get_release_status(os), ==, OSINFO_RELEASE_STATUS_SNAPSHOT);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/test5");
    g_assert_nonnull(os);
    str = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "test5");
    /* 'test5' OS intentionnally contains an invalid release status */
    g_test_expect_message(NULL, G_LOG_LEVEL_CRITICAL,
                          "*(osinfo_entity_get_param_value_enum): should not be reached*");
    g_assert_cmpint(osinfo_os_get_release_status(os), ==, OSINFO_RELEASE_STATUS_RELEASED);

    g_object_unref(loader);
}


static void
test_devices_filter(void)
{
    OsinfoOs *os = osinfo_os_new("awesome");
    OsinfoDevice *dev1 = osinfo_device_new("e1000");
    OsinfoDevice *dev2 = osinfo_device_new("sb16");
    OsinfoFilter *filter = osinfo_filter_new();

    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "audio");

    OsinfoDeviceLink *link1 = osinfo_os_add_device(os, dev1);
    osinfo_entity_add_param(OSINFO_ENTITY(link1), "driver", "pci-e1000");
    OsinfoDeviceLink *link2 = osinfo_os_add_device(os, dev2);
    osinfo_entity_add_param(OSINFO_ENTITY(link2), "driver", "isa-sb16");

    osinfo_filter_add_constraint(filter, "class", "network");

    OsinfoDeviceList *devices = osinfo_os_get_devices(os, filter);

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(devices)), ==, 1);
    OsinfoEntity *ent = osinfo_list_get_nth(OSINFO_LIST(devices), 0);
    g_assert_true(OSINFO_IS_DEVICE(ent));
    g_assert_true(OSINFO_DEVICE(ent) == dev1);

    g_object_unref(devices);
    g_object_unref(filter);
    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(os);
}


static void
test_device_driver(void)
{
    OsinfoOs *os = osinfo_os_new("awesome");
    OsinfoDevice *dev1 = osinfo_device_new("e1000");
    OsinfoDevice *dev2 = osinfo_device_new("rtl8139");
    OsinfoFilter *filter = osinfo_filter_new();

    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "network");

    OsinfoDeviceLink *link1 = osinfo_os_add_device(os, dev1);
    osinfo_entity_add_param(OSINFO_ENTITY(link1), "driver", "pci-e1000");
    OsinfoDeviceLink *link2 = osinfo_os_add_device(os, dev2);
    osinfo_entity_add_param(OSINFO_ENTITY(link2), "driver", "pci-8139");

    osinfo_filter_add_constraint(filter, "class", "network");

    osinfo_filter_clear_constraints(filter);
    osinfo_filter_add_constraint(filter, "class", "audio");

    g_object_unref(filter);
    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(os);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/os/basic", test_basic);
    g_test_add_func("/os/loader", test_loader);
    g_test_add_func("/os/devices", test_devices);
    g_test_add_func("/os/devices_filter", test_devices_filter);
    g_test_add_func("/os/device_driver", test_device_driver);

    /* Upfront so we don't confuse valgrind */
    osinfo_platform_get_type();
    osinfo_device_get_type();
    osinfo_os_get_type();
    osinfo_oslist_get_type();
    osinfo_devicelist_get_type();
    osinfo_filter_get_type();

    return g_test_run();
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
