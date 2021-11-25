/*
 * libosinfo: a virtualization platform
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
#include <osinfo/osinfo_product.h>
#include <osinfo/osinfo_device.h>
#include <osinfo/osinfo_devicelist.h>

/*
 * Type macros.
 */
#define OSINFO_TYPE_PLATFORM (osinfo_platform_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoPlatform,
                                           osinfo_platform,
                                           OSINFO,
                                           PLATFORM,
                                           OsinfoProduct)

OsinfoPlatform *osinfo_platform_new(const gchar *id);

OsinfoDeviceList *osinfo_platform_get_devices(OsinfoPlatform *platform, OsinfoFilter *filter);
OsinfoDeviceList *osinfo_platform_get_all_devices(OsinfoPlatform *platform, OsinfoFilter *filter);
OsinfoDeviceLinkList *osinfo_platform_get_device_links(OsinfoPlatform *platform, OsinfoFilter *filter);

OsinfoDeviceLink *osinfo_platform_add_device(OsinfoPlatform *platform, OsinfoDevice *dev);
