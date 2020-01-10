/*
 * libosinfo: a list of software products
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
#include <osinfo/osinfo_list.h>

#ifndef __OSINFO_PRODUCTLIST_H__
# define __OSINFO_PRODUCTLIST_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_PRODUCTLIST (osinfo_productlist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoProductList,
                                           osinfo_productlist,
                                           OSINFO,
                                           PRODUCTLIST,
                                           OsinfoList)

OsinfoProductList *osinfo_productlist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoProductList *osinfo_productlist_new_copy(OsinfoProductList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoProductList *osinfo_productlist_new_filtered(OsinfoProductList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoProductList *osinfo_productlist_new_intersection(OsinfoProductList *sourceOne, OsinfoProductList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoProductList *osinfo_productlist_new_union(OsinfoProductList *sourceOne, OsinfoProductList *sourceTwo);

#endif /* __OSINFO_PRODUCTLIST_H__ */
