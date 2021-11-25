/*
 * libosinfo: a list of devices
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
#include <osinfo/osinfo_filter.h>
#include <osinfo/osinfo_list.h>

#include <osinfo/osinfo_macros.h>

#define OSINFO_TYPE_DEVICELIST (osinfo_devicelist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDeviceList,
                                           osinfo_devicelist,
                                           OSINFO,
                                           DEVICELIST,
                                           OsinfoList)

OsinfoDeviceList *osinfo_devicelist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoDeviceList *osinfo_devicelist_new_copy(OsinfoDeviceList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoDeviceList *osinfo_devicelist_new_filtered(OsinfoDeviceList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoDeviceList *osinfo_devicelist_new_intersection(OsinfoDeviceList *sourceOne, OsinfoDeviceList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoDeviceList *osinfo_devicelist_new_union(OsinfoDeviceList *sourceOne, OsinfoDeviceList *sourceTwo);
