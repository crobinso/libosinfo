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
test_get_devices(void)
{
    OsinfoDeviceLinkList *devlink_list = osinfo_devicelinklist_new();
    OsinfoDevice *dev1 = osinfo_device_new("wibble1");
    OsinfoDevice *dev2 = osinfo_device_new("wibble2");
    OsinfoDevice *dev3 = osinfo_device_new("wibble3");
    OsinfoDevice *dev4 = osinfo_device_new("wibble4");
    OsinfoDeviceLink *ent1 = osinfo_devicelink_new(dev1);
    OsinfoDeviceLink *ent2 = osinfo_devicelink_new(dev2);
    OsinfoDeviceLink *ent3 = osinfo_devicelink_new(dev3);
    OsinfoDeviceLink *ent4 = osinfo_devicelink_new(dev4);
    OsinfoDeviceList *dev_list;

    osinfo_list_add(OSINFO_LIST(devlink_list), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(devlink_list), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(devlink_list), OSINFO_ENTITY(ent3));
    osinfo_list_add(OSINFO_LIST(devlink_list), OSINFO_ENTITY(ent4));
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(devlink_list)), ==, 4);

    dev_list = osinfo_devicelinklist_get_devices(devlink_list, NULL);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(dev_list)), ==, 4);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(dev_list)); i++) {
        OsinfoDevice *dev = OSINFO_DEVICE(osinfo_list_get_nth(OSINFO_LIST(dev_list), i));
        if (dev == dev1)
            has1 = TRUE;
        else if (dev == dev2)
            has2 = TRUE;
        else if (dev == dev3)
            has3 = TRUE;
        else if (dev == dev4)
            has4 = TRUE;
        else
            hasBad = TRUE;
    }
    g_assert_true(has1);
    g_assert_true(has2);
    g_assert_true(has3);
    g_assert_true(has4);
    g_assert_false(hasBad);

    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(dev3);
    g_object_unref(dev4);
    g_object_unref(ent1);
    g_object_unref(ent2);
    g_object_unref(ent3);
    g_object_unref(ent4);
    g_object_unref(dev_list);
    g_object_unref(devlink_list);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/devicelinklist/get_devices", test_get_devices);

    /* Upfront so we don't confuse valgrind */
    osinfo_device_get_type();
    osinfo_devicelist_get_type();
    osinfo_filter_get_type();

    return g_test_run();
}
