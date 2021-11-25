/*
 * libosinfo: A single hardware device
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
#include <osinfo/osinfo_entity.h>

#include <osinfo/osinfo_macros.h>

#define OSINFO_TYPE_DEVICE (osinfo_device_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDevice,
                                           osinfo_device,
                                           OSINFO,
                                           DEVICE,
                                           OsinfoEntity);

#define OSINFO_DEVICE_PROP_VENDOR     "vendor"
#define OSINFO_DEVICE_PROP_VENDOR_ID  "vendor-id"
#define OSINFO_DEVICE_PROP_PRODUCT    "product"
#define OSINFO_DEVICE_PROP_PRODUCT_ID "product-id"
#define OSINFO_DEVICE_PROP_NAME       "name"
#define OSINFO_DEVICE_PROP_CLASS      "class"
#define OSINFO_DEVICE_PROP_BUS_TYPE   "bus-type"
#define OSINFO_DEVICE_PROP_SUBSYSTEM  "subsystem"

OsinfoDevice *osinfo_device_new(const gchar *id);

const gchar *osinfo_device_get_vendor(OsinfoDevice *dev);
const gchar *osinfo_device_get_vendor_id(OsinfoDevice *dev);
const gchar *osinfo_device_get_product(OsinfoDevice *dev);
const gchar *osinfo_device_get_product_id(OsinfoDevice *dev);
const gchar *osinfo_device_get_bus_type(OsinfoDevice *dev);
const gchar *osinfo_device_get_class(OsinfoDevice *dev);
const gchar *osinfo_device_get_name(OsinfoDevice *dev);
const gchar *osinfo_device_get_subsystem(OsinfoDevice *dev);
