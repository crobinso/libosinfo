/*
 * libosinfo: An installed image of a (guest) OS
 *
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_image
 * @short_description: A pre-installed image for a (guest) OS
 * @see_also: #OsinfoOs
 *
 * #OsinfoImage is an entity representing an installation image
 * a (guest) operating system.
 */

struct _OsinfoImagePrivate
{
    GWeakRef os;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoImage, osinfo_image, OSINFO_TYPE_ENTITY);

enum {
    PROP_0,

    PROP_ARCHITECTURE,
    PROP_FORMAT,
    PROP_URL,
    PROP_CLOUD_INIT,
};

static void
osinfo_image_get_property(GObject *object,
                         guint property_id,
                         GValue *value,
                         GParamSpec *pspec)
{
    OsinfoImage *image = OSINFO_IMAGE(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        g_value_set_string(value, osinfo_image_get_architecture(image));
        break;

    case PROP_FORMAT:
        g_value_set_string(value, osinfo_image_get_format(image));
        break;

    case PROP_URL:
        g_value_set_string(value, osinfo_image_get_url(image));
        break;

    case PROP_CLOUD_INIT:
        g_value_set_boolean(value, osinfo_image_get_cloud_init(image));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_image_set_property(GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    OsinfoImage *image = OSINFO_IMAGE(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        osinfo_entity_set_param(OSINFO_ENTITY(image),
                                OSINFO_IMAGE_PROP_ARCHITECTURE,
                                g_value_get_string(value));
        break;

    case PROP_FORMAT:
        osinfo_entity_set_param(OSINFO_ENTITY(image),
                                OSINFO_IMAGE_PROP_FORMAT,
                                g_value_get_string(value));
        break;

    case PROP_URL:
        osinfo_entity_set_param(OSINFO_ENTITY(image),
                                OSINFO_IMAGE_PROP_URL,
                                g_value_get_string(value));
        break;

    case PROP_CLOUD_INIT:
        osinfo_entity_set_param_boolean(OSINFO_ENTITY(image),
                                        OSINFO_IMAGE_PROP_CLOUD_INIT,
                                        g_value_get_boolean(value));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void osinfo_image_dispose(GObject *obj)
{
    OsinfoImage *image = OSINFO_IMAGE(obj);

    g_weak_ref_clear(&image->priv->os);

    G_OBJECT_CLASS(osinfo_image_parent_class)->dispose(obj);
}

static void
osinfo_image_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_image_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_image_class_init(OsinfoImageClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);
    GParamSpec *pspec;

    g_klass->dispose = osinfo_image_dispose;
    g_klass->finalize = osinfo_image_finalize;
    g_klass->get_property = osinfo_image_get_property;
    g_klass->set_property = osinfo_image_set_property;

    /**
     * OsinfoImage:architecture:
     *
     * The target hardware architecture of this image.
     */
    pspec = g_param_spec_string("architecture",
                                "ARCHITECTURE",
                                _("CPU Architecture"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_ARCHITECTURE, pspec);

    /**
     * OsinfoImage:format:
     *
     * The image format.
     */
    pspec = g_param_spec_string("format",
                                "FORMAT",
                                _("The image format"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_FORMAT, pspec);

    /**
     * OsinfoImage:url:
     *
     * The URL to this image.
     */
    pspec = g_param_spec_string("url",
                                "URL",
                                _("The URL to this image"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_URL, pspec);

    /**
     * OsinfoImage:cloud-init:
     *
     * Whether the image supports cloud-init customizations or not.
     */
    pspec = g_param_spec_string("cloud-init",
                                "CloudInit",
                                _("Whether cloud-init customizations are supported or not"),
                                FALSE /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_CLOUD_INIT, pspec);
}

static void
osinfo_image_init(OsinfoImage *image)
{
    image->priv = osinfo_image_get_instance_private(image);

    g_weak_ref_init(&image->priv->os, NULL);
}

OsinfoImage *osinfo_image_new(const gchar *id,
                              const gchar *architecture,
                              const gchar *format)
{
    OsinfoImage *image;

    image = g_object_new(OSINFO_TYPE_IMAGE,
                        "id", id,
                        OSINFO_IMAGE_PROP_ARCHITECTURE, architecture,
                        OSINFO_IMAGE_PROP_FORMAT, format,
                        NULL);

    return image;
}

/**
 * osinfo_image_get_architecture:
 * @image: an #OsinfoImage instance
 *
 * Retrieves the target hardware architecture of the OS @image provides.
 *
 * Returns: (transfer none): the hardware architecture, or NULL
 *
 * Since: 1.3.0
 */
const gchar *osinfo_image_get_architecture(OsinfoImage *image)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(image),
                                         OSINFO_IMAGE_PROP_ARCHITECTURE);
}

/**
 * osinfo_image_get_format:
 * @image: an #OsinfoImage instance
 *
 * The format of the @image
 *
 * Returns: (transfer none): the format, or NULL
 *
 * Since: 1.3.0
 */
const gchar *osinfo_image_get_format(OsinfoImage *image)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(image),
                                         OSINFO_IMAGE_PROP_FORMAT);
}

/**
 * osinfo_image_get_url:
 * @image: an #OsinfoImage instance
 *
 * The URL to the @image
 *
 * Returns: (transfer none): the URL, or NULL
 *
 * Since: 1.3.0
 */
const gchar *osinfo_image_get_url(OsinfoImage *image)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(image),
                                         OSINFO_IMAGE_PROP_URL);
}

/**
 * osinfo_image_get_cloud_init:
 * @image: an #OsinfoImage instance
 *
 * Whether @image supports cloud init customizations
 *
 * Returns: #TRUE if @image supports cloud init customizations, #FALSE
 * otherwise.
 *
 * Since: 1.3.0
 */
gboolean osinfo_image_get_cloud_init(OsinfoImage *image)
{
    return osinfo_entity_get_param_value_boolean_with_default
            (OSINFO_ENTITY(image), OSINFO_IMAGE_PROP_CLOUD_INIT, FALSE);
}

/**
 * osinfo_image_get_os:
 * @image: an #OsinfoImage instance
 *
 * Returns: (transfer full): the operating system, or NULL
 *
 * Since: 1.5.0
 */
OsinfoOs *osinfo_image_get_os(OsinfoImage *image)
{
    g_return_val_if_fail(OSINFO_IS_IMAGE(image), NULL);

    return g_weak_ref_get(&image->priv->os);
}

/**
 * osinfo_image_set_os
 * @image: an #OsinfoImage instance
 * @os: an #OsinfoOs instance
 *
 * Sets the #OsinfoOs associated to the #OsinfoImage instance.
 *
 * Since: 1.5.0
 */
void osinfo_image_set_os(OsinfoImage *image, OsinfoOs *os)
{
    g_return_if_fail(OSINFO_IS_IMAGE(image));

    g_object_ref(os);
    g_weak_ref_set(&image->priv->os, os);
    g_object_unref(os);
}

/**
 * osinfo_image_get_os_variants:
 * @image: an #OsinfoImage instance
 *
 * Gets the varriants of the associated operating system
 *
 * Returns: (transfer full): the operating system variants, or NULL
 *
 * Since: 1.5.0
 */
OsinfoOsVariantList *osinfo_image_get_os_variants(OsinfoImage *image)
{
    OsinfoOs *os;
    OsinfoOsVariantList *os_variants;
    OsinfoOsVariantList *image_variants;
    GList *ids, *node;
    OsinfoFilter *filter;

    g_return_val_if_fail(OSINFO_IS_IMAGE(image), NULL);

    os = osinfo_image_get_os(image);
    if (os == NULL)
        return NULL;

    os_variants = osinfo_os_get_variant_list(os);
    g_object_unref(os);

    ids = osinfo_entity_get_param_value_list(OSINFO_ENTITY(image),
                                             OSINFO_IMAGE_PROP_VARIANT);
    filter = osinfo_filter_new();
    image_variants = osinfo_os_variantlist_new();
    for (node = ids; node != NULL; node = node->next) {
        osinfo_filter_clear_constraints(filter);
        osinfo_filter_add_constraint(filter,
                                     OSINFO_ENTITY_PROP_ID,
                                     (const char *) node->data);
        osinfo_list_add_filtered(OSINFO_LIST(image_variants),
                                 OSINFO_LIST(os_variants),
                                 filter);
    }
    g_object_unref(os_variants);

    return image_variants;
}
