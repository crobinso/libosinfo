/*
 * libosinfo: OS installation configuration parameter
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

#pragma once

#include <glib-object.h>

#include <osinfo/osinfo_macros.h>

#define OSINFO_TYPE_INSTALL_CONFIG_PARAM (osinfo_install_config_param_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoInstallConfigParam,
                                           osinfo_install_config_param,
                                           OSINFO,
                                           INSTALL_CONFIG_PARAM,
                                           OsinfoEntity)

#define OSINFO_INSTALL_CONFIG_PARAM_PROP_DATAMAP "value-map"
#define OSINFO_INSTALL_CONFIG_PARAM_PROP_NAME    "name"
#define OSINFO_INSTALL_CONFIG_PARAM_PROP_POLICY  "policy"

/**
 * OsinfoInstallConfigParamPolicy:
 * @OSINFO_INSTALL_CONFIG_PARAM_POLICY_NONE: no policy defined
 * @OSINFO_INSTALL_CONFIG_PARAM_POLICY_REQUIRED: config parameter is required
 * @OSINFO_INSTALL_CONFIG_PARAM_POLICY_OPTIONAL: config parameter may be omitted
 *
 * Policy for configuration parameter presence
 *
 * Since: 0.2.2
 */
typedef enum {
    OSINFO_INSTALL_CONFIG_PARAM_POLICY_NONE,
    OSINFO_INSTALL_CONFIG_PARAM_POLICY_REQUIRED,
    OSINFO_INSTALL_CONFIG_PARAM_POLICY_OPTIONAL,
} OsinfoInstallConfigParamPolicy;

OsinfoInstallConfigParam *osinfo_install_config_param_new(const gchar *name);

const gchar *osinfo_install_config_param_get_name(OsinfoInstallConfigParam *config_param);

OsinfoInstallConfigParamPolicy osinfo_install_config_param_get_policy(OsinfoInstallConfigParam *config_param);

gboolean osinfo_install_config_param_is_required(OsinfoInstallConfigParam *config_param);

gboolean osinfo_install_config_param_is_optional(OsinfoInstallConfigParam *config_param);

void osinfo_install_config_param_set_value_map(OsinfoInstallConfigParam *config_param, OsinfoDatamap *datamap);
OsinfoDatamap *osinfo_install_config_param_get_value_map(OsinfoInstallConfigParam *config_param);
