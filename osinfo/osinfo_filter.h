/*
 * libosinfo: a mechanism to filter enities
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
#include <osinfo/osinfo_entity.h>

#ifndef __OSINFO_FILTER_H__
# define __OSINFO_FILTER_H__

# define OSINFO_TYPE_FILTER (osinfo_filter_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE(OsinfoFilter,
                                 osinfo_filter,
                                 OSINFO,
                                 FILTER,
                                 GObject)

struct _OsinfoFilterClass
{
    GObjectClass parent_class;

    gboolean (*matches)(OsinfoFilter *filter, OsinfoEntity *entity);
};

OsinfoFilter *osinfo_filter_new(void);

void osinfo_filter_add_constraint(OsinfoFilter *filter,
                                  const gchar *propName,
                                  const gchar *propVal);

void osinfo_filter_clear_constraint(OsinfoFilter *filter,
                                    const gchar *propName);
void osinfo_filter_clear_constraints(OsinfoFilter *filter);

GList *osinfo_filter_get_constraint_keys(OsinfoFilter *filter);
GList *osinfo_filter_get_constraint_values(OsinfoFilter *filter,
                                           const gchar *propName);

gboolean osinfo_filter_matches(OsinfoFilter *filter,
                               OsinfoEntity *entity);

#endif /* __OSINFO_FILTER_H__ */
