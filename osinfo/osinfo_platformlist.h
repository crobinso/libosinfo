/*
 * libosinfo: a list of platforms
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

#include <glib-object.h>
#include <osinfo/osinfo_filter.h>
#include <osinfo/osinfo_productlist.h>

#ifndef __OSINFO_PLATFORMLIST_H__
# define __OSINFO_PLATFORMLIST_H__

# define OSINFO_TYPE_PLATFORMLIST (osinfo_platformlist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoPlatformList,
                                           osinfo_platformlist,
                                           OSINFO,
                                           PLATFORMLIST,
                                           OsinfoProductList)

OsinfoPlatformList *osinfo_platformlist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoPlatformList *osinfo_platformlist_new_copy(OsinfoPlatformList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoPlatformList *osinfo_platformlist_new_filtered(OsinfoPlatformList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoPlatformList *osinfo_platformlist_new_intersection(OsinfoPlatformList *sourceOne, OsinfoPlatformList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoPlatformList *osinfo_platformlist_new_union(OsinfoPlatformList *sourceOne, OsinfoPlatformList *sourceTwo);

#endif /* __OSINFO_PLATFORMLIST_H__ */
