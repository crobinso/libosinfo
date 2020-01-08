/*
 * libosinfo: a list of pre-installed images
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

#include <glib-object.h>
#include <osinfo/osinfo_list.h>

#ifndef __OSINFO_IMAGELIST_H__
# define __OSINFO_IMAGELIST_H__

/*
 * Type macros.
 */
# define OSINFO_TYPE_IMAGELIST                  (osinfo_imagelist_get_type ())
# define OSINFO_IMAGELIST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_IMAGELIST, OsinfoImageList))
# define OSINFO_IS_IMAGELIST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_IMAGELIST))
# define OSINFO_IMAGELIST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_IMAGELIST, OsinfoImageListClass))
# define OSINFO_IS_IMAGELIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_IMAGELIST))
# define OSINFO_IMAGELIST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_IMAGELIST, OsinfoImageListClass))

typedef struct _OsinfoImageList        OsinfoImageList;

typedef struct _OsinfoImageListClass   OsinfoImageListClass;

typedef struct _OsinfoImageListPrivate OsinfoImageListPrivate;

/* object */
struct _OsinfoImageList
{
    OsinfoList parent_instance;

    /* public */

    /* private */
    OsinfoImageListPrivate *priv;
};

/* class */
struct _OsinfoImageListClass
{
    /*< private >*/
    OsinfoListClass parent_class;

    /* class members */
};

GType osinfo_imagelist_get_type(void);

OsinfoImageList *osinfo_imagelist_new(void);

#endif /* __OSINFO_IMAGELIST_H__ */
