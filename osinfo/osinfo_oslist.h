/*
 * libosinfo: a list of operating systems
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
#include <osinfo/osinfo_productlist.h>

#ifndef __OSINFO_OSLIST_H__
# define __OSINFO_OSLIST_H__

/*
 * Type macros.
 */
# define OSINFO_TYPE_OSLIST                  (osinfo_oslist_get_type ())
# define OSINFO_OSLIST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_OSLIST, OsinfoOsList))
# define OSINFO_IS_OSLIST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_OSLIST))
# define OSINFO_OSLIST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_OSLIST, OsinfoOsListClass))
# define OSINFO_IS_OSLIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_OSLIST))
# define OSINFO_OSLIST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_OSLIST, OsinfoOsListClass))

typedef struct _OsinfoOsList        OsinfoOsList;

typedef struct _OsinfoOsListClass   OsinfoOsListClass;

typedef struct _OsinfoOsListPrivate OsinfoOsListPrivate;

/* object */
struct _OsinfoOsList
{
    OsinfoProductList parent_instance;

    /* public */

    /* private */
    OsinfoOsListPrivate *priv;
};

/* class */
struct _OsinfoOsListClass
{
    /*< private >*/
    OsinfoProductListClass parent_class;

    /* class members */
};

GType osinfo_oslist_get_type(void);

OsinfoOsList *osinfo_oslist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoOsList *osinfo_oslist_new_copy(OsinfoOsList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoOsList *osinfo_oslist_new_filtered(OsinfoOsList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoOsList *osinfo_oslist_new_intersection(OsinfoOsList *sourceOne, OsinfoOsList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoOsList *osinfo_oslist_new_union(OsinfoOsList *sourceOne, OsinfoOsList *sourceTwo);

#endif /* __OSINFO_OSLIST_H__ */
