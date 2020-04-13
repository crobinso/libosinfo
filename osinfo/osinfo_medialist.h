/*
 * libosinfo: a list of installation media
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
#include <osinfo/osinfo_list.h>

#ifndef __OSINFO_MEDIALIST_H__
# define __OSINFO_MEDIALIST_H__

/*
 * Type macros.
 */
# define OSINFO_TYPE_MEDIALIST                  (osinfo_medialist_get_type ())
# define OSINFO_MEDIALIST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_MEDIALIST, OsinfoMediaList))
# define OSINFO_IS_MEDIALIST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_MEDIALIST))
# define OSINFO_MEDIALIST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_MEDIALIST, OsinfoMediaListClass))
# define OSINFO_IS_MEDIALIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_MEDIALIST))
# define OSINFO_MEDIALIST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_MEDIALIST, OsinfoMediaListClass))

typedef struct _OsinfoMediaList        OsinfoMediaList;

typedef struct _OsinfoMediaListClass   OsinfoMediaListClass;

typedef struct _OsinfoMediaListPrivate OsinfoMediaListPrivate;

/* object */
struct _OsinfoMediaList
{
    OsinfoList parent_instance;

    /* public */

    /* private */
    OsinfoMediaListPrivate *priv;
};

/* class */
struct _OsinfoMediaListClass
{
    /*< private >*/
    OsinfoListClass parent_class;

    /* class members */
};

GType osinfo_medialist_get_type(void);

OsinfoMediaList *osinfo_medialist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoMediaList *osinfo_medialist_new_copy(OsinfoMediaList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoMediaList *osinfo_medialist_new_filtered(OsinfoMediaList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoMediaList *osinfo_medialist_new_intersection(OsinfoMediaList *sourceOne, OsinfoMediaList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoMediaList *osinfo_medialist_new_union(OsinfoMediaList *sourceOne, OsinfoMediaList *sourceTwo);

#endif /* __OSINFO_MEDIALIST_H__ */
