/*
 * libosinfo:
 *
 * Copyright (C) 2009-2012, 2014 Red Hat, Inc.
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
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xsltInternals.h>
#include <glib/gi18n-lib.h>
#include "osinfo_install_script_private.h"

G_DEFINE_TYPE(OsinfoInstallScript, osinfo_install_script, OSINFO_TYPE_ENTITY);

#define OSINFO_INSTALL_SCRIPT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OSINFO_TYPE_INSTALL_SCRIPT, OsinfoInstallScriptPrivate))

/**
 * SECTION:osinfo_install_script
 * @short_description: OS install script generation
 * @see_also: #OsinfoInstallConfig
 *
 * #OsinfoInstallScript is an object used to generate an
 * automated installation script for an OS. The OS
 * configuration data (language, keyboard, timezone, ...)
 * comes from an #OsinfoInstallConfig object.
 */

struct _OsinfoInstallScriptPrivate
{
    gchar *output_prefix;
    gchar *output_filename;
    OsinfoInstallConfigParamList *config_params;
    OsinfoAvatarFormat *avatar;
};

enum {
    PROP_0,

    PROP_TEMPLATE_URI,
    PROP_TEMPLATE_DATA,
    PROP_PROFILE,
    PROP_PRODUCT_KEY_FORMAT,
    PROP_PATH_FORMAT,
    PROP_AVATAR_FORMAT,
    PROP_PREFERRED_INJECTION_METHOD,
    PROP_INSTALLATION_SOURCE
};

typedef struct _OsinfoInstallScriptGenerateData OsinfoInstallScriptGenerateData;
typedef struct _OsinfoInstallScriptGenerateOutputData OsinfoInstallScriptGenerateOutputData;
typedef struct _OsinfoInstallScriptGenerateSyncData OsinfoInstallScriptGenerateSyncData;


static void
osinfo_install_script_set_property(GObject    *object,
                                   guint       property_id,
                                   const GValue     *value,
                                   GParamSpec *pspec)
{
    OsinfoInstallScript *script = OSINFO_INSTALL_SCRIPT(object);
    const gchar *data;

    switch (property_id) {
    case PROP_TEMPLATE_URI:
        data = g_value_get_string(value);
        if (data)
            osinfo_entity_set_param(OSINFO_ENTITY(script),
                                    OSINFO_INSTALL_SCRIPT_PROP_TEMPLATE_URI,
                                    data);
        break;

    case PROP_TEMPLATE_DATA:
        data = g_value_get_string(value);
        if (data)
            osinfo_entity_set_param(OSINFO_ENTITY(script),
                                    OSINFO_INSTALL_SCRIPT_PROP_TEMPLATE_DATA,
                                    data);
        break;

    case PROP_PROFILE:
        data = g_value_get_string(value);
        if (data)
            osinfo_entity_set_param(OSINFO_ENTITY(script),
                                    OSINFO_INSTALL_SCRIPT_PROP_PROFILE,
                                    data);
        break;

    case PROP_PREFERRED_INJECTION_METHOD:
        osinfo_install_script_set_preferred_injection_method(script,
                                                             g_value_get_flags(value));
        break;

    case PROP_INSTALLATION_SOURCE:
        osinfo_install_script_set_installation_source(script,
                                                      g_value_get_enum(value));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_install_script_get_property(GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    OsinfoInstallScript *script = OSINFO_INSTALL_SCRIPT(object);

    switch (property_id) {
    case PROP_TEMPLATE_URI:
        g_value_set_string(value,
                           osinfo_install_script_get_template_uri(script));
        break;

    case PROP_TEMPLATE_DATA:
        g_value_set_string(value,
                           osinfo_install_script_get_template_data(script));
        break;

    case PROP_PROFILE:
        g_value_set_string(value,
                           osinfo_install_script_get_profile(script));
        break;

    case PROP_PRODUCT_KEY_FORMAT:
        g_value_set_string(value,
                           osinfo_install_script_get_product_key_format(script));
        break;

    case PROP_PATH_FORMAT:
        g_value_set_enum(value,
                         osinfo_install_script_get_path_format(script));
        break;

    case PROP_AVATAR_FORMAT:
        g_value_set_object(value,
                           osinfo_install_script_get_avatar_format(script));
        break;

    case PROP_PREFERRED_INJECTION_METHOD:
        g_value_set_flags(value,
                          osinfo_install_script_get_preferred_injection_method(script));
        break;

    case PROP_INSTALLATION_SOURCE:
        g_value_set_enum(value,
                         osinfo_install_script_get_installation_source(script));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}


static void
osinfo_install_script_finalize(GObject *object)
{
    OsinfoInstallScript *script = OSINFO_INSTALL_SCRIPT(object);
    g_free(script->priv->output_prefix);
    g_free(script->priv->output_filename);

    if (script->priv->config_params)
        g_object_unref(script->priv->config_params);

    if (script->priv->avatar)
        g_object_unref(script->priv->avatar);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_install_script_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_install_script_class_init(OsinfoInstallScriptClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);
    GParamSpec *pspec;

    g_klass->get_property = osinfo_install_script_get_property;
    g_klass->set_property = osinfo_install_script_set_property;
    g_klass->finalize = osinfo_install_script_finalize;

    pspec = g_param_spec_string("template-uri",
                                "TemplateURI",
                                _("URI for install script template"),
                                NULL /* default value */,
                                G_PARAM_READABLE |
                                G_PARAM_WRITABLE |
                                G_PARAM_CONSTRUCT_ONLY |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_TEMPLATE_URI,
                                    pspec);

    pspec = g_param_spec_string("template-data",
                                "TemplateData",
                                _("Data for install script template"),
                                NULL /* default value */,
                                G_PARAM_READABLE |
                                G_PARAM_WRITABLE |
                                G_PARAM_CONSTRUCT_ONLY |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_TEMPLATE_DATA,
                                    pspec);

    pspec = g_param_spec_string("profile",
                                "Profile",
                                _("Install script profile name"),
                                NULL /* default value */,
                                G_PARAM_READABLE |
                                G_PARAM_WRITABLE |
                                G_PARAM_CONSTRUCT_ONLY |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_PROFILE,
                                    pspec);

    pspec = g_param_spec_string("product-key-format",
                                "Product Key Format",
                                _("Product key format mask"),
                                NULL /* default value */,
                                G_PARAM_READABLE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_PRODUCT_KEY_FORMAT,
                                    pspec);

    pspec = g_param_spec_enum("path-format",
                              "Path Format",
                              _("Expected path format"),
                              OSINFO_TYPE_PATH_FORMAT,
                              OSINFO_PATH_FORMAT_UNIX /* default value */,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_PATH_FORMAT,
                                    pspec);

    pspec = g_param_spec_object("avatar-format",
                                "Avatar Format",
                                _("Expected avatar format"),
                                OSINFO_TYPE_AVATAR_FORMAT,
                                G_PARAM_READABLE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_AVATAR_FORMAT,
                                    pspec);

    pspec = g_param_spec_flags("preferred-injection-method",
                               "Preferred Injection Method",
                               _("The preferred injection method"),
                               OSINFO_TYPE_INSTALL_SCRIPT_INJECTION_METHOD,
                               OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK, /* default value */
                               G_PARAM_READABLE |
                               G_PARAM_WRITABLE |
                               G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_PREFERRED_INJECTION_METHOD,
                                    pspec);

    pspec = g_param_spec_enum("installation-source",
                              "Installation Source",
                              _("The installation source to be used"),
                              OSINFO_TYPE_INSTALL_SCRIPT_INSTALLATION_SOURCE,
                              OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_MEDIA,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass,
                                    PROP_INSTALLATION_SOURCE,
                                    pspec);

    g_type_class_add_private(klass, sizeof(OsinfoInstallScriptPrivate));
}

void osinfo_install_script_add_config_param(OsinfoInstallScript *script, OsinfoInstallConfigParam *param)
{
    g_return_if_fail(OSINFO_IS_INSTALL_SCRIPT(script));
    g_return_if_fail(OSINFO_IS_INSTALL_CONFIG_PARAM(param));

    osinfo_list_add(OSINFO_LIST(script->priv->config_params),
                    OSINFO_ENTITY(param));
}

/**
 * osinfo_install_script_has_config_param:
 * @script: the install script
 * @config_param: an #OsinfoInstallConfigParam
 *
 * Returns whether the @script has the @config_param searched or not.
 *
 * This code assumes that the 'id' and 'name' entity properties are the same.
 *
 * Since: 0.2.0
 */
gboolean osinfo_install_script_has_config_param(OsinfoInstallScript *script, OsinfoInstallConfigParam *config_param)
{
    const char *name = osinfo_install_config_param_get_name(config_param);
    return osinfo_install_script_has_config_param_name(script, name);
}

/**
 * osinfo_install_script_has_config_param_name:
 * @script: the install script
 * @name: the configuration parameter name
 *
 * Returns whether the @script has a configuration parameter matching @name or not.
 *
 * Since: 0.2.0
 */
gboolean osinfo_install_script_has_config_param_name(OsinfoInstallScript *script, const gchar *name)
{
    OsinfoList *l = OSINFO_LIST(script->priv->config_params);
    return (osinfo_list_find_by_id(l, name) != NULL);
}

/**
 * osinfo_install_script_get_config_param_list:
 * @script: the install script
 *
 * Get the list of valid config parameters for @script.
 *
 * Returns: (transfer container) (element-type OsinfoInstallScript): the
 * list of valid #OsinfoInstallConfigParam parameters. Free with
 * g_list_free() when done. The elements are owned by libosinfo.
 *
 * Since: 0.2.0
 */
GList *osinfo_install_script_get_config_param_list(OsinfoInstallScript *script)
{
    return osinfo_list_get_elements(OSINFO_LIST(script->priv->config_params));
}

/**
 * osinfo_install_script_get_config_params:
 * @script: the install script
 *
 * Get the list of valid config parameters for @script.
 *
 * Returns: (transfer none): the list of valid #OsinfoInstallConfigParam
 * parameters.
 *
 * Since: 0.2.3
 */
OsinfoInstallConfigParamList *osinfo_install_script_get_config_params(OsinfoInstallScript *script)
{
    return script->priv->config_params;
}

/**
 * osinfo_install_script_get_config_param:
 * @script: the install script
 * @name: name of the parameter
 *
 * Get a config param from the config param's list
 *
 * Returns: (transfer full): the sought config param, if exists.
 *                           NULL otherwise.
 *
 * This code assumes that the 'id' and 'name' entity properties are
 * the same.
 *
 * Since: 0.2.1
 */
OsinfoInstallConfigParam *
osinfo_install_script_get_config_param(OsinfoInstallScript *script,
                                       const gchar *name)
{
    OsinfoInstallConfigParam *param;
    OsinfoList *l = OSINFO_LIST(script->priv->config_params);
    param = OSINFO_INSTALL_CONFIG_PARAM(osinfo_list_find_by_id(l, name));
    if (param == NULL)
        return NULL;

    return g_object_ref(G_OBJECT(param));
}

static void
osinfo_install_script_init(OsinfoInstallScript *list)
{
    list->priv = OSINFO_INSTALL_SCRIPT_GET_PRIVATE(list);
    list->priv->config_params = osinfo_install_config_paramlist_new();
}


/**
 * oisinfo_install_script_new:
 * @id: a unique identifier
 *
 * Construct an empty new install script.
 *
 * Returns: (transfer full): a new install script
 *
 * Since: 0.2.0
 */
OsinfoInstallScript *osinfo_install_script_new(const gchar *id)
{
    return g_object_new(OSINFO_TYPE_INSTALL_SCRIPT,
                        "id", id,
                        NULL);
}

/**
 * osinfo_install_script_new_data:
 * @id: a unique identifier
 * @profile: the profile of script
 * @templateData: style sheet data
 *
 * Construct a new install script from stylesheet data
 *
 * Returns: (transfer full): an new install script
 *
 * Since: 0.2.0
 */
OsinfoInstallScript *osinfo_install_script_new_data(const gchar *id,
                                                    const gchar *profile,
                                                    const gchar *templateData)
{
    return g_object_new(OSINFO_TYPE_INSTALL_SCRIPT,
                        "id", id,
                        "profile", profile,
                        "template-data", templateData,
                        NULL);
}


/**
 * osinfo_install_script_new_uri:
 * @id: a unique identifier
 * @profile: the profile of script
 * @templateUri: style sheet URI
 *
 * Construct a new install script from a stylesheet URI
 *
 * Returns: (transfer full): an new install script
 *
 * Since: 0.2.0
 */
OsinfoInstallScript *osinfo_install_script_new_uri(const gchar *id,
                                                   const gchar *profile,
                                                   const gchar *templateUri)
{
    return g_object_new(OSINFO_TYPE_INSTALL_SCRIPT,
                        "id", id,
                        "profile", profile,
                        "template-uri", templateUri,
                        NULL);
}


/**
 * osinfo_install_script_get_template_uri:
 * @script: the install script
 *
 * Returns the stylesheet URI used to construct the install script.
 *
 * Since: 0.2.0
 */
const gchar *osinfo_install_script_get_template_uri(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(script),
                                         OSINFO_INSTALL_SCRIPT_PROP_TEMPLATE_URI);
}

/**
 * osinfo_install_script_get_template_data:
 * @script: the install script
 *
 * Returns the stylesheet data used to construct the install script.
 *
 * Since: 0.2.0
 */
const gchar *osinfo_install_script_get_template_data(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(script),
                                         OSINFO_INSTALL_SCRIPT_PROP_TEMPLATE_DATA);
}

/**
 * osinfo_install_script_get_profile:
 * @script: the install script
 *
 * Returns a string representing the install script profile that's going to be
 * used.
 *
 * The values supported are "jeos" for minimal installations and "desktop" for
 * workstation/desktop installations.
 *
 * Since: 0.2.0
 */
const gchar *osinfo_install_script_get_profile(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(script),
                                         OSINFO_INSTALL_SCRIPT_PROP_PROFILE);
}

/**
 * osinfo_install_script_get_product_key_format:
 * @script: the install script
 *
 * If this function returns a non-NULL string, it means that the @script
 * requires you to specify product registration key through #OsinfoInstallConfig
 * instance passed to script generation methods.
 *
 * The returned string specifies the expected format of the product key like this:
 *
 * @ - any character
 * % - alphabet
 * # - numeric character
 * $ - alphanumeric character
 *
 * All other characters represent themselves.
 *
 * For example in case of installer for Microsoft Windows XP, you'll get
 * "$$$$$-$$$$$-$$$$$-$$$$$-$$$$$". That means a product key consists of 24
 * alphanumeric characters and 4 '-' characters at (0-based) indices 5, 11, 17
 * and 23.
 *
 * Returns: (transfer none): Product key format mask, or NULL.
 *
 * Since: 0.2.2
 */
const gchar *osinfo_install_script_get_product_key_format(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(script),
                                         OSINFO_INSTALL_SCRIPT_PROP_PRODUCT_KEY_FORMAT);
}

/**
 * osinfo_install_script_set_output_prefix:
 * @script: the install script
 * @prefix: a prefix to be added to the file generated
 *
 * Mind that not all installers support any name for the installer scripts.
 *
 * Since: 0.2.0
 */
void osinfo_install_script_set_output_prefix(OsinfoInstallScript *script,
                                             const gchar *prefix)
{
    const char *output_filename =
        osinfo_install_script_get_expected_filename(script);

    g_free(script->priv->output_prefix);
    script->priv->output_prefix = g_strdup(prefix);

    /* update output_filename whenever output_prefix is changed */
    g_free(script->priv->output_filename);
    script->priv->output_filename = g_strjoin("-",
                                              prefix,
                                              output_filename,
                                              NULL);
}

/**
 * osinfo_install_script_get_output_prefix:
 * @script: the install script
 *
 * Returns: the prefix of the file generated
 *
 * Since: 0.2.8
 */
const gchar *osinfo_install_script_get_output_prefix(OsinfoInstallScript *script)
{
    return script->priv->output_prefix;
}

/**
 * osinfo_install_script_get_expected_filename:
 * @script: the install script
 *
 * Some operating systems (as Windows) expect that script filename has
 * particular name to work.
 *
 * Returns: (transfer none): the expected script filename
 *
 * Since: 0.2.1
 */
const gchar *osinfo_install_script_get_expected_filename(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(script),
                                         OSINFO_INSTALL_SCRIPT_PROP_EXPECTED_FILENAME);
}

/**
 * osinfo_install_script_get_output_filename:
 * @script: the install script
 *
 * Some operating systems are able to use any script filename, allowing the
 * application to set the filename as desired. libosinfo provides this
 * functionality by set the expected filename's prefix using
 * osinfo_install_script_set_output_prefix() function.
 *
 * Returns: (transfer none): the output script filename
 *
 * Since: 0.2.0
 */
const gchar *osinfo_install_script_get_output_filename(OsinfoInstallScript *script)
{
    if (script->priv->output_filename == NULL)
        return osinfo_install_script_get_expected_filename(script);

    return script->priv->output_filename;
}

void
osinfo_install_script_set_avatar_format(OsinfoInstallScript *script,
                                        OsinfoAvatarFormat *avatar)
{
    g_return_if_fail(OSINFO_IS_INSTALL_SCRIPT(script));
    g_return_if_fail(OSINFO_IS_AVATAR_FORMAT(avatar));

    if (script->priv->avatar != NULL)
        g_object_unref(script->priv->avatar);
    script->priv->avatar = g_object_ref(avatar);
}

/**
 * osinfo_install_script_get_avatar_format:
 * @script: the install script
 *
 * Some install scripts have restrictions on the format of the user avatar. Use
 * this method to retrieve those restrictions in the form of an
 * #OsinfoAvatarFormat instance.
 *
 * Returns: (transfer none): The avatar format, or NULL if there is no restrictions on the
 *                           format of avatar
 *
 * Since: 0.2.2
 */
OsinfoAvatarFormat *osinfo_install_script_get_avatar_format(OsinfoInstallScript *script)
{
    g_return_val_if_fail(OSINFO_IS_INSTALL_SCRIPT(script), NULL);

    if (script->priv->avatar == NULL)
        return NULL;

    return script->priv->avatar;
}

struct _OsinfoInstallScriptGenerateData {
    GTask *res;
    OsinfoOs *os;
    OsinfoMedia *media;
    OsinfoTree *tree;
    OsinfoInstallConfig *config;
    OsinfoInstallScript *script;
};


static void osinfo_install_script_generate_data_free(OsinfoInstallScriptGenerateData *data)
{
    g_object_unref(data->os);
    if (data->media != NULL)
        g_object_unref(data->media);
    if (data->tree != NULL)
        g_object_unref(data->tree);
    g_object_unref(data->config);
    g_object_unref(data->script);
    g_object_unref(data->res);
    g_free(data);
}

struct _OsinfoInstallScriptGenerateOutputData {
    GTask *res;
    GError *error;
    GFile *file;
    GFileOutputStream *stream;
    gchar *output;
    gssize output_len;
    gssize output_pos;
};

static void osinfo_install_script_generate_output_data_free(OsinfoInstallScriptGenerateOutputData *data)
{
    g_object_unref(data->stream);
    g_object_unref(data->res);
    g_free(data);
}


static xsltStylesheetPtr osinfo_install_script_load_template(const gchar *uri,
                                                             const gchar *template,
                                                             GError **error)
{
    xsltStylesheetPtr xslt = NULL;
    xmlDocPtr doc = NULL;
    xmlParserCtxtPtr pctxt;

    /* Set up a parser context so we can catch the details of XML errors. */
    pctxt = xmlNewParserCtxt();
    if (!pctxt || !pctxt->sax) {
        g_set_error_literal(error, OSINFO_ERROR, 0,
                            _("Unable to create XML parser context"));
        goto cleanup;
    }

    if (!(doc = xmlCtxtReadDoc(pctxt, BAD_CAST template, uri, NULL,
                               XML_PARSE_NOENT | XML_PARSE_NONET |
                               XML_PARSE_NOWARNING))) {
        g_set_error_literal(error, OSINFO_ERROR, 0,
                            _("Unable to read XSL template"));
        goto cleanup;
    }

    if (!(xslt = xsltParseStylesheetDoc(doc))) {
        g_set_error_literal(error, OSINFO_ERROR, 0,
                            _("Unable to parse XSL template"));
        goto cleanup;
    }

 cleanup:
    xmlFreeParserCtxt(pctxt);
    return xslt;
}


static OsinfoDatamap *
osinfo_install_script_get_param_datamap(OsinfoInstallScript *script,
                                        const gchar *param_name)
{
    OsinfoEntity *entity;
    OsinfoInstallConfigParam *param;

    if (!script->priv->config_params)
        return NULL;

    entity = osinfo_list_find_by_id(OSINFO_LIST(script->priv->config_params),
                                    param_name);
    if (entity == NULL) {
        g_debug("%s is not a known parameter for this config", param_name);
        return NULL;
    }

    param = OSINFO_INSTALL_CONFIG_PARAM(entity);
    return osinfo_install_config_param_get_value_map(param);
}


static GList *
osinfo_install_script_get_param_value_list(OsinfoInstallScript *script,
                                           OsinfoInstallConfig *config,
                                           const gchar *key)
{
    GList *values;
    GList *it;
    OsinfoDatamap *map;

    values = osinfo_entity_get_param_value_list(OSINFO_ENTITY(config), key);
    if (values == NULL)
        return NULL;

    map = osinfo_install_script_get_param_datamap(script, key);
    if (map != NULL) {
        for (it = values; it != NULL; it = it->next) {
            const char *transformed_value;
            transformed_value = osinfo_datamap_lookup(map, it->data);
            if (transformed_value == NULL) {
                continue;
            }
            it->data = (gpointer)transformed_value;
        }
    }

    return values;
}

static void propagate_libxml_error(GError **error, const char *format, ...) G_GNUC_PRINTF(2, 3);

static void propagate_libxml_error(GError **error, const char *format, ...)
{
    xmlErrorPtr err = xmlGetLastError();
    char *prefix;
    va_list ap;

    va_start(ap, format);
    prefix = g_strdup_vprintf(format, ap);
    va_end(ap);

    if (err == NULL) {
        g_set_error_literal(error, OSINFO_ERROR, 0, prefix);
    } else {
        g_set_error(error, OSINFO_ERROR, 0, "%s: %s", prefix, err->message);
    }
    g_free(prefix);
}

static xmlNodePtr osinfo_install_script_generate_entity_xml(OsinfoInstallScript *script,
                                                            OsinfoEntity *entity,
                                                            const gchar *name,
                                                            GError **error)
{
    xmlNodePtr node = NULL;
    xmlNodePtr data = NULL;
    GList *keys = NULL;
    GList *tmp1;

    if (!(node = xmlNewDocNode(NULL, NULL, (xmlChar*)name, NULL))) {
        propagate_libxml_error(error, _("Unable to create XML node '%s'"), name);
        goto error;
    }

    if (!(data = xmlNewDocRawNode(NULL, NULL, (const xmlChar*)"id",
                                  (const xmlChar*)osinfo_entity_get_id(entity)))) {
        propagate_libxml_error(error, _("Unable to create XML node 'id'"));
        goto error;
    }
    if (!(xmlAddChild(node, data))) {
        propagate_libxml_error(error, _("Unable to add XML child"));
        goto error;
    }
    data = NULL;

    tmp1 = keys = osinfo_entity_get_param_keys(entity);
    while (tmp1) {
        GList *values;
        GList *tmp2;

        if (OSINFO_IS_INSTALL_CONFIG(entity))
            values = osinfo_install_script_get_param_value_list(script,
                                                                OSINFO_INSTALL_CONFIG(entity),
                                                                tmp1->data);
        else
            values = osinfo_entity_get_param_value_list(entity, tmp1->data);

        tmp2 = values;
        while (tmp2) {
            if (!(data = xmlNewDocRawNode(NULL, NULL, (const xmlChar*)tmp1->data,
                                          (const xmlChar*)tmp2->data))) {
                propagate_libxml_error(error, _("Unable to create XML node '%s'"),
                                       (const gchar *)tmp1->data);
                goto error;
            }
            if (!(xmlAddChild(node, data))) {
                propagate_libxml_error(error, _("Unable to add XML child"));
                goto error;
            }
            data = NULL;

            tmp2 = tmp2->next;
        }
        g_list_free(values);

        tmp1 = tmp1->next;
    }
    g_list_free(keys);

    return node;

 error:
    g_list_free(keys);
    xmlFreeNode(data);
    xmlFreeNode(node);
    return NULL;
}


static xmlDocPtr osinfo_install_script_generate_config_xml(OsinfoInstallScript *script,
                                                           OsinfoOs *os,
                                                           OsinfoMedia *media,
                                                           OsinfoTree *tree,
                                                           OsinfoInstallConfig *config,
                                                           const gchar *node_name,
                                                           GError **error)
{
    xmlDocPtr doc = xmlNewDoc((xmlChar *)"1.0");
    xmlNodePtr root;
    xmlNodePtr node;

    root = xmlNewDocNode(NULL,
                         NULL,
                         (xmlChar*)node_name,
                         NULL);
    xmlDocSetRootElement(doc, root);

    if (!(node = osinfo_install_script_generate_entity_xml(script,
                                                           OSINFO_ENTITY(script),
                                                           "script",
                                                           error)))
        goto error;
    if (!(xmlAddChild(root, node))) {
        propagate_libxml_error(error, _("Unable to set XML root"));
        goto error;
    }

    if (!(node = osinfo_install_script_generate_entity_xml(script,
                                                           OSINFO_ENTITY(os),
                                                           "os",
                                                           error)))
        goto error;
    if (!(xmlAddChild(root, node))) {
        propagate_libxml_error(error, _("Unable to set XML root"));
        goto error;
    }

    if (media != NULL) {
        if (!(node = osinfo_install_script_generate_entity_xml(script,
                                                               OSINFO_ENTITY(media),
                                                               "media",
                                                               error)))
            goto error;
        if (!(xmlAddChild(root, node))) {
            propagate_libxml_error(error, _("Unable to set 'media' node"));
            goto error;
        }
    }

    if (tree != NULL) {
        if (!(node = osinfo_install_script_generate_entity_xml(script,
                                                               OSINFO_ENTITY(tree),
                                                               "tree",
                                                               error)))
            goto error;
        if (!(xmlAddChild(root, node))) {
            propagate_libxml_error(error, _("Unable to set 'tree' node"));
            goto error;
        }
    }

    if (!(node = osinfo_install_script_generate_entity_xml(script,
                                                           OSINFO_ENTITY(config),
                                                           "config",
                                                           error)))
        goto error;
    if (!(xmlAddChild(root, node))) {
        propagate_libxml_error(error, _("Unable to set XML root"));
        goto error;
    }

    return doc;

 error:
    xmlFreeDoc(doc);
    doc = NULL;
    return NULL;
}

static gchar *osinfo_install_script_apply_xslt(xsltStylesheetPtr ss,
                                               xmlDocPtr doc,
                                               GError **error)
{
    xsltTransformContextPtr ctxt;
    xmlChar *xsltResult;
    gchar *ret = NULL;
    xmlDocPtr docOut = NULL;
    int len;

    if (!(ctxt = xsltNewTransformContext(ss, doc))) {
        g_set_error_literal(error, OSINFO_ERROR, 0, _("Unable to create XSL transform context"));
        goto cleanup;
    }

    if (!(docOut = xsltApplyStylesheetUser(ss, doc, NULL, NULL, NULL, ctxt))) {
        g_set_error_literal(error, OSINFO_ERROR, 0, _("Unable to apply XSL transform context"));
        goto cleanup;
    }

    if (xsltSaveResultToString(&xsltResult, &len, docOut, ss) < 0) {
        g_set_error_literal(error, OSINFO_ERROR, 0, _("Unable to convert XSL output to string"));
        goto cleanup;
    }
    ret = g_strdup((gchar *)xsltResult);
    xmlFree(xsltResult);

 cleanup:
    xmlFreeDoc(docOut);
    xsltFreeTransformContext(ctxt);
    return ret;
}


static gboolean osinfo_install_script_apply_template(OsinfoInstallScript *script,
                                                     OsinfoOs *os,
                                                     OsinfoMedia *media,
                                                     OsinfoTree *tree,
                                                     const gchar *templateUri,
                                                     const gchar *template,
                                                     const gchar *node_name,
                                                     gchar **result,
                                                     OsinfoInstallConfig *config,
                                                     GError **error)
{
    gboolean ret = FALSE;
    xsltStylesheetPtr templateXsl = osinfo_install_script_load_template(templateUri, template, error);
    xmlDocPtr configXml = osinfo_install_script_generate_config_xml(script, os, media, tree, config, node_name, error);

    if (!templateXsl || !configXml)
        goto cleanup;

    if (!(*result = osinfo_install_script_apply_xslt(templateXsl, configXml, error)))
        goto cleanup;

    ret = TRUE;

 cleanup:
    xsltFreeStylesheet(templateXsl);
    xmlFreeDoc(configXml);
    return ret;
}


static void osinfo_install_script_template_loaded(GObject *src,
                                                  GAsyncResult *res,
                                                  gpointer user_data)
{
    OsinfoInstallScriptGenerateData *data = user_data;
    GError *error = NULL;
    gchar *input = NULL;
    gchar *output = NULL;
    gsize length = 0;
    GFile *file = G_FILE(src);
    gchar *uri = g_file_get_uri(file);

    if (!g_file_load_contents_finish(file,
                                     res,
                                     &input,
                                     &length,
                                     NULL,
                                     &error)) {
        g_prefix_error(&error, _("Failed to load script template %s: "), uri);
        g_task_return_error(data->res, error);
        goto cleanup;
    }


    if (!osinfo_install_script_apply_template(data->script,
                                              data->os,
                                              data->media,
                                              data->tree,
                                              uri,
                                              input,
                                              "install-script-config",
                                              &output,
                                              data->config,
                                              &error)) {
        g_prefix_error(&error, _("Failed to apply script template %s: "), uri);
        g_task_return_error(data->res, error);
        goto cleanup;
    }

    g_task_return_pointer(data->res, output, g_free);

 cleanup:
    g_free(input);
    osinfo_install_script_generate_data_free(data);
    g_free(uri);
}


static void osinfo_install_script_generate_async_common(OsinfoInstallScript *script,
                                                        OsinfoOs *os,
                                                        OsinfoMedia *media,
                                                        OsinfoTree *tree,
                                                        OsinfoInstallConfig *config,
                                                        GCancellable *cancellable,
                                                        GAsyncReadyCallback callback,
                                                        gpointer user_data)
{
    OsinfoInstallScriptGenerateData *data;
    const gchar *templateData;
    const gchar *templateUri;

    g_return_if_fail(os != NULL);

    data = g_new0(OsinfoInstallScriptGenerateData, 1);
    templateData = osinfo_install_script_get_template_data(script);
    templateUri = osinfo_install_script_get_template_uri(script);

    data->os = g_object_ref(os);
    if (media != NULL)
        data->media = g_object_ref(media);
    if (tree != NULL)
        data->tree = g_object_ref(tree);
    data->config = g_object_ref(config);
    data->script = g_object_ref(script);
    data->res = g_task_new(G_OBJECT(script),
                           cancellable,
                           callback,
                           user_data);

    if (templateData) {
        GError *error = NULL;
        gchar *output;
        if (!osinfo_install_script_apply_template(script,
                                                  os,
                                                  media,
                                                  NULL,
                                                  "<data>",
                                                  templateData,
                                                  "install-script-config",
                                                  &output,
                                                  data->config,
                                                  &error)) {
            g_prefix_error(&error, "%s", _("Failed to apply script template: "));
            g_task_return_error(data->res, error);
            osinfo_install_script_generate_data_free(data);
            return;
        }
        g_task_return_pointer(data->res, output, g_free);
        osinfo_install_script_generate_data_free(data);
    } else {
        GFile *file = g_file_new_for_uri(templateUri);

        g_file_load_contents_async(file,
                                   cancellable,
                                   osinfo_install_script_template_loaded,
                                   data);
        g_object_unref(file);
    }
}

/**
 * osinfo_install_script_generate_async:
 * @script:     the install script
 * @os:         the os
 * @config:     the install script config
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_install_script_generate(). From the callback,
 * call #osinfo_install_script_generate_finish() to conclude this call and get
 * the generated script.
 *
 * If you are generating the script for a specific media, it is recommended that
 * you use #osinfo_install_script_generate_for_media_async() instead.
 *
 * Since: 0.2.0
 */
void osinfo_install_script_generate_async(OsinfoInstallScript *script,
                                          OsinfoOs *os,
                                          OsinfoInstallConfig *config,
                                          GCancellable *cancellable,
                                          GAsyncReadyCallback callback,
                                          gpointer user_data)
{
    osinfo_install_script_generate_async_common(script,
                                                os,
                                                NULL,
                                                NULL,
                                                config,
                                                cancellable,
                                                callback,
                                                user_data);
}

static gpointer osinfo_install_script_generate_finish_common(OsinfoInstallScript *script,
                                                             GAsyncResult *res,
                                                             GError **error)
{
    GTask *task = G_TASK(res);

    g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    return g_task_propagate_pointer(task, error);
}

/**
 * osinfo_install_script_generate_finish:
 * @script: the install script
 * @res:    a #GAsyncResult
 * @error:  The location where to store any error, or NULL
 *
 * Returns: (transfer full): the generated script, or NULL on error
 *
 * Since: 0.2.0
 */
gchar *osinfo_install_script_generate_finish(OsinfoInstallScript *script,
                                             GAsyncResult *res,
                                             GError **error)
{
    return osinfo_install_script_generate_finish_common(script,
                                                        res,
                                                        error);
}

/**
 * osinfo_install_script_generate_for_media_finish:
 * @script: the install script
 * @res:    a #GAsyncResult
 * @error:  The location where to store any error, or NULL
 *
 * Returns: (transfer full): the generated script, or NULL on error
 *
 * Since: 0.2.12
 */
gchar *osinfo_install_script_generate_for_media_finish(OsinfoInstallScript *script,
                                                       GAsyncResult *res,
                                                       GError **error)
{
    return osinfo_install_script_generate_finish_common(script,
                                                        res,
                                                        error);
}

/**
 * osinfo_install_script_generate_for_tree_finish:
 * @script: the install script
 * @res:    a #GAsyncResult
 * @error:  the location where to story any error, or NULL
 *
 * Returns: (transfer full): the generated script, or NULL or error
 *
 * Since: 1.6.0
 */
gchar *osinfo_install_script_generate_for_tree_finish(OsinfoInstallScript *script,
                                                      GAsyncResult *res,
                                                      GError **error)
{
    return osinfo_install_script_generate_finish_common(script,
                                                        res,
                                                        error);
}

/**
 * osinfo_install_script_generate_output_finish:
 * @script: the install script
 * @res:    a #GAsyncResult
 * @error:  The location where to store any error, or NULL
 *
 * Returns: (transfer full): a file containing the script, or NULL on error
 *
 * Since: 0.2.3
 */
GFile *osinfo_install_script_generate_output_finish(OsinfoInstallScript *script,
                                                    GAsyncResult *res,
                                                    GError **error)
{
    return osinfo_install_script_generate_finish_common(script,
                                                        res,
                                                        error);
}

/**
 * osinfo_install_script_generate_output_for_media_finish:
 * @script: the install script
 * @res:    a #GAsyncResult
 * @error:  the location where to store any error, or NULL
 *
 * Returns: (transfer full): a file containing the script, or NULL on error.
 *
 * Since: 0.2.12
 */
GFile *osinfo_install_script_generate_output_for_media_finish(OsinfoInstallScript *script,
                                                              GAsyncResult *res,
                                                              GError **error)
{
    return osinfo_install_script_generate_finish_common(script,
                                                        res,
                                                        error);
}

/**
 * osinfo_install_script_generate_output_for_tree_finish:
 * @script: the install script
 * @res:    a #GAsyncResult
 * @error:  the location where to store any error, or NULL
 *
 * Returns: (transfer full): a file containing the script, or NULL on error.
 *
 * Since: 1.6.0
 */
GFile *osinfo_install_script_generate_output_for_tree_finish(OsinfoInstallScript *script,
                                                             GAsyncResult *res,
                                                             GError **error)
{
    return osinfo_install_script_generate_finish_common(script,
                                                        res,
                                                        error);
}

struct _OsinfoInstallScriptGenerateSyncData {
    GMainLoop *loop;
    GError *error;
    gchar *output;
    GFile *file;
};

static void osinfo_install_script_generate_output_done(GObject *src,
                                                       GAsyncResult *res,
                                                       gpointer user_data)
{
    OsinfoInstallScriptGenerateSyncData *data = user_data;

    data->file =
        osinfo_install_script_generate_output_finish(OSINFO_INSTALL_SCRIPT(src),
                                                     res,
                                                     &data->error);
    g_main_loop_quit(data->loop);
}

static void osinfo_install_script_generate_output_for_media_done(GObject *src,
                                                                 GAsyncResult *res,
                                                                 gpointer user_data)
{
    OsinfoInstallScriptGenerateSyncData *data = user_data;

    data->file =
        osinfo_install_script_generate_output_for_media_finish(OSINFO_INSTALL_SCRIPT(src),
                                                               res,
                                                               &data->error);
    g_main_loop_quit(data->loop);
}

static void osinfo_install_script_generate_output_for_tree_done(GObject *src,
                                                                 GAsyncResult *res,
                                                                 gpointer user_data)
{
    OsinfoInstallScriptGenerateSyncData *data = user_data;

    data->file =
        osinfo_install_script_generate_output_for_tree_finish(OSINFO_INSTALL_SCRIPT(src),
                                                              res,
                                                              &data->error);
    g_main_loop_quit(data->loop);
}

static void osinfo_install_script_generate_output_close_file(GObject *src,
                                                            GAsyncResult *res,
                                                            gpointer user_data)
{
    OsinfoInstallScriptGenerateOutputData *data = user_data;

    g_output_stream_close_finish(G_OUTPUT_STREAM(src),
                                 res,
                                 &data->error);

    g_task_return_pointer(data->res, data->file, NULL);

    osinfo_install_script_generate_output_data_free(data);
}

static void osinfo_install_script_generate_done(GObject *src,
                                                GAsyncResult *res,
                                                gpointer user_data)
{
    OsinfoInstallScriptGenerateSyncData *data = user_data;

    data->output =
        osinfo_install_script_generate_finish(OSINFO_INSTALL_SCRIPT(src),
                                              res,
                                              &data->error);
    g_main_loop_quit(data->loop);
}

/**
 * osinfo_install_script_generate:
 * @script:     the install script
 * @os:         the os
 * @config:     the install script config
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates an install script.
 *
 * Returns: (transfer full): the script as string.
 *
 * If you are generating the script for a specific media, it is recommended
 * that you use #osinfo_install_script_generate_for_media() instead.
 *
 * If you are generating the script for a specific tree, it is recommended
 * that you use #osinfo_install_script_generate_for_tree() in instead.
 *
 * Since: 0.2.0
 */
gchar *osinfo_install_script_generate(OsinfoInstallScript *script,
                                      OsinfoOs *os,
                                      OsinfoInstallConfig *config,
                                      GCancellable *cancellable,
                                      GError **error)
{
    GMainLoop *loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);
    OsinfoInstallScriptGenerateSyncData data = {
        loop, NULL, NULL, NULL
    };

    osinfo_install_script_generate_async(script,
                                         os,
                                         config,
                                         cancellable,
                                         osinfo_install_script_generate_done,
                                         &data);

    g_main_loop_run(loop);

    if (data.error)
        g_propagate_error(error, data.error);

    g_main_loop_unref(loop);

    return data.output;
}

/**
 * osinfo_install_script_generate_for_media_async:
 * @script:     the install script
 * @media:      the media
 * @config:     the install script config
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_install_script_generate_for_media(). From the
 * callback, call #osinfo_install_script_generate_for_media_finish() to
 * conclude this call and get the generated script.
 *
 * Since: 0.2.12
 */
void osinfo_install_script_generate_for_media_async(OsinfoInstallScript *script,
                                                    OsinfoMedia *media,
                                                    OsinfoInstallConfig *config,
                                                    GCancellable *cancellable,
                                                    GAsyncReadyCallback callback,
                                                    gpointer user_data) {
    OsinfoOs *os;

    g_return_if_fail(media != NULL);

    os = osinfo_media_get_os(media);

    osinfo_install_script_generate_async_common(script,
                                                os,
                                                media,
                                                NULL,
                                                config,
                                                cancellable,
                                                callback,
                                                user_data);

    g_object_unref(os);
}

static void osinfo_install_script_generate_for_media_done(GObject *src,
                                                          GAsyncResult *res,
                                                          gpointer user_data)
{
    OsinfoInstallScriptGenerateSyncData *data = user_data;

    data->output =
        osinfo_install_script_generate_for_media_finish(OSINFO_INSTALL_SCRIPT(src),
                                                        res,
                                                        &data->error);
    g_main_loop_quit(data->loop);
}

/**
 * osinfo_install_script_generate_for_media:
 * @script:     the install script
 * @media:      the media
 * @config:     the install script config
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates an install script. The media @media must have been identified
 * successfully using #osinfo_db_identify_media() before calling this function.
 *
 * Returns: (transfer full): the script as string.
 *
 * Since: 0.2.12
 */
gchar *osinfo_install_script_generate_for_media(OsinfoInstallScript *script,
                                                OsinfoMedia *media,
                                                OsinfoInstallConfig *config,
                                                GCancellable *cancellable,
                                                GError **error)
{
    GMainLoop *loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);
    OsinfoInstallScriptGenerateSyncData data = {
        loop, NULL, NULL, NULL
    };

    osinfo_install_script_generate_for_media_async(script,
                                                   media,
                                                   config,
                                                   cancellable,
                                                   osinfo_install_script_generate_for_media_done,
                                                   &data);

    g_main_loop_run(loop);

    if (data.error)
        g_propagate_error(error, data.error);

    g_main_loop_unref(loop);

    return data.output;
}

/**
 * osinfo_install_script_generate_for_tree_async:
 * @script:     the install script
 * @tree:       the tree
 * @config:     the install script config
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_install_script_generate_for_tree(). From the
 * callback, call #osinfo_install_script_generate_for_tree_finish() to
 * conclude this call and get the generated script.
 *
 * Since: 1.6.0
 */
void osinfo_install_script_generate_for_tree_async(OsinfoInstallScript *script,
                                                    OsinfoTree *tree,
                                                    OsinfoInstallConfig *config,
                                                    GCancellable *cancellable,
                                                    GAsyncReadyCallback callback,
                                                    gpointer user_data) {
    OsinfoOs *os;

    g_return_if_fail(tree != NULL);

    os = osinfo_tree_get_os(tree);

    osinfo_install_script_generate_async_common(script,
                                                os,
                                                NULL,
                                                tree,
                                                config,
                                                cancellable,
                                                callback,
                                                user_data);

    g_object_unref(os);
}

static void osinfo_install_script_generate_for_tree_done(GObject *src,
                                                         GAsyncResult *res,
                                                         gpointer user_data)
{
    OsinfoInstallScriptGenerateSyncData *data = user_data;

    data->output =
        osinfo_install_script_generate_for_tree_finish(OSINFO_INSTALL_SCRIPT(src),
                                                        res,
                                                        &data->error);
    g_main_loop_quit(data->loop);
}

/**
 * osinfo_install_script_generate_for_tree:
 * @script:     the install script
 * @tree:       the tree
 * @config:     the install script config
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates an install script. The tree @tree must have been identified
 * successfully using #osinfo_db_identify_tree() before calling this function.
 *
 * Returns: (transfer full): the script as string.
 *
 * Since: 1.6.0
 */
gchar *osinfo_install_script_generate_for_tree(OsinfoInstallScript *script,
                                               OsinfoTree *tree,
                                               OsinfoInstallConfig *config,
                                               GCancellable *cancellable,
                                               GError **error)
{
    GMainLoop *loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);
    OsinfoInstallScriptGenerateSyncData data = {
        loop, NULL, NULL, NULL
    };

    osinfo_install_script_generate_for_tree_async(script,
                                                   tree,
                                                   config,
                                                   cancellable,
                                                   osinfo_install_script_generate_for_tree_done,
                                                   &data);

    g_main_loop_run(loop);

    if (data.error)
        g_propagate_error(error, data.error);

    g_main_loop_unref(loop);

    return data.output;
}

static void osinfo_install_script_generate_output_write_file(GObject *src,
                                                             GAsyncResult *res,
                                                             gpointer user_data)
{
    OsinfoInstallScriptGenerateOutputData *data = user_data;

    if (data->stream == NULL)
        data->stream = g_file_replace_finish(G_FILE(src), res, &data->error);
    else
        data->output_pos += g_output_stream_write_finish(G_OUTPUT_STREAM(data->stream),
                                                                         res,
                                                                         &data->error);

    if (data->output_pos < data->output_len) {
        g_output_stream_write_async(G_OUTPUT_STREAM(data->stream),
                                    data->output + data->output_pos,
                                    data->output_len - data->output_pos,
                                    G_PRIORITY_DEFAULT,
                                    g_task_get_cancellable(data->res),
                                    osinfo_install_script_generate_output_write_file,
                                    data);

    } else {
        g_output_stream_close_async(G_OUTPUT_STREAM(data->stream),
                                    G_PRIORITY_DEFAULT,
                                    g_task_get_cancellable(data->res),
                                    osinfo_install_script_generate_output_close_file,
                                    data);
    }
}

static void osinfo_install_script_generate_output_async_common(OsinfoInstallScript *script,
                                                               OsinfoOs *os,
                                                               OsinfoMedia *media,
                                                               OsinfoTree *tree,
                                                               OsinfoInstallConfig *config,
                                                               GFile *output_dir,
                                                               GCancellable *cancellable,
                                                               GAsyncReadyCallback callback,
                                                               gpointer user_data)
{
    const gchar *filename;
    const gchar *prefix;
    OsinfoInstallScriptGenerateOutputData *data =
        g_new0(OsinfoInstallScriptGenerateOutputData, 1);

    OsinfoInstallScriptGenerateSyncData *data_sync = user_data;

    data->res = g_task_new(G_OBJECT(script),
                           cancellable,
                           callback,
                           user_data);

    data->error = data_sync->error;
    if (media != NULL) {
        data->output = osinfo_install_script_generate_for_media(script,
                                                                media,
                                                                config,
                                                                cancellable,
                                                                &data->error);
    } else if (tree != NULL) {
        data->output = osinfo_install_script_generate_for_tree(script,
                                                               tree,
                                                               config,
                                                               cancellable,
                                                               &data->error);
    } else {
        data->output = osinfo_install_script_generate(script,
                                                      os,
                                                      config,
                                                      cancellable,
                                                      &data->error);
    }
    data->output_pos = 0;
    data->output_len = strlen(data->output);

    prefix =
        osinfo_install_script_get_output_prefix(script);
    filename =
        osinfo_install_script_get_output_filename(script);

    if (prefix) {
        gchar *output_filename  = g_strdup_printf("%s-%s", prefix, filename);
        data->file = g_file_get_child(output_dir, output_filename);
        g_free(output_filename);
    } else {
        data->file = g_file_get_child(output_dir, filename);
    }

    g_file_replace_async(data->file,
                         NULL,
                         TRUE,
                         G_FILE_CREATE_NONE,
                         G_PRIORITY_DEFAULT,
                         cancellable,
                         osinfo_install_script_generate_output_write_file,
                         data);
}

/**
 * osinfo_install_script_generate_output_async:
 * @script:     the install script
 * @os:         the os
 * @config:     the install script config
 * @output_dir: the directory where the file containing the output script
 *              will be written
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_install_script_generate_output(). From the
 * callback, call #osinfo_install_script_generate_output_finish() to conclude
 * this call and get the generated script.
 *
 * If you are generating the script for a specific media, it is recommended that
 * you use #osinfo_install_script_generate_output_for_media_async() instead.
 *
 * Since: 0.2.0
 */
void osinfo_install_script_generate_output_async(OsinfoInstallScript *script,
                                                 OsinfoOs *os,
                                                 OsinfoInstallConfig *config,
                                                 GFile *output_dir,
                                                 GCancellable *cancellable,
                                                 GAsyncReadyCallback callback,
                                                 gpointer user_data)
{
    osinfo_install_script_generate_output_async_common(script,
                                                       os,
                                                       NULL,
                                                       NULL,
                                                       config,
                                                       output_dir,
                                                       cancellable,
                                                       callback,
                                                       user_data);
}

static GFile *osinfo_install_script_generate_output_common(OsinfoInstallScript *script,
                                                           OsinfoOs *os,
                                                           OsinfoMedia *media,
                                                           OsinfoTree *tree,
                                                           OsinfoInstallConfig *config,
                                                           GFile *output_dir,
                                                           GCancellable *cancellable,
                                                           GError **error)
{
    GMainLoop *loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);
    OsinfoInstallScriptGenerateSyncData data = {
        loop, NULL, NULL, NULL
    };

    if (media != NULL) {
        osinfo_install_script_generate_output_for_media_async
            (script,
             media,
             config,
             output_dir,
             cancellable,
             osinfo_install_script_generate_output_for_media_done,
             &data);
    } else if (tree != NULL) {
        osinfo_install_script_generate_output_for_tree_async
            (script,
             tree,
             config,
             output_dir,
             cancellable,
             osinfo_install_script_generate_output_for_tree_done,
             &data);
    } else {
        osinfo_install_script_generate_output_async
            (script,
             os,
             config,
             output_dir,
             cancellable,
             osinfo_install_script_generate_output_done,
             &data);
    }

    g_main_loop_run(loop);

    if (data.error)
        g_propagate_error(error, data.error);

    g_main_loop_unref(loop);

    return data.file;
}

/**
 * osinfo_install_script_generate_output:
 * @script:     the install script
 * @os:         the os entity
 * @config:     the install script config
 * @output_dir: the directory where the file containing the output script
 *              will be written
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates an install script that is written to the returned file.
 *
 * Returns: (transfer full): a file containing the script
 *
 * If you are generating the script for a specific media, it is recommended
 * that you use #osinfo_install_script_generate_output_for_media() instead.
 *
 * If you are generating the script for a specific tree, it is recommended
 * that you use #osinfo_install_script_generate_output_for_tree() instead.
 *
 * Since: 0.2.0
 */
GFile *osinfo_install_script_generate_output(OsinfoInstallScript *script,
                                             OsinfoOs *os,
                                             OsinfoInstallConfig *config,
                                             GFile *output_dir,
                                             GCancellable *cancellable,
                                             GError **error)
{
    return osinfo_install_script_generate_output_common(script,
                                                        os,
                                                        NULL,
                                                        NULL,
                                                        config,
                                                        output_dir,
                                                        cancellable,
                                                        error);
}

/**
 * osinfo_install_script_generate_output_for_media_async:
 * @script:     the install script
 * @media:      the media
 * @config:     the install script config
 * @output_dir: the directory where the file containing the output script
 *              will be written
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_install_script_generate_output_for_media().
 * From the callback, call
 * #osinfo_install_script_generate_output_for_media_finish() to conclude this
 * call and get the generated file.
 *
 * Since: 0.2.12
 */
void osinfo_install_script_generate_output_for_media_async(OsinfoInstallScript *script,
                                                           OsinfoMedia *media,
                                                           OsinfoInstallConfig *config,
                                                           GFile *output_dir,
                                                           GCancellable *cancellable,
                                                           GAsyncReadyCallback callback,
                                                           gpointer user_data)
{
    osinfo_install_script_generate_output_async_common(script,
                                                       NULL,
                                                       media,
                                                       NULL,
                                                       config,
                                                       output_dir,
                                                       cancellable,
                                                       callback,
                                                       user_data);
}

/**
 * osinfo_install_script_generate_output_for_media:
 * @script:     the install script
 * @media:      the media
 * @config:     the install script config
 * @output_dir: the directory where the file containing the output script
 *              will be written
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates an install script that is written to the returned file.
 *
 * Returns: (transfer full): a file containing the script.
 *
 * Since: 0.2.12
 */
GFile *osinfo_install_script_generate_output_for_media(OsinfoInstallScript *script,
                                                       OsinfoMedia *media,
                                                       OsinfoInstallConfig *config,
                                                       GFile *output_dir,
                                                       GCancellable *cancellable,
                                                       GError **error)
{
    return osinfo_install_script_generate_output_common(script,
                                                        NULL,
                                                        media,
                                                        NULL,
                                                        config,
                                                        output_dir,
                                                        cancellable,
                                                        error);
}

/**
 * osinfo_install_script_generate_output_for_tree_async:
 * @script:     the install script
 * @tree:       the tree
 * @config:     the install script config
 * @output_dir: the directory where the file containing the output script
 *              will be written
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_install_script_generate_output_for_tree().
 * From the callback, call
 * #osinfo_install_script_generate_output_for_tree_finish() to conclude this
 * call and get the generated file.
 *
 * Since: 1.6.0
 */
void osinfo_install_script_generate_output_for_tree_async(OsinfoInstallScript *script,
                                                          OsinfoTree *tree,
                                                          OsinfoInstallConfig *config,
                                                          GFile *output_dir,
                                                          GCancellable *cancellable,
                                                          GAsyncReadyCallback callback,
                                                          gpointer user_data)
{
    osinfo_install_script_generate_output_async_common(script,
                                                       NULL,
                                                       NULL,
                                                       tree,
                                                       config,
                                                       output_dir,
                                                       cancellable,
                                                       callback,
                                                       user_data);
}

/**
 * osinfo_install_script_generate_output_for_tree:
 * @script:     the install script
 * @tree:       the tree
 * @config:     the install script config
 * @output_dir: the directory where the file containing the output script
 *              will be written
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates an install script that is written to the returned file.
 *
 * Returns: (transfer full): a file containing the script.
 *
 * Since: 1.6.0
 */
GFile *osinfo_install_script_generate_output_for_tree(OsinfoInstallScript *script,
                                                      OsinfoTree *tree,
                                                      OsinfoInstallConfig *config,
                                                      GFile *output_dir,
                                                      GCancellable *cancellable,
                                                      GError **error)
{
    return osinfo_install_script_generate_output_common(script,
                                                        NULL,
                                                        NULL,
                                                        tree,
                                                        config,
                                                        output_dir,
                                                        cancellable,
                                                        error);
}

/**
 * osinfo_install_script_generate_command_line:
 * @script: the install script
 * @os:     the os entity
 * @config: the install script config
 *
 * Some install scripts need to pass a command line to the kernel, Such install
 * scripts belong to OSs that provide paths to the kernel and initrd files that
 * can be used to directly boot
 * (http://wiki.qemu.org/download/qemu-doc.html#direct_005flinux_005fboot)
 * the OS in order to pass the needed commandline to it.
 *
 * If you are generating the command line for a specific media, it is
 * recommended that you use
 * #osinfo_install_script_generate_command_line_for_media() instead.
 *
 * If you are generating the command line for a specific tree, it is
 * recommended that you use
 * #osinfo_install_script_generate_command_line_for_tree() instead.
 *
 * Returns: (transfer full): The generated command line string, NULL otherwise.
 *
 * Since: 0.2.7
 */
gchar *osinfo_install_script_generate_command_line(OsinfoInstallScript *script,
                                                   OsinfoOs *os,
                                                   OsinfoInstallConfig *config)
{
    const gchar *templateData = osinfo_install_script_get_template_data(script);
    gchar *output = NULL;

    if (templateData) {
        GError *error = NULL;
        if (!osinfo_install_script_apply_template(script,
                                                  os,
                                                  NULL,
                                                  NULL,
                                                  "<data>",
                                                  templateData,
                                                  "command-line",
                                                  &output,
                                                  config,
                                                  &error)) {
            g_prefix_error(&error, "%s", _("Failed to apply script template: "));
        }
    }

    return output;
}

/**
 * osinfo_install_script_generate_command_line_for_media:
 * @script: the install script
 * @media:  the media
 * @config: the install script config
 *
 * Some install scripts need to pass a command line to the kernel, Such install
 * scripts belong to OSs that provide paths to the kernel and initrd files that
 * can be used to directly boot
 * (http://wiki.qemu.org/download/qemu-doc.html#direct_005flinux_005fboot)
 * the OS in order to pass the needed commandline to it.
 *
 * The media @media must have been identified successfully using
 * #osinfo_db_identify_media() before calling this function.
 *
 * Returns: (transfer full): The generated command line string, NULL otherwise.
 *
 * Since: 0.2.12
 */
gchar *osinfo_install_script_generate_command_line_for_media(OsinfoInstallScript *script,
                                                             OsinfoMedia *media,
                                                             OsinfoInstallConfig *config)
{
    const gchar *templateData = osinfo_install_script_get_template_data(script);
    gchar *output = NULL;
    OsinfoOs *os;

    g_return_val_if_fail(media != NULL, NULL);

    os = osinfo_media_get_os(media);
    g_return_val_if_fail(os != NULL, NULL);

    if (templateData) {
        GError *error = NULL;
        if (!osinfo_install_script_apply_template(script,
                                                  os,
                                                  media,
                                                  NULL,
                                                  "<data>",
                                                  templateData,
                                                  "command-line",
                                                  &output,
                                                  config,
                                                  &error)) {
            g_prefix_error(&error, "%s", _("Failed to apply script template: "));
        }
    }
    g_object_unref(os);

    return output;
}

/**
 * osinfo_install_script_generate_command_line_for_tree:
 * @script: the install script
 * @tree:   the tree
 * @config: the install script config
 *
 * Some install scripts need to pass a command line to the kernel, Such install
 * scripts belong to OSs that provide paths to the kernel and initrd files that
 * can be used to directly boot
 * (http://wiki.qemu.org/download/qemu-doc.html#direct_005flinux_005fboot)
 * the OS in order to pass the needed commandline to it.
 *
 * The tree @tree must have been identified successfully using
 * #osinfo_db_identify_tree() before calling this function.
 *
 * Returns: (transfer full): The generated command line string, NULL otherwise.
 */
gchar *osinfo_install_script_generate_command_line_for_tree(OsinfoInstallScript *script,
                                                            OsinfoTree *tree,
                                                            OsinfoInstallConfig *config)
{
    const gchar *templateData = osinfo_install_script_get_template_data(script);
    gchar *output = NULL;
    OsinfoOs *os;

    g_return_val_if_fail(tree != NULL, NULL);

    os = osinfo_tree_get_os(tree);
    g_return_val_if_fail(os != NULL, NULL);

    if (templateData) {
        GError *error = NULL;
        if (!osinfo_install_script_apply_template(script,
                                                  os,
                                                  NULL,
                                                  tree,
                                                  "<data>",
                                                  templateData,
                                                  "command-line",
                                                  &output,
                                                  config,
                                                  &error)) {
            g_prefix_error(&error, "%s", _("Failed to apply script template: "));
        }
    }
    g_object_unref(os);

    return output;
}

/**
 * osinfo_install_script_get_path_format:
 * @script: the install script
 *
 * Returns the path format to be used for the files and disks which will
 * be used during the installation.
 *
 * Returns: the path format to be used. OSINFO_PATH_FORMAT_UNIX is the
 *          default option.
 *
 * Since: 0.2.2
 */
OsinfoPathFormat osinfo_install_script_get_path_format(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_enum
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_PATH_FORMAT,
         OSINFO_TYPE_PATH_FORMAT,
         OSINFO_PATH_FORMAT_UNIX);
}

/**
 * osinfo_install_script_get_can_pre_install_drivers:
 * @script: the install script
 *
 * Whether install script can install drivers at the very beginning of
 * installation. This is needed for devices for which the OS in question does
 * not have out of the box support for and devices are required/prefered to be
 * available during actual installation.
 *
 * Returns: TRUE if install script supports pre-installable drivers, FALSE otherwise.
 *
 * Since: 0.2.2
 */
gboolean osinfo_install_script_get_can_pre_install_drivers(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_boolean
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_CAN_PRE_INSTALL_DRIVERS);
}

/**
 * osinfo_install_script_get_can_post_install_drivers:
 * @script: the install script
 *
 * Whether install script can install drivers at the very end of installation
 * This can be used for installing devices which the OS in question does not
 * have out of the box support.
 *
 * Returns: TRUE if install script supports post-installable drivers, FALSE otherwise.
 *
 * Since: 0.2.2
 */
gboolean osinfo_install_script_get_can_post_install_drivers(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_boolean
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_CAN_POST_INSTALL_DRIVERS);
}

/**
 * osinfo_install_script_get_pre_install_drivers_signing_req:
 * @script: the install script
 *
 * If install script can install drivers at the very beginning of installation,
 * this function retrieves the requirement about signed status of drivers.
 *
 * Returns: (type OsinfoDeviceDriverSigningReq):
 *
 * Since: 0.2.6
 */
int osinfo_install_script_get_pre_install_drivers_signing_req(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_enum
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_PRE_INSTALL_DRIVERS_SIGNING_REQ,
         OSINFO_TYPE_DEVICE_DRIVER_SIGNING_REQ,
         OSINFO_DEVICE_DRIVER_SIGNING_REQ_NONE);
}

/**
 * osinfo_install_script_get_post_install_drivers_signing_req:
 * @script: the install script
 *
 * If install script can install drivers at the end of installation, this
 * function retrieves the requirement about signed status of drivers.
 *
 * Returns: (type OsinfoDeviceDriverSigningReq):
 *
 * Since: 0.2.6
 */
int osinfo_install_script_get_post_install_drivers_signing_req(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_enum
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_POST_INSTALL_DRIVERS_SIGNING_REQ,
         OSINFO_TYPE_DEVICE_DRIVER_SIGNING_REQ,
         OSINFO_DEVICE_DRIVER_SIGNING_REQ_NONE);
}

/**
 * osinfo_install_script_get_injection_methods:
 * @script: the install script
 *
 * Retrieve the supported method to inject the script in to the installation process.
 *
 * Returns: (type OsinfoInstallScriptInjectionMethod): bitwise-or of
 * supported methods for install script injection.
 *
 * Since: 0.2.10
 */
unsigned int osinfo_install_script_get_injection_methods(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_int64_with_default
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_INJECTION_METHOD,
         OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK);
}

/**
 * osinfo_install_script_get_needs_internet:
 * @script: the install script
 *
 * Some install scripts cannot ensure that they work without an internet connection.
 *
 * Returns: TRUE if script needs an internet connection, FALSE otherwise
 * internet connection.
 *
 * Since: 0.2.11
 */
gboolean osinfo_install_script_get_needs_internet(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_boolean_with_default
        (OSINFO_ENTITY(script),
         OSINFO_INSTALL_SCRIPT_PROP_NEEDS_INTERNET,
         FALSE);
}

/**
 * osinfo_install_script_set_preferred_injection_method:
 * @script: the install script
 * @method: one of the injection methods:
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_CDROM,
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK,
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_FLOPPY,
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_INITRD,
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_WEB
 *
 * Set the preferred injection method to be used with the @script
 *
 * Since: 1.3.0
 */
void osinfo_install_script_set_preferred_injection_method(OsinfoInstallScript *script,
                                                          OsinfoInstallScriptInjectionMethod method)
{
    GFlagsClass *flags_class;
    guint i;

    flags_class = g_type_class_ref(OSINFO_TYPE_INSTALL_SCRIPT_INJECTION_METHOD);
    for (i = 0; i < flags_class->n_values; i++) {
        if ((flags_class->values[i].value & method) != 0) {
            osinfo_entity_set_param(OSINFO_ENTITY(script),
                                    OSINFO_INSTALL_SCRIPT_PROP_PREFERRED_INJECTION_METHOD,
                                    flags_class->values[i].value_nick);
            break;
        }
    }
    g_type_class_unref(flags_class);
}

/**
 * osinfo_install_script_get_preferred_injection_method:
 * @script: the install script
 *
 * Returns: the preferred injection method for the script. If none is set and
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK is supported,
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK is returned, otherwise
 * OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_INITRD is returned.
 *
 * Since: 1.3.0
 */
OsinfoInstallScriptInjectionMethod
osinfo_install_script_get_preferred_injection_method(OsinfoInstallScript *script)
{
    GFlagsClass *flags_class;
    GFlagsValue *value;
    const gchar *nick;
    guint supported_methods;

    nick = osinfo_entity_get_param_value(OSINFO_ENTITY(script),
            OSINFO_INSTALL_SCRIPT_PROP_PREFERRED_INJECTION_METHOD);

    if (nick == NULL) {
        supported_methods = osinfo_install_script_get_injection_methods(script);
        if ((supported_methods & OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK) != 0)
            return OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK;
        else if ((supported_methods & OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_INITRD) != 0)
            return OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_INITRD;
        else
            return OSINFO_INSTALL_SCRIPT_INJECTION_METHOD_DISK;
    }

    flags_class = g_type_class_ref(OSINFO_TYPE_INSTALL_SCRIPT_INJECTION_METHOD);
    value = g_flags_get_value_by_nick(flags_class, nick);
    g_type_class_unref(flags_class);

    return value->value;
}

/**
 * osinfo_install_script_set_installation_source:
 * @script: the install script
 * @source: one of the installation sources:
 * OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_MEDIA,
 * OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_NETWORK
 *
 * Set the installation source to be used with the @script.
 *
 * Since: 1.3.0
 */
void osinfo_install_script_set_installation_source(OsinfoInstallScript *script,
                                                   OsinfoInstallScriptInstallationSource source)
{
    osinfo_entity_set_param_enum(OSINFO_ENTITY(script),
            OSINFO_INSTALL_SCRIPT_PROP_INSTALLATION_SOURCE,
            source,
            OSINFO_TYPE_INSTALL_SCRIPT_INSTALLATION_SOURCE);
}

/**
 * osinfo_install_script_get_installation_source:
 * @script: the install script
 *
 * Returns: the installation source to be used with the script. If none is set, it defaults to
 * OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_MEDIA.
 *
 * Since: 1.3.0
 */
OsinfoInstallScriptInstallationSource
osinfo_install_script_get_installation_source(OsinfoInstallScript *script)
{
    return osinfo_entity_get_param_value_enum(OSINFO_ENTITY(script),
            OSINFO_INSTALL_SCRIPT_PROP_INSTALLATION_SOURCE,
            OSINFO_TYPE_INSTALL_SCRIPT_INSTALLATION_SOURCE,
            OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_MEDIA);
}
