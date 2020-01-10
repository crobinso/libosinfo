/*
 * libosinfo: a list of deployments
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

#ifndef __OSINFO_DEPLOYMENTLIST_H__
# define __OSINFO_DEPLOYMENTLIST_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_DEPLOYMENTLIST (osinfo_deploymentlist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDeploymentList,
                                           osinfo_deploymentlist,
                                           OSINFO,
                                           DEPLOYMENTLIST,
                                           OsinfoList)

OsinfoDeploymentList *osinfo_deploymentlist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoDeploymentList *osinfo_deploymentlist_new_copy(OsinfoDeploymentList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoDeploymentList *osinfo_deploymentlist_new_filtered(OsinfoDeploymentList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoDeploymentList *osinfo_deploymentlist_new_intersection(OsinfoDeploymentList *sourceOne, OsinfoDeploymentList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoDeploymentList *osinfo_deploymentlist_new_union(OsinfoDeploymentList *sourceOne, OsinfoDeploymentList *sourceTwo);

#endif /* __OSINFO_DEPLOYMENTLIST_H__ */
