/*
 * libosinfo: An installed image of a (guest) OS
 *
 * Copyright (C) 2018 Red Hat, Inc.
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
 *
 * Authors:
 *  Fabiano Fidêncio <fidencio@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

G_DEFINE_TYPE(OsinfoImage, osinfo_image, OSINFO_TYPE_ENTITY);

#define OSINFO_IMAGE_GET_PRIVATE(obj)                    \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj),                \
                                  OSINFO_TYPE_IMAGE,     \
                                  OsinfoImagePrivate))

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
    gboolean unused;
};

enum {
    PROP_0,

    PROP_ARCHITECTURE,
    PROP_FORMAT,
    PROP_URL,
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

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
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

    g_klass->finalize = osinfo_image_finalize;
    g_klass->get_property = osinfo_image_get_property;
    g_klass->set_property = osinfo_image_set_property;
    g_type_class_add_private(klass, sizeof(OsinfoImagePrivate));

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
}

static void
osinfo_image_init(OsinfoImage *image)
{
    image->priv = OSINFO_IMAGE_GET_PRIVATE(image);
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
 */
const gchar *osinfo_image_get_url(OsinfoImage *image)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(image),
                                         OSINFO_IMAGE_PROP_URL);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
