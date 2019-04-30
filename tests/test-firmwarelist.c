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

#include <config.h>

#include <osinfo/osinfo.h>

static void
test_union(void)
{
    OsinfoFirmwareList *list1 = osinfo_firmwarelist_new();
    OsinfoFirmwareList *list2 = osinfo_firmwarelist_new();
    OsinfoFirmwareList *list3;
    OsinfoFirmware *ent1 = osinfo_firmware_new("foo1", "x86_64", "efi");
    OsinfoFirmware *ent2 = osinfo_firmware_new("foo2", "x86_64", "bios");
    OsinfoFirmware *ent3 = osinfo_firmware_new("foo3", "i686", "bios");
    OsinfoFirmware *ent4 = osinfo_firmware_new("foo4", "i686", "efi");

    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));

    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent4));

    list3 = OSINFO_FIRMWARELIST(osinfo_list_new_union(OSINFO_LIST(list1), OSINFO_LIST(list2)));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list3)), ==, 4);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list3)); i++) {
        OsinfoFirmware *ent = OSINFO_FIRMWARE(osinfo_list_get_nth(OSINFO_LIST(list3), i));
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
    OsinfoFirmwareList *list1 = osinfo_firmwarelist_new();
    OsinfoFirmwareList *list2 = osinfo_firmwarelist_new();
    OsinfoFirmwareList *list3;
    OsinfoFirmware *ent1 = osinfo_firmware_new("foo1", "x86_64", "efi");
    OsinfoFirmware *ent2 = osinfo_firmware_new("foo2", "x86_64", "bios");
    OsinfoFirmware *ent3 = osinfo_firmware_new("foo3", "i686", "bios");
    OsinfoFirmware *ent4 = osinfo_firmware_new("foo4", "i686", "efi");


    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));

    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent3));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent4));

    list3 = OSINFO_FIRMWARELIST(osinfo_list_new_intersection(OSINFO_LIST(list1), OSINFO_LIST(list2)));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list3)), ==, 2);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list3)); i++) {
        OsinfoFirmware *ent = OSINFO_FIRMWARE(osinfo_list_get_nth(OSINFO_LIST(list3), i));
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
    OsinfoFirmwareList *list1 = osinfo_firmwarelist_new();
    OsinfoFirmwareList *list2;
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoFirmware *ent1 = osinfo_firmware_new("foo1", "x86_64", "efi");
    OsinfoFirmware *ent2 = osinfo_firmware_new("foo2", "x86_64", "bios");
    OsinfoFirmware *ent3 = osinfo_firmware_new("foo3", "i686", "bios");
    OsinfoFirmware *ent4 = osinfo_firmware_new("foo4", "i686", "efi");

    osinfo_filter_add_constraint(filter, OSINFO_FIRMWARE_PROP_TYPE, "efi");

    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent4));

    list2 = OSINFO_FIRMWARELIST(osinfo_list_new_filtered(OSINFO_LIST(list1), filter));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list2)), ==, 2);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list2)); i++) {
        OsinfoFirmware *ent = OSINFO_FIRMWARE(osinfo_list_get_nth(OSINFO_LIST(list2), i));
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
    g_assert_false(has3);
    g_assert_true(has4);
    g_assert_false(hasBad);

    g_object_unref(ent1);
    g_object_unref(ent2);
    g_object_unref(ent3);
    g_object_unref(ent4);
    g_object_unref(filter);
    g_object_unref(list1);
    g_object_unref(list2);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/firmwarelist/union", test_union);
    g_test_add_func("/firmwarelist/intersect", test_intersect);
    g_test_add_func("/firmwarelist/filter", test_filter);

    /* Upfront so we don't confuse valgrind */
    osinfo_entity_get_type();
    osinfo_firmware_get_type();
    osinfo_firmwarelist_get_type();
    osinfo_filter_get_type();
    osinfo_list_get_type();

    return g_test_run();
}
