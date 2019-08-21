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

#include <osinfo/osinfo.h>


static void
test_basic(void)
{
    OsinfoTree *tree = osinfo_tree_new("foo", "x86_64");

    g_assert_true(OSINFO_IS_TREE(tree));
    g_assert_cmpstr((const char *)osinfo_entity_get_id(OSINFO_ENTITY(tree)), ==, "foo");
    g_assert_cmpstr((const char *)osinfo_tree_get_architecture(tree), ==, "x86_64");

    g_object_unref(tree);
}

static void
test_os_variants(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoTree *tree;
    OsinfoTreeList *list;
    OsinfoOsVariant *variant;
    OsinfoOsVariantList *variant_list;
    gint list_len;
    GError *error = NULL;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    os = osinfo_db_get_os(db, "http://libosinfo.org/test/tree");

    list = osinfo_os_get_tree_list(os);
    list_len = osinfo_list_get_length(OSINFO_LIST(list));
    g_assert_cmpint(list_len, ==, 1);

    tree = OSINFO_TREE(osinfo_list_get_nth(OSINFO_LIST(list), 0));
    variant_list = osinfo_tree_get_os_variants(tree);
    list_len = osinfo_list_get_length(OSINFO_LIST(variant_list));
    g_assert_cmpint(list_len, ==, 1);

    variant = OSINFO_OS_VARIANT(osinfo_list_get_nth(OSINFO_LIST(variant_list), 0));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(variant)), ==, "bar");
    g_assert_cmpstr(osinfo_os_variant_get_name(variant), ==, "Tree Bar");

    g_object_unref(variant_list);
    g_object_unref(list);
    g_object_unref(loader);
}

static void
test_create_from_treeinfo(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoTree *tree;
    GError *error = NULL;
    const gchar *treeinfo = "[general]\n"
                            "arch = ppc64le\n"
                            "family = Tree\n"
                            "version = unknown\n"
                            "[images-ppc64le]\n"
                            "boot.iso = images/boot.iso\n"
                            "efiboot.img = images/efiboot.img\n"
                            "initrd = images/pxeboot/initrd.img\n"
                            "kernel = images/pxeboot/vmlinuz\n"
                            "macboot.img = images/macboot.img\n";

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = osinfo_loader_get_db(loader);

    tree = osinfo_tree_create_from_treeinfo(treeinfo,
                                            "libosinfo.org/test",
                                            &error);
    g_assert_no_error(error);

    g_assert_cmpstr(osinfo_tree_get_treeinfo_arch(tree), ==, "ppc64le");
    g_assert_cmpstr(osinfo_tree_get_treeinfo_family(tree), ==, "Tree");
    g_assert_cmpstr(osinfo_tree_get_treeinfo_version(tree), ==, "unknown");

    g_assert_true(osinfo_db_identify_tree(db, tree));

    os = osinfo_tree_get_os(tree);
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(os)), ==, "http://libosinfo.org/test/tree");
    g_object_unref(os);

    g_object_unref(loader);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/tree/basic", test_basic);
    g_test_add_func("/tree/os-variants", test_os_variants);
    g_test_add_func("/tree/create-from-treeinfo", test_create_from_treeinfo);

    /* Upfront so we don't confuse valgrind */
    osinfo_tree_get_type();
    osinfo_treelist_get_type();
    osinfo_os_variantlist_get_type();
    osinfo_loader_get_type();
    osinfo_db_get_type();
    osinfo_os_get_type();

    return g_test_run();
}
