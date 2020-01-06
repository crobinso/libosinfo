/*
 * libosinfo:
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

#include <osinfo/osinfo.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_install_config_param
 * @short_description: OS install configuration parameters (and its policies)
 * @see_also: #OsinfoInstallScript, #OsinfoInstallConfig
 *
 * #OsinfoInstallConfigParam is an entity for describing all parameters that
 * can be set in an automated installation. It is used to help applications to
 * generate an automated installation script. The actual parameter values
 * for an #OsinfoInstallScript must be set using an #OsinfoInstallConfig
 * object.
 */

struct _OsinfoInstallConfigParamPrivate
{
    OsinfoDatamap *value_map;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoInstallConfigParam, osinfo_install_config_param, OSINFO_TYPE_ENTITY);

enum {
    PROP_0,

    PROP_NAME,
    PROP_POLICY,
    PROP_VALUE_MAP,

    LAST_PROP
};
static GParamSpec *properties[LAST_PROP];

static void
osinfo_install_config_param_set_property(GObject *object,
                                         guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
    OsinfoInstallConfigParam *config_param =
        OSINFO_INSTALL_CONFIG_PARAM(object);

    switch (property_id) {
    case PROP_NAME:
        osinfo_entity_set_param(OSINFO_ENTITY(config_param),
                                OSINFO_INSTALL_CONFIG_PARAM_PROP_NAME,
                                g_value_get_string(value));
        break;

    case PROP_VALUE_MAP:
        osinfo_install_config_param_set_value_map(config_param,
                                                  g_value_get_object(value));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_install_config_param_get_property(GObject *object,
                                         guint property_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
    OsinfoInstallConfigParam *config_param =
        OSINFO_INSTALL_CONFIG_PARAM(object);

    switch (property_id) {
    case PROP_NAME:
    {
        const gchar *name;

        name = osinfo_install_config_param_get_name(config_param);
        g_value_set_string(value, name);
        break;
    }
    case PROP_POLICY:
    {
        OsinfoInstallConfigParamPolicy policy;

        policy = osinfo_install_config_param_get_policy(config_param);
        g_value_set_enum(value, policy);
        break;
    }
    case PROP_VALUE_MAP:
        g_value_set_object(value, config_param->priv->value_map);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_install_config_param_finalize(GObject *object)
{
    OsinfoInstallConfigParam *config_param;
    config_param = OSINFO_INSTALL_CONFIG_PARAM(object);
    g_clear_object(&config_param->priv->value_map);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_install_config_param_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_install_config_param_class_init(OsinfoInstallConfigParamClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->set_property = osinfo_install_config_param_set_property;
    g_klass->get_property = osinfo_install_config_param_get_property;
    g_klass->finalize = osinfo_install_config_param_finalize;

    /**
     * OsinfoInstallConfigParam:name:
     *
     * The name of the configuration parameter.
     **/
    properties[PROP_NAME] = g_param_spec_string("name",
                                                "Name",
                                                _("Parameter name"),
                                                NULL,
                                                G_PARAM_WRITABLE |
                                                G_PARAM_READABLE |
                                                G_PARAM_CONSTRUCT_ONLY |
                                                G_PARAM_STATIC_STRINGS);
    /**
     * OsinfoInstallConfigParam:policy:
     *
     * The policy of the configuration parameter.
     **/
    properties[PROP_POLICY] = g_param_spec_enum("policy",
                                                "Policy",
                                                _("Parameter policy"),
                                                OSINFO_TYPE_INSTALL_CONFIG_PARAM_POLICY,
                                                OSINFO_INSTALL_CONFIG_PARAM_POLICY_OPTIONAL,
                                                G_PARAM_READABLE |
                                                G_PARAM_STATIC_STRINGS);
    /**
     * OsinfoInstallConfigParam:value-map:
     *
     * The mapping between generic values and OS-specific values for this
     * configuration parameter
     **/
    properties[PROP_VALUE_MAP] = g_param_spec_object("value-map",
                                                     "Value Mapping",
                                                     _("Parameter Value Mapping"),
                                                     OSINFO_TYPE_DATAMAP,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(g_klass, LAST_PROP, properties);
}

static void
osinfo_install_config_param_init(OsinfoInstallConfigParam *config_param)
{
    config_param->priv = osinfo_install_config_param_get_instance_private(config_param);
}

/**
 * osinfo_install_config_param_new:
 * @name: the configuration parameter name
 *
 * Construct a new configuration parameter for an #OsinfoInstallScript.
 *
 * Returns: (transfer full): the new configuration parameter
 *
 * Since: 0.2.0
 */
OsinfoInstallConfigParam *osinfo_install_config_param_new(const gchar *name)
{
    return g_object_new(OSINFO_TYPE_INSTALL_CONFIG_PARAM,
                        "id", name,
                        "name", name,
                        NULL);
}

/**
 * osinfo_install_config_param_get_name:
 * @config_param: the configuration parameter
 *
 * Returns: (transfer none): the name of the configuration parameter
 *
 * Since: 0.2.0
 */
const gchar *osinfo_install_config_param_get_name(OsinfoInstallConfigParam *config_param)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(config_param),
                                         OSINFO_INSTALL_CONFIG_PARAM_PROP_NAME);
}

/**
 * osinfo_install_config_param_get_policy:
 * @config_param: the configuration parameter
 *
 * Returns: the policy of the configuration parameter
 *
 * Since: 0.2.0
 */
OsinfoInstallConfigParamPolicy osinfo_install_config_param_get_policy(OsinfoInstallConfigParam *config_param)
{
    return osinfo_entity_get_param_value_enum(OSINFO_ENTITY(config_param),
                                              OSINFO_INSTALL_CONFIG_PARAM_PROP_POLICY,
                                              OSINFO_TYPE_INSTALL_CONFIG_PARAM_POLICY,
                                              OSINFO_INSTALL_CONFIG_PARAM_POLICY_OPTIONAL);
}

/**
 * osinfo_install_config_param_is_required:
 * @config_param: the configuration parameter
 *
 * Returns: TRUE if the config_param is required. FALSE otherwise.
 *
 * Since: 0.2.1
 */
gboolean osinfo_install_config_param_is_required(OsinfoInstallConfigParam *config_param)
{
    return (osinfo_install_config_param_get_policy(config_param) ==
            OSINFO_INSTALL_CONFIG_PARAM_POLICY_REQUIRED);
}

/**
 * osinfo_install_config_param_is_optional:
 * @config_param: the configuration parameter
 *
 * Returns: TRUE if the config_param is optional. FALSE otherwise.
 *
 * Since: 0.2.1
 */
gboolean osinfo_install_config_param_is_optional(OsinfoInstallConfigParam *config_param)
{
    return (osinfo_install_config_param_get_policy(config_param) ==
            OSINFO_INSTALL_CONFIG_PARAM_POLICY_OPTIONAL);
}

/**
 * osinfo_install_config_param_get_value_map:
 * @config_param: the configuration parameter
 *
 * Returns: (transfer none): The data map used to transform values set for this
 *                           parameter to OS-specific values, or NULL.
 *
 * Since: 0.2.8
 */
OsinfoDatamap *osinfo_install_config_param_get_value_map(OsinfoInstallConfigParam *config_param)
{
    return config_param->priv->value_map;
}

/**
 * osinfo_install_config_param_set_value_map:
 * @config_param: the configuration parameter
 * @datamap: a #OsinfoDatamap to transform values this parameter is set to,
 * or NULL to disable transformations for this parameter
 *
 * After a call to osinfo_install_config_param_set_value_map(), @datamap will
 * be used to transform values set for this parameter to OS-specific
 * values. A NULL @datamap will disable transformations.
 *
 * Since: 0.2.8
 */
void osinfo_install_config_param_set_value_map(OsinfoInstallConfigParam *config_param, OsinfoDatamap *datamap)
{
    g_return_if_fail(OSINFO_IS_INSTALL_CONFIG_PARAM(config_param));

    if (config_param->priv->value_map != NULL)
        g_object_unref(G_OBJECT(config_param->priv->value_map));
    config_param->priv->value_map = g_object_ref(datamap);
}
