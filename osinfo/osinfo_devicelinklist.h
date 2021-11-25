/*
 * libosinfo: a list of devicelinks
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

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_DEVICELINKLIST (osinfo_devicelinklist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDeviceLinkList,
                                           osinfo_devicelinklist,
                                           OSINFO,
                                           DEVICELINKLIST,
                                           OsinfoList)

OsinfoDeviceLinkList *osinfo_devicelinklist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_copy(OsinfoDeviceLinkList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_filtered(OsinfoDeviceLinkList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_intersection(OsinfoDeviceLinkList *sourceOne, OsinfoDeviceLinkList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoDeviceLinkList *osinfo_devicelinklist_new_union(OsinfoDeviceLinkList *sourceOne, OsinfoDeviceLinkList *sourceTwo);

OsinfoDeviceList *osinfo_devicelinklist_get_devices(OsinfoDeviceLinkList *list, OsinfoFilter *filter);
