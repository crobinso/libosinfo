/*
 * libosinfo: a list of devicelinks
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

#include <glib-object.h>
#include <osinfo/osinfo_filter.h>
#include <osinfo/osinfo_list.h>

#ifndef __OSINFO_DEVICELINKLIST_H__
# define __OSINFO_DEVICELINKLIST_H__

/*
 * Type macros.
 */
# define OSINFO_TYPE_DEVICELINKLIST                  (osinfo_devicelinklist_get_type ())
# define OSINFO_DEVICELINKLIST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_DEVICELINKLIST, OsinfoDeviceLinkList))
# define OSINFO_IS_DEVICELINKLIST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_DEVICELINKLIST))
# define OSINFO_DEVICELINKLIST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_DEVICELINKLIST, OsinfoDeviceLinkListClass))
# define OSINFO_IS_DEVICELINKLIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_DEVICELINKLIST))
# define OSINFO_DEVICELINKLIST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_DEVICELINKLIST, OsinfoDeviceLinkListClass))

typedef struct _OsinfoDeviceLinkList        OsinfoDeviceLinkList;

typedef struct _OsinfoDeviceLinkListClass   OsinfoDeviceLinkListClass;

typedef struct _OsinfoDeviceLinkListPrivate OsinfoDeviceLinkListPrivate;

/* object */
struct _OsinfoDeviceLinkList
{
    OsinfoList parent_instance;

    /* public */

    /* private */
    OsinfoDeviceLinkListPrivate *priv;
};

/* class */
struct _OsinfoDeviceLinkListClass
{
    /*< private >*/
    OsinfoListClass parent_class;

    /* class members */
};

GType osinfo_devicelinklist_get_type(void);

OsinfoDeviceLinkList *osinfo_devicelinklist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_copy(OsinfoDeviceLinkList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_filtered(OsinfoDeviceLinkList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_intersection(OsinfoDeviceLinkList *sourceOne, OsinfoDeviceLinkList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_union(OsinfoDeviceLinkList *sourceOne, OsinfoDeviceLinkList *sourceTwo);

OsinfoDeviceList *osinfo_devicelinklist_get_devices(OsinfoDeviceLinkList *list, OsinfoFilter *filter);

#endif /* __OSINFO_DEVICELINKLIST_H__ */
