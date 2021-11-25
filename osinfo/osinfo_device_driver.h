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

#pragma once

#include <glib-object.h>
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>
#include <osinfo/osinfo_devicelist.h>

#include <osinfo/osinfo_macros.h>

#define OSINFO_TYPE_DEVICE_DRIVER (osinfo_device_driver_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDeviceDriver,
                                           osinfo_device_driver,
                                           OSINFO,
                                           DEVICE_DRIVER,
                                           OsinfoEntity)

#define OSINFO_DEVICE_DRIVER_PROP_ARCHITECTURE    "arch"
#define OSINFO_DEVICE_DRIVER_PROP_LOCATION        "location"
#define OSINFO_DEVICE_DRIVER_PROP_PRE_INSTALLABLE "pre-installable"
#define OSINFO_DEVICE_DRIVER_PROP_FILE            "file"
#define OSINFO_DEVICE_DRIVER_PROP_DEVICE          "device"
#define OSINFO_DEVICE_DRIVER_PROP_SIGNED          "signed"
#define OSINFO_DEVICE_DRIVER_PROP_PRIORITY        "priority"
#define OSINFO_DEVICE_DRIVER_DEFAULT_PRIORITY     50

const gchar *osinfo_device_driver_get_architecture(OsinfoDeviceDriver *driver);
const gchar *osinfo_device_driver_get_location(OsinfoDeviceDriver *driver);
gboolean osinfo_device_driver_get_pre_installable(OsinfoDeviceDriver *driver);
GList *osinfo_device_driver_get_files(OsinfoDeviceDriver *driver);
OsinfoDeviceList *osinfo_device_driver_get_devices(OsinfoDeviceDriver *driver);
gboolean osinfo_device_driver_get_signed(OsinfoDeviceDriver *driver);
gint64 osinfo_device_driver_get_priority(OsinfoDeviceDriver *driver);
