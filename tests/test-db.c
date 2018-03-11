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
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>

static void
test_basic(void)
{
    OsinfoDb *db = osinfo_db_new();

    g_assert_true(OSINFO_IS_DB(db));

    g_object_unref(db);
}


static void
test_device(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoDevice *dev1 = osinfo_device_new("dev1");
    OsinfoDevice *dev2 = osinfo_device_new("dev2");
    OsinfoDevice *dev3 = osinfo_device_new("dev3");

    osinfo_db_add_device(db, dev1);
    osinfo_db_add_device(db, dev2);
    osinfo_db_add_device(db, dev3);

    OsinfoDeviceList *list = osinfo_db_get_device_list(db);
    g_assert_true(OSINFO_ENTITY(dev1) == osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(OSINFO_ENTITY(dev2) == osinfo_list_get_nth(OSINFO_LIST(list), 1));
    g_assert_true(OSINFO_ENTITY(dev3) == osinfo_list_get_nth(OSINFO_LIST(list), 2));
    g_object_unref(list);

    OsinfoDevice *dev = osinfo_db_get_device(db, "dev2");
    g_assert_nonnull(dev);
    g_assert_true(dev == dev2);

    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(dev3);
    g_object_unref(db);
}


static void
test_platform(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoPlatform *hv1 = osinfo_platform_new("hv1");
    OsinfoPlatform *hv2 = osinfo_platform_new("hv2");
    OsinfoPlatform *hv3 = osinfo_platform_new("hv3");

    osinfo_db_add_platform(db, hv1);
    osinfo_db_add_platform(db, hv2);
    osinfo_db_add_platform(db, hv3);

    OsinfoPlatformList *list = osinfo_db_get_platform_list(db);
    g_assert_true(OSINFO_ENTITY(hv1) == osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(OSINFO_ENTITY(hv2) == osinfo_list_get_nth(OSINFO_LIST(list), 1));
    g_assert_true(OSINFO_ENTITY(hv3) == osinfo_list_get_nth(OSINFO_LIST(list), 2));
    g_object_unref(list);

    OsinfoPlatform *hv = osinfo_db_get_platform(db, "hv2");
    g_assert_nonnull(hv);
    g_assert_true(hv == hv2);

    g_object_unref(hv1);
    g_object_unref(hv2);
    g_object_unref(hv3);
    g_object_unref(db);
}


static void
test_os(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoOs *os1 = osinfo_os_new("os1");
    OsinfoOs *os2 = osinfo_os_new("os2");
    OsinfoOs *os3 = osinfo_os_new("os3");

    osinfo_db_add_os(db, os1);
    osinfo_db_add_os(db, os2);
    osinfo_db_add_os(db, os3);

    OsinfoOsList *list = osinfo_db_get_os_list(db);
    g_assert_true(OSINFO_ENTITY(os1) == osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(OSINFO_ENTITY(os2) == osinfo_list_get_nth(OSINFO_LIST(list), 1));
    g_assert_true(OSINFO_ENTITY(os3) == osinfo_list_get_nth(OSINFO_LIST(list), 2));
    g_object_unref(list);

    OsinfoOs *os = osinfo_db_get_os(db, "os2");
    g_assert_nonnull(os);
    g_assert_true(os == os2);

    g_object_unref(os1);
    g_object_unref(os2);
    g_object_unref(os3);
    g_object_unref(db);
}



static void
test_prop_device(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoDevice *dev1 = osinfo_device_new("dev1");
    OsinfoDevice *dev2 = osinfo_device_new("dev2");
    OsinfoDevice *dev3 = osinfo_device_new("dev3");

    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "audio");
    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "input");
    osinfo_entity_add_param(OSINFO_ENTITY(dev3), "class", "display");
    osinfo_entity_add_param(OSINFO_ENTITY(dev3), "fruit", "apple");

    osinfo_db_add_device(db, dev1);
    osinfo_db_add_device(db, dev2);
    osinfo_db_add_device(db, dev3);

    GList *uniq = osinfo_db_unique_values_for_property_in_device(db, "class");
    GList *tmp = uniq;
    gboolean hasNetwork = FALSE;
    gboolean hasAudio = FALSE;
    gboolean hasInput = FALSE;
    gboolean hasDisplay = FALSE;
    gboolean hasBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "network") == 0)
            hasNetwork = TRUE;
        else if (g_strcmp0(tmp->data, "audio") == 0)
            hasAudio = TRUE;
        else if (g_strcmp0(tmp->data, "input") == 0)
            hasInput = TRUE;
        else if (g_strcmp0(tmp->data, "display") == 0)
            hasDisplay = TRUE;
        else
            hasBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_true(hasNetwork);
    g_assert_true(hasAudio);
    g_assert_true(hasInput);
    g_assert_true(hasDisplay);
    g_assert_false(hasBad);

    g_list_free(uniq);

    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(dev3);
    g_object_unref(db);
}


static void
test_prop_platform(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoPlatform *hv1 = osinfo_platform_new("hv1");
    OsinfoPlatform *hv2 = osinfo_platform_new("hv2");
    OsinfoPlatform *hv3 = osinfo_platform_new("hv3");

    osinfo_entity_add_param(OSINFO_ENTITY(hv1), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(hv1), "vendor", "frog");
    osinfo_entity_add_param(OSINFO_ENTITY(hv2), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(hv3), "vendor", "dog");
    osinfo_entity_add_param(OSINFO_ENTITY(hv3), "arch", "x86");

    osinfo_db_add_platform(db, hv1);
    osinfo_db_add_platform(db, hv2);
    osinfo_db_add_platform(db, hv3);

    GList *uniq = osinfo_db_unique_values_for_property_in_platform(db, "vendor");
    GList *tmp = uniq;
    gboolean hasAcme = FALSE;
    gboolean hasFrog = FALSE;
    gboolean hasDog = FALSE;
    gboolean hasBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "acme") == 0)
            hasAcme = TRUE;
        else if (g_strcmp0(tmp->data, "frog") == 0)
            hasFrog = TRUE;
        else if (g_strcmp0(tmp->data, "dog") == 0)
            hasDog = TRUE;
        else
            hasBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_true(hasAcme);
    g_assert_true(hasFrog);
    g_assert_true(hasDog);
    g_assert_false(hasBad);

    g_list_free(uniq);

    g_object_unref(hv1);
    g_object_unref(hv2);
    g_object_unref(hv3);
    g_object_unref(db);
}


static void
test_prop_os(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoOs *os1 = osinfo_os_new("os1");
    OsinfoOs *os2 = osinfo_os_new("os2");
    OsinfoOs *os3 = osinfo_os_new("os3");

    osinfo_entity_add_param(OSINFO_ENTITY(os1), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(os1), "vendor", "frog");
    osinfo_entity_add_param(OSINFO_ENTITY(os2), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(os3), "vendor", "dog");
    osinfo_entity_add_param(OSINFO_ENTITY(os3), "arch", "x86");

    osinfo_db_add_os(db, os1);
    osinfo_db_add_os(db, os2);
    osinfo_db_add_os(db, os3);

    GList *uniq = osinfo_db_unique_values_for_property_in_os(db, "vendor");
    GList *tmp = uniq;
    gboolean hasAcme = FALSE;
    gboolean hasFrog = FALSE;
    gboolean hasDog = FALSE;
    gboolean hasBad = FALSE;
    while (tmp) {
        if (g_strcmp0(tmp->data, "acme") == 0)
            hasAcme = TRUE;
        else if (g_strcmp0(tmp->data, "frog") == 0)
            hasFrog = TRUE;
        else if (g_strcmp0(tmp->data, "dog") == 0)
            hasDog = TRUE;
        else
            hasBad = TRUE;
        tmp = tmp->next;
    }
    g_assert_true(hasAcme);
    g_assert_true(hasFrog);
    g_assert_true(hasDog);
    g_assert_false(hasBad);

    g_list_free(uniq);

    g_object_unref(os1);
    g_object_unref(os2);
    g_object_unref(os3);
    g_object_unref(db);
}



static void
test_rel_os(void)
{
    OsinfoDb *db = osinfo_db_new();
    OsinfoOs *os1 = osinfo_os_new("os1");
    OsinfoOs *os2 = osinfo_os_new("os2");
    OsinfoOs *os3 = osinfo_os_new("os3");
    OsinfoOs *os4 = osinfo_os_new("os4");
    OsinfoOs *os5 = osinfo_os_new("os5");

    osinfo_db_add_os(db, os1);
    osinfo_db_add_os(db, os2);
    osinfo_db_add_os(db, os3);
    osinfo_db_add_os(db, os4);
    osinfo_db_add_os(db, os5);

    osinfo_product_add_related(OSINFO_PRODUCT(os1), OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, OSINFO_PRODUCT(os2));
    osinfo_product_add_related(OSINFO_PRODUCT(os1), OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, OSINFO_PRODUCT(os3));
    osinfo_product_add_related(OSINFO_PRODUCT(os2), OSINFO_PRODUCT_RELATIONSHIP_CLONES, OSINFO_PRODUCT(os4));
    osinfo_product_add_related(OSINFO_PRODUCT(os3), OSINFO_PRODUCT_RELATIONSHIP_UPGRADES, OSINFO_PRODUCT(os5));

    OsinfoOsList *sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
    gboolean hasOs1 = FALSE;
    gboolean hasOs2 = FALSE;
    gboolean hasOs3 = FALSE;
    gboolean hasOs4 = FALSE;
    gboolean hasOs5 = FALSE;
    gboolean hasBad = FALSE;
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(sublist)); i++) {
        OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(sublist), i));

        if (ent == os1)
            hasOs1 = TRUE;
        else if (ent == os2)
            hasOs2 = TRUE;
        else if (ent == os3)
            hasOs3 = TRUE;
        else if (ent == os4)
            hasOs4 = TRUE;
        else if (ent == os5)
            hasOs5 = TRUE;
        else
            hasBad = TRUE;

    }
    g_assert_false(hasOs1);
    g_assert_true(hasOs2);
    g_assert_true(hasOs3);
    g_assert_false(hasOs4);
    g_assert_false(hasOs5);
    g_assert_false(hasBad);

    g_object_unref(sublist);

    sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_PRODUCT_RELATIONSHIP_UPGRADES);
    hasOs1 = hasOs2 = hasOs3 = hasOs4 = hasOs5 = hasBad = FALSE;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(sublist)); i++) {
        OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(sublist), i));

        if (ent == os1)
            hasOs1 = TRUE;
        else if (ent == os2)
            hasOs2 = TRUE;
        else if (ent == os3)
            hasOs3 = TRUE;
        else if (ent == os4)
            hasOs4 = TRUE;
        else if (ent == os5)
            hasOs5 = TRUE;
        else
            hasBad = TRUE;

    }
    g_assert_false(hasOs1);
    g_assert_false(hasOs2);
    g_assert_false(hasOs3);
    g_assert_false(hasOs4);
    g_assert_true(hasOs5);
    g_assert_false(hasBad);

    g_object_unref(sublist);

    sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_PRODUCT_RELATIONSHIP_CLONES);
    hasOs1 = hasOs2 = hasOs3 = hasOs4 = hasOs5 = hasBad = FALSE;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(sublist)); i++) {
        OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(sublist), i));

        if (ent == os1)
            hasOs1 = TRUE;
        else if (ent == os2)
            hasOs2 = TRUE;
        else if (ent == os3)
            hasOs3 = TRUE;
        else if (ent == os4)
            hasOs4 = TRUE;
        else if (ent == os5)
            hasOs5 = TRUE;
        else
            hasBad = TRUE;

    }
    g_assert_false(hasOs1);
    g_assert_false(hasOs2);
    g_assert_false(hasOs3);
    g_assert_true(hasOs4);
    g_assert_false(hasOs5);
    g_assert_false(hasBad);

    g_object_unref(sublist);


    g_object_unref(os1);
    g_object_unref(os2);
    g_object_unref(os3);
    g_object_unref(os4);
    g_object_unref(os5);
    g_object_unref(db);
}




int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/db/basic", test_basic);
    g_test_add_func("/db/device", test_device);
    g_test_add_func("/db/platform", test_platform);
    g_test_add_func("/db/os", test_os);
    g_test_add_func("/db/prop_device", test_prop_device);
    g_test_add_func("/db/prop_platform", test_prop_platform);
    g_test_add_func("/db/prop_os", test_prop_os);
    g_test_add_func("/db/rel_os", test_rel_os);

    /* Upfront so we don't confuse valgrind */
    osinfo_entity_get_type();
    osinfo_db_get_type();
    osinfo_device_get_type();
    osinfo_platform_get_type();
    osinfo_os_get_type();
    osinfo_list_get_type();
    osinfo_devicelist_get_type();
    osinfo_platformlist_get_type();
    osinfo_oslist_get_type();
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
