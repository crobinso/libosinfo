/*
 * libosinfo: a list of entities
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

#ifndef __OSINFO_LIST_H__
# define __OSINFO_LIST_H__

# include <osinfo/osinfo_filter.h>
# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_LIST (osinfo_list_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoList,
                                           osinfo_list,
                                           OSINFO,
                                           LIST,
                                           GObject)

GType osinfo_list_get_element_type(OsinfoList *list);
gint osinfo_list_get_length(OsinfoList *list);
OsinfoEntity *osinfo_list_get_nth(OsinfoList *list, gint idx);
OsinfoEntity *osinfo_list_find_by_id(OsinfoList *list, const gchar *id);

GList *osinfo_list_get_elements(OsinfoList *list);

void osinfo_list_add(OsinfoList *list, OsinfoEntity *entity);
void osinfo_list_add_filtered(OsinfoList *list, OsinfoList *source, OsinfoFilter *filter);
void osinfo_list_add_intersection(OsinfoList *list, OsinfoList *sourceOne, OsinfoList *sourceTwo);
void osinfo_list_add_union(OsinfoList *list, OsinfoList *sourceOne, OsinfoList *sourceTwo);
void osinfo_list_add_all(OsinfoList *list, OsinfoList *source);

OsinfoList *osinfo_list_new_copy(OsinfoList *source);
OsinfoList *osinfo_list_new_filtered(OsinfoList *source, OsinfoFilter *filter);
OsinfoList *osinfo_list_new_intersection(OsinfoList *sourceOne, OsinfoList *sourceTwo);
OsinfoList *osinfo_list_new_union(OsinfoList *sourceOne, OsinfoList *sourceTwo);

#endif /* __OSINFO_LIST_H__ */
