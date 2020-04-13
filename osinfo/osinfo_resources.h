/*
 * libosinfo: Required or recommended resources for an (guest) OS
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
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>

#ifndef __OSINFO_RESOURCES_H__
# define __OSINFO_RESOURCES_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_ARCHITECTURE_ALL "all"
# define OSINFO_MEGAHERTZ        1000000
# define OSINFO_KIBIBYTES        1024
# define OSINFO_MEBIBYTES        1048576
# define OSINFO_GIBIBYTES        1073741824

# define OSINFO_TYPE_RESOURCES (osinfo_resources_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoResources,
                                           osinfo_resources,
                                           OSINFO,
                                           RESOURCES,
                                           OsinfoEntity)

# define OSINFO_RESOURCES_PROP_ARCHITECTURE "architecture"
# define OSINFO_RESOURCES_PROP_CPU          "cpu"
# define OSINFO_RESOURCES_PROP_N_CPUS       "n-cpus"
# define OSINFO_RESOURCES_PROP_RAM          "ram"
# define OSINFO_RESOURCES_PROP_STORAGE      "storage"

OsinfoResources *osinfo_resources_new(const gchar *id, const gchar *architecture);

const gchar *osinfo_resources_get_architecture(OsinfoResources *resources);
gint osinfo_resources_get_n_cpus(OsinfoResources *resources);
gint64 osinfo_resources_get_cpu(OsinfoResources *resources);
gint64 osinfo_resources_get_ram(OsinfoResources *resources);
gint64 osinfo_resources_get_storage(OsinfoResources *resources);

void osinfo_resources_set_n_cpus(OsinfoResources *resources, gint n_cpus);
void osinfo_resources_set_cpu(OsinfoResources *resources, gint64 cpu);
void osinfo_resources_set_ram(OsinfoResources *resources, gint64 ram);
void osinfo_resources_set_storage(OsinfoResources *resources, gint64 storage);

#endif /* __OSINFO_RESOURCES_H__ */
