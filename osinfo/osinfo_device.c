/*
 * libosinfo:
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

#include <osinfo/osinfo.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_device
 * @short_description: A hardware device
 * @see_also: #OsinfoOs, #OsinfoPlatform
 *
 * #OsinfoDevice is an entity representing some kind of hardware
 * device. Devices can be associated with operating systems
 * and platforms.
 */

struct _OsinfoDevicePrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDevice, osinfo_device, OSINFO_TYPE_ENTITY);

static void osinfo_device_finalize(GObject *object);

static void
osinfo_device_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_device_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_device_class_init(OsinfoDeviceClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_device_finalize;
}

static void
osinfo_device_init(OsinfoDevice *device)
{
    device->priv = osinfo_device_get_instance_private(device);
}

OsinfoDevice *osinfo_device_new(const gchar *id)
{
    return g_object_new(OSINFO_TYPE_DEVICE,
                        "id", id,
                        NULL);
}


const gchar *osinfo_device_get_vendor(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_VENDOR);
}

const gchar *osinfo_device_get_vendor_id(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_VENDOR_ID);
}

const gchar *osinfo_device_get_product(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_PRODUCT);
}

const gchar *osinfo_device_get_product_id(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_PRODUCT_ID);
}

const gchar *osinfo_device_get_bus_type(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_BUS_TYPE);
}

const gchar *osinfo_device_get_class(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_CLASS);
}

const gchar *osinfo_device_get_name(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_NAME);
}

/**
 * osinfo_device_get_subsystem:
 * @dev: the device
 *
 * Returns: the value of the device's subsystem.
 *
 * Since: 0.2.1
 */
const gchar *osinfo_device_get_subsystem(OsinfoDevice *dev)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(dev), OSINFO_DEVICE_PROP_SUBSYSTEM);
}
