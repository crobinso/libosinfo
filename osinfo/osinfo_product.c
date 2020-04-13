/*
 * osinfo:
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPPRODUCTE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <osinfo/osinfo.h>

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

#include "osinfo/osinfo_product_private.h"

/**
 * SECTION:osinfo_product
 * @short_description: An software product
 * @see_also: #OsinfoProduct, #OsinfoPlatform
 *
 * #OsinfoProduct is an entity representing an software
 * product. There are relationships amongst products
 * to declare which are newest releases, which are clones
 * and which are derived from a common ancestry.
 */

/**
 * OsinfoProductRelationship:
 * @OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM: a descendent (RHEL-5 derives from Fedora-6)
 * @OSINFO_PRODUCT_RELATIONSHIP_CLONES: a perfect clone (CentOS-5 clones RHEL-5)
 * @OSINFO_PRODUCT_RELATIONSHIP_UPGRADES: a new version release (RHEL-6 upgrades RHEL-4)
 *
 * Enum values used to form relationships between products
 */
struct _OsinfoProductPrivate
{
    // Value: Array of product_link structs
    GList *productLinks;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(OsinfoProduct, osinfo_product, OSINFO_TYPE_ENTITY);

struct _OsinfoProductProductLink {
    /* <product> 'verbs' <other_product>
     * fedora11 upgrades fedora10
     * centproduct clones rhel
     * scientificlinux derives from rhel
     */
    OsinfoProductRelationship relshp;
    OsinfoProduct *otherProduct;
};

enum {
    PROP_0,

    PROP_NAME,
    PROP_SHORT_ID,
    PROP_VENDOR,
    PROP_VERSION,
    PROP_CODENAME,
    PROP_LOGO,

    LAST_PROP
};
static GParamSpec *properties[LAST_PROP];

static void osinfo_product_link_free(gpointer data)
{
    struct _OsinfoProductProductLink *prodlink = data;
    g_object_unref(prodlink->otherProduct);
    g_free(prodlink);
}


static void
osinfo_product_finalize(GObject *object)
{
    OsinfoProduct *product = OSINFO_PRODUCT(object);

    g_list_free_full(product->priv->productLinks, osinfo_product_link_free);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_product_parent_class)->finalize(object);
}

static void
osinfo_product_get_property(GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    OsinfoProduct *product = OSINFO_PRODUCT(object);

    switch (property_id) {
    case PROP_NAME:
        g_value_set_string(value,
                           osinfo_product_get_name(product));
        break;

    case PROP_SHORT_ID:
        g_value_set_string(value,
                           osinfo_product_get_short_id(product));
        break;

    case PROP_VENDOR:
        g_value_set_string(value,
                           osinfo_product_get_vendor(product));
        break;

    case PROP_VERSION:
        g_value_set_string(value,
                           osinfo_product_get_version(product));
        break;

    case PROP_CODENAME:
        g_value_set_string(value,
                           osinfo_product_get_codename(product));
        break;

    case PROP_LOGO:
        g_value_set_string(value,
                           osinfo_product_get_logo(product));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

/* Init functions */
static void
osinfo_product_class_init(OsinfoProductClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->get_property = osinfo_product_get_property;
    g_klass->finalize = osinfo_product_finalize;

    /**
     * OsinfoProduct:name:
     *
     * The name of this product.
     */
    properties[PROP_NAME] = g_param_spec_string("name",
                                                "Name",
                                                _("Name"),
                                                NULL /* default value */,
                                                G_PARAM_READABLE |
                                                G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoProduct:short-id:
     *
     * The short ID of this product.
     */
    properties[PROP_SHORT_ID] = g_param_spec_string("short-id",
                                                    "ShortID",
                                                    _("Short ID"),
                                                    NULL /* default value */,
                                                    G_PARAM_READABLE |
                                                    G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoProduct:vendor:
     *
     * The Vendor of this product.
     */
    properties[PROP_VENDOR] = g_param_spec_string("vendor",
                                                  "Vendor",
                                                  _("Vendor"),
                                                  NULL /* default value */,
                                                  G_PARAM_READABLE |
                                                  G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoProduct:version:
     *
     * The version of the product.
     */
    properties[PROP_VERSION] = g_param_spec_string("version",
                                                   "Version",
                                                   _("Version"),
                                                   NULL /* default value */,
                                                   G_PARAM_READABLE |
                                                   G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoProduct:codename:
     *
     * The codename of this product.
     */
    properties[PROP_CODENAME] = g_param_spec_string("codename",
                                                    "Codename",
                                                    _("Codename"),
                                                    NULL /* default value */,
                                                    G_PARAM_READABLE |
                                                    G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoProduct:logo:
     *
     * The URI of the logo of the product.
     */
    properties[PROP_LOGO] = g_param_spec_string("logo",
                                                "Logo",
                                                _("URI of the logo"),
                                                NULL /* default value */,
                                                G_PARAM_READABLE |
                                                G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(g_klass, LAST_PROP, properties);
}

static void
osinfo_product_init(OsinfoProduct *product)
{
    product->priv = osinfo_product_get_instance_private(product);
    product->priv->productLinks = NULL;
}


/**
 * osinfo_product_get_related:
 * @product: an product
 * @relshp: the relationship to query
 *
 * Get a list of products satisfying the requested
 * relationship
 *
 * Returns: (transfer full): a list of related products
 */
OsinfoProductList *osinfo_product_get_related(OsinfoProduct *product, OsinfoProductRelationship relshp)
{
    OsinfoProductList *newList;
    GList *tmp;

    g_return_val_if_fail(OSINFO_IS_PRODUCT(product), NULL);

    // Create our list
    newList = osinfo_productlist_new();
    tmp = product->priv->productLinks;

    while (tmp) {
        struct _OsinfoProductProductLink *prodlink = tmp->data;

        if (prodlink->relshp == relshp)
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(prodlink->otherProduct));

        tmp = tmp->next;
    }

    return newList;
}


/**
 * osinfo_product_add_related:
 * @product: a product
 * @relshp: the relationship
 * @otherproduct: (transfer none): the product to relate to
 *
 * Add an association between two products
 */
void osinfo_product_add_related(OsinfoProduct *product, OsinfoProductRelationship relshp, OsinfoProduct *otherproduct)
{
    struct _OsinfoProductProductLink *productLink;

    g_return_if_fail(OSINFO_IS_PRODUCT(product));
    g_return_if_fail(OSINFO_IS_PRODUCT(otherproduct));

    productLink = g_new0(struct _OsinfoProductProductLink, 1);

    g_object_ref(otherproduct);
    productLink->otherProduct = otherproduct;
    productLink->relshp = relshp;

    product->priv->productLinks = g_list_prepend(product->priv->productLinks, productLink);
}

const gchar *osinfo_product_get_vendor(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_VENDOR);
}
const gchar *osinfo_product_get_version(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_VERSION);
}
const gchar *osinfo_product_get_short_id(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_SHORT_ID);
}
const gchar *osinfo_product_get_name(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_NAME);
}

/**
 * osinfo_product_get_codename:
 * @product: an product
 *
 * Returns: The product's codename, or NULL.
 */
const gchar *osinfo_product_get_codename(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_CODENAME);
}

/**
 * osinfo_product_get_release_date_string:
 * @product: an product
 *
 * Returns: A string representing the product's release date, or NULL.
 *
 * Since: 0.2.8
 */
const gchar *osinfo_product_get_release_date_string(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_RELEASE_DATE);
}

/**
 * osinfo_product_get_eol_date_string:
 * @product: an product
 *
 * Returns: A string representing the product's EOL date, or NULL.
 *
 * Since: 0.2.8
 */
const gchar *osinfo_product_get_eol_date_string(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_EOL_DATE);
}


static GDate *date_from_string(const gchar *str)
{
    int y, m, d;
    const gchar *tmp;

    y = strtoll(str, NULL, 10);
    tmp = strchr(str, '-');
    m = strtoll(tmp+1, NULL, 10);
    tmp = strchr(tmp+1, '-');
    d = strtoll(tmp+1, NULL, 10);
    return g_date_new_dmy(d, m, y);
}

/**
 * osinfo_product_get_release_date:
 * @product: an product
 *
 * Returns: (transfer full): A #GDate representing the product's release date,
 *                           or NULL.
 *
 * Since: 0.2.8
 */
GDate *osinfo_product_get_release_date(OsinfoProduct *product)
{
    const gchar *str = osinfo_product_get_release_date_string(product);
    if (!str)
        return NULL;

    return date_from_string(str);
}


/**
 * osinfo_product_get_eol_date:
 * @product: an product
 *
 * Returns: (transfer full): A #GDate representing the product's EOL date,
 *                           or NULL.
 *
 * Since: 0.2.8
 */
GDate *osinfo_product_get_eol_date(OsinfoProduct *product)
{
    const gchar *str = osinfo_product_get_eol_date_string(product);
    if (!str)
        return NULL;

    return date_from_string(str);
}

const gchar *osinfo_product_get_logo(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(product), OSINFO_PRODUCT_PROP_LOGO);
}

static OsinfoList *osinfo_list_append(OsinfoList *appendee,
                                      OsinfoList *appended)
{
    OsinfoList *result;
    result = osinfo_list_new_union(appendee, appended);
    g_object_unref(G_OBJECT(appendee));

    return result;
}

/**
 * osinfo_product_foreach_related:
 * @product: a product
 * @flags: bitwise-or of #OsinfoProductForeachFlag
 * @foreach_func: the function to call for each related product
 * @user_data: user data to pass to foreach_func()
 *
 * Call @foreach_func for @product and for each #OsinfoProduct related
 * to @product. The meaning of 'related' is defined by the @flag parameter,
 * and can be products @product derives from or products which @product
 * upgrades or clones, or a combination of these, or none.
 */
void osinfo_product_foreach_related(OsinfoProduct *product,
                                    unsigned int flags,
                                    OsinfoProductForeach foreach_func,
                                    gpointer user_data)
{
    OsinfoList *related_list;
    OsinfoProductList *tmp_related;
    guint i;

    foreach_func(product, user_data);

    related_list = OSINFO_LIST(osinfo_productlist_new());
    if (flags & OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM) {
        tmp_related = osinfo_product_get_related
                      (product, OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM);
        related_list = osinfo_list_append(related_list,
                                          OSINFO_LIST(tmp_related));
        g_object_unref(G_OBJECT(tmp_related));
    }

    if (flags & OSINFO_PRODUCT_FOREACH_FLAG_UPGRADES) {
        tmp_related = osinfo_product_get_related
                      (product, OSINFO_PRODUCT_RELATIONSHIP_UPGRADES);
        related_list = osinfo_list_append(related_list,
                                          OSINFO_LIST(tmp_related));
        g_object_unref(G_OBJECT(tmp_related));
    }

    if (flags & OSINFO_PRODUCT_FOREACH_FLAG_CLONES) {
        tmp_related = osinfo_product_get_related
                      (product, OSINFO_PRODUCT_RELATIONSHIP_CLONES);
        related_list = osinfo_list_append(related_list,
                                          OSINFO_LIST(tmp_related));
        g_object_unref(G_OBJECT(tmp_related));
    }

    for (i = 0; i < osinfo_list_get_length(related_list); i++) {
        OsinfoEntity *related;

        related = osinfo_list_get_nth(related_list, i);
        osinfo_product_foreach_related(OSINFO_PRODUCT(related),
                                       flags,
                                       foreach_func,
                                       user_data);
    }
    g_object_unref(related_list);
}

/**
 * osinfo_product_get_short_id_list:
 * @product: an #OsinfoProduct
 *
 * Retrieve all the short-ids associated with the product.
 *
 * Returns: (transfer container) (element-type utf8): the list of short-ids.
 *
 * Since: 1.5.0
 */
GList *osinfo_product_get_short_id_list(OsinfoProduct *product)
{
    return osinfo_entity_get_param_value_list(OSINFO_ENTITY(product),
                                              OSINFO_PRODUCT_PROP_SHORT_ID);
}
