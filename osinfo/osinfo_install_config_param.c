/*
 * libosinfo:
 *
 * Copyright (C) 2009-2012 Red Hat, Inc.
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *   Fabiano Fidêncio <fabiano@fidencio.org>
 */

#include <config.h>

#include <osinfo/osinfo.h>

G_DEFINE_TYPE (OsinfoInstallConfigParam, osinfo_install_config_param, OSINFO_TYPE_ENTITY);

#define OSINFO_INSTALL_CONFIG_PARAM_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), OSINFO_TYPE_INSTALL_CONFIG_PARAM, OsinfoInstallConfigParamPrivate))

/**
 * SECTION:osinfo_install_config_param
 * @short_description: OS install configuration parameters (and its policies)
 * @see_also: #OsinfoInstallScript, #OsinfoInstallSciptConfig
 *
 * #OsinfoInstallConfigParam is an entity for representing all parameters that
 * can be set in an automated installation. It is used to help applications to
 * generate an automated installation script
 */

struct _OsinfoInstallConfigParamPrivate
{
    gchar *name;
    OsinfoInstallConfigParamPolicy policy;
};

enum {
    PROP_0,

    PROP_NAME,
    PROP_POLICY,
};

static void
osinfo_install_config_param_set_property(GObject *object,
                                         guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
    OsinfoInstallConfigParam *config_param =
        OSINFO_INSTALL_CONFIG_PARAM (object);

    switch (property_id)
        {
        case PROP_NAME:
            config_param->priv->name = g_value_dup_string(value);
            break;
        case PROP_POLICY:
            {
            const gchar *policy = g_value_get_string(value);
            if (g_strcmp0(policy, "required") == 0)
                config_param->priv->policy =
                    OSINFO_INSTALL_CONFIG_PARAM_POLICY_REQUIRED;
            else if (g_strcmp0(policy, "optional") == 0)
                config_param->priv->policy =
                    OSINFO_INSTALL_CONFIG_PARAM_POLICY_OPTIONAL;
            break;
            }
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
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
        OSINFO_INSTALL_CONFIG_PARAM (object);

    switch (property_id)
        {
        case PROP_NAME:
            g_value_set_string(value, config_param->priv->name);
            break;
        case PROP_POLICY:
            g_value_set_enum(value, config_param->priv->policy);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
        }
}


static void
osinfo_install_config_param_finalize(GObject *object)
{
    OsinfoInstallConfigParam *config_param =
        OSINFO_INSTALL_CONFIG_PARAM(object);
    g_free(config_param->priv->name);

    /* Chain up to the parent class */
    G_OBJECT_CLASS (osinfo_install_config_param_parent_class)->finalize (object);
}

/* Init functions */
static void
osinfo_install_config_param_class_init (OsinfoInstallConfigParamClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_klass->set_property = osinfo_install_config_param_set_property;
    g_klass->get_property = osinfo_install_config_param_get_property;

    /**
     * OsinfoInstallConfigParam:name:
     *
     * The name of the configuration parameter.
     **/
    pspec = g_param_spec_string("name",
                                "Name",
                                "Parameter name",
                                NULL,
                                G_PARAM_WRITABLE |
                                G_PARAM_READABLE |
                                G_PARAM_CONSTRUCT_ONLY |
                                G_PARAM_STATIC_NAME |
                                G_PARAM_STATIC_NICK |
                                G_PARAM_STATIC_BLURB);
    g_object_class_install_property(g_klass,
                                    PROP_NAME,
                                    pspec);
    /**
     * OsinfoInstallConfigParam:policy:
     *
     * The policy of the configuration parameter.
     **/
    pspec = g_param_spec_string("policy",
                                "Policy",
                                "Parameter policy",
                                NULL,
                                G_PARAM_WRITABLE |
                                G_PARAM_READABLE |
                                G_PARAM_CONSTRUCT_ONLY |
                                G_PARAM_STATIC_NAME |
                                G_PARAM_STATIC_NICK |
                                G_PARAM_STATIC_BLURB);
    g_object_class_install_property(g_klass,
                                    PROP_POLICY,
                                    pspec);


    g_klass->finalize = osinfo_install_config_param_finalize;

    g_type_class_add_private (klass, sizeof (OsinfoInstallConfigParamPrivate));
}

static void
osinfo_install_config_param_init (OsinfoInstallConfigParam *config_param)
{
    OsinfoInstallConfigParamPrivate *priv;
    config_param->priv = priv =
        OSINFO_INSTALL_CONFIG_PARAM_GET_PRIVATE(config_param);

    config_param->priv->name = NULL;
    config_param->priv->policy = OSINFO_INSTALL_CONFIG_PARAM_POLICY_NONE;
}


/**
 * osinfo_install_config_param_new:
 * @name: the configuration parameter name
 * @policy: the configuration parameter policy
 *
 * Construct a new configuration parameter to a #OsinfoInstallScript.
 *
 * Returns: (transfer full): the new configuration parameter
 */
OsinfoInstallConfigParam *osinfo_install_config_param_new(const gchar *name,
                                                          const gchar *policy)
{
    return g_object_new(OSINFO_TYPE_INSTALL_CONFIG_PARAM,
                        "name", name,
                        "policy", policy,
                        NULL);
}

/**
 * osinfo_install_config_param_get_name:
 * @config_param: the configuration parameter
 *
 * Returns: (transfer none): the name of the configuration parameter
 */
const gchar *osinfo_install_config_param_get_name(const OsinfoInstallConfigParam *config_param)
{
    return config_param->priv->name;
}

/**
 * osinfo_install_config_param_get_policy:
 * @config_param: the configuration parameter
 *
 * Returns: (transfer none): the policy of the configuration parameter
 */
OsinfoInstallConfigParamPolicy osinfo_install_config_param_get_policy(const OsinfoInstallConfigParam *config_param)
{
    return config_param->priv->policy;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */