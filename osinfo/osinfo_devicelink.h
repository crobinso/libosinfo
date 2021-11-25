/*
 * libosinfo: A reference to a hardware device
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

#define OSINFO_TYPE_DEVICELINK (osinfo_devicelink_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDeviceLink,
                                           osinfo_devicelink,
                                           OSINFO,
                                           DEVICELINK,
                                           OsinfoEntity)

#define OSINFO_DEVICELINK_PROP_DRIVER   "driver"
#define OSINFO_DEVICELINK_PROP_SUPPORTED  "supported"

OsinfoDeviceLink *osinfo_devicelink_new(OsinfoDevice *target);

OsinfoDevice *osinfo_devicelink_get_target(OsinfoDeviceLink *link);

const gchar *osinfo_devicelink_get_driver(OsinfoDeviceLink *link);
