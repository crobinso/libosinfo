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

/* OsinfoEntity is abstract, so we need to trivially subclass it to test it */
typedef struct _OsinfoDummy        OsinfoDummy;
typedef struct _OsinfoDummyClass   OsinfoDummyClass;

struct _OsinfoDummy
{
    OsinfoEntity parent_instance;
};

struct _OsinfoDummyClass
{
    OsinfoEntityClass parent_class;
};

GType osinfo_dummy_get_type(void);

G_DEFINE_TYPE(OsinfoDummy, osinfo_dummy, OSINFO_TYPE_ENTITY);

static void osinfo_dummy_class_init(OsinfoDummyClass *klass G_GNUC_UNUSED){}
static void osinfo_dummy_init(OsinfoDummy *self G_GNUC_UNUSED) {}


typedef struct _OsinfoDummyList        OsinfoDummyList;
typedef struct _OsinfoDummyListClass   OsinfoDummyListClass;

struct _OsinfoDummyList
{
    OsinfoList parent_instance;
};

struct _OsinfoDummyListClass
{
    OsinfoListClass parent_class;
};

GType osinfo_dummy_list_get_type(void);

G_DEFINE_TYPE(OsinfoDummyList, osinfo_dummy_list, OSINFO_TYPE_LIST);

static void osinfo_dummy_list_class_init(OsinfoDummyListClass *klass G_GNUC_UNUSED){}
static void osinfo_dummy_list_init(OsinfoDummyList *self G_GNUC_UNUSED) {}



static void
test_basic(void)
{
    OsinfoList *list = g_object_new(osinfo_dummy_list_get_type(), NULL);

    g_assert_cmpint(osinfo_list_get_length(list), ==, 0);
    g_assert_null(osinfo_list_find_by_id(list, "wibble"));

    GType type;
    g_object_get(list, "element-type", &type, NULL);
    g_assert_cmpint(type, ==, OSINFO_TYPE_ENTITY);

    type = osinfo_list_get_element_type(list);
    g_assert_cmpint(type, ==, OSINFO_TYPE_ENTITY);

    g_object_unref(list);
}



static void
test_lookup(void)
{
    OsinfoList *list = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "wibble", NULL);

    osinfo_list_add(list, ent);

    g_assert_cmpint(osinfo_list_get_length(list), ==, 1);
    g_assert_true(osinfo_list_get_nth(list, 0) == ent);
    g_assert_nonnull(osinfo_list_find_by_id(list, "wibble"));
    g_assert_null(osinfo_list_find_by_id(list, "fish"));

    g_object_unref(ent);
    g_object_unref(list);
}



static void
test_duplicate(void)
{
    OsinfoList *list = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "wibble", NULL);
    OsinfoEntity *ent_dup = g_object_new(osinfo_dummy_get_type(), "id", "wibble", NULL);

    osinfo_list_add(list, ent);
    osinfo_list_add(list, ent_dup);

    g_assert_cmpint(osinfo_list_get_length(list), ==, 1);
    g_assert_true(osinfo_list_get_nth(list, 0) == ent_dup);
    g_assert_nonnull(osinfo_list_find_by_id(list, "wibble"));
    g_assert_null(osinfo_list_find_by_id(list, "fish"));

    g_object_unref(ent);
    g_object_unref(ent_dup);
    g_object_unref(list);
}



static void
test_union(void)
{
    OsinfoList *list1 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoList *list2 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoList *list3 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoEntity *ent1 = g_object_new(osinfo_dummy_get_type(), "id", "wibble1", NULL);
    OsinfoEntity *ent1_dup = g_object_new(osinfo_dummy_get_type(), "id", "wibble1", NULL);
    OsinfoEntity *ent2 = g_object_new(osinfo_dummy_get_type(), "id", "wibble2", NULL);
    OsinfoEntity *ent3 = g_object_new(osinfo_dummy_get_type(), "id", "wibble3", NULL);
    OsinfoEntity *ent4 = g_object_new(osinfo_dummy_get_type(), "id", "wibble4", NULL);

    osinfo_list_add(list1, ent1);
    osinfo_list_add(list1, ent2);
    osinfo_list_add(list1, ent3);

    osinfo_list_add(list2, ent1);
    osinfo_list_add(list2, ent1_dup);
    osinfo_list_add(list2, ent4);

    osinfo_list_add_union(list3, list1, list2);

    g_assert_cmpint(osinfo_list_get_length(list3), ==, 4);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(list3); i++) {
        OsinfoEntity *ent = osinfo_list_get_nth(list3, i);
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
    g_object_unref(ent1_dup);
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
    OsinfoList *list1 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoList *list2 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoList *list3 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoEntity *ent1 = g_object_new(osinfo_dummy_get_type(), "id", "wibble1", NULL);
    OsinfoEntity *ent1_dup = g_object_new(osinfo_dummy_get_type(), "id", "wibble1", NULL);
    OsinfoEntity *ent2 = g_object_new(osinfo_dummy_get_type(), "id", "wibble2", NULL);
    OsinfoEntity *ent3 = g_object_new(osinfo_dummy_get_type(), "id", "wibble3", NULL);
    OsinfoEntity *ent4 = g_object_new(osinfo_dummy_get_type(), "id", "wibble4", NULL);

    osinfo_list_add(list1, ent1);
    osinfo_list_add(list1, ent2);
    osinfo_list_add(list1, ent3);

    osinfo_list_add(list2, ent1);
    osinfo_list_add(list2, ent1_dup);
    osinfo_list_add(list2, ent3);
    osinfo_list_add(list2, ent4);

    osinfo_list_add_intersection(list3, list1, list2);

    g_assert_cmpint(osinfo_list_get_length(list3), ==, 2);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(list3); i++) {
        OsinfoEntity *ent = osinfo_list_get_nth(list3, i);
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
    g_object_unref(ent1_dup);
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
    OsinfoList *list1 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoList *list2 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoEntity *ent1 = g_object_new(osinfo_dummy_get_type(), "id", "wibble1", NULL);
    OsinfoEntity *ent2 = g_object_new(osinfo_dummy_get_type(), "id", "wibble2", NULL);
    OsinfoEntity *ent3 = g_object_new(osinfo_dummy_get_type(), "id", "wibble3", NULL);
    OsinfoEntity *ent4 = g_object_new(osinfo_dummy_get_type(), "id", "wibble4", NULL);

    osinfo_entity_add_param(ent1, "class", "network");
    osinfo_entity_add_param(ent1, "class", "wilma");
    osinfo_entity_add_param(ent2, "class", "network");
    osinfo_entity_add_param(ent3, "class", "network");
    osinfo_entity_add_param(ent3, "class", "audio");
    osinfo_entity_add_param(ent4, "class", "audio");

    osinfo_filter_add_constraint(filter, "class", "network");

    osinfo_list_add(list1, ent1);
    osinfo_list_add(list1, ent2);
    osinfo_list_add(list1, ent3);
    osinfo_list_add(list1, ent4);

    osinfo_list_add_filtered(list2, list1, filter);

    g_assert_cmpint(osinfo_list_get_length(list2), ==, 3);

    gboolean has1 = FALSE;
    gboolean has2 = FALSE;
    gboolean has3 = FALSE;
    gboolean has4 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(list2); i++) {
        OsinfoEntity *ent = osinfo_list_get_nth(list2, i);
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

struct iterateData {
    OsinfoEntity *ent1;
    OsinfoEntity *ent2;
    OsinfoEntity *ent3;
    OsinfoEntity *ent4;
    gboolean has1;
    gboolean has2;
    gboolean has3;
    gboolean has4;
    gboolean hasBad;
};

static void iterator(gpointer data, gpointer opaque)
{
    OsinfoEntity *ent = OSINFO_ENTITY(data);
    struct iterateData *idata = opaque;

    if (idata->ent1 == ent)
        idata->has1 = TRUE;
    else if (idata->ent2 == ent)
        idata->has2 = TRUE;
    else if (idata->ent3 == ent)
        idata->has3 = TRUE;
    else if (idata->ent4 == ent)
        idata->has4 = TRUE;
    else
        idata->hasBad = TRUE;
}

static void
test_iterate(void)
{
    OsinfoList *list1 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoList *list2 = g_object_new(osinfo_dummy_list_get_type(), NULL);
    OsinfoEntity *ent1 = g_object_new(osinfo_dummy_get_type(), "id", "wibble1", NULL);
    OsinfoEntity *ent2 = g_object_new(osinfo_dummy_get_type(), "id", "wibble2", NULL);
    OsinfoEntity *ent3 = g_object_new(osinfo_dummy_get_type(), "id", "wibble3", NULL);
    OsinfoEntity *ent4 = g_object_new(osinfo_dummy_get_type(), "id", "wibble4", NULL);

    osinfo_list_add(list1, ent1);
    osinfo_list_add(list1, ent2);
    osinfo_list_add(list1, ent3);

    osinfo_list_add(list2, ent1);
    osinfo_list_add(list2, ent4);

    struct iterateData data = {
        ent1, ent2, ent3, ent4,
        FALSE, FALSE, FALSE, FALSE, FALSE
    };
    GList *elements = osinfo_list_get_elements(list1);
    g_list_foreach(elements, iterator, &data);
    g_list_free(elements);
    g_assert_true(data.has1);
    g_assert_true(data.has2);
    g_assert_true(data.has3);
    g_assert_false(data.has4);
    g_assert_false(data.hasBad);

    data.has1 = data.has2 = data.has3 = data.has4 = data.hasBad = FALSE;

    elements = osinfo_list_get_elements(list2);
    g_list_foreach(elements, iterator, &data);
    g_list_free(elements);
    g_assert_true(data.has1);
    g_assert_false(data.has2);
    g_assert_false(data.has3);
    g_assert_true(data.has4);
    g_assert_false(data.hasBad);

    g_object_unref(ent1);
    g_object_unref(ent2);
    g_object_unref(ent3);
    g_object_unref(ent4);
    g_object_unref(list1);
    g_object_unref(list2);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/list/basic", test_basic);
    g_test_add_func("/list/lookup", test_lookup);
    g_test_add_func("/list/duplicate", test_duplicate);
    g_test_add_func("/list/union", test_union);
    g_test_add_func("/list/intersect", test_intersect);
    g_test_add_func("/list/filter", test_filter);
    g_test_add_func("/list/iterate", test_iterate);

    /* Upfront so we don't confuse valgrind */
    osinfo_dummy_get_type();
    osinfo_dummy_list_get_type();
    osinfo_filter_get_type();

    return g_test_run();
}
