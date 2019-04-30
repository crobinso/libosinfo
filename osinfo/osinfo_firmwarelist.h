/*
 * libosinfo: a list of firmwares
 *
 * Copyright (C) 2019 Red Hat, Inc.
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

#ifndef __OSINFO_FIRMWARELIST_H__
#define __OSINFO_FIRMWARELIST_H__

/*
 * Type macros.
 */
#define OSINFO_TYPE_FIRMWARELIST                  (osinfo_firmwarelist_get_type ())
#define OSINFO_FIRMWARELIST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_FIRMWARELIST, OsinfoFirmwareList))
#define OSINFO_IS_FIRMWARELIST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_FIRMWARELIST))
#define OSINFO_FIRMWARELIST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_FIRMWARELIST, OsinfoFirmwareListClass))
#define OSINFO_IS_FIRMWARELIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_FIRMWARELIST))
#define OSINFO_FIRMWARELIST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_FIRMWARELIST, OsinfoFirmwareListClass))

typedef struct _OsinfoFirmwareList        OsinfoFirmwareList;

typedef struct _OsinfoFirmwareListClass   OsinfoFirmwareListClass;

typedef struct _OsinfoFirmwareListPrivate OsinfoFirmwareListPrivate;

/* object */
struct _OsinfoFirmwareList
{
    OsinfoList parent_instance;

    /* public */

    /* private */
    OsinfoFirmwareListPrivate *priv;
};

/* class */
struct _OsinfoFirmwareListClass
{
    /*< private >*/
    OsinfoListClass parent_class;

    /* class members */
};

GType osinfo_firmwarelist_get_type(void);

OsinfoFirmwareList *osinfo_firmwarelist_new(void);

#endif /* __OSINFO_FIRMWARELIST_H__ */
