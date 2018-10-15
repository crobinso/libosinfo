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
    OsinfoPlatform *platform = osinfo_platform_new("awesome");

    g_assert_true(OSINFO_IS_PLATFORM(platform));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(platform)), ==, "awesome");

    g_object_unref(platform);
}

static void
test_devices(void)
{
    OsinfoPlatform *hv = osinfo_platform_new("awesome");
    OsinfoDevice *dev1 = osinfo_device_new("e1000");
    OsinfoDevice *dev2 = osinfo_device_new("rtl8139");

    OsinfoDeviceLink *link1 = osinfo_platform_add_device(hv, dev1);
    osinfo_entity_add_param(OSINFO_ENTITY(link1), "device", "pci-e1000");
    OsinfoDeviceLink *link2 = osinfo_platform_add_device(hv, dev2);
    osinfo_entity_add_param(OSINFO_ENTITY(link2), "device", "pci-8139");

    OsinfoDeviceList *devices = osinfo_platform_get_devices(hv, NULL);

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(devices)), ==, 2);

    gboolean hasDev1 = FALSE;
    gboolean hasDev2 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(devices)); i++) {
        OsinfoEntity *ent = osinfo_list_get_nth(OSINFO_LIST(devices), i);
        g_assert_true(OSINFO_IS_DEVICE(ent));
        if (OSINFO_DEVICE(ent) == dev1)
            hasDev1 = TRUE;
        else if (OSINFO_DEVICE(ent) == dev2)
            hasDev2 = TRUE;
        else
            hasBad = TRUE;
    }
    g_assert_true(hasDev1);
    g_assert_true(hasDev2);
    g_assert_false(hasBad);

    g_object_unref(devices);
    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(hv);
}


static void
test_devices_filter(void)
{
    OsinfoPlatform *hv = osinfo_platform_new("awesome");
    OsinfoDevice *dev1 = osinfo_device_new("e1000");
    OsinfoDevice *dev2 = osinfo_device_new("sb16");
    OsinfoFilter *filter = osinfo_filter_new();

    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "audio");

    OsinfoDeviceLink *link1 = osinfo_platform_add_device(hv, dev1);
    osinfo_entity_add_param(OSINFO_ENTITY(link1), "device", "pci-e1000");
    OsinfoDeviceLink *link2 = osinfo_platform_add_device(hv, dev2);
    osinfo_entity_add_param(OSINFO_ENTITY(link2), "device", "isa-sb16");

    osinfo_filter_add_constraint(filter, "class", "network");

    OsinfoDeviceList *devices = osinfo_platform_get_devices(hv, filter);

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(devices)), ==, 1);
    OsinfoEntity *ent = osinfo_list_get_nth(OSINFO_LIST(devices), 0);
    g_assert_true(OSINFO_IS_DEVICE(ent));
    g_assert_true(OSINFO_DEVICE(ent) == dev1);

    g_object_unref(devices);
    g_object_unref(filter);
    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(hv);
}



int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/platform/basic", test_basic);
    g_test_add_func("/platform/devices", test_devices);
    g_test_add_func("/platform/devices_filter", test_devices_filter);

    /* Upfront so we don't confuse valgrind */
    osinfo_platform_get_type();
    osinfo_device_get_type();
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
