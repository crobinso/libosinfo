/*
 * libosinfo: an operating system deployment for a platform
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
#include <osinfo/osinfo_platform.h>
#include <osinfo/osinfo_os.h>

#ifndef __OSINFO_DEPLOYMENT_H__
# define __OSINFO_DEPLOYMENT_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_DEPLOYMENT (osinfo_deployment_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoDeployment,
                                           osinfo_deployment,
                                           OSINFO,
                                           DEPLOYMENT,
                                           OsinfoEntity)

OsinfoDeployment *osinfo_deployment_new(const gchar *id,
                                        OsinfoOs *os,
                                        OsinfoPlatform *platform);

OsinfoOs *osinfo_deployment_get_os(OsinfoDeployment *deployment);
OsinfoPlatform *osinfo_deployment_get_platform(OsinfoDeployment *deployment);

OsinfoDevice *osinfo_deployment_get_preferred_device(OsinfoDeployment *deployment, OsinfoFilter *filter);
OsinfoDeviceLink *osinfo_deployment_get_preferred_device_link(OsinfoDeployment *deployment, OsinfoFilter *filter);

OsinfoDeviceList *osinfo_deployment_get_devices(OsinfoDeployment *deployment, OsinfoFilter *filter);
OsinfoDeviceLinkList *osinfo_deployment_get_device_links(OsinfoDeployment *deployment, OsinfoFilter *filter);
OsinfoDeviceLink *osinfo_deployment_add_device(OsinfoDeployment *deployment, OsinfoDevice *dev);

#endif /* __OSINFO_DEPLOYMENT_H__ */
