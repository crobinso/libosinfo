/*
 * libosinfo:
 *
 * Copyright (C) 2009-2020 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <osinfo/osinfo.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_productfilter
 * @short_description: an operating system filter
 * @see_also: #OsinfoFilter, #OsinfoProduct
 *
 * #OsinfoProductFilter is a specialization of #OsinfoFilter that
 * can also set constraints against operating system
 * relationships. It can only be used to filter entities
 * that are #OsinfoProduct objects.
 */

struct _OsinfoProductFilterPrivate
{
    // Key: relationship type
    // Value: GList of OsinfoProduct *
    // Note: Only used when productfiltering OsinfoProduct objects
    GHashTable *productConstraints;

    GDate *supportDate;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoProductFilter, osinfo_productfilter, OSINFO_TYPE_FILTER);

static void osinfo_productfilter_finalize(GObject *object);
static gboolean osinfo_productfilter_matches_default(OsinfoFilter *productfilter, OsinfoEntity *entity);

static void
osinfo_productfilter_finalize(GObject *object)
{
    OsinfoProductFilter *productfilter = OSINFO_PRODUCTFILTER(object);

    g_hash_table_unref(productfilter->priv->productConstraints);

    if (productfilter->priv->supportDate)
        g_date_free(productfilter->priv->supportDate);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_productfilter_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_productfilter_class_init(OsinfoProductFilterClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);
    OsinfoFilterClass *filter_klass = OSINFO_FILTER_CLASS(klass);

    g_klass->finalize = osinfo_productfilter_finalize;

    filter_klass->matches = osinfo_productfilter_matches_default;
}


/**
 * osinfo_productfilter_new:
 *
 * Construct a new filter that matches all operating
 * systems
 *
 * Returns: (transfer full): a new filter
 */
OsinfoProductFilter *osinfo_productfilter_new(void)
{
    return g_object_new(OSINFO_TYPE_PRODUCTFILTER, NULL);
}


static void
osinfo_productfilter_product_constraints_free(gpointer relshps)
{
    g_list_free_full(relshps, g_object_unref);
}

static void
osinfo_productfilter_init(OsinfoProductFilter *productfilter)
{
    productfilter->priv = osinfo_productfilter_get_instance_private(productfilter);
    productfilter->priv->productConstraints =
        g_hash_table_new_full(g_direct_hash,
                              g_direct_equal,
                              NULL,
                              osinfo_productfilter_product_constraints_free);
}


/**
 * osinfo_productfilter_add_product_constraint:
 * @productfilter: a filter object
 * @relshp: the relationship to filter on
 * @product: (transfer none): the target product to filter on
 *
 * Adds a constraint that matches products which
 * have a relationship @relshp with @product. Multiple constraints
 * can be set for the same @relshp or @product, in which case
 * all must match
 */
gint osinfo_productfilter_add_product_constraint(OsinfoProductFilter *productfilter, OsinfoProductRelationship relshp, OsinfoProduct *product)
{
    GList *values = NULL;
    gpointer origKey, foundValue;
    gboolean found;

    g_return_val_if_fail(OSINFO_IS_PRODUCTFILTER(productfilter), -1);
    g_return_val_if_fail(OSINFO_IS_PRODUCT(product), -1);

    // First check if there exists an array of entries for this key
    // If not, create a ptrarray of strings for this key and insert into map
    found = g_hash_table_lookup_extended(productfilter->priv->productConstraints, GINT_TO_POINTER(relshp), &origKey, &foundValue);
    if (found) {
        values = foundValue;
        g_hash_table_steal(productfilter->priv->productConstraints, GINT_TO_POINTER(relshp));
    }
    g_object_ref(product);
    values = g_list_prepend(values, product);
    g_hash_table_insert(productfilter->priv->productConstraints, GINT_TO_POINTER(relshp), values);

    return 0;
}

/**
 * osinfo_productfilter_clear_product_constraint:
 * @productfilter: a filter object
 * @relshp: the relationship to clear
 *
 * Remove all constraints for the relationship @relshp
 */
void osinfo_productfilter_clear_product_constraint(OsinfoProductFilter *productfilter, OsinfoProductRelationship relshp)
{
    g_return_if_fail(OSINFO_IS_PRODUCTFILTER(productfilter));

    g_hash_table_remove(productfilter->priv->productConstraints, (gpointer) relshp);
}


/**
 * osinfo_productfilter_clear_product_constraints:
 * @productfilter: a filter object
 *
 * Remove all relationship constraints
 */
void osinfo_productfilter_clear_product_constraints(OsinfoProductFilter *productfilter)
{
    g_return_if_fail(OSINFO_IS_PRODUCTFILTER(productfilter));

    g_hash_table_remove_all(productfilter->priv->productConstraints);
}


/**
 * osinfo_productfilter_get_product_constraint_values:
 * @productfilter: a filter object
 * @relshp: a relationship to query
 *
 * Retrieve a list of all operating systems that are
 * the target of constraint for the  relationship
 * @relshp.
 *
 * Returns: (transfer container) (element-type OsinfoProduct): a list of operating systems
 */
GList *osinfo_productfilter_get_product_constraint_values(OsinfoProductFilter *productfilter, OsinfoProductRelationship relshp)
{
    GList *values;

    g_return_val_if_fail(OSINFO_IS_PRODUCTFILTER(productfilter), NULL);

    values = g_hash_table_lookup(productfilter->priv->productConstraints, GINT_TO_POINTER(relshp));

    return g_list_copy(values);
}


void osinfo_productfilter_add_support_date_constraint(OsinfoProductFilter *productfilter, GDate *when)
{
    g_return_if_fail(OSINFO_IS_PRODUCTFILTER(productfilter));

    if (productfilter->priv->supportDate)
        g_date_free(productfilter->priv->supportDate);
    productfilter->priv->supportDate = NULL;
    if (when) {
        productfilter->priv->supportDate = g_date_new_dmy(g_date_get_day(when),
                                                          g_date_get_month(when),
                                                          g_date_get_year(when));
    }
}



struct osinfo_productfilter_match_args {
    OsinfoProductFilter *productfilter;
    OsinfoEntity *entity;
    gboolean matched;
};


static void osinfo_productfilter_match_product_iterator(gpointer key, gpointer value, gpointer data)
{
    struct osinfo_productfilter_match_args *args = data;
    OsinfoProduct *product = OSINFO_PRODUCT(args->entity);
    OsinfoProductRelationship relshp = GPOINTER_TO_INT(key);
    GList *relProducts = value;
    OsinfoProductList *productlist = osinfo_product_get_related(product, relshp);
    gboolean ret = TRUE;

    if (relProducts && osinfo_list_get_length(OSINFO_LIST(productlist)) == 0) {
        ret = FALSE;
        goto cleanup;
    }

    while (relProducts) {
        OsinfoProduct *currProduct = relProducts->data;
        int i;
        gboolean found = FALSE;
        for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(productlist)); i++) {
            OsinfoProduct *testProduct = OSINFO_PRODUCT(osinfo_list_get_nth(OSINFO_LIST(productlist), i));
            if (testProduct == currProduct) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            ret = FALSE;
            goto cleanup;
        }

        relProducts = relProducts->next;
    }

 cleanup:
    g_object_unref(productlist);
    args->matched = ret;
}

static gboolean osinfo_productfilter_matches_default(OsinfoFilter *filter, OsinfoEntity *entity)
{
    OsinfoProductFilter *productfilter;
    struct osinfo_productfilter_match_args args;

    g_return_val_if_fail(OSINFO_IS_PRODUCTFILTER(filter), FALSE);
    g_return_val_if_fail(OSINFO_IS_PRODUCT(entity), FALSE);

    if (!OSINFO_FILTER_CLASS(osinfo_productfilter_parent_class)->matches(filter, entity))
        return FALSE;

    productfilter = OSINFO_PRODUCTFILTER(filter);

    args.productfilter = productfilter;
    args.entity = entity;
    args.matched = TRUE;

    g_hash_table_foreach(productfilter->priv->productConstraints,
                         osinfo_productfilter_match_product_iterator,
                         &args);

    if (productfilter->priv->supportDate) {
        GDate *when = productfilter->priv->supportDate;
        GDate *release;
        GDate *eol;

        release = osinfo_product_get_release_date(OSINFO_PRODUCT(entity));
        if (release != NULL) {
            gboolean newer;
            newer = (g_date_compare(release, when) > 0);
            g_date_free(release);
            if (newer)
                return FALSE;
        }

        eol = osinfo_product_get_eol_date(OSINFO_PRODUCT(entity));
        if (eol != NULL) {
            gboolean older;
            older = (g_date_compare(eol, when) < 0);
            g_date_free(eol);
            if (older)
                return FALSE;
        }
    }

    return args.matched;
}
