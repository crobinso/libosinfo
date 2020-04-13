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

/* OsinfoProduct is abstract, so we need to trivially subclass it to test it */
typedef struct _OsinfoDummy        OsinfoDummy;
typedef struct _OsinfoDummyClass   OsinfoDummyClass;

struct _OsinfoDummy
{
    OsinfoProduct parent_instance;
};

struct _OsinfoDummyClass
{
    OsinfoProductClass parent_class;
};

GType osinfo_dummy_get_type(void);

G_DEFINE_TYPE(OsinfoDummy, osinfo_dummy, OSINFO_TYPE_PRODUCT);

static void osinfo_dummy_class_init(OsinfoDummyClass *klass G_GNUC_UNUSED){}
static void osinfo_dummy_init(OsinfoDummy *self G_GNUC_UNUSED) {}

static OsinfoProduct *osinfo_dummy_new(const gchar *id) {
    return g_object_new(osinfo_dummy_get_type(), "id", id, NULL);
}


static void
test_basic(void)
{
    OsinfoProductFilter *productfilter = osinfo_productfilter_new();
    OsinfoProduct *product1 = osinfo_dummy_new("pretty");
    OsinfoProduct *product2 = osinfo_dummy_new("ugly");
    GList *tmp;

    g_assert_true(OSINFO_IS_PRODUCTFILTER(productfilter));
    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product1);
    tmp = osinfo_productfilter_get_product_constraint_values(productfilter,
                                                               OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
    g_assert_nonnull(tmp);
    g_assert_true(tmp->data == product1);
    g_assert_null(tmp->next);
    g_list_free(tmp);

    tmp = osinfo_productfilter_get_product_constraint_values(productfilter,
                                                   OSINFO_PRODUCT_RELATIONSHIP_CLONES);
    g_assert_null(tmp);


    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    tmp = osinfo_productfilter_get_product_constraint_values(productfilter,
                                                   OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
    g_assert_nonnull(tmp);
    g_assert_true(tmp->data == product2);
    g_assert_nonnull(tmp->next);
    g_assert_true(tmp->next->data == product1);
    g_assert_null(tmp->next->next);
    g_list_free(tmp);

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_UPGRADES, product1);
    tmp = osinfo_productfilter_get_product_constraint_values(productfilter,
                                                   OSINFO_PRODUCT_RELATIONSHIP_UPGRADES);
    g_assert_nonnull(tmp);
    g_assert_true(tmp->data == product1);
    g_assert_null(tmp->next);
    g_list_free(tmp);

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_CLONES, product1);
    tmp = osinfo_productfilter_get_product_constraint_values(productfilter,
                                                   OSINFO_PRODUCT_RELATIONSHIP_CLONES);
    g_assert_nonnull(tmp);
    g_assert_true(tmp->data == product1);
    g_assert_null(tmp->next);
    g_list_free(tmp);

    g_object_unref(product2);
    g_object_unref(product1);
    g_object_unref(productfilter);
}


static void
test_productfilter_single(void)
{
    OsinfoProductFilter *productfilter = osinfo_productfilter_new();
    OsinfoProduct *product1 = osinfo_dummy_new("hot");
    OsinfoProduct *product2 = osinfo_dummy_new("or");
    OsinfoProduct *product3 = osinfo_dummy_new("not");
    OsinfoProduct *product4 = osinfo_dummy_new("burger");


    osinfo_product_add_related(product1, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    osinfo_product_add_related(product3, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product4);

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));

    g_object_unref(product1);
    g_object_unref(product2);
    g_object_unref(product3);
    g_object_unref(product4);
    g_object_unref(productfilter);
}


static void
test_productfilter_multi(void)
{
    OsinfoProductFilter *productfilter = osinfo_productfilter_new();
    OsinfoProduct *product1 = osinfo_dummy_new("hot");
    OsinfoProduct *product2 = osinfo_dummy_new("or");
    OsinfoProduct *product3 = osinfo_dummy_new("not");
    OsinfoProduct *product4 = osinfo_dummy_new("burger");
    OsinfoProduct *product5 = osinfo_dummy_new("cat");

    osinfo_product_add_related(product1, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    osinfo_product_add_related(product1, OSINFO_PRODUCT_RELATIONSHIP_CLONES, product5);
    osinfo_product_add_related(product3, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    osinfo_product_add_related(product3, OSINFO_PRODUCT_RELATIONSHIP_CLONES, product4);

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_CLONES, product5);

    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product5)));

    osinfo_productfilter_clear_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_CLONES);

    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product5)));

    osinfo_productfilter_clear_product_constraints(productfilter);

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_UPGRADES, product5);
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product5)));

    g_object_unref(product1);
    g_object_unref(product2);
    g_object_unref(product3);
    g_object_unref(product4);
    g_object_unref(product5);
    g_object_unref(productfilter);
}


static void
test_productfilter_combine(void)
{
    OsinfoProductFilter *productfilter = osinfo_productfilter_new();
    OsinfoProduct *product1 = osinfo_dummy_new("hot");
    OsinfoProduct *product2 = osinfo_dummy_new("or");
    OsinfoProduct *product3 = osinfo_dummy_new("not");
    OsinfoProduct *product4 = osinfo_dummy_new("burger");

    osinfo_product_add_related(product1, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    osinfo_product_add_related(product3, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product4);

    osinfo_entity_add_param(OSINFO_ENTITY(product1), "vendor", "drevil");
    osinfo_entity_add_param(OSINFO_ENTITY(product3), "vendor", "acme");

    osinfo_productfilter_add_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM, product2);
    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));

    osinfo_filter_add_constraint(OSINFO_FILTER(productfilter), "vendor", "acme");
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));

    osinfo_productfilter_clear_product_constraint(productfilter, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product1)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product2)));
    g_assert_true(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product3)));
    g_assert_false(osinfo_filter_matches(OSINFO_FILTER(productfilter), OSINFO_ENTITY(product4)));

    g_object_unref(product1);
    g_object_unref(product2);
    g_object_unref(product3);
    g_object_unref(product4);
    g_object_unref(productfilter);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/productfilter/basic", test_basic);
    g_test_add_func("/productfilter/productfilter_single", test_productfilter_single);
    g_test_add_func("/productfilter/productfilter_multi", test_productfilter_multi);
    g_test_add_func("/productfilter/productfilter_combine", test_productfilter_combine);

    /* Upfront so we don't confuse valgrind */
    osinfo_entity_get_type();
    osinfo_filter_get_type();
    osinfo_list_get_type();
    osinfo_productlist_get_type();
    osinfo_productfilter_get_type();
    osinfo_product_get_type();

    return g_test_run();
}
