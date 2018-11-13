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


static void test_n_cpus(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint minimum_cpus, recommended_cpus;

    minimum_cpus = osinfo_resources_get_n_cpus(minimum);
    recommended_cpus = osinfo_resources_get_n_cpus(recommended);

    if (recommended_cpus >= 0 && minimum_cpus >= 0)
        g_assert_true(recommended_cpus >= minimum_cpus);
}


static void test_cpu(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint64 minimum_cpu, recommended_cpu;

    minimum_cpu = osinfo_resources_get_cpu(minimum);
    recommended_cpu = osinfo_resources_get_cpu(recommended);

    if (recommended_cpu >= 0 && minimum_cpu >= 0)
        g_assert_true(recommended_cpu >= minimum_cpu);
}


static void test_ram(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint64 minimum_ram, recommended_ram;

    minimum_ram = osinfo_resources_get_ram(minimum);
    recommended_ram = osinfo_resources_get_ram(recommended);

    if (recommended_ram >= 0 && minimum_ram >= 0)
        g_assert_true(recommended_ram >= minimum_ram);
}


static void test_storage(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint64 minimum_storage, recommended_storage;

    minimum_storage = osinfo_resources_get_storage(minimum);
    recommended_storage = osinfo_resources_get_storage(recommended);

    if (recommended_storage >= 0 && minimum_storage >= 0)
        g_assert_true(recommended_storage >= minimum_storage);
}

static void
test_resources_minimum_recommended(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    OsinfoOsList *oslist;
    GList *oses;
    GList *oses_it;
    GError *error = NULL;

    g_assert_true(OSINFO_IS_LOADER(loader));
    g_assert_true(OSINFO_IS_DB(db));

    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);

    oslist = osinfo_db_get_os_list(db);
    oses = osinfo_list_get_elements(OSINFO_LIST(oslist));

    for (oses_it = oses; oses_it != NULL; oses_it = oses_it->next) {
        OsinfoOs *os = oses_it->data;
        OsinfoResourcesList *minimum_list, *recommended_list;
        GList *minimum_resources, *recommended_resources;
        GList *resources_it;
        const gchar *minimum_arch, *recommended_arch;

        minimum_list = osinfo_os_get_minimum_resources(os);
        minimum_resources = osinfo_list_get_elements(OSINFO_LIST(minimum_list));

        recommended_list = osinfo_os_get_recommended_resources(os);
        recommended_resources = osinfo_list_get_elements(OSINFO_LIST(recommended_list));

        /* That's fine as not all OSes have those fields filled */
        if (minimum_resources == NULL || recommended_resources == NULL)
            goto next;

        for (resources_it = minimum_resources; resources_it != NULL; resources_it = resources_it->next) {
            OsinfoResources *minimum = resources_it->data;
            GList *tmp = recommended_resources;

            minimum_arch = osinfo_resources_get_architecture(minimum);

            for (; tmp != NULL; tmp = tmp->next) {
                OsinfoResources *recommended = tmp->data;

                recommended_arch = osinfo_resources_get_architecture(recommended);

                 if (g_str_equal(minimum_arch, recommended_arch)) {
                    const gchar *name;

                    name = osinfo_product_get_name(OSINFO_PRODUCT(os));

                    g_test_message("checking %s (architecture: %s)",
                                   name, minimum_arch);

                    test_n_cpus(minimum, recommended);
                    test_cpu(minimum, recommended);
                    test_ram(minimum, recommended);
                    test_storage(minimum, recommended);
                    break;
                }
            }
        }

 next:
        g_list_free(minimum_resources);
        g_list_free(recommended_resources);
        g_object_unref(minimum_list);
        g_object_unref(recommended_list);
    }

    g_object_unref(oslist);
    g_list_free(oses);

    g_object_unref(loader);
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
    g_test_add_func("/os/resources/minimum_recommended", test_resources_minimum_recommended);

    /* Upfront so we don't confuse valgrind */
    osinfo_platform_get_type();
    osinfo_device_get_type();
    osinfo_os_get_type();
    osinfo_oslist_get_type();
    osinfo_devicelist_get_type();
    osinfo_filter_get_type();
    osinfo_resources_get_type();
    osinfo_resourceslist_get_type();

    return g_test_run();
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
