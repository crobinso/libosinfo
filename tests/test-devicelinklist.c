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
test_union(void)
{
    OsinfoDeviceLinkList *list1 = osinfo_devicelinklist_new();
    OsinfoDeviceLinkList *list2 = osinfo_devicelinklist_new();
    OsinfoDeviceLinkList *list3;
    OsinfoDevice *dev1 = osinfo_device_new("wibble1");
    OsinfoDevice *dev2 = osinfo_device_new("wibble2");
    OsinfoDevice *dev3 = osinfo_device_new("wibble3");
    OsinfoDevice *dev4 = osinfo_device_new("wibble4");
    OsinfoDeviceLink *ent1 = osinfo_devicelink_new(dev1);
    OsinfoDeviceLink *ent2 = osinfo_devicelink_new(dev2);
    OsinfoDeviceLink *ent3 = osinfo_devicelink_new(dev3);
    OsinfoDeviceLink *ent4 = osinfo_devicelink_new(dev4);

    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));

    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent4));

    list3 = OSINFO_DEVICELINKLIST(osinfo_list_new_union(OSINFO_LIST(list1), OSINFO_LIST(list2)));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list3)), ==, 4);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list3)); i++) {
        OsinfoDeviceLink *ent = OSINFO_DEVICELINK(osinfo_list_get_nth(OSINFO_LIST(list3), i));
        if (ent == ent1)
            has1 = TRUE;
        else if (ent == ent2)
            has2 = TRUE;
        else if (ent == ent3)
            has3 = TRUE;
        else if (ent == ent4)
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
    g_object_unref(list1);
    g_object_unref(list2);
    g_object_unref(list3);
}


static void
test_intersect(void)
{
    OsinfoDeviceLinkList *list1 = osinfo_devicelinklist_new();
    OsinfoDeviceLinkList *list2 = osinfo_devicelinklist_new();
    OsinfoDeviceLinkList *list3;
    OsinfoDevice *dev1 = osinfo_device_new("wibble1");
    OsinfoDevice *dev2 = osinfo_device_new("wibble2");
    OsinfoDevice *dev3 = osinfo_device_new("wibble3");
    OsinfoDevice *dev4 = osinfo_device_new("wibble4");
    OsinfoDeviceLink *ent1 = osinfo_devicelink_new(dev1);
    OsinfoDeviceLink *ent2 = osinfo_devicelink_new(dev2);
    OsinfoDeviceLink *ent3 = osinfo_devicelink_new(dev3);
    OsinfoDeviceLink *ent4 = osinfo_devicelink_new(dev4);

    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));

    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent3));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent4));

    list3 = OSINFO_DEVICELINKLIST(osinfo_list_new_intersection(OSINFO_LIST(list1), OSINFO_LIST(list2)));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list3)), ==, 2);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list3)); i++) {
        OsinfoDeviceLink *ent = OSINFO_DEVICELINK(osinfo_list_get_nth(OSINFO_LIST(list3), i));
        if (ent == ent1)
            has1 = TRUE;
        else if (ent == ent2)
            has2 = TRUE;
        else if (ent == ent3)
            has3 = TRUE;
        else if (ent == ent4)
            has4 = TRUE;
        else
            hasBad = TRUE;
    }
    g_assert_true(has1);
    g_assert_false(has2);
    g_assert_true(has3);
    g_assert_false(has4);
    g_assert_false(hasBad);

    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(dev3);
    g_object_unref(dev4);
    g_object_unref(ent1);
    g_object_unref(ent2);
    g_object_unref(ent3);
    g_object_unref(ent4);
    g_object_unref(list1);
    g_object_unref(list2);
    g_object_unref(list3);
}


static void
test_filter(void)
{
    OsinfoDeviceLinkList *list1 = osinfo_devicelinklist_new();
    OsinfoDeviceLinkList *list2;
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoDevice *dev1 = osinfo_device_new("wibble1");
    OsinfoDevice *dev2 = osinfo_device_new("wibble2");
    OsinfoDevice *dev3 = osinfo_device_new("wibble3");
    OsinfoDevice *dev4 = osinfo_device_new("wibble4");
    OsinfoDeviceLink *ent1 = osinfo_devicelink_new(dev1);
    OsinfoDeviceLink *ent2 = osinfo_devicelink_new(dev2);
    OsinfoDeviceLink *ent3 = osinfo_devicelink_new(dev3);
    OsinfoDeviceLink *ent4 = osinfo_devicelink_new(dev4);

    osinfo_entity_add_param(OSINFO_ENTITY(ent1), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(ent1), "class", "wilma");
    osinfo_entity_add_param(OSINFO_ENTITY(ent2), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(ent3), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(ent3), "class", "audio");
    osinfo_entity_add_param(OSINFO_ENTITY(ent4), "class", "audio");

    osinfo_filter_add_constraint(filter, "class", "network");

    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent4));

    list2 = OSINFO_DEVICELINKLIST(osinfo_list_new_filtered(OSINFO_LIST(list1), filter));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list2)), ==, 3);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list2)); i++) {
        OsinfoDeviceLink *ent = OSINFO_DEVICELINK(osinfo_list_get_nth(OSINFO_LIST(list2), i));
        if (ent == ent1)
            has1 = TRUE;
        else if (ent == ent2)
            has2 = TRUE;
        else if (ent == ent3)
            has3 = TRUE;
        else if (ent == ent4)
            has4 = TRUE;
        else
            hasBad = TRUE;
    }
    g_assert_true(has1);
    g_assert_true(has2);
    g_assert_true(has3);
    g_assert_false(has4);
    g_assert_false(hasBad);

    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(dev3);
    g_object_unref(dev4);
    g_object_unref(ent1);
    g_object_unref(ent2);
    g_object_unref(ent3);
    g_object_unref(ent4);
    g_object_unref(filter);
    g_object_unref(list1);
    g_object_unref(list2);
}


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

    g_test_add_func("/devicelinklist/union", test_union);
    g_test_add_func("/devicelinklist/intersect", test_intersect);
    g_test_add_func("/devicelinklist/filter", test_filter);
    g_test_add_func("/devicelinklist/get_devices", test_get_devices);

    /* Upfront so we don't confuse valgrind */
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
