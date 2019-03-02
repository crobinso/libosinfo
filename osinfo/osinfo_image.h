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

#include <glib-object.h>
#include <gio/gio.h>

#ifndef __OSINFO_IMAGE_H__
#define __OSINFO_IMAGE_H__

/*
 * Type macros.
 */
#define OSINFO_TYPE_IMAGE                  (osinfo_image_get_type ())
#define OSINFO_IMAGE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_IMAGE, OsinfoImage))
#define OSINFO_IS_IMAGE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_IMAGE))
#define OSINFO_IMAGE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_IMAGE, OsinfoImageClass))
#define OSINFO_IS_IMAGE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_IMAGE))
#define OSINFO_IMAGE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_IMAGE, OsinfoImageClass))

typedef struct _OsinfoImage        OsinfoImage;

typedef struct _OsinfoImageClass   OsinfoImageClass;

typedef struct _OsinfoImagePrivate OsinfoImagePrivate;

#define OSINFO_IMAGE_PROP_ARCHITECTURE      "architecture"
#define OSINFO_IMAGE_PROP_FORMAT            "format"
#define OSINFO_IMAGE_PROP_URL               "url"
#define OSINFO_IMAGE_PROP_CLOUD_INIT        "cloud-init"

/* object */
struct _OsinfoImage
{
    OsinfoEntity parent_instance;

    /* public */

    /* private */
    OsinfoImagePrivate *priv;
};

/* class */
struct _OsinfoImageClass
{
    /*< private >*/
    OsinfoEntityClass parent_class;

    /* class members */
};

GType osinfo_image_get_type(void);

OsinfoImage *osinfo_image_new(const gchar *id, const gchar *architecture, const gchar *format);
const gchar *osinfo_image_get_architecture(OsinfoImage *image);
const gchar *osinfo_image_get_format(OsinfoImage *image);
const gchar *osinfo_image_get_url(OsinfoImage *image);
gboolean osinfo_image_get_cloud_init(OsinfoImage *image);

#endif /* __OSINFO_IMAGE_H__ */
