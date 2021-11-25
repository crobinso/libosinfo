/*
 * libosinfo: a list of minimum/recommended resources
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
#include <osinfo/osinfo_list.h>

#include <osinfo/osinfo_macros.h>

#define OSINFO_TYPE_RESOURCESLIST (osinfo_resourceslist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoResourcesList,
                                           osinfo_resourceslist,
                                           OSINFO,
                                           RESOURCESLIST,
                                           OsinfoList)

OsinfoResourcesList *osinfo_resourceslist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoResourcesList *osinfo_resourceslist_new_copy(OsinfoResourcesList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoResourcesList *osinfo_resourceslist_new_filtered
                                (OsinfoResourcesList *source,
                                 OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoResourcesList *osinfo_resourceslist_new_intersection
                                (OsinfoResourcesList *sourceOne,
                                 OsinfoResourcesList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoResourcesList *osinfo_resourceslist_new_union
                                (OsinfoResourcesList *sourceOne,
                                 OsinfoResourcesList *sourceTwo);
