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

#include <osinfo/osinfo.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

G_DEFINE_TYPE(OsinfoFirmware, osinfo_firmware, OSINFO_TYPE_ENTITY);

#define OSINFO_FIRMWARE_GET_PRIVATE(obj)                    \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj),                \
                                  OSINFO_TYPE_FIRMWARE,     \
                                  OsinfoFirmwarePrivate))

/**
 * SECTION:osinfo_firmware
 * @short_description: A firmware representation for a (guest) OS
 * @see_also: #OsinfoOs
 *
 * #OsinfoFirmware is an entity representing a firmware used to boot
 * a (guest) operating system.
 */

struct _OsinfoFirmwarePrivate
{
    gboolean unused;
};

enum {
    PROP_0,

    PROP_ARCHITECTURE,
    PROP_FIRMWARE_TYPE,
};

static void
osinfo_firmware_get_property(GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
    OsinfoFirmware *firmware = OSINFO_FIRMWARE(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        g_value_set_string(value, osinfo_firmware_get_architecture(firmware));
        break;

    case PROP_FIRMWARE_TYPE:
        g_value_set_string(value, osinfo_firmware_get_firmware_type(firmware));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_firmware_set_property(GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    OsinfoFirmware *firmware = OSINFO_FIRMWARE(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        osinfo_entity_set_param(OSINFO_ENTITY(firmware),
                                OSINFO_FIRMWARE_PROP_ARCHITECTURE,
                                g_value_get_string(value));
        break;

    case PROP_FIRMWARE_TYPE:
        osinfo_entity_set_param(OSINFO_ENTITY(firmware),
                                OSINFO_FIRMWARE_PROP_TYPE,
                                g_value_get_string(value));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_firmware_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_firmware_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_firmware_class_init(OsinfoFirmwareClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);
    GParamSpec *pspec;

    g_klass->finalize = osinfo_firmware_finalize;
    g_klass->get_property = osinfo_firmware_get_property;
    g_klass->set_property = osinfo_firmware_set_property;
    g_type_class_add_private(klass, sizeof(OsinfoFirmwarePrivate));

    /**
     * OsinfoFirmware:architecture:
     *
     * The target hardware architecture of this firmware.
     */
    pspec = g_param_spec_string("architecture",
                                "ARCHITECTURE",
                                _("CPU Architecture"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_ARCHITECTURE, pspec);

    /**
     * OsinfoFirmware:firmware-type:
     *
     * The firmware type.
     */
    pspec = g_param_spec_string("type",
                                "TYPE",
                                _("The firmware type"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_FIRMWARE_TYPE, pspec);
}

static void
osinfo_firmware_init(OsinfoFirmware *firmware)
{
    firmware->priv = OSINFO_FIRMWARE_GET_PRIVATE(firmware);
}

OsinfoFirmware *osinfo_firmware_new(const gchar *id,
                                    const gchar *architecture,
                                    const gchar *type)
{
    OsinfoFirmware *firmware;

    firmware = g_object_new(OSINFO_TYPE_FIRMWARE,
                            "id", id,
                            OSINFO_FIRMWARE_PROP_ARCHITECTURE, architecture,
                            OSINFO_FIRMWARE_PROP_TYPE, type,
                            NULL);

    return firmware;
}

/**
 * osinfo_firmware_get_architecture:
 * @firmware: an #OsinfoFirmware instance
 *
 * Retrieves the target hardware architecture of the OS @firmware provides.
 *
 * Returns: (transfer none): the hardware architecture, or NULL
 */
const gchar *osinfo_firmware_get_architecture(OsinfoFirmware *firmware)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(firmware),
                                         OSINFO_FIRMWARE_PROP_ARCHITECTURE);
}

/**
 * osinfo_firmware_get_format:
 * @firmware: an #OsinfoFirmware instance
 *
 * The format of the @firmware
 *
 * Returns: (transfer none): the format, or NULL
 */
const gchar *osinfo_firmware_get_firmware_type(OsinfoFirmware *firmware)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(firmware),
                                         OSINFO_FIRMWARE_PROP_TYPE);
}

/**
 * osinfo_firmware_is_supported:
 * @firmware: an #OsinfoFirmware instance
 *
 * Whether the @firmware is supported or not
 *
 * Returns: TRUE if supported, FALSE otherwise.
 */
gboolean osinfo_firmware_is_supported(OsinfoFirmware *firmware)
{
    return osinfo_entity_get_param_value_boolean(OSINFO_ENTITY(firmware),
                                                 OSINFO_FIRMWARE_PROP_SUPPORTED);
}
