/*
 * libosinfo: an operating system
 *
 * Copyright (C) 2009-2010 Red Hat, Inc
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *   Arjun Roy <arroy@redhat.com>
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <glib-object.h>
#include <osinfo/osinfo_product.h>
#include <osinfo/osinfo_device.h>
#include <osinfo/osinfo_devicelist.h>
#include <osinfo/osinfo_medialist.h>
#include <osinfo/osinfo_media.h>
#include <osinfo/osinfo_resources.h>
#include <osinfo/osinfo_resourceslist.h>

#ifndef __OSINFO_OS_H__
#define __OSINFO_OS_H__

/*
 * Type macros.
 */
#define OSINFO_TYPE_OS                  (osinfo_os_get_type ())
#define OSINFO_OS(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_OS, OsinfoOs))
#define OSINFO_IS_OS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_OS))
#define OSINFO_OS_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_OS, OsinfoOsClass))
#define OSINFO_IS_OS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_OS))
#define OSINFO_OS_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_OS, OsinfoOsClass))

typedef struct _OsinfoOs        OsinfoOs;

typedef struct _OsinfoOsClass   OsinfoOsClass;

typedef struct _OsinfoOsPrivate OsinfoOsPrivate;

#define OSINFO_OS_PROP_FAMILY    "family"

/* object */
struct _OsinfoOs
{
    OsinfoProduct parent_instance;

    /* public */

    /* private */
    OsinfoOsPrivate *priv;
};

/* class */
struct _OsinfoOsClass
{
    OsinfoProductClass parent_class;

    /* class members */
};

GType osinfo_os_get_type(void);

OsinfoOs *osinfo_os_new(const gchar *id);

OsinfoDeviceList *osinfo_os_get_devices(OsinfoOs *os, OsinfoFilter *filter);
OsinfoDeviceList *osinfo_os_get_all_devices(OsinfoOs *os, OsinfoFilter *filter);
OsinfoDeviceList *osinfo_os_get_devices_by_property(OsinfoOs *os,
                                                    const char *property,
                                                    const char *value,
                                                    gboolean inherited);
OsinfoDeviceLinkList *osinfo_os_get_device_links(OsinfoOs *os, OsinfoFilter *filter);

OsinfoDeviceLink *osinfo_os_add_device(OsinfoOs *os, OsinfoDevice *dev);
const gchar *osinfo_os_get_family(OsinfoOs *os);
OsinfoMediaList *osinfo_os_get_media_list(OsinfoOs *os);
void osinfo_os_add_media(OsinfoOs *os, OsinfoMedia *media);
OsinfoResourcesList *osinfo_os_get_minimum_resources(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_recommended_resources(OsinfoOs *os);
void osinfo_os_add_minimum_resources(OsinfoOs *os, OsinfoResources *resources);
void osinfo_os_add_recommended_resources(OsinfoOs *os, OsinfoResources *resources);

#endif /* __OSINFO_OS_H__ */
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
