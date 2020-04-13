/*
 * libosinfo: a list of installation tree
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

#ifndef __OSINFO_TREELIST_H__
# define __OSINFO_TREELIST_H__

/*
 * Type macros.
 */
# define OSINFO_TYPE_TREELIST                  (osinfo_treelist_get_type ())
# define OSINFO_TREELIST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_TREELIST, OsinfoTreeList))
# define OSINFO_IS_TREELIST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_TREELIST))
# define OSINFO_TREELIST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_TREELIST, OsinfoTreeListClass))
# define OSINFO_IS_TREELIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_TREELIST))
# define OSINFO_TREELIST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_TREELIST, OsinfoTreeListClass))

typedef struct _OsinfoTreeList        OsinfoTreeList;

typedef struct _OsinfoTreeListClass   OsinfoTreeListClass;

typedef struct _OsinfoTreeListPrivate OsinfoTreeListPrivate;

/* object */
struct _OsinfoTreeList
{
    OsinfoList parent_instance;

    /* public */

    /* private */
    OsinfoTreeListPrivate *priv;
};

/* class */
struct _OsinfoTreeListClass
{
    /*< private >*/
    OsinfoListClass parent_class;

    /* class members */
};

GType osinfo_treelist_get_type(void);

OsinfoTreeList *osinfo_treelist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoTreeList *osinfo_treelist_new_copy(OsinfoTreeList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoTreeList *osinfo_treelist_new_filtered(OsinfoTreeList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoTreeList *osinfo_treelist_new_intersection(OsinfoTreeList *sourceOne, OsinfoTreeList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoTreeList *osinfo_treelist_new_union(OsinfoTreeList *sourceOne, OsinfoTreeList *sourceTwo);

#endif /* __OSINFO_TREELIST_H__ */
