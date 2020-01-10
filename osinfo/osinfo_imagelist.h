/*
 * libosinfo: a list of pre-installed images
 *
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
#include <osinfo/osinfo_list.h>

#ifndef __OSINFO_IMAGELIST_H__
# define __OSINFO_IMAGELIST_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_IMAGELIST (osinfo_imagelist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoImageList,
                                           osinfo_imagelist,
                                           OSINFO,
                                           IMAGELIST,
                                           OsinfoList)

OsinfoImageList *osinfo_imagelist_new(void);

#endif /* __OSINFO_IMAGELIST_H__ */
