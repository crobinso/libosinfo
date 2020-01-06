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


static void
test_id(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);
    gchar *id;

    g_assert_cmpstr(osinfo_entity_get_id(ent), ==, "myentity");

    g_object_get(ent, "id", &id, NULL);
    g_assert_cmpstr(id, ==, "myentity");
    g_free(id);

    g_object_unref(ent);
}

static void
test_empty_props(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);
    GList *keys;
    const gchar *value;
    GList *values;

    keys = osinfo_entity_get_param_keys(ent);
    g_assert_nonnull(keys);
    g_assert_null(keys->next);
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "wibble");
    g_assert_null(value);

    values = osinfo_entity_get_param_value_list(ent, "wibble");
    g_assert_null(values);

    g_object_unref(ent);
}

static void
test_single_prop_value(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);
    GList *keys;
    const gchar *value;
    GList *values;

    osinfo_entity_add_param(ent, "hello", "world");

    keys = osinfo_entity_get_param_keys(ent);
    g_assert_nonnull(keys);
    g_assert_null(keys->next->next);
    g_assert_cmpstr(keys->data, ==, "hello");
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "hello");
    g_assert_cmpstr(value, ==, "world");
    value = osinfo_entity_get_param_value(ent, "world");
    g_assert_null(value);

    values = osinfo_entity_get_param_value_list(ent, "hello");
    g_assert_nonnull(values);
    g_assert_null(values->next);
    g_assert_cmpstr(values->data, ==, "world");
    g_list_free(values);

    g_object_unref(ent);
}

static void
test_multi_prop_value(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);
    GList *keys;
    const gchar *value;
    GList *values;

    osinfo_entity_add_param(ent, "hello", "world");
    osinfo_entity_add_param(ent, "hello", "fred");
    osinfo_entity_add_param(ent, "hello", "elephant");

    keys = osinfo_entity_get_param_keys(ent);
    g_assert_nonnull(keys);
    g_assert_null(keys->next->next);
    g_assert_cmpstr(keys->data, ==, "hello");
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "hello");
    g_assert_cmpstr(value, ==, "world");
    value = osinfo_entity_get_param_value(ent, "world");
    g_assert_null(value);

    values = osinfo_entity_get_param_value_list(ent, "hello");
    g_assert_nonnull(values);
    g_assert_nonnull(values->next);
    g_assert_nonnull(values->next->next);
    g_assert_null(values->next->next->next);
    g_assert_cmpstr(values->data, ==, "world");
    g_assert_cmpstr(values->next->data, ==, "fred");
    g_assert_cmpstr(values->next->next->data, ==, "elephant");
    g_list_free(values);

    g_object_unref(ent);
}

static void
test_multi_props(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);
    GList *keys;
    GList *tmp;
    const gchar *value;
    GList *values;
    gboolean foundHello;
    gboolean foundFish;
    gboolean foundKevin;
    gboolean foundBad;


    osinfo_entity_add_param(ent, "hello", "world");
    osinfo_entity_add_param(ent, "fish", "food");
    osinfo_entity_add_param(ent, "kevin", "bacon");

    keys = osinfo_entity_get_param_keys(ent);
    tmp = keys;
    foundHello = FALSE;
    foundFish = FALSE;
    foundKevin = FALSE;
    foundBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "hello") == 0)
            foundHello = TRUE;
        else if (g_strcmp0(tmp->data, "fish") == 0)
            foundFish = TRUE;
        else if (g_strcmp0(tmp->data, "kevin") == 0)
            foundKevin = TRUE;
        else if (g_strcmp0(tmp->data, "id") != 0)
            foundBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_true(foundHello);
    g_assert_true(foundFish);
    g_assert_true(foundKevin);
    g_assert_false(foundBad);
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "hello");
    g_assert_cmpstr(value, ==, "world");
    value = osinfo_entity_get_param_value(ent, "fish");
    g_assert_cmpstr(value, ==, "food");
    value = osinfo_entity_get_param_value(ent, "kevin");
    g_assert_cmpstr(value, ==, "bacon");

    values = osinfo_entity_get_param_value_list(ent, "hello");
    g_assert_nonnull(values);
    g_assert_null(values->next);
    g_assert_cmpstr(values->data, ==, "world");
    g_list_free(values);

    values = osinfo_entity_get_param_value_list(ent, "fish");
    g_assert_nonnull(values);
    g_assert_null(values->next);
    g_assert_cmpstr(values->data, ==, "food");
    g_list_free(values);

    values = osinfo_entity_get_param_value_list(ent, "kevin");
    g_assert_nonnull(values);
    g_assert_null(values->next);
    g_assert_cmpstr(values->data, ==, "bacon");
    g_list_free(values);

    g_object_unref(ent);
}


static void
test_multi_props_clear(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);
    GList *keys;
    GList *tmp;
    gboolean foundHello;
    gboolean foundFish;
    gboolean foundBad;
    const gchar *value;

    osinfo_entity_add_param(ent, "hello", "world");
    osinfo_entity_add_param(ent, "fish", "food");

    keys = osinfo_entity_get_param_keys(ent);
    tmp = keys;
    foundHello = FALSE;
    foundFish = FALSE;
    foundBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "hello") == 0)
            foundHello = TRUE;
        else if (g_strcmp0(tmp->data, "fish") == 0)
            foundFish = TRUE;
        else if (g_strcmp0(tmp->data, "id") != 0)
            foundBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_true(foundHello);
    g_assert_true(foundFish);
    g_assert_false(foundBad);
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "hello");
    g_assert_cmpstr(value, ==, "world");
    value = osinfo_entity_get_param_value(ent, "fish");
    g_assert_cmpstr(value, ==, "food");

    osinfo_entity_clear_param(ent, "hello");

    keys = osinfo_entity_get_param_keys(ent);
    tmp = keys;
    foundHello = FALSE;
    foundFish = FALSE;
    foundBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "hello") == 0)
            foundHello = TRUE;
        else if (g_strcmp0(tmp->data, "fish") == 0)
            foundFish = TRUE;
        else if (g_strcmp0(tmp->data, "id") != 0)
            foundBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_false(foundHello);
    g_assert_true(foundFish);
    g_assert_false(foundBad);
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "hello");
    g_assert_null(value);
    value = osinfo_entity_get_param_value(ent, "fish");
    g_assert_cmpstr(value, ==, "food");

    osinfo_entity_add_param(ent, "hello", "world");
    osinfo_entity_clear_param(ent, "fish");

    keys = osinfo_entity_get_param_keys(ent);
    tmp = keys;
    foundHello = FALSE;
    foundFish = FALSE;
    foundBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "hello") == 0)
            foundHello = TRUE;
        else if (g_strcmp0(tmp->data, "fish") == 0)
            foundFish = TRUE;
        else if (g_strcmp0(tmp->data, "id") != 0)
            foundBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_true(foundHello);
    g_assert_false(foundFish);
    g_assert_false(foundBad);
    g_list_free(keys);

    value = osinfo_entity_get_param_value(ent, "hello");
    g_assert_cmpstr(value, ==, "world");
    value = osinfo_entity_get_param_value(ent, "fish");
    g_assert_null(value);

    g_object_unref(ent);
}


static void
test_int64_props(void)
{
    OsinfoEntity *ent = g_object_new(osinfo_dummy_get_type(), "id", "myentity", NULL);

    osinfo_entity_set_param_int64(ent, "my_int", 10);
    g_assert_true(osinfo_entity_get_param_value_int64(ent, "my_int") == 10);
    osinfo_entity_set_param_int64(ent, "my_neg_int", -20);
    g_assert_true(osinfo_entity_get_param_value_int64(ent, "my_neg_int") == -20);
    osinfo_entity_set_param_int64(ent, "my_str", 30);
    g_assert_true(osinfo_entity_get_param_value_int64(ent, "my_str") == 30);
    osinfo_entity_set_param_int64(ent, "my_neg_str", -40);
    g_assert_true(osinfo_entity_get_param_value_int64(ent, "my_neg_str") == -40);
    g_assert_true(osinfo_entity_get_param_value_int64_with_default(ent, "my_neg_str", 1234) == -40);

    g_assert_true(osinfo_entity_get_param_value_int64(ent, "missing") == -1);
    g_assert_true(osinfo_entity_get_param_value_int64_with_default(ent, "missing", 1234) == 1234);

    g_object_unref(ent);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/entity/id", test_id);
    g_test_add_func("/entity/empty_props", test_empty_props);
    g_test_add_func("/entity/single_prop_value", test_single_prop_value);
    g_test_add_func("/entity/multi_prop_value", test_multi_prop_value);
    g_test_add_func("/entity/multi_props", test_multi_props);
    g_test_add_func("/entity/multi_props_clear", test_multi_props_clear);
    g_test_add_func("/entity/int64_props", test_int64_props);

    /* Upfront so we don't confuse valgrind */
    osinfo_dummy_get_type();

    return g_test_run();
}
