/*
 * libosinfo: a list of installation configuration parameters
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
#include <osinfo/osinfo_list.h>

#ifndef __OSINFO_INSTALL_CONFIG_PARAMLIST_H__
# define __OSINFO_INSTALL_CONFIG_PARAMLIST_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_INSTALL_CONFIG_PARAMLIST (osinfo_install_config_paramlist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoInstallConfigParamList,
                                           osinfo_install_config_paramlist,
                                           OSINFO,
                                           INSTALL_CONFIG_PARAMLIST,
                                           OsinfoList)

OsinfoInstallConfigParamList *osinfo_install_config_paramlist_new(void);

#endif /* __OSINFO_INSTALL_CONFIG_PARAMLIST_H__ */
