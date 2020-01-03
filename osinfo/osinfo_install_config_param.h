/*
 * libosinfo: OS installation configuration parameter
 *
 * Copyright (C) 2009-2012 Red Hat, Inc.
 * Copyright (C) 2012 Fabiano Fidêncio
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
 *
 * Authors:
 *   Fabiano Fidêncio <fabiano@fidencio.org>
 */

#include <glib-object.h>

#ifndef __OSINFO_INSTALL_CONFIG_PARAM_H__
# define __OSINFO_INSTALL_CONFIG_PARAM_H__

/*
 * Type macros.
 */
# define OSINFO_TYPE_INSTALL_CONFIG_PARAM            (osinfo_install_config_param_get_type ())
# define OSINFO_INSTALL_CONFIG_PARAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_INSTALL_CONFIG_PARAM, OsinfoInstallConfigParam))
# define OSINFO_IS_INSTALL_CONFIG_PARAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_INSTALL_CONFIG_PARAM))
# define OSINFO_INSTALL_CONFIG_PARAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_INSTALL_CONFIG_PARAM, OsinfoInstallConfigParamClass))
# define OSINFO_IS_INSTALL_CONFIG_PARAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_INSTALL_CONFIG_PARAM))
# define OSINFO_INSTALL_CONFIG_PARAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_INSTALL_CONFIG_PARAM, OsinfoInstallConfigParamClass))

# define OSINFO_INSTALL_CONFIG_PARAM_PROP_DATAMAP "value-map"
# define OSINFO_INSTALL_CONFIG_PARAM_PROP_NAME    "name"
# define OSINFO_INSTALL_CONFIG_PARAM_PROP_POLICY  "policy"

typedef struct _OsinfoInstallConfigParam        OsinfoInstallConfigParam;
typedef struct _OsinfoInstallConfigParamClass   OsinfoInstallConfigParamClass;
typedef struct _OsinfoInstallConfigParamPrivate OsinfoInstallConfigParamPrivate;
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

/* object */
struct _OsinfoInstallConfigParam
{
    OsinfoEntity parent_instance;
    /* public */

    /* private */
    OsinfoInstallConfigParamPrivate *priv;
};

/* class */
struct _OsinfoInstallConfigParamClass
{
    /*< private >*/
    OsinfoEntityClass parent_class;

    /* class members */
};

GType osinfo_install_config_param_get_type(void);

OsinfoInstallConfigParam *osinfo_install_config_param_new(const gchar *name);

const gchar *osinfo_install_config_param_get_name(OsinfoInstallConfigParam *config_param);

OsinfoInstallConfigParamPolicy osinfo_install_config_param_get_policy(OsinfoInstallConfigParam *config_param);

gboolean osinfo_install_config_param_is_required(OsinfoInstallConfigParam *config_param);

gboolean osinfo_install_config_param_is_optional(OsinfoInstallConfigParam *config_param);

void osinfo_install_config_param_set_value_map(OsinfoInstallConfigParam *config_param, OsinfoDatamap *datamap);
OsinfoDatamap *osinfo_install_config_param_get_value_map(OsinfoInstallConfigParam *config_param);


#endif /* __OSINFO_INSTALL_CONFIG_PARAM_H__ */
