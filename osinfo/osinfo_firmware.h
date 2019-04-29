/*
 * libosinfo: A firmware representation for a (guest) OS
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
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>

#ifndef __OSINFO_FIRMWARE_H__
#define __OSINFO_FIRMWARE_H__

/*
 * Type macros.
 */
#define OSINFO_TYPE_FIRMWARE                  (osinfo_firmware_get_type ())
#define OSINFO_FIRMWARE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_FIRMWARE, OsinfoFirmware))
#define OSINFO_IS_FIRMWARE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_FIRMWARE))
#define OSINFO_FIRMWARE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_FIRMWARE, OsinfoFirmwareClass))
#define OSINFO_IS_FIRMWARE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_FIRMWARE))
#define OSINFO_FIRMWARE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_FIRMWARE, OsinfoFirmwareClass))

#define OSINFO_FIRMWARE_PROP_ARCHITECTURE   "architecture"
#define OSINFO_FIRMWARE_PROP_TYPE           "type"
#define OSINFO_FIRMWARE_PROP_SUPPORTED      "supported"

typedef struct _OsinfoFirmware          OsinfoFirmware;
typedef struct _OsinfoFirmwareClass     OsinfoFirmwareClass;
typedef struct _OsinfoFirmwarePrivate   OsinfoFirmwarePrivate;

/* object */
struct _OsinfoFirmware
{
    OsinfoEntity parent_instance;

    /* public */

    /* private */
    OsinfoFirmwarePrivate *priv;
};

/* class */
struct _OsinfoFirmwareClass
{
    /*< private >*/
    OsinfoEntityClass parent_class;

    /* class members */
};

GType osinfo_firmware_get_type(void);

OsinfoFirmware *osinfo_firmware_new(const gchar *id, const gchar *architecture, const gchar *type);
const gchar *osinfo_firmware_get_architecture(OsinfoFirmware *firmware);
const gchar *osinfo_firmware_get_firmware_type(OsinfoFirmware *firmware);
gboolean osinfo_firmware_is_supported(OsinfoFirmware *firmware);

#endif /* __OSINFO_FIRMWARE_H__ */
