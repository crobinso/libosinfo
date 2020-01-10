/*
 * libosinfo: The variant of an OS
 *
 * Copyright (C) 2013-2020 Red Hat, Inc.
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
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>

#ifndef __OSINFO_OS_VARIANT_H__
# define __OSINFO_OS_VARIANT_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_OS_VARIANT (osinfo_os_variant_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoOsVariant,
                                           osinfo_os_variant,
                                           OSINFO,
                                           OS_VARIANT,
                                           OsinfoEntity)

# define OSINFO_OS_VARIANT_PROP_NAME "name"

OsinfoOsVariant *osinfo_os_variant_new(const gchar *id);
const gchar *osinfo_os_variant_get_name(OsinfoOsVariant *variant);

#endif /* __OSINFO_OS_VARIANT_H__ */
