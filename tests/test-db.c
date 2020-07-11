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
    OsinfoDeviceList *list;
    OsinfoDevice *dev;

    osinfo_db_add_device(db, dev1);
    osinfo_db_add_device(db, dev2);
    osinfo_db_add_device(db, dev3);

    list = osinfo_db_get_device_list(db);
    g_assert_true(OSINFO_ENTITY(dev1) == osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(OSINFO_ENTITY(dev2) == osinfo_list_get_nth(OSINFO_LIST(list), 1));
    g_assert_true(OSINFO_ENTITY(dev3) == osinfo_list_get_nth(OSINFO_LIST(list), 2));
    g_object_unref(list);

    dev = osinfo_db_get_device(db, "dev2");
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
    OsinfoPlatformList *list;
    OsinfoPlatform *hv;

    osinfo_db_add_platform(db, hv1);
    osinfo_db_add_platform(db, hv2);
    osinfo_db_add_platform(db, hv3);

    list = osinfo_db_get_platform_list(db);
    g_assert_true(OSINFO_ENTITY(hv1) == osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(OSINFO_ENTITY(hv2) == osinfo_list_get_nth(OSINFO_LIST(list), 1));
    g_assert_true(OSINFO_ENTITY(hv3) == osinfo_list_get_nth(OSINFO_LIST(list), 2));
    g_object_unref(list);

    hv = osinfo_db_get_platform(db, "hv2");
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
    OsinfoOsList *list;
    OsinfoOs *os;

    osinfo_db_add_os(db, os1);
    osinfo_db_add_os(db, os2);
    osinfo_db_add_os(db, os3);

    list = osinfo_db_get_os_list(db);
    g_assert_true(OSINFO_ENTITY(os1) == osinfo_list_get_nth(OSINFO_LIST(list), 0));
    g_assert_true(OSINFO_ENTITY(os2) == osinfo_list_get_nth(OSINFO_LIST(list), 1));
    g_assert_true(OSINFO_ENTITY(os3) == osinfo_list_get_nth(OSINFO_LIST(list), 2));
    g_object_unref(list);

    os = osinfo_db_get_os(db, "os2");
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
    GList *uniq;
    GList *tmp;
    gboolean hasNetwork;
    gboolean hasAudio;
    gboolean hasInput;
    gboolean hasDisplay;
    gboolean hasBad;

    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");
    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "audio");
    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "input");
    osinfo_entity_add_param(OSINFO_ENTITY(dev3), "class", "display");
    osinfo_entity_add_param(OSINFO_ENTITY(dev3), "fruit", "apple");

    osinfo_db_add_device(db, dev1);
    osinfo_db_add_device(db, dev2);
    osinfo_db_add_device(db, dev3);

    uniq = osinfo_db_unique_values_for_property_in_device(db, "class");
    tmp = uniq;
    hasNetwork = FALSE;
    hasAudio = FALSE;
    hasInput = FALSE;
    hasDisplay = FALSE;
    hasBad = FALSE;
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
    GList *uniq;
    GList *tmp;
    gboolean hasAcme;
    gboolean hasFrog;
    gboolean hasDog;
    gboolean hasBad;

    osinfo_entity_add_param(OSINFO_ENTITY(hv1), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(hv1), "vendor", "frog");
    osinfo_entity_add_param(OSINFO_ENTITY(hv2), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(hv3), "vendor", "dog");
    osinfo_entity_add_param(OSINFO_ENTITY(hv3), "arch", "x86");

    osinfo_db_add_platform(db, hv1);
    osinfo_db_add_platform(db, hv2);
    osinfo_db_add_platform(db, hv3);

    uniq = osinfo_db_unique_values_for_property_in_platform(db, "vendor");
    tmp = uniq;
    hasAcme = FALSE;
    hasFrog = FALSE;
    hasDog = FALSE;
    hasBad = FALSE;
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
    GList *uniq;
    GList *tmp;
    gboolean hasAcme;
    gboolean hasFrog;
    gboolean hasDog;
    gboolean hasBad;

    osinfo_entity_add_param(OSINFO_ENTITY(os1), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(os1), "vendor", "frog");
    osinfo_entity_add_param(OSINFO_ENTITY(os2), "vendor", "acme");
    osinfo_entity_add_param(OSINFO_ENTITY(os3), "vendor", "dog");
    osinfo_entity_add_param(OSINFO_ENTITY(os3), "arch", "x86");

    osinfo_db_add_os(db, os1);
    osinfo_db_add_os(db, os2);
    osinfo_db_add_os(db, os3);

    uniq = osinfo_db_unique_values_for_property_in_os(db, "vendor");
    tmp = uniq;
    hasAcme = FALSE;
    hasFrog = FALSE;
    hasDog = FALSE;
    hasBad = FALSE;
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
    OsinfoOsList *sublist;
    gboolean hasOs1;
    gboolean hasOs2;
    gboolean hasOs3;
    gboolean hasOs4;
    gboolean hasOs5;
    gboolean hasBad;

    osinfo_db_add_os(db, os1);
    osinfo_db_add_os(db, os2);
    osinfo_db_add_os(db, os3);
    osinfo_db_add_os(db, os4);
    osinfo_db_add_os(db, os5);

    osinfo_product_add_related(OSINFO_PRODUCT(os1), OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, OSINFO_PRODUCT(os2));
    osinfo_product_add_related(OSINFO_PRODUCT(os1), OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, OSINFO_PRODUCT(os3));
    osinfo_product_add_related(OSINFO_PRODUCT(os2), OSINFO_PRODUCT_RELATIONSHIP_CLONES, OSINFO_PRODUCT(os4));
    osinfo_product_add_related(OSINFO_PRODUCT(os3), OSINFO_PRODUCT_RELATIONSHIP_UPGRADES, OSINFO_PRODUCT(os5));

    sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
    hasOs1 = FALSE;
    hasOs2 = FALSE;
    hasOs3 = FALSE;
    hasOs4 = FALSE;
    hasOs5 = FALSE;
    hasBad = FALSE;
    for (int i = 0; i < osinfo_list_get_length(OSINFO_LIST(sublist)); i++) {
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
    for (int i = 0; i < osinfo_list_get_length(OSINFO_LIST(sublist)); i++) {
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
    for (int i = 0; i < osinfo_list_get_length(OSINFO_LIST(sublist)); i++) {
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



static void
test_identify_media(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoMedia *media;
    OsinfoInstallScriptList *scripts;
    OsinfoOs *os;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    media = osinfo_media_new("foo", "ppc64le");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_VOLUME_ID,
                            "DB Media");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_SYSTEM_ID,
                            "LINUX");

    g_assert_true(osinfo_db_identify_media(db, media));
    g_assert_cmpstr(osinfo_media_get_architecture(media), ==, "ppc64le");
    g_assert_true(osinfo_media_get_live(media));
    g_assert_true(osinfo_media_get_installer(media));
    g_assert_false(osinfo_media_supports_installer_script(media));
    g_assert_cmpint(osinfo_media_get_installer_reboots(media), ==, 6);
    g_assert_false(osinfo_media_get_eject_after_install(media));
    g_assert_cmpstr(osinfo_media_get_kernel_path(media), ==, "isolinux/vmlinuz");
    g_assert_cmpstr(osinfo_media_get_initrd_path(media), ==, "isolinux/initrd.img");
    scripts = osinfo_media_get_install_script_list(media);
    g_assert_nonnull(scripts);
    g_assert_cmpint(osinfo_list_get_length(OSINFO_LIST(scripts)), ==, 1);
    os = osinfo_media_get_os(media);
    g_assert_nonnull(os);
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(os)), ==, "http://libosinfo.org/test/db/media");
    g_object_unref(scripts);
    g_object_unref(media);

    media = osinfo_media_new("foo", "ppc64le");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_VOLUME_ID,
                            "Media DB");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_SYSTEM_ID,
                            "LINUX");

    g_assert_false(osinfo_db_identify_media(db, media));
    g_object_unref(media);

    /* Matching against an "all" architecture */
    media = osinfo_media_new("foo", "x86_64");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_VOLUME_ID,
                            "bootimg");
    g_assert_true(osinfo_db_identify_media(db, media));
    g_assert_cmpstr(osinfo_media_get_architecture(media), ==, "all");
    g_object_unref(media);

    /* Matching against a known architecture, which has to have precedence */
    media = osinfo_media_new("foo", "i686");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_VOLUME_ID,
                            "bootimg");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_SYSTEM_ID,
                            "LINUX");
    g_assert_true(osinfo_db_identify_media(db, media));
    g_assert_cmpstr(osinfo_media_get_architecture(media), ==, "i686");
    g_object_unref(media);

    /* Matching against a versioned ISO, which has to have precedence against the rolling */
    media = osinfo_media_new("foo", "x86_64");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_VOLUME_ID,
                            "ROLLING_VERSIONED");
    g_assert_true(osinfo_db_identify_media(db, media));
    os = osinfo_media_get_os(media);
    g_assert_nonnull(os);
    g_assert_cmpstr(osinfo_product_get_short_id(OSINFO_PRODUCT(os)), ==, "versioned");

    g_object_unref(loader);
}


static OsinfoTree *
create_tree(const gchar *arch, gboolean set_treeinfo_arch)
{
    OsinfoTree *tree;

    tree = osinfo_tree_new("foo", arch);
    osinfo_entity_set_param(OSINFO_ENTITY(tree),
                            OSINFO_TREE_PROP_TREEINFO_FAMILY,
                            "Tree");
    osinfo_entity_set_param(OSINFO_ENTITY(tree),
                            OSINFO_TREE_PROP_TREEINFO_VERSION,
                            "unknown");
    if (set_treeinfo_arch)
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                 OSINFO_TREE_PROP_TREEINFO_ARCH,
                                 arch);

    return tree;
}


static void
test_identify_tree(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoTree *tree;

    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    /* Matching against an "all" architecture" */
    tree = create_tree("x86_64", FALSE);
    g_assert_true(osinfo_db_identify_tree(db, tree));
    g_assert_cmpstr(osinfo_tree_get_architecture(tree), ==, "all");
    g_object_unref(tree);

    /* Matching against a known architecture, which has to have precedence */
    tree = create_tree("i686", TRUE);
    g_assert_true(osinfo_db_identify_tree(db, tree));
    g_assert_cmpstr(osinfo_tree_get_architecture(tree), ==, "i686");
    g_object_unref(tree);

    g_object_unref(loader);
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
    g_test_add_func("/db/identify_media", test_identify_media);
    g_test_add_func("/db/identify_tree", test_identify_tree);

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
    osinfo_loader_get_type();
    osinfo_install_script_get_type();
    osinfo_install_scriptlist_get_type();
    osinfo_tree_get_type();
    osinfo_media_get_type();

    return g_test_run();
}
