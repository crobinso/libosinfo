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
 */

#include <osinfo/osinfo.h>
#include "osinfo/osinfo_product_private.h"



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
    OsinfoTreeList *treelist;
    OsinfoTree *tree;
    GError *error = NULL;
    gsize i = 0;
    gint treelist_len;
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

    os = osinfo_db_get_os(db, "http://fedoraproject.org/fedora/16");
    g_assert_nonnull(os);
    str = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(str, ==, "fedora16");
    treelist = osinfo_os_get_tree_list(os);
    treelist_len = osinfo_list_get_length(OSINFO_LIST(treelist));
    g_assert_cmpint(treelist_len, ==, 2);

    for (i = 0; i < treelist_len; i++) {
        tree = OSINFO_TREE(osinfo_list_get_nth(OSINFO_LIST(treelist), i));

        str = osinfo_tree_get_architecture(tree);
        if (g_str_equal(str, "i686")) {
            g_assert_true(osinfo_tree_has_treeinfo(tree));
            str = osinfo_tree_get_treeinfo_family(tree);
            g_assert_cmpstr(str, ==, "Fedora");
            str = osinfo_tree_get_treeinfo_version(tree);
            g_assert_cmpstr(str, ==, "16");
            str = osinfo_tree_get_treeinfo_arch(tree);
            g_assert_cmpstr(str, ==, "i386");
        } else {
            g_assert_false(osinfo_tree_has_treeinfo(tree));
        }
    }

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


static void
test_device_driver_priority_helper(gint64* expected_priorities,
                                   OsinfoDeviceDriverList *(*get_device_drivers_func)(OsinfoOs *),
                                   gint expected_ddlist_length)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoDeviceDriverList *ddlist;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/drivers/priority");

    ddlist = get_device_drivers_func(os);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(ddlist)), ==, expected_ddlist_length);
    for (int i = 0; i < osinfo_list_get_length(OSINFO_LIST(ddlist)); i++) {
        OsinfoDeviceDriver *dd = OSINFO_DEVICE_DRIVER(osinfo_list_get_nth(OSINFO_LIST(ddlist), i));
        g_assert_cmpint(osinfo_device_driver_get_priority(dd), ==, expected_priorities[i]);
    }

    g_object_unref(db);
}


static void
test_device_driver_priority(void)
{
    gint64 expected_priorities[] = { OSINFO_DEVICE_DRIVER_DEFAULT_PRIORITY, 100 };

    test_device_driver_priority_helper(expected_priorities,
                                       osinfo_os_get_device_drivers,
                                       2);
}


static void
test_device_driver_prioritized_priority(void)
{
    gint64 expected_priorities[] = { 100 };

    test_device_driver_priority_helper(expected_priorities,
                                       osinfo_os_get_device_drivers_prioritized,
                                       1);
}


static void test_resources_basic(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoResourcesList *resources_list;
    OsinfoResources *resources;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/resources/basic");

    resources_list = osinfo_os_get_minimum_resources(os);
    resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(resources_list), 0));
    g_assert_cmpint(osinfo_resources_get_n_cpus(resources), ==, 1);
    g_assert_cmpint(osinfo_resources_get_cpu(resources), ==, 1);
    g_assert_cmpint(osinfo_resources_get_ram(resources), ==, 1);
    g_assert_cmpint(osinfo_resources_get_storage(resources), ==, 1);

    g_clear_object(&resources_list);
    resources_list = osinfo_os_get_recommended_resources(os);
    resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(resources_list), 0));
    g_assert_cmpint(osinfo_resources_get_n_cpus(resources), ==, 2);
    g_assert_cmpint(osinfo_resources_get_cpu(resources), ==, 2);
    g_assert_cmpint(osinfo_resources_get_ram(resources), ==, 2);
    g_assert_cmpint(osinfo_resources_get_storage(resources), ==, 2);

    g_clear_object(&resources_list);
    resources_list = osinfo_os_get_maximum_resources(os);
    resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(resources_list), 0));
    g_assert_cmpint(osinfo_resources_get_n_cpus(resources), ==, 3);
    g_assert_cmpint(osinfo_resources_get_cpu(resources), ==, 3);
    g_assert_cmpint(osinfo_resources_get_ram(resources), ==, 3);
    g_assert_cmpint(osinfo_resources_get_storage(resources), ==, 3);

    g_clear_object(&resources_list);
    resources_list = osinfo_os_get_network_install_resources(os);
    resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(resources_list), 0));
    g_assert_cmpint(osinfo_resources_get_n_cpus(resources), ==, 4);
    g_assert_cmpint(osinfo_resources_get_cpu(resources), ==, 4);
    g_assert_cmpint(osinfo_resources_get_ram(resources), ==, 4);
    g_assert_cmpint(osinfo_resources_get_storage(resources), ==, 4);

    g_object_unref(resources_list);
    g_object_unref(db);
}


static void
devices_inheritance_basic_check(OsinfoDb *db,
                                const gchar *os_id)
{
    OsinfoOs *os;
    OsinfoDeviceList *dev_list;
    OsinfoDevice *dev;

    g_debug("Testing \"%s\"", os_id);

    os = osinfo_db_get_os(db, os_id);
    g_assert(OSINFO_IS_OS(os));

    dev_list = osinfo_os_get_all_devices(os, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(dev_list)), ==, 1);

    dev = OSINFO_DEVICE(osinfo_list_get_nth(OSINFO_LIST(dev_list), 0));
    g_assert(OSINFO_IS_DEVICE(dev));

    g_object_unref(dev_list);
}


static void
test_devices_inheritance_basic(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    /*
     * "http://libosinfo.org/test/os/devices/basic/1 has one device set
     */
    devices_inheritance_basic_check(db, "http://libosinfo.org/test/os/devices/basic/1");

    /*
     * http://libosinfo.org/test/os/devices/basic/2 derives-from
     * http://libosinfo.org/test/os/devices/basic/1
     */
    devices_inheritance_basic_check(db, "http://libosinfo.org/test/os/devices/basic/2");

    /*
     * http://libosinfo.org/test/os/devices/basic/2-clone clones
     * http://libosinfo.org/test/os/devices/basic/2
     */
    devices_inheritance_basic_check(db, "http://libosinfo.org/test/os/devices/basic/2-clone");

    g_object_unref(db);
}


static void
devices_inheritance_removal_check_devs(OsinfoDb *db,
                                       const gchar *os_id,
                                       gint all_devs_list_len,
                                       gint dev_list_len,
                                       gint all_devlinks_list_len,
                                       gint devlink_list_len)
{
    OsinfoOs *os;
    OsinfoDeviceLinkList *devlink_list, *all_devlinks_list;
    OsinfoDeviceList *dev_list, *all_devs_list;

    g_debug("Testing \"%s\"", os_id);

    os = osinfo_db_get_os(db, os_id);
    g_assert(OSINFO_IS_OS(os));

    all_devs_list = osinfo_os_get_all_devices(os, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(all_devs_list)), ==, all_devs_list_len);
    g_object_unref(all_devs_list);

    dev_list = osinfo_os_get_devices(os, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(dev_list)), ==, dev_list_len);
    g_object_unref(dev_list);

    all_devlinks_list = osinfo_os_get_all_device_links(os, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(all_devlinks_list)), ==, all_devlinks_list_len);
    g_object_unref(all_devlinks_list);

    devlink_list = osinfo_os_get_device_links(os, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(devlink_list)), ==, devlink_list_len);
    g_object_unref(devlink_list);
}


static void
test_devices_inheritance_removal(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    /*
     * "http://libosinfo.org/test/os/devices/basic/1 has one device set
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/basic/1",
                                           1, 1, 1, 1);

    /*
     * http://libosinfo.org/test/os/devices/removed/1 derives-from
     * http://libosinfo.org/test/os/devices/basic/1 ...
     * And the device is marked as removed
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/removed/1",
                                           0, 0, 0, 0);

    /*
     * http://libosinfo.org/test/os/devices/removed/2 derives-from
     * http://libosinfo.org/test/os/devices/removed/1
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/removed/2",
                                           0, 0, 0, 0);

    /*
     * http://libosinfo.org/test/os/devices/removed/2-clone clones
     * http://libosinfo.org/test/os/devices/removed/2
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/removed/2-clone",
                                           0, 0, 0, 0);

    /*
     * http://libosinfo.org/test/os/devices/removed/3 derives-from
     * http://libosinfo.org/test/os/devices/removed/2
     * And the device is not marked as removed anymore
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/removed/3",
                                           1, 1, 1, 1);

    /*
     * http://libosinfo.org/test/os/devices/removed/4 derives-from
     * http://libosinfo.org/test/os/devices/removed/3
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/removed/4",
                                           1, 0, 1, 0);


    /*
     * http://libosinfo.org/test/os/devices/removed/4-clone clones
     * http://libosinfo.org/test/os/devices/removed/4
     */
    devices_inheritance_removal_check_devs(db,
                                           "http://libosinfo.org/test/os/devices/removed/4-clone",
                                           1, 0, 1, 0);

    g_object_unref(db);
}


static void
check_resources(OsinfoDb *db,
                const gchar *os_id,
                OsinfoResourcesList *(*get_resourceslist)(OsinfoOs *),
                gint list_len,
                gint expected_n_cpus,
                gint64 expected_cpu,
                gint64 expected_ram,
                gint64 expected_storage)
{
    OsinfoOs *os;
    OsinfoResourcesList *resourceslist;
    OsinfoResources *resources;

    g_test_message("Testing \"%s\"", os_id);

    os = osinfo_db_get_os(db, os_id);
    g_assert_true(OSINFO_IS_OS(os));

    resourceslist = get_resourceslist(os);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(resourceslist)), ==, list_len);

    resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(resourceslist), 0));
    g_assert_true(OSINFO_IS_RESOURCES(resources));

    g_assert_cmpint(osinfo_resources_get_n_cpus(resources), ==, expected_n_cpus);
    g_assert_cmpint(osinfo_resources_get_cpu(resources), ==, expected_cpu);
    g_assert_cmpint(osinfo_resources_get_ram(resources), ==, expected_ram);
    g_assert_cmpint(osinfo_resources_get_storage(resources), ==, expected_storage);

    g_object_unref(resourceslist);
}


static void
test_resources_inheritance(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    /**
     * os1:
     * - minimum
     *   - n_cpus: 1
     *   - cpu: 1
     *   - ram: 1
     *   - storage: 1
     * - recommended:
     *   - n_cpus: 2
     *   - cpu: 2
     *   - ram: 2
     *   - storage: 2
     */
    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/1",
                    osinfo_os_get_minimum_resources,
                    1, 1, 1, 1, 1);

    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/1",
                    osinfo_os_get_recommended_resources,
                    1, 2, 2, 2, 2);

    /**
     * os2 (derives-from os1):
     * - inherit: true
     *
     * - minimum
     *   - n_cpus: 1
     *   - cpu: 1
     *   - ram: 1
     *   - storage: 1
     * - recommended:
     *   - n_cpus: 2
     *   - cpu: 2
     *   - ram: 2
     *   - storage: 2
     */
    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/2",
                    osinfo_os_get_minimum_resources,
                    1, 1, 1, 1, 1);

    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/2",
                    osinfo_os_get_recommended_resources,
                    1, 2, 2, 2, 2);

    /**
     * os3 (derives-from os2):
     * - minimum
     *   - n_cpus: 3
     *   - ram: 3
     * - recommended:
     *   - n_cpus: 6
     *   - ram: 6
     */
    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/3",
                    osinfo_os_get_minimum_resources,
                    1, 3, -1, 3, -1);

    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/3",
                    osinfo_os_get_recommended_resources,
                    1, 6, -1, 6, -1);

    /**
     * os4 (derives-from os3):
     * - minimum
     *   - cpu: 3
     *   - storage: 3
     * - recommended:
     *   - cpus: 6
     *   - storage: 6
     */
    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/4",
                    osinfo_os_get_minimum_resources,
                    1, -1, 3, -1, 3);

    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/4",
                    osinfo_os_get_recommended_resources,
                    1, -1, 6, -1, 6);

    /**
     * os5 (derives-from os4):
     * - inherit: true
     *
     * - minimum
     *   - n_cpus: 3
     *   - cpu: 3
     *   - ram: 3
     *   - storage: 3
     * - recommended:
     *   - n_cpus: 6
     *   - cpus: 6
     *   - ram: 6
     *   - storage: 6
     */
    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/5",
                    osinfo_os_get_minimum_resources,
                    1, 3, 3, 3, 3);

    check_resources(db,
                    "http://libosinfo.org/test/os/resources/inheritance/5",
                    osinfo_os_get_recommended_resources,
                    1, 6, 6, 6, 6);

    g_object_unref(db);
}


static void
test_find_install_script(void)
{
    OsinfoOs *os;
    OsinfoInstallScript *in, *out;

    os = osinfo_os_new("awesome");
    in = osinfo_install_script_new_data("script", "jeos", "foo");
    osinfo_os_add_install_script(os, in);

    out = osinfo_os_find_install_script(os, "default");
    g_assert_null(out);

    out = osinfo_os_find_install_script(os, OSINFO_INSTALL_SCRIPT_PROFILE_DESKTOP);
    g_assert_null(out);

    out = osinfo_os_find_install_script(os, OSINFO_INSTALL_SCRIPT_PROFILE_JEOS);
    g_assert_nonnull(out);

    g_object_unref(in);
    g_object_unref(os);
}

static void
test_multiple_short_ids(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    GList *shortid_list, *l;
    GError *error = NULL;
    const gchar *shortid;
    const gchar *expected_short_id_list[] = {"shortid0", "shortid1", "shortid2"};
    gsize i;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/multipleshortids");
    g_assert(OSINFO_IS_OS(os));

    shortid = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    g_assert_cmpstr(shortid, ==, expected_short_id_list[0]);

    shortid_list = osinfo_product_get_short_id_list(OSINFO_PRODUCT(os));
    for (l = shortid_list, i = 0; l != NULL; l = l->next, i++) {
        g_assert_cmpstr(l->data, ==, expected_short_id_list[i]);
    }
    g_list_free(shortid_list);

    g_object_unref(db);
}

static void
test_kernel_url_arg(void)
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

    g_debug("Testing http://libosinfo.org/test/os/kernel-url-arg1\n");
    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/kernel-url-arg1");
    g_assert_nonnull(os);
    str = osinfo_os_get_kernel_url_argument(os);
    g_assert_cmpstr(str, ==, "install");

    g_debug("Testing http://libosinfo.org/test/os/kernel-url-arg2\n");
    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/kernel-url-arg2");
    g_assert_nonnull(os);
    str = osinfo_os_get_kernel_url_argument(os);
    g_assert_cmpstr(str, ==, "install");

    g_debug("Testing http://libosinfo.org/test/os/kernel-url-arg3\n");
    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/kernel-url-arg3");
    g_assert_nonnull(os);
    str = osinfo_os_get_kernel_url_argument(os);
    g_assert_cmpstr(str, ==, "new.install");

    g_object_unref(loader);
}


static void
check_firmwares(OsinfoDb *db,
                const gchar *os_id,
                gint list_len)
{
    OsinfoOs *os;
    OsinfoFirmwareList *firmwarelist;

    g_test_message("Testing \"%s\"", os_id);

    os = osinfo_db_get_os(db, os_id);
    g_assert_true(OSINFO_IS_OS(os));

    firmwarelist = osinfo_os_get_firmware_list(os, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(firmwarelist)), ==, list_len);

    g_object_unref(firmwarelist);
}

static void
test_firmwares_inheritance(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    /**
     * os1:
     * - firmwares:
     *   - explicitly added
     *     - x86_64 | bios
     *     - x86_64 | efi
     *   - explicitly removed
     *   - expected:
     *     - x86_64 | bios
     *     - x86_64 | efi

     */
    check_firmwares(db,
                    "http://libosinfo.org/test/os/firmwares/inheritance/1",
                    2);

    /**
     * os2 (derives-from os1):
     * - firmwares
     *   - explicitly added
     *   - explicitly removed
     *   - expected:
     *     - x86_64 | bios
     *     - x86_64 | efi
     */
    check_firmwares(db,
                    "http://libosinfo.org/test/os/firmwares/inheritance/2",
                    2);

    /**
     * os3 (derives-from os2):
     * - firmwares
     *   - explicitly added
     *   - explicitly removed
     *     - x86_64 | bios
     *   - expected:
     *     - x86_64 | efi

     */
    check_firmwares(db,
                    "http://libosinfo.org/test/os/firmwares/inheritance/3",
                    1);

    /**
     * os4 (derives-from os3):
     * - firmwares
     *   - explicitly added
     *   - explicitly removed
     *   - expected:
     *     - x86_64 | efi
     */
    check_firmwares(db,
                    "http://libosinfo.org/test/os/firmwares/inheritance/4",
                    1);

    /**
     * os5 (derives-from os4):
     * - firmwares
     *   - explicitly added
     *     - x86_64 | bios
     *   - explicitly removed
     *   - expected:
     *     - x86_64 | bios
     *     - x86_64 | efi
     */
    check_firmwares(db,
                    "http://libosinfo.org/test/os/firmwares/inheritance/5",
                    2);

    /**
     * os6 (derives-from os5):
     * - firmwares
     *   - explicitly added
     *   - explicitly removed
     *   - expected:
     *     - x86_64 | bios
     *     - x86_64 | efi
     */
    check_firmwares(db,
                    "http://libosinfo.org/test/os/firmwares/inheritance/6",
                    2);

    g_object_unref(db);
}

static void
test_cloud_image_username_arg(void)
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

    g_debug("Testing http://libosinfo.org/test/os/cloud-image-username-arg1\n");
    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/cloud-image-username-arg1");
    g_assert_nonnull(os);
    str = osinfo_os_get_cloud_image_username(os);
    g_assert_cmpstr(str, ==, "cloud");

    g_debug("Testing http://libosinfo.org/test/os/cloud-image-username-arg2\n");
    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/cloud-image-username-arg2");
    g_assert_nonnull(os);
    str = osinfo_os_get_cloud_image_username(os);
    g_assert_cmpstr(str, ==, "cloud");

    g_debug("Testing http://libosinfo.org/test/os/cloud-image-username-arg3\n");
    os = osinfo_db_get_os(db, "http://libosinfo.org/test/os/cloud-image-username-arg3");
    g_assert_nonnull(os);
    str = osinfo_os_get_cloud_image_username(os);
    g_assert_cmpstr(str, ==, "new.cloud");

    g_object_unref(loader);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();

    g_test_add_func("/os/basic", test_basic);
    g_test_add_func("/os/loader", test_loader);
    g_test_add_func("/os/devices", test_devices);
    g_test_add_func("/os/devices_filter", test_devices_filter);
    g_test_add_func("/os/device_driver", test_device_driver);
    g_test_add_func("/os/device_driver/priority", test_device_driver_priority);
    g_test_add_func("/os/device_driver/prioritized_priority",
                    test_device_driver_prioritized_priority);
    g_test_add_func("/os/devices/inheritance/basic",
                    test_devices_inheritance_basic);
    g_test_add_func("/os/devices/inheritance/removal",
                    test_devices_inheritance_removal);
    g_test_add_func("/os/resources/basic", test_resources_basic);
    g_test_add_func("/os/resources/inheritance", test_resources_inheritance);
    g_test_add_func("/os/find_install_script", test_find_install_script);
    g_test_add_func("/os/mulitple_short_ids", test_multiple_short_ids);
    g_test_add_func("/os/kernel_url_arg", test_kernel_url_arg);
    g_test_add_func("/os/firmwares/inheritance", test_firmwares_inheritance);
    g_test_add_func("/os/cloud_image_username_arg", test_cloud_image_username_arg);

    /* Upfront so we don't confuse valgrind */
    osinfo_platform_get_type();
    osinfo_device_get_type();
    osinfo_os_get_type();
    osinfo_oslist_get_type();
    osinfo_devicelist_get_type();
    osinfo_filter_get_type();
    osinfo_resources_get_type();
    osinfo_resourceslist_get_type();
    osinfo_install_script_get_type();
    osinfo_tree_get_type();
    osinfo_treelist_get_type();

    return g_test_run();
}
