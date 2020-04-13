/*
 * libosinfo: Device driver
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
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

#include "osinfo_device_driver_private.h"

/**
 * SECTION:osinfo_device_driver
 * @short_description: Information about device driver
 * @see_also: #OsinfoOs
 *
 * #OsinfoDeviceDriver is an entity representing device drivers for an (guest)
 * operating system.
 */

struct _OsinfoDeviceDriverPrivate
{
    OsinfoDeviceList *devices;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDeviceDriver, osinfo_device_driver, OSINFO_TYPE_ENTITY);

static void
osinfo_device_driver_finalize(GObject *object)
{
    OsinfoDeviceDriver *driver = OSINFO_DEVICE_DRIVER(object);

    g_object_unref(driver->priv->devices);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_device_driver_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_device_driver_class_init(OsinfoDeviceDriverClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_device_driver_finalize;
}

static void
osinfo_device_driver_init(OsinfoDeviceDriver *driver)
{
    driver->priv = osinfo_device_driver_get_instance_private(driver);
    driver->priv->devices = osinfo_devicelist_new();
}

OsinfoDeviceDriver *osinfo_device_driver_new(const gchar *id)
{
    OsinfoDeviceDriver *driver;

    driver = g_object_new(OSINFO_TYPE_DEVICE_DRIVER,
                           "id", id,
                           NULL);

    return driver;
}

/**
 * osinfo_device_driver_get_architecture:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Retrieves the target hardware architecture of @driver.
 *
 * Returns: (transfer none): the hardware architecture.
 *
 * Since: 0.2.2
 */
const gchar *osinfo_device_driver_get_architecture(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(driver),
                                         OSINFO_DEVICE_DRIVER_PROP_ARCHITECTURE);
}

/**
 * osinfo_device_driver_get_location:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Retrieves the location of the @driver as a URL.
 *
 * Returns: (transfer none): the location of the driver.
 *
 * Since: 0.2.2
 */
const gchar *osinfo_device_driver_get_location(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(driver),
                                         OSINFO_DEVICE_DRIVER_PROP_LOCATION);
}

/**
 * osinfo_device_driver_get_files:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Retrieves the names of driver files under the location returned by
 * #osinfo_device_driver_get_location.
 *
 * Returns: (transfer container) (element-type utf8): The list of driver files.
 *
 * Since: 0.2.2
 */
GList *osinfo_device_driver_get_files(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value_list(OSINFO_ENTITY(driver),
                                              OSINFO_DEVICE_DRIVER_PROP_FILE);
}

/**
 * osinfo_device_driver_get_pre_installable:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Returns: TRUE if @driver is pre-installable, FALSE otherwise.
 *
 * Since: 0.2.2
 */
gboolean osinfo_device_driver_get_pre_installable(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value_boolean
                (OSINFO_ENTITY(driver),
                 OSINFO_DEVICE_DRIVER_PROP_PRE_INSTALLABLE);
}

/**
 * osinfo_device_driver_get_devices:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Returns: (transfer none): The list of devices supported by this driver.
 *
 * Since: 0.2.2
 */
OsinfoDeviceList *osinfo_device_driver_get_devices(OsinfoDeviceDriver *driver)
{
    g_return_val_if_fail(OSINFO_IS_DEVICE_DRIVER(driver), NULL);

    return driver->priv->devices;
}

void osinfo_device_driver_add_device(OsinfoDeviceDriver *driver,
                                     OsinfoDevice *device)
{
    g_return_if_fail(OSINFO_IS_DEVICE_DRIVER(driver));
    g_return_if_fail(OSINFO_IS_DEVICE(device));

    osinfo_list_add(OSINFO_LIST(driver->priv->devices),
                    OSINFO_ENTITY(device));
}

/**
 * osinfo_device_driver_get_signed:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Some OS vendors recommend or require device drivers to be signed by them
 * before these device drivers could be installed on their OS.
 *
 * Returns: TRUE if @driver is signed, FALSE otherwise.
 *
 * Since: 0.2.6
 */
gboolean osinfo_device_driver_get_signed(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value_boolean
                (OSINFO_ENTITY(driver),
                 OSINFO_DEVICE_DRIVER_PROP_SIGNED);
}

/**
 * osinfo_device_driver_get_priority:
 * @driver: an #OsinfoDeviceDriver instance
 *
 * Returns the priority of the device driver. The higher the value, the more
 * important it is.
 *
 * Returns: the priority of the device driver.
 *
 * Since: 1.7.0
 */
gint64 osinfo_device_driver_get_priority(OsinfoDeviceDriver *driver)
{
    return osinfo_entity_get_param_value_int64_with_default
                (OSINFO_ENTITY(driver),
                 OSINFO_DEVICE_DRIVER_PROP_PRIORITY,
                 OSINFO_DEVICE_DRIVER_DEFAULT_PRIORITY);
}
