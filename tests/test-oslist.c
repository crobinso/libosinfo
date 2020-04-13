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


static void
test_union(void)
{
    OsinfoOsList *list1 = osinfo_oslist_new();
    OsinfoOsList *list2 = osinfo_oslist_new();
    OsinfoOsList *list3;
    OsinfoOs *ent1 = osinfo_os_new("wibble1");
    OsinfoOs *ent2 = osinfo_os_new("wibble2");
    OsinfoOs *ent3 = osinfo_os_new("wibble3");
    OsinfoOs *ent4 = osinfo_os_new("wibble4");

    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));

    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent4));

    list3 = OSINFO_OSLIST(osinfo_list_new_union(OSINFO_LIST(list1), OSINFO_LIST(list2)));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list3)), ==, 4);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list3)); i++) {
        OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(list3), i));
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
    OsinfoOsList *list1 = osinfo_oslist_new();
    OsinfoOsList *list2 = osinfo_oslist_new();
    OsinfoOsList *list3;
    OsinfoOs *ent1 = osinfo_os_new("wibble1");
    OsinfoOs *ent2 = osinfo_os_new("wibble2");
    OsinfoOs *ent3 = osinfo_os_new("wibble3");
    OsinfoOs *ent4 = osinfo_os_new("wibble4");


    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent2));
    osinfo_list_add(OSINFO_LIST(list1), OSINFO_ENTITY(ent3));

    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent1));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent3));
    osinfo_list_add(OSINFO_LIST(list2), OSINFO_ENTITY(ent4));

    list3 = OSINFO_OSLIST(osinfo_list_new_intersection(OSINFO_LIST(list1), OSINFO_LIST(list2)));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list3)), ==, 2);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list3)); i++) {
        OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(list3), i));
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
    OsinfoOsList *list1 = osinfo_oslist_new();
    OsinfoOsList *list2;
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoOs *ent1 = osinfo_os_new("wibble1");
    OsinfoOs *ent2 = osinfo_os_new("wibble2");
    OsinfoOs *ent3 = osinfo_os_new("wibble3");
    OsinfoOs *ent4 = osinfo_os_new("wibble4");

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

    list2 = OSINFO_OSLIST(osinfo_list_new_filtered(OSINFO_LIST(list1), filter));

    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(list2)), ==, 3);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list2)); i++) {
        OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(list2), i));
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

    g_test_add_func("/oslist/union", test_union);
    g_test_add_func("/oslist/intersect", test_intersect);
    g_test_add_func("/oslist/filter", test_filter);

    /* Upfront so we don't confuse valgrind */
    osinfo_os_get_type();
    osinfo_oslist_get_type();
    osinfo_filter_get_type();

    return g_test_run();
}
