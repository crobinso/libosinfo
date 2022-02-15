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

#include <glib/gi18n-lib.h>

#include <osinfo/osinfo.h>

#include <gio/gio.h>

#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlreader.h>
#include "ignore-value.h"
#include "osinfo_install_script_private.h"
#include "osinfo_device_driver_private.h"
#include "osinfo_resources_private.h"

#ifndef USB_IDS
# define USB_IDS PKG_DATA_DIR "/usb.ids"
#endif
#ifndef PCI_IDS
# define PCI_IDS PKG_DATA_DIR "/pci.ids"
#endif

/**
 * SECTION:osinfo_loader
 * @short_description: An database loader
 * @see_also: #OsinfoDb
 *
 * #OsinfoLoader provides a way to populate an #OsinfoDb from
 * a set of XML documents.
 *
 */

struct _OsinfoLoaderPrivate
{
    OsinfoDb *db;
    GHashTable *xpath_cache;
    GHashTable *entity_refs;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoLoader, osinfo_loader, G_TYPE_OBJECT);

struct _OsinfoEntityKey
{
    const char *name;
    GType type;
};
typedef struct _OsinfoEntityKey OsinfoEntityKey;

typedef struct OsinfoLoaderEntityFiles OsinfoLoaderEntityFiles;

struct OsinfoLoaderEntityFiles {
    GFile *master;
    GList *extensions;
};

static void
osinfo_loader_finalize(GObject *object)
{
    OsinfoLoader *loader = OSINFO_LOADER(object);

    g_object_unref(loader->priv->db);
    g_hash_table_destroy(loader->priv->xpath_cache);

    g_hash_table_destroy(loader->priv->entity_refs);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_loader_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_loader_class_init(OsinfoLoaderClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    g_klass->finalize = osinfo_loader_finalize;
}


static void xpath_cache_value_free(gpointer values)
{
    xmlXPathFreeCompExpr(values);
}

static void
osinfo_loader_init(OsinfoLoader *loader)
{
    loader->priv = osinfo_loader_get_instance_private(loader);
    loader->priv->db = osinfo_db_new();
    loader->priv->xpath_cache = g_hash_table_new_full(g_str_hash,
                                                      g_str_equal,
                                                      g_free,
                                                      xpath_cache_value_free);
    loader->priv->entity_refs = g_hash_table_new_full(g_str_hash,
                                                      g_str_equal,
                                                      g_free,
                                                      NULL);
}

static gchar *
osinfo_build_internal_id(const gchar *relpath,
                         const gchar *id,
                         gint number)
{
    gchar *driver_id = NULL;
    g_autofree gchar *dirname = NULL;

    dirname = g_path_get_dirname(relpath);
    if (g_str_has_suffix(dirname, ".d")) {
        g_autofree gchar *basename = NULL;

        basename = g_path_get_basename(relpath);
        driver_id = g_strdup_printf("%s:%s:%u", id, basename, number);
    } else {
        driver_id = g_strdup_printf("%s:%u", id, number);
    }

    return driver_id;
}

/**
 * osinfo_loader_new:
 *
 * Create a new database loader
 *
 * Returns: (transfer full): a loader object
 */
OsinfoLoader *osinfo_loader_new(void)
{
    return g_object_new(OSINFO_TYPE_LOADER, NULL);
}

/**
 * osinfo_error_quark:
 *
 * Gets a #GQuark representing the string "libosinfo"
 *
 * Returns: the #GQuark representing the string.
 *
 * Since: 1.3.0
 **/
GQuark
osinfo_error_quark(void)
{
    static GQuark quark = 0;

    if (!quark)
        quark = g_quark_from_static_string("libosinfo");

    return quark;
}

#define OSINFO_LOADER_SET_ERROR(err, msg)                                        \
    g_set_error_literal((err), OSINFO_ERROR, 0, (msg));

static gboolean error_is_set(GError **error)
{
    return ((error != NULL) && (*error != NULL));
}

static xmlXPathCompExprPtr osinfo_loader_get_comp_xpath(OsinfoLoader *loader,
                                                        const char *xpath)
{
    xmlXPathCompExprPtr comp = g_hash_table_lookup(loader->priv->xpath_cache,
                                                   xpath);
    if (comp == NULL) {
        comp = xmlXPathCompile(BAD_CAST xpath);
        g_hash_table_insert(loader->priv->xpath_cache, g_strdup(xpath), comp);
    }
    return comp;
}

static int
osinfo_loader_nodeset(const char *xpath,
                      OsinfoLoader *loader,
                      xmlXPathContextPtr ctxt,
                      xmlNodePtr **list,
                      GError **err)
{
    xmlXPathObjectPtr obj;
    xmlNodePtr relnode;
    int ret;
    xmlXPathCompExprPtr comp;

    g_return_val_if_fail(ctxt != NULL, -1);
    g_return_val_if_fail(xpath != NULL, -1);
    g_return_val_if_fail(loader != NULL, -1);

    comp = osinfo_loader_get_comp_xpath(loader, xpath);

    if (list != NULL) {
        g_warn_if_fail(*list == NULL);
        *list = NULL;
    }

    relnode = ctxt->node;
    obj = xmlXPathCompiledEval(comp, ctxt);
    ctxt->node = relnode;
    if (obj == NULL)
        return 0;
    if (obj->type != XPATH_NODESET) {
        g_set_error(err, OSINFO_ERROR, 0,
                    _("Expected a nodeset in XPath query %s"), xpath);
        xmlXPathFreeObject(obj);
        return -1;
    }
    if ((obj->nodesetval == NULL)  || (obj->nodesetval->nodeNr < 0)) {
        xmlXPathFreeObject(obj);
        return 0;
    }

    ret = obj->nodesetval->nodeNr;
    if (list != NULL && ret) {
        *list = g_new0(xmlNodePtr, ret);
        memcpy(*list, obj->nodesetval->nodeTab,
               ret * sizeof(xmlNodePtr));
    }
    xmlXPathFreeObject(obj);
    return ret;
}

static gchar *
osinfo_loader_string(const char *xpath,
                     OsinfoLoader *loader,
                     xmlXPathContextPtr ctxt,
                     GError **err)
{
    xmlXPathObjectPtr obj;
    xmlNodePtr relnode;
    gchar *ret;
    xmlXPathCompExprPtr comp;

    g_return_val_if_fail(ctxt != NULL, NULL);
    g_return_val_if_fail(xpath != NULL, NULL);
    g_return_val_if_fail(loader != NULL, NULL);

    comp = osinfo_loader_get_comp_xpath(loader, xpath);
    relnode = ctxt->node;
    obj = xmlXPathCompiledEval(comp, ctxt);

    ctxt->node = relnode;
    if ((obj == NULL) || (obj->type != XPATH_STRING) ||
        (obj->stringval == NULL) || (obj->stringval[0] == 0)) {
        xmlXPathFreeObject(obj);
        return NULL;
    }
    ret = g_strdup((char *) obj->stringval);
    xmlXPathFreeObject(obj);

    return ret;
}

static gboolean
osinfo_loader_boolean(const char *xpath,
                      OsinfoLoader *loader,
                      xmlXPathContextPtr ctxt,
                      GError **err)
{

    xmlNodePtr *nodes = NULL;
    int count;
    int i;
    gboolean ret = FALSE;

    g_return_val_if_fail(ctxt != NULL, FALSE);
    g_return_val_if_fail(xpath != NULL, FALSE);
    g_return_val_if_fail(loader != NULL, FALSE);

    count = osinfo_loader_nodeset(xpath, loader, ctxt, &nodes, err);

    if (count < 0) {
        return FALSE;
    }
    for (i = 0; i < count; i++) {
        xmlNodePtr node = nodes[i];

        if (!node->children) {
            /* node is present -> TRUE */
            ret = TRUE;
            break;
        }
        if (node->children->type != XML_TEXT_NODE) {
            OSINFO_LOADER_SET_ERROR(err, _("Expected a text node attribute value"));
            goto cleanup;
        }

        if (g_strcmp0((const char *)nodes[i]->children->content, "true") == 0) {
            ret = TRUE;
            break;
        }
    }
cleanup:
    g_free(nodes);
    return ret;
}

static gchar *
osinfo_loader_doc(const char *xpath,
                  OsinfoLoader *loader,
                  xmlXPathContextPtr ctxt,
                  GError **err)
{
    xmlXPathObjectPtr obj;
    xmlNodePtr relnode;
    gchar *ret;
    xmlBufferPtr buf;
    xmlXPathCompExprPtr comp;

    g_return_val_if_fail(ctxt != NULL, NULL);
    g_return_val_if_fail(xpath != NULL, NULL);
    g_return_val_if_fail(loader != NULL, NULL);

    comp = osinfo_loader_get_comp_xpath(loader, xpath);

    relnode = ctxt->node;
    obj = xmlXPathCompiledEval(comp, ctxt);
    ctxt->node = relnode;
    if ((obj == NULL) || (obj->type != XPATH_NODESET)) {
        xmlXPathFreeObject(obj);
        return NULL;
    }

    if (!(buf = xmlBufferCreate())) {
        xmlXPathFreeObject(obj);
        OSINFO_LOADER_SET_ERROR(err, "Cannot allocate buffer");
        return NULL;
    }
    if (xmlNodeDump(buf, NULL, obj->nodesetval->nodeTab[0], 0, 1) < 0) {
        xmlXPathFreeObject(obj);
        OSINFO_LOADER_SET_ERROR(err, "Cannot format stylesheet");
    }
    ret = g_strdup((char *)buf->content);

    xmlBufferFree(buf);
    xmlXPathFreeObject(obj);
    return ret;
}

static void osinfo_loader_entity(OsinfoLoader *loader,
                                 OsinfoEntity *entity,
                                 const OsinfoEntityKey *keys,
                                 xmlXPathContextPtr ctxt,
                                 xmlNodePtr root,
                                 GError **err)
{
    int i = 0;
    const gchar * const *langs = g_get_language_names();
    xmlNodePtr *custom = NULL;
    int ncustom;

    /* Standard well-known keys first, allow single value only */
    for (i = 0; keys != NULL && keys[i].name != NULL; i++) {
        gchar *value_str = NULL;
        gboolean value_bool = FALSE;
        gchar *xpath = NULL;
        int j;

        /* We are guaranteed to have at least the default "C" locale and we
         * want to ignore that, hence the NULL check on index 'j + 1'.
         */
        if (keys[i].type == G_TYPE_STRING) {
            xmlNodePtr it;
            for (it = root->children; it; it = it->next) {
                if (xmlStrEqual(it->name, BAD_CAST keys[i].name)) {
                    xmlChar *lang = xmlGetProp(it, BAD_CAST "lang");
                    if (lang == NULL)
                        continue;

                    for (j = 0; langs[j + 1] != NULL; j++) {
                        if (xmlStrEqual(lang, BAD_CAST langs[j])) {
                            gchar *content = (gchar *) it->children->content;
                            value_str = g_strdup(content);
                            break;
                        }
                    }
                    xmlFree(lang);
                }
            }
        }

        switch (keys[i].type) {
            case G_TYPE_STRING:
                if (value_str == NULL) {
                    xpath = g_strdup_printf("string(./%s)", keys[i].name);
                    value_str = osinfo_loader_string(xpath, loader, ctxt, err);
                    g_free(xpath);
                }
                break;
            case G_TYPE_BOOLEAN:
                xpath = g_strdup_printf("./%s", keys[i].name);
                value_bool = osinfo_loader_boolean(xpath, loader, ctxt, err);
                g_free(xpath);
                break;
            default:
                g_warn_if_reached();
                break;
        }

        switch (keys[i].type) {
            case G_TYPE_STRING:
                if (value_str) {
                    osinfo_entity_set_param(entity, keys[i].name, value_str);
                    g_free(value_str);
                    value_str = NULL;
                }
                break;
            case G_TYPE_BOOLEAN:
                osinfo_entity_set_param_boolean(entity, keys[i].name, value_bool);
                break;
            default:
                g_warn_if_reached();
                break;
        }
        g_warn_if_fail(value_str == NULL);
    }

    /* Then any site specific custom keys. x-... Can be repeated */
    ncustom = osinfo_loader_nodeset("./*[substring(name(),1,2)='x-']",
                                    loader, ctxt, &custom, err);
    if (error_is_set(err))
        return;

    for (i = 0; i < ncustom; i++) {
        xmlNodePtr param = custom[i];

        if (!param->children ||
            param->children->type != XML_TEXT_NODE) {
            OSINFO_LOADER_SET_ERROR(err, _("Expected a text node attribute value"));
            goto cleanup;
        }

        osinfo_entity_add_param(entity,
                                (const char *)custom[i]->name,
                                (const char *)custom[i]->children->content);
    }

 cleanup:
    g_free(custom);
}

static OsinfoDatamap *osinfo_loader_get_datamap(OsinfoLoader *loader,
                                                const gchar *id)
{
    OsinfoDatamap *datamap = osinfo_db_get_datamap(loader->priv->db, id);
    if (!datamap) {
        datamap = osinfo_datamap_new(id);
        osinfo_db_add_datamap(loader->priv->db, datamap);
        g_object_unref(datamap);
        g_hash_table_insert(loader->priv->entity_refs, g_strdup(id), datamap);
    }
    return datamap;
}

static OsinfoDevice *osinfo_loader_get_device(OsinfoLoader *loader,
                                              const gchar *id)
{
    OsinfoDevice *dev = osinfo_db_get_device(loader->priv->db, id);
    if (!dev) {
        dev = osinfo_device_new(id);
        osinfo_db_add_device(loader->priv->db, dev);
        g_object_unref(dev);
        g_hash_table_insert(loader->priv->entity_refs, g_strdup(id), dev);
    }
    return dev;
}

static OsinfoOs *osinfo_loader_get_os(OsinfoLoader *loader,
                                      const gchar *id)
{
    OsinfoOs *os = osinfo_db_get_os(loader->priv->db, id);
    if (!os) {
        os = osinfo_os_new(id);
        osinfo_db_add_os(loader->priv->db, os);
        g_object_unref(os);
        g_hash_table_insert(loader->priv->entity_refs, g_strdup(id), os);
    }
    return os;
}

static OsinfoPlatform *osinfo_loader_get_platform(OsinfoLoader *loader,
                                                  const gchar *id)
{
    OsinfoPlatform *platform = osinfo_db_get_platform(loader->priv->db, id);
    if (!platform) {
        platform = osinfo_platform_new(id);
        osinfo_db_add_platform(loader->priv->db, platform);
        g_object_unref(platform);
        g_hash_table_insert(loader->priv->entity_refs, g_strdup(id), platform);
    }
    return platform;
}

static OsinfoInstallScript *osinfo_loader_get_install_script(OsinfoLoader *loader,
                                                             const gchar *id)
{
    OsinfoInstallScript *script = osinfo_db_get_install_script(loader->priv->db, id);
    if (!script) {
        script = osinfo_install_script_new(id);
        osinfo_db_add_install_script(loader->priv->db, script);
        g_object_unref(script);
        g_hash_table_insert(loader->priv->entity_refs, g_strdup(id), script);
    }
    return script;
}


static gboolean osinfo_loader_check_id(const gchar *relpath,
                                       const gchar *type,
                                       const gchar *id)
{
    gchar *name;
    gchar *suffix;
    gboolean sep = FALSE;
    gchar *reldir;
    gboolean extension;
    gsize i;
    gboolean ret = FALSE;

    if (!relpath)
        return TRUE;

    if (g_str_has_prefix(id, "http://")) {
        suffix = g_strdup(id + strlen("http://"));
    } else {
        suffix = g_strdup(id);
    }
    for (i = 0; suffix[i]; i++) {
        if (suffix[i] == '/' && !sep) {
            sep = TRUE;
        } else if (!isalnum(suffix[i]) &&
                   suffix[i] != '-' &&
                   suffix[i] != '.' &&
                   suffix[i] != '_') {
                suffix[i] = '-';
        }
    }
    reldir = g_path_get_dirname(relpath);
    if (g_str_has_suffix(reldir, ".d")) {
        name = g_strdup_printf("%s/%s.d", type, suffix);
        extension = TRUE;
    } else {
        name = g_strdup_printf("%s/%s.xml", type, suffix);
        extension = FALSE;
    }
    g_free(suffix);

    if (!g_str_equal(extension ? reldir : relpath, name)) {
        g_warning("Entity %s must be in file %s not %s",
                  id, name, extension ? reldir : relpath);
        goto cleanup;
    }

    ret = TRUE;

 cleanup:
    g_free(reldir);
    g_free(name);
    return ret;
}


static void osinfo_loader_device(OsinfoLoader *loader,
                                 const gchar *relpath,
                                 xmlXPathContextPtr ctxt,
                                 xmlNodePtr root,
                                 GError **err)
{
    OsinfoDevice *device;
    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");
    const OsinfoEntityKey keys[] = {
        { OSINFO_DEVICE_PROP_VENDOR, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_VENDOR_ID, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_PRODUCT, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_PRODUCT_ID, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_SUBSYSTEM, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_BUS_TYPE, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_CLASS, G_TYPE_STRING },
        { OSINFO_DEVICE_PROP_NAME, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };
    if (!id) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing device id property"));
        return;
    }
    if (!osinfo_loader_check_id(relpath, "device", id)) {
        xmlFree(id);
        return;
    }

    device = osinfo_loader_get_device(loader, id);
    g_hash_table_remove(loader->priv->entity_refs, id);
    xmlFree(id);

    osinfo_loader_entity(loader, OSINFO_ENTITY(device), keys, ctxt, root, err);
}

static void osinfo_loader_device_link(OsinfoLoader *loader,
                                      OsinfoEntity *entity,
                                      const gchar *xpath,
                                      xmlXPathContextPtr ctxt,
                                      xmlNodePtr root,
                                      GError **err)
{
    xmlNodePtr *related = NULL;
    int nrelated = osinfo_loader_nodeset(xpath, loader, ctxt, &related, err);
    int i;
    if (error_is_set(err))
        return;

    for (i = 0; i < nrelated; i++) {
        OsinfoDevice *dev;
        OsinfoDeviceLink *devlink = NULL;
        const OsinfoEntityKey keys[] = {
            { OSINFO_DEVICELINK_PROP_DRIVER, G_TYPE_STRING },
            { NULL, G_TYPE_INVALID }
        };
        gchar *id = (gchar *)xmlGetProp(related[i], BAD_CAST "id");
        gchar *supported;
        xmlNodePtr saved;
        if (!id) {
            OSINFO_LOADER_SET_ERROR(err, _("Missing device link id property"));
            goto cleanup;
        }
        dev = osinfo_loader_get_device(loader, id);
        xmlFree(id);

        if (OSINFO_IS_PLATFORM(entity)) {
            devlink = osinfo_platform_add_device(OSINFO_PLATFORM(entity), dev);
        } else if (OSINFO_IS_OS(entity)) {
            devlink = osinfo_os_add_device(OSINFO_OS(entity), dev);
        } else if (OSINFO_IS_DEPLOYMENT(entity)) {
            devlink = osinfo_deployment_add_device(OSINFO_DEPLOYMENT(entity), dev);
        }

        supported = (gchar *)xmlGetProp(related[i],
                                               BAD_CAST OSINFO_DEVICELINK_PROP_SUPPORTED);
        if (supported != NULL) {
            osinfo_entity_set_param_boolean(OSINFO_ENTITY(devlink),
                                            OSINFO_DEVICELINK_PROP_SUPPORTED,
                                            g_str_equal(supported, "false") ? FALSE : TRUE);
            xmlFree(supported);
        }

        saved = ctxt->node;
        ctxt->node = related[i];
        osinfo_loader_entity(loader, OSINFO_ENTITY(devlink), keys, ctxt, root, err);
        ctxt->node = saved;
        if (error_is_set(err))
            goto cleanup;
    }

 cleanup:
    g_free(related);
}

static void osinfo_loader_product_relshp(OsinfoLoader *loader,
                                         OsinfoProduct *product,
                                         OsinfoProductRelationship relshp,
                                         const gchar *name,
                                         xmlXPathContextPtr ctxt,
                                         xmlNodePtr root,
                                         GError **err)
{
    xmlNodePtr *related = NULL;
    xmlNodePtr it;

    if (error_is_set(err))
        return;

    for (it = root->children; it; it = it->next) {
        gchar *id;
        OsinfoProduct *relproduct;

        if (it->type != XML_ELEMENT_NODE)
            continue;

        if (!xmlStrEqual(it->name, BAD_CAST name))
            continue;

        id = (gchar *) xmlGetProp(it, BAD_CAST "id");
        if (!id) {
            OSINFO_LOADER_SET_ERROR(err, _("Missing product upgrades id property"));
            goto cleanup;
        }
        if (OSINFO_IS_PLATFORM(product))
            relproduct = OSINFO_PRODUCT(osinfo_loader_get_platform(loader, id));
        else
            relproduct = OSINFO_PRODUCT(osinfo_loader_get_os(loader, id));
        xmlFree(id);

        osinfo_product_add_related(product, relshp, relproduct);
    }

 cleanup:
    g_free(related);
}

static void osinfo_loader_product(OsinfoLoader *loader,
                                  OsinfoProduct *product,
                                  xmlXPathContextPtr ctxt,
                                  xmlNodePtr root,
                                  GError **err)
{
    const OsinfoEntityKey keys[] = {
        { OSINFO_PRODUCT_PROP_NAME, G_TYPE_STRING },
        { OSINFO_PRODUCT_PROP_VENDOR, G_TYPE_STRING },
        { OSINFO_PRODUCT_PROP_VERSION, G_TYPE_STRING },
        { OSINFO_PRODUCT_PROP_LOGO, G_TYPE_STRING },
        { OSINFO_PRODUCT_PROP_RELEASE_DATE, G_TYPE_STRING },
        { OSINFO_PRODUCT_PROP_EOL_DATE, G_TYPE_STRING },
        { OSINFO_PRODUCT_PROP_CODENAME, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };
    xmlNodePtr *nodes = NULL;
    gsize nnodes, i;

    osinfo_loader_entity(loader, OSINFO_ENTITY(product), keys, ctxt, root, err);
    if (error_is_set(err))
        return;

    nnodes = osinfo_loader_nodeset("./short-id", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        return;

    for (i = 0; i < nnodes; i++)
        osinfo_entity_add_param(OSINFO_ENTITY(product),
                                OSINFO_PRODUCT_PROP_SHORT_ID,
                                (const gchar *)nodes[i]->children->content);
    g_free(nodes);

    osinfo_loader_product_relshp(loader, product,
                                 OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM,
                                 "derives-from",
                                 ctxt,
                                 root,
                                 err);
    if (error_is_set(err))
        return;

    osinfo_loader_product_relshp(loader, product,
                                 OSINFO_PRODUCT_RELATIONSHIP_CLONES,
                                 "clones",
                                 ctxt,
                                 root,
                                 err);
    if (error_is_set(err))
        return;

    osinfo_loader_product_relshp(loader, product,
                                 OSINFO_PRODUCT_RELATIONSHIP_UPGRADES,
                                 "upgrades",
                                 ctxt,
                                 root,
                                 err);
}

static void osinfo_loader_platform(OsinfoLoader *loader,
                                   const gchar *relpath,
                                   xmlXPathContextPtr ctxt,
                                   xmlNodePtr root,
                                   GError **err)
{
    OsinfoPlatform *platform;
    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");
    if (!id) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing platform id property"));
        return;
    }
    if (!osinfo_loader_check_id(relpath, "platform", id)) {
        xmlFree(id);
        return;
    }

    platform = osinfo_loader_get_platform(loader, id);
    g_hash_table_remove(loader->priv->entity_refs, id);
    xmlFree(id);

    osinfo_loader_entity(loader, OSINFO_ENTITY(platform), NULL, ctxt, root, err);
    if (error_is_set(err))
        return;

    osinfo_loader_product(loader, OSINFO_PRODUCT(platform), ctxt, root, err);
    if (error_is_set(err))
        return;

    osinfo_loader_device_link(loader, OSINFO_ENTITY(platform),
                              "./devices/device", ctxt, root, err);
    if (error_is_set(err))
        return;
}

static void osinfo_loader_deployment(OsinfoLoader *loader,
                                     const gchar *relpath,
                                     xmlXPathContextPtr ctxt,
                                     xmlNodePtr root,
                                     GError **err)
{
    OsinfoOs *os;
    OsinfoPlatform *platform;
    OsinfoDeployment *deployment;
    gchar *platformid;
    gchar *osid;
    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");
    if (!id) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing deployment id property"));
        return;
    }
    if (!osinfo_loader_check_id(relpath, "deployment", id)) {
        xmlFree(id);
        return;
    }

    osid = osinfo_loader_string("string(./os/@id)", loader, ctxt, err);
    if (!osid && 0) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing deployment os id property"));
        xmlFree(id);
        return;
    }
    os = osinfo_loader_get_os(loader, osid);
    g_free(osid);

    platformid = osinfo_loader_string("string(./platform/@id)", loader,
                                             ctxt, err);
    if (!platformid) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing deployment platform id property"));
        xmlFree(id);
        return;
    }
    platform = osinfo_loader_get_platform(loader, platformid);
    g_free(platformid);

    deployment = osinfo_deployment_new(id, os, platform);
    xmlFree(id);

    osinfo_loader_entity(loader, OSINFO_ENTITY(deployment), NULL, ctxt, root, err);
    if (error_is_set(err)) {
        g_object_unref(G_OBJECT(deployment));
        return;
    }

    osinfo_loader_device_link(loader, OSINFO_ENTITY(deployment),
                              "./devices/device", ctxt, root, err);
    if (error_is_set(err)) {
        g_object_unref(G_OBJECT(deployment));
        return;
    }

    osinfo_db_add_deployment(loader->priv->db, deployment);
}

static void osinfo_loader_datamap(OsinfoLoader *loader,
                                  const gchar *relpath,
                                  xmlXPathContextPtr ctxt,
                                  xmlNodePtr root,
                                  GError **err)
{
    xmlNodePtr *nodes = NULL;
    guint i;
    int nnodes;
    OsinfoDatamap *map;
    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");

    if (!id) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing os id property"));
        return;
    }
    if (!osinfo_loader_check_id(relpath, "datamap", id)) {
        xmlFree(id);
        return;
    }

    map = osinfo_loader_get_datamap(loader, id);
    g_hash_table_remove(loader->priv->entity_refs, id);

    nnodes = osinfo_loader_nodeset("./entry", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *inval = (gchar *)xmlGetProp(nodes[i], BAD_CAST "inval");
        gchar *outval;

        if (inval == NULL)
            continue;
        outval = (gchar *)xmlGetProp(nodes[i], BAD_CAST "outval");
        osinfo_datamap_insert(map, inval, outval);

        xmlFree(inval);
        xmlFree(outval);
    }

cleanup:
    g_free(nodes);
    xmlFree(id);
}

static void osinfo_loader_install_config_params(OsinfoLoader *loader,
                                                OsinfoEntity *entity,
                                                const gchar *xpath,
                                                xmlXPathContextPtr ctxt,
                                                xmlNodePtr root,
                                                GError **err)
{
    xmlNodePtr *nodes = NULL;
    int nnodes = osinfo_loader_nodeset(xpath, loader, ctxt, &nodes, err);
    int i;
    if (error_is_set(err))
        return;

    for (i = 0; i < nnodes; i++) {
        gchar *name = (gchar *)xmlGetProp(nodes[i], BAD_CAST OSINFO_INSTALL_CONFIG_PARAM_PROP_NAME);
        gchar *policy = (gchar *)xmlGetProp(nodes[i], BAD_CAST OSINFO_INSTALL_CONFIG_PARAM_PROP_POLICY);
        gchar *mapid = (gchar *)xmlGetProp(nodes[i], BAD_CAST OSINFO_INSTALL_CONFIG_PARAM_PROP_DATAMAP);
        OsinfoInstallConfigParam *param = osinfo_install_config_param_new(name);
        osinfo_entity_set_param(OSINFO_ENTITY(param),
                                OSINFO_INSTALL_CONFIG_PARAM_PROP_POLICY,
                                policy);
        osinfo_install_script_add_config_param(OSINFO_INSTALL_SCRIPT(entity),
                                               param);
        if (mapid != NULL) {
            OsinfoDatamap *map;
            map = osinfo_loader_get_datamap(loader, mapid);
            if (map != NULL)
                osinfo_install_config_param_set_value_map(param, map);
        }

        xmlFree(mapid);
        xmlFree(name);
        xmlFree(policy);
        g_object_unref(param);
    };

    g_free(nodes);
}

static OsinfoAvatarFormat *osinfo_loader_avatar_format(OsinfoLoader *loader,
                                                       xmlXPathContextPtr ctxt,
                                                       xmlNodePtr root,
                                                       GError **err)
{
    OsinfoAvatarFormat *avatar_format;
    const OsinfoEntityKey keys[] = {
        { OSINFO_AVATAR_FORMAT_PROP_MIME_TYPE, G_TYPE_STRING },
        { OSINFO_AVATAR_FORMAT_PROP_WIDTH, G_TYPE_STRING },
        { OSINFO_AVATAR_FORMAT_PROP_HEIGHT, G_TYPE_STRING },
        { OSINFO_AVATAR_FORMAT_PROP_ALPHA, G_TYPE_BOOLEAN },
        { NULL, G_TYPE_INVALID }
    };

    avatar_format = osinfo_avatar_format_new();

    osinfo_loader_entity(loader, OSINFO_ENTITY(avatar_format), keys, ctxt, root, err);
    if (error_is_set(err)) {
        g_object_unref(avatar_format);

        return NULL;
    }

    return avatar_format;
}

static void osinfo_loader_install_script(OsinfoLoader *loader,
                                         const gchar *relpath,
                                         xmlXPathContextPtr ctxt,
                                         xmlNodePtr root,
                                         GError **err)
{
    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");
    const OsinfoEntityKey keys[] = {
        { OSINFO_INSTALL_SCRIPT_PROP_PROFILE, G_TYPE_STRING },
        { OSINFO_INSTALL_SCRIPT_PROP_PRODUCT_KEY_FORMAT, G_TYPE_STRING },
        { OSINFO_INSTALL_SCRIPT_PROP_PATH_FORMAT, G_TYPE_STRING },
        { OSINFO_INSTALL_SCRIPT_PROP_EXPECTED_FILENAME, G_TYPE_STRING },
        { OSINFO_INSTALL_SCRIPT_PROP_NEEDS_INTERNET, G_TYPE_BOOLEAN },
        { OSINFO_INSTALL_SCRIPT_PROP_CAN_PRE_INSTALL_DRIVERS, G_TYPE_BOOLEAN },
        { OSINFO_INSTALL_SCRIPT_PROP_CAN_POST_INSTALL_DRIVERS, G_TYPE_BOOLEAN },
        { OSINFO_INSTALL_SCRIPT_PROP_PRE_INSTALL_DRIVERS_SIGNING_REQ, G_TYPE_STRING },
        { OSINFO_INSTALL_SCRIPT_PROP_POST_INSTALL_DRIVERS_SIGNING_REQ, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };
    gchar *value = NULL;
    xmlNodePtr *nodes = NULL;
    int i, nnodes;
    unsigned int injection_methods = 0;
    GFlagsClass *flags_class;
    OsinfoInstallScript *installScript;

    if (!id) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing install script id property"));
        return;
    }

    if (!osinfo_loader_check_id(relpath, "install-script", id)) {
        xmlFree(id);
        return;
    }
    installScript = osinfo_loader_get_install_script(loader, id);
    g_hash_table_remove(loader->priv->entity_refs, id);
    xmlFree(id);

    osinfo_loader_entity(loader, OSINFO_ENTITY(installScript), keys, ctxt, root, err);
    if (error_is_set(err))
        goto error;

    value = osinfo_loader_doc("./template/*[1]", loader, ctxt, err);
    if (error_is_set(err))
        goto error;
    if (value)
        osinfo_entity_set_param(OSINFO_ENTITY(installScript),
                                OSINFO_INSTALL_SCRIPT_PROP_TEMPLATE_DATA,
                                value);
    g_free(value);

    value = osinfo_loader_string("./template/@uri", loader, ctxt, err);
    if (error_is_set(err))
        goto error;
    if (value)
        osinfo_entity_set_param(OSINFO_ENTITY(installScript),
                                OSINFO_INSTALL_SCRIPT_PROP_TEMPLATE_URI,
                                value);
    g_free(value);

    osinfo_loader_install_config_params(loader,
                                        OSINFO_ENTITY(installScript),
                                        "./config/*",
                                        ctxt,
                                        root,
                                        err);

    nnodes = osinfo_loader_nodeset("./avatar-format", loader, ctxt, &nodes,
                                   err);
    if (error_is_set(err))
        goto error;

    if (nnodes > 0) {
        OsinfoAvatarFormat *avatar_format;

        xmlNodePtr saved = ctxt->node;
        ctxt->node = nodes[0];
        avatar_format = osinfo_loader_avatar_format(loader, ctxt, root, err);
        ctxt->node = saved;
        if (error_is_set(err))
            goto error;

        osinfo_install_script_set_avatar_format(installScript, avatar_format);
        g_object_unref(avatar_format);
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./injection-method", loader, ctxt, &nodes,
                                   err);
    if (error_is_set(err))
        goto error;

    flags_class = g_type_class_ref(OSINFO_TYPE_INSTALL_SCRIPT_INJECTION_METHOD);
    for (i = 0; i < nnodes; i++) {
        const gchar *nick = (const gchar *) nodes[i]->children->content;
        injection_methods |= g_flags_get_value_by_nick(flags_class, nick)->value;
    }
    osinfo_entity_set_param_int64(OSINFO_ENTITY(installScript),
                                  OSINFO_INSTALL_SCRIPT_PROP_INJECTION_METHOD,
                                  injection_methods);

    g_type_class_unref(flags_class);
    g_free(nodes);


    osinfo_db_add_install_script(loader->priv->db, installScript);

    return;

 error:
    g_free(nodes);
    g_free(value);
    g_object_unref(installScript);
}

static OsinfoMedia *osinfo_loader_media(OsinfoLoader *loader,
                                         xmlXPathContextPtr ctxt,
                                         xmlNodePtr root,
                                         const gchar *id,
                                         GError **err)
{
    xmlNodePtr *nodes = NULL;
    gint nnodes;
    guint i;

    gchar *arch = (gchar *)xmlGetProp(root, BAD_CAST "arch");
    xmlChar *live = xmlGetProp(root, BAD_CAST OSINFO_MEDIA_PROP_LIVE);
    xmlChar *installer = xmlGetProp(root, BAD_CAST OSINFO_MEDIA_PROP_INSTALLER);
    xmlChar *installer_reboots =
            xmlGetProp(root, BAD_CAST OSINFO_MEDIA_PROP_INSTALLER_REBOOTS);
    xmlChar *eject_after_install =
            xmlGetProp(root, BAD_CAST OSINFO_MEDIA_PROP_EJECT_AFTER_INSTALL);
    xmlChar *installer_script =
            xmlGetProp(root, BAD_CAST OSINFO_MEDIA_PROP_INSTALLER_SCRIPT);
    const OsinfoEntityKey keys[] = {
        { OSINFO_MEDIA_PROP_URL, G_TYPE_STRING },
        { OSINFO_MEDIA_PROP_KERNEL, G_TYPE_STRING },
        { OSINFO_MEDIA_PROP_INITRD, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };

    OsinfoMedia *media = osinfo_media_new(id, arch);
    xmlFree(arch);

    osinfo_loader_entity(loader, OSINFO_ENTITY(media), keys, ctxt, root, err);
    if (live) {
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_LIVE,
                                (gchar *)live);
        xmlFree(live);
    }

    if (installer) {
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_INSTALLER,
                                (gchar *)installer);
        xmlFree(installer);
    }

    if (installer_reboots) {
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_INSTALLER_REBOOTS,
                                (gchar *)installer_reboots);
        xmlFree(installer_reboots);
    }

    if (eject_after_install) {
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_EJECT_AFTER_INSTALL,
                                (gchar *)eject_after_install);
        xmlFree(eject_after_install);
    }

    if (installer_script) {
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_INSTALLER_SCRIPT,
                                (gchar *)installer_script);
        xmlFree(installer_script);
    }

    nnodes = osinfo_loader_nodeset("./variant", loader, ctxt, &nodes, err);
    if (error_is_set(err)) {
        g_object_unref(media);
        return NULL;
    }

    for (i = 0; i < nnodes; i++) {
        gchar *variant_id = (gchar *)xmlGetProp(nodes[i], BAD_CAST "id");
        osinfo_entity_add_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_VARIANT,
                                variant_id);
        xmlFree(variant_id);
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./iso/*", loader, ctxt, &nodes, err);
    if (error_is_set(err)) {
        g_object_unref(media);
        return NULL;
    }

    for (i = 0; i < nnodes; i++) {
        if (!nodes[i]->children ||
            nodes[i]->children->type != XML_TEXT_NODE ||
            (!g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_MEDIA_PROP_VOLUME_ID) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_MEDIA_PROP_SYSTEM_ID) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_MEDIA_PROP_PUBLISHER_ID) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_MEDIA_PROP_APPLICATION_ID) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_MEDIA_PROP_LANG) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_MEDIA_PROP_VOLUME_SIZE)))
            continue;

        if (g_str_equal((const gchar *)nodes[i]->name,
                        OSINFO_MEDIA_PROP_LANG)) {
            gchar *regex = (gchar *)xmlGetProp(nodes[i], BAD_CAST "regex");
            if (g_strcmp0(regex, "true") == 0) {
                gchar *datamap;
                osinfo_entity_set_param(OSINFO_ENTITY(media),
                                        OSINFO_MEDIA_PROP_LANG_REGEX,
                                        (const gchar *)nodes[i]->children->content);
                datamap = (gchar *)xmlGetProp(nodes[i], BAD_CAST OSINFO_MEDIA_PROP_LANG_MAP);
                if (datamap != NULL)
                    osinfo_entity_set_param(OSINFO_ENTITY(media),
                                            OSINFO_MEDIA_PROP_LANG_MAP,
                                            datamap);
                xmlFree(datamap);
            } else {
                osinfo_entity_add_param(OSINFO_ENTITY(media),
                                        OSINFO_MEDIA_PROP_LANG,
                                        (const gchar *)nodes[i]->children->content);
            }
            xmlFree(regex);
        } else {
            osinfo_entity_set_param(OSINFO_ENTITY(media),
                                    (const gchar *)nodes[i]->name,
                                    (const gchar *)nodes[i]->children->content);
        }
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./installer/script", loader, ctxt, &nodes,
                                   err);
    if (error_is_set(err)) {
        g_object_unref(media);
        return NULL;
    }

    for (i = 0; i < nnodes; i++) {
        OsinfoInstallScript *script;
        gchar *scriptid;

        scriptid = (gchar *)xmlGetProp(nodes[i], BAD_CAST "id");
        if (scriptid == NULL) {
            OSINFO_LOADER_SET_ERROR(err, _("Missing Media install script property"));
            g_object_unref(media);
            return NULL;
        }

        script = osinfo_loader_get_install_script(loader, scriptid);
        xmlFree(scriptid);

        osinfo_media_add_install_script(media, script);
    }

    g_free(nodes);

    return media;
}

static OsinfoTree *osinfo_loader_tree(OsinfoLoader *loader,
                                         xmlXPathContextPtr ctxt,
                                         xmlNodePtr root,
                                         const gchar *id,
                                         GError **err)
{
    xmlNodePtr *nodes = NULL;
    gint nnodes;
    guint i;

    gchar *arch = (gchar *)xmlGetProp(root, BAD_CAST "arch");
    const OsinfoEntityKey keys[] = {
        { OSINFO_TREE_PROP_URL, G_TYPE_STRING },
        { OSINFO_TREE_PROP_KERNEL, G_TYPE_STRING },
        { OSINFO_TREE_PROP_INITRD, G_TYPE_STRING },
        { OSINFO_TREE_PROP_BOOT_ISO, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };

    OsinfoTree *tree = osinfo_tree_new(id, arch);
    xmlFree(arch);

    nnodes = osinfo_loader_nodeset("./variant", loader, ctxt, &nodes, err);
    if (error_is_set(err)) {
        g_object_unref(tree);
        return NULL;
    }

    for (i = 0; i < nnodes; i++) {
        gchar *variant_id = (gchar *)xmlGetProp(nodes[i], BAD_CAST "id");
        osinfo_entity_add_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_VARIANT,
                                variant_id);
        xmlFree(variant_id);
    }

    osinfo_loader_entity(loader, OSINFO_ENTITY(tree), keys, ctxt, root, err);

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./treeinfo/*", loader, ctxt, &nodes, err);
    if (error_is_set(err)) {
        g_object_unref(G_OBJECT(tree));
        return NULL;
    }

    osinfo_entity_set_param_boolean(OSINFO_ENTITY(tree),
                                    OSINFO_TREE_PROP_HAS_TREEINFO,
                                    nnodes == 0 ? FALSE : TRUE);

    for (i = 0; i < nnodes; i++) {
        if (!nodes[i]->children ||
            nodes[i]->children->type != XML_TEXT_NODE)
            continue;

        if (g_str_equal((const gchar *)nodes[i]->name,
                        OSINFO_TREE_PROP_TREEINFO_FAMILY + strlen("treeinfo-")))
            osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                    OSINFO_TREE_PROP_TREEINFO_FAMILY,
                                    (const gchar *)nodes[i]->children->content);
        else if (g_str_equal((const gchar *)nodes[i]->name,
                             OSINFO_TREE_PROP_TREEINFO_VARIANT + strlen("treeinfo-")))
            osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                    OSINFO_TREE_PROP_TREEINFO_VARIANT,
                                    (const gchar *)nodes[i]->children->content);
        else if (g_str_equal((const gchar *)nodes[i]->name,
                             OSINFO_TREE_PROP_TREEINFO_VERSION + strlen("treeinfo-")))
            osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                    OSINFO_TREE_PROP_TREEINFO_VERSION,
                                    (const gchar *)nodes[i]->children->content);
        else if (g_str_equal((const gchar *)nodes[i]->name,
                             OSINFO_TREE_PROP_TREEINFO_ARCH + strlen("treeinfo-")))
            osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                    OSINFO_TREE_PROP_TREEINFO_ARCH,
                                    (const gchar *)nodes[i]->children->content);
    }

    g_free(nodes);

    return tree;
}

static OsinfoFirmware *osinfo_loader_firmware(OsinfoLoader *loader,
                                              xmlXPathContextPtr ctxt,
                                              xmlNodePtr root,
                                              const gchar *id,
                                              GError **err)
{
    gchar *arch = (gchar *)xmlGetProp(root, BAD_CAST "arch");
    gchar *type = (gchar *)xmlGetProp(root, BAD_CAST "type");
    gchar *supported = (gchar *)xmlGetProp(root, BAD_CAST "supported");
    gboolean is_supported = TRUE;

    OsinfoFirmware *firmware = osinfo_firmware_new(id, arch, type);
    xmlFree(arch);
    xmlFree(type);

    if (supported != NULL) {
        is_supported = g_str_equal(supported, "true");
        xmlFree(supported);
    }

    osinfo_entity_set_param_boolean(OSINFO_ENTITY(firmware),
                                    OSINFO_FIRMWARE_PROP_SUPPORTED,
                                    is_supported);

    return firmware;
}

static OsinfoImage *osinfo_loader_image(OsinfoLoader *loader,
                                        xmlXPathContextPtr ctxt,
                                        xmlNodePtr root,
                                        const gchar *id,
                                        GError **err)
{
    const OsinfoEntityKey keys[] = {
        { OSINFO_IMAGE_PROP_URL, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };
    xmlNodePtr *nodes = NULL;
    gint nnodes;
    guint i;
    gchar *arch = (gchar *)xmlGetProp(root, BAD_CAST "arch");
    gchar *format = (gchar *)xmlGetProp(root,
                                        BAD_CAST OSINFO_IMAGE_PROP_FORMAT);
    gchar *cloud_init = (gchar *)xmlGetProp(root,
                                            BAD_CAST OSINFO_IMAGE_PROP_CLOUD_INIT);
    OsinfoImage *image = osinfo_image_new(id, arch, format);
    xmlFree(arch);
    xmlFree(format);

    nnodes = osinfo_loader_nodeset("./variant", loader, ctxt, &nodes, err);
    if (error_is_set(err)) {
        g_object_unref(image);
        return NULL;
    }

    for (i = 0; i < nnodes; i++) {
        gchar *variant_id = (gchar *)xmlGetProp(nodes[i], BAD_CAST "id");
        osinfo_entity_add_param(OSINFO_ENTITY(image),
                                OSINFO_IMAGE_PROP_VARIANT,
                                variant_id);
        xmlFree(variant_id);
    }
    g_free(nodes);

    osinfo_loader_entity(loader, OSINFO_ENTITY(image), keys, ctxt, root, err);
    if (cloud_init) {
        osinfo_entity_set_param(OSINFO_ENTITY(image),
                                OSINFO_IMAGE_PROP_CLOUD_INIT,
                                (gchar *)cloud_init);

        xmlFree(cloud_init);
    }

    return image;
}

static OsinfoOsVariant *osinfo_loader_os_variant(OsinfoLoader *loader,
                                                 xmlXPathContextPtr ctxt,
                                                 xmlNodePtr root,
                                                 GError **err)
{
    const OsinfoEntityKey keys[] = {
        { OSINFO_OS_VARIANT_PROP_NAME, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };

    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");
    OsinfoOsVariant *variant= osinfo_os_variant_new(id);
    xmlFree(id);

    osinfo_loader_entity(loader, OSINFO_ENTITY(variant), keys, ctxt, root, err);

    return variant;
}

static OsinfoResources *osinfo_loader_resources(OsinfoLoader *loader,
                                                xmlXPathContextPtr ctxt,
                                                xmlNodePtr root,
                                                const gchar *id,
                                                const gchar *name,
                                                GError **err)
{
    xmlNodePtr *nodes = NULL;
    OsinfoResources *resources = NULL;
    guint i;

    gchar *arch = (gchar *)xmlGetProp(root, BAD_CAST "arch");
    gchar *inherit_str = (gchar *)xmlGetProp(root, BAD_CAST "inherit");
    gchar *node_path = g_strjoin("/", ".", name, "*", NULL);
    gint nnodes = osinfo_loader_nodeset(node_path, loader, ctxt, &nodes, err);
    gboolean inherit = inherit_str != NULL && g_str_equal(inherit_str, "true");
    g_free(node_path);
    g_free(inherit_str);
    if (error_is_set(err) || (!inherit && nnodes < 1))
        goto EXIT;

    resources = osinfo_resources_new(id, arch);
    osinfo_resources_set_inherit(resources, inherit);

    for (i = 0; i < nnodes; i++) {
        if (!nodes[i]->children ||
            nodes[i]->children->type != XML_TEXT_NODE ||
            (!g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_RESOURCES_PROP_CPU) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_RESOURCES_PROP_N_CPUS) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_RESOURCES_PROP_RAM) &&
             !g_str_equal((const gchar *)nodes[i]->name,
                          OSINFO_RESOURCES_PROP_STORAGE)))
            continue;

        osinfo_entity_set_param(OSINFO_ENTITY(resources),
                                (const gchar *)nodes[i]->name,
                                (const gchar *)nodes[i]->children->content);
    }

EXIT:
    g_free(nodes);
    xmlFree(arch);

    return resources;
}

static void osinfo_loader_resources_list(OsinfoLoader *loader,
                                         xmlXPathContextPtr ctxt,
                                         xmlNodePtr root,
                                         const gchar *id,
                                         OsinfoOs *os,
                                         GError **err)
{
    OsinfoResources *resources;

    resources = osinfo_loader_resources(loader, ctxt, root, id, "minimum", err);
    if (error_is_set(err))
        goto EXIT;

    if (resources != NULL)
        osinfo_os_add_minimum_resources(os, resources);

    g_clear_object(&resources);
    resources = osinfo_loader_resources(loader, ctxt, root, id, "recommended", err);
    if (error_is_set(err))
        goto EXIT;

    if (resources != NULL)
        osinfo_os_add_recommended_resources(os, resources);

    g_clear_object(&resources);
    resources = osinfo_loader_resources(loader, ctxt, root, id, "maximum", err);
    if (error_is_set(err))
        goto EXIT;

    if (resources != NULL)
        osinfo_os_add_maximum_resources(os, resources);

    g_clear_object(&resources);
    resources = osinfo_loader_resources(loader, ctxt, root, id, "network-install", err);
    if (error_is_set(err))
        goto EXIT;

    if (resources != NULL)
        osinfo_os_add_network_install_resources(os, resources);

EXIT:
    g_clear_object(&resources);
}

static OsinfoDeviceDriver *osinfo_loader_driver(OsinfoLoader *loader,
                                                xmlXPathContextPtr ctxt,
                                                xmlNodePtr root,
                                                const gchar *id,
                                                GError **err)
{
    xmlNodePtr *nodes = NULL;
    gint nnodes;
    guint i;

    xmlChar *arch = xmlGetProp(root, BAD_CAST OSINFO_DEVICE_DRIVER_PROP_ARCHITECTURE);
    xmlChar *location = xmlGetProp(root, BAD_CAST OSINFO_DEVICE_DRIVER_PROP_LOCATION);
    xmlChar *preinst = xmlGetProp(root, BAD_CAST OSINFO_DEVICE_DRIVER_PROP_PRE_INSTALLABLE);
    xmlChar *is_signed = xmlGetProp(root, BAD_CAST OSINFO_DEVICE_DRIVER_PROP_SIGNED);
    xmlChar *priority = xmlGetProp(root, BAD_CAST OSINFO_DEVICE_DRIVER_PROP_PRIORITY);

    OsinfoDeviceDriver *driver = osinfo_device_driver_new(id);

    if (arch) {
        osinfo_entity_set_param(OSINFO_ENTITY(driver),
                                OSINFO_DEVICE_DRIVER_PROP_ARCHITECTURE,
                                (gchar *)arch);
        xmlFree(arch);
    }

    if (location) {
        osinfo_entity_set_param(OSINFO_ENTITY(driver),
                                OSINFO_DEVICE_DRIVER_PROP_LOCATION,
                                (gchar *)location);
        xmlFree(location);
    }

    if (preinst) {
        osinfo_entity_set_param(OSINFO_ENTITY(driver),
                                OSINFO_DEVICE_DRIVER_PROP_PRE_INSTALLABLE,
                                (gchar *)preinst);
        xmlFree(preinst);
    }

    if (is_signed) {
        osinfo_entity_set_param(OSINFO_ENTITY(driver),
                                OSINFO_DEVICE_DRIVER_PROP_SIGNED,
                                (gchar *)is_signed);
        xmlFree(is_signed);
    }

    if (priority) {
        osinfo_entity_set_param(OSINFO_ENTITY(driver),
                                OSINFO_DEVICE_DRIVER_PROP_PRIORITY,
                                (gchar *)priority);
        xmlFree(priority);
    }

    nnodes = osinfo_loader_nodeset("./*", loader, ctxt, &nodes, err);
    if (error_is_set(err)) {
        g_object_unref(G_OBJECT(driver));
        return NULL;
    }

    for (i = 0; i < nnodes; i++) {
        if (nodes[i]->children &&
            nodes[i]->children->type == XML_TEXT_NODE &&
            (g_str_equal((const gchar *)nodes[i]->name,
                         OSINFO_DEVICE_DRIVER_PROP_FILE))) {
            osinfo_entity_add_param(OSINFO_ENTITY(driver),
                                    (const gchar *)nodes[i]->name,
                                    (const gchar *)nodes[i]->children->content);
        } else if (g_str_equal((const gchar *)nodes[i]->name,
                               OSINFO_DEVICE_DRIVER_PROP_DEVICE)) {
            xmlChar *device_id = xmlGetProp(nodes[i], BAD_CAST "id");
            OsinfoDevice *device = osinfo_loader_get_device(loader,
                                                            (gchar *)device_id);
            xmlFree(device_id);

            osinfo_device_driver_add_device(driver, device);
        }
    }

    g_free(nodes);

    return driver;
}


static void osinfo_loader_os(OsinfoLoader *loader,
                             const gchar *relpath,
                             xmlXPathContextPtr ctxt,
                             xmlNodePtr root,
                             GError **err)
{
    xmlNodePtr *nodes = NULL;
    guint i;
    int nnodes;

    gchar *id = (gchar *)xmlGetProp(root, BAD_CAST "id");
    const OsinfoEntityKey keys[] = {
        { OSINFO_OS_PROP_FAMILY, G_TYPE_STRING },
        { OSINFO_OS_PROP_DISTRO, G_TYPE_STRING },
        { OSINFO_OS_PROP_RELEASE_STATUS, G_TYPE_STRING },
        { OSINFO_OS_PROP_KERNEL_URL_ARGUMENT, G_TYPE_STRING },
        { OSINFO_OS_PROP_CLOUD_IMAGE_USERNAME, G_TYPE_STRING },
        { NULL, G_TYPE_INVALID }
    };
    OsinfoOs *os;

    if (!id) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing os id property"));
        return;
    }
    if (!osinfo_loader_check_id(relpath, "os", id)) {
        xmlFree(id);
        return;
    }

    os = osinfo_loader_get_os(loader, id);
    g_hash_table_remove(loader->priv->entity_refs, id);

    osinfo_loader_entity(loader, OSINFO_ENTITY(os), keys, ctxt, root, err);
    if (error_is_set(err))
        goto cleanup;

    osinfo_loader_product(loader, OSINFO_PRODUCT(os), ctxt, root, err);
    if (error_is_set(err))
        goto cleanup;

    osinfo_loader_device_link(loader, OSINFO_ENTITY(os),
                              "./devices/device", ctxt, root, err);
    if (error_is_set(err))
        goto cleanup;

    nnodes = osinfo_loader_nodeset("./firmware", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *firmware_id;
        OsinfoFirmware *firmware;
        xmlNodePtr saved = ctxt->node;

        ctxt->node = nodes[i];
        firmware_id = osinfo_build_internal_id(relpath, id, i);
        firmware = osinfo_loader_firmware(loader, ctxt, nodes[i], firmware_id, err);
        g_free(firmware_id);
        ctxt->node = saved;
        if (error_is_set(err))
            goto cleanup;

        osinfo_os_add_firmware(os, firmware);
        g_object_unref(firmware);
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./media", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *media_id;
        OsinfoMedia *media;
        xmlNodePtr saved = ctxt->node;

        ctxt->node = nodes[i];
        media_id = osinfo_build_internal_id(relpath, id, i);
        media = osinfo_loader_media(loader, ctxt, nodes[i], media_id, err);
        g_free(media_id);
        ctxt->node = saved;
        if (error_is_set(err))
            goto cleanup;

        osinfo_os_add_media(os, media);
        g_object_unref(media);
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./tree", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *tree_id;
        OsinfoTree *tree;
        xmlNodePtr saved = ctxt->node;

        ctxt->node = nodes[i];
        tree_id = osinfo_build_internal_id(relpath, id, i);
        tree = osinfo_loader_tree(loader, ctxt, nodes[i], tree_id, err);
        g_free(tree_id);
        ctxt->node = saved;
        if (error_is_set(err))
            goto cleanup;

        osinfo_os_add_tree(os, tree);
        g_object_unref(G_OBJECT(tree));
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./image", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *image_id;
        OsinfoImage *image;
        xmlNodePtr saved = ctxt->node;

        ctxt->node = nodes[i];
        image_id = osinfo_build_internal_id(relpath, id, i);
        image = osinfo_loader_image(loader, ctxt, nodes[i], image_id, err);
        g_free(image_id);
        ctxt->node = saved;
        if (error_is_set(err))
            goto cleanup;

        osinfo_os_add_image(os, image);
        g_object_unref(G_OBJECT(image));
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./variant", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        OsinfoOsVariant *variant;
        xmlNodePtr saved = ctxt->node;
        ctxt->node = nodes[i];
        variant = osinfo_loader_os_variant(loader,
                                           ctxt,
                                           nodes[i],
                                           err);
        ctxt->node = saved;
        if (error_is_set(err))
            goto cleanup;

        osinfo_os_add_variant(os, variant);
        g_object_unref(G_OBJECT(variant));
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./resources", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *resources_id;
        xmlNodePtr saved = ctxt->node;
        ctxt->node = nodes[i];
        resources_id = osinfo_build_internal_id(relpath, id, i);

        osinfo_loader_resources_list(loader,
                                     ctxt,
                                     nodes[i],
                                     resources_id,
                                     os,
                                     err);
        g_free(resources_id);
        ctxt->node = saved;
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./installer/script", loader, ctxt, &nodes,
                                   err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        OsinfoInstallScript *script;
        gchar *scriptid = (gchar *)xmlGetProp(nodes[i], BAD_CAST "id");
        if (!scriptid) {
            OSINFO_LOADER_SET_ERROR(err, _("Missing OS install script property"));
            goto cleanup;
        }
        script = osinfo_loader_get_install_script(loader, scriptid);
        xmlFree(scriptid);

        osinfo_os_add_install_script(os, script);
    }

    g_clear_pointer(&nodes, g_free);
    nnodes = osinfo_loader_nodeset("./driver", loader, ctxt, &nodes, err);
    if (error_is_set(err))
        goto cleanup;

    for (i = 0; i < nnodes; i++) {
        gchar *driver_id;
        OsinfoDeviceDriver *driver;
        xmlNodePtr saved = ctxt->node;

        ctxt->node = nodes[i];
        driver_id = osinfo_build_internal_id(relpath, id, i);
        driver= osinfo_loader_driver(loader,
                                     ctxt,
                                     nodes[i],
                                     driver_id,
                                     err);
        g_free(driver_id);
        ctxt->node = saved;
        if (error_is_set(err))
            break;

        osinfo_os_add_device_driver(os, driver);
        g_object_unref(driver);
    }

cleanup:
    g_free(nodes);
    xmlFree(id);
}

static void osinfo_loader_root(OsinfoLoader *loader,
                               const gchar *relpath,
                               xmlXPathContextPtr ctxt,
                               xmlNodePtr root,
                               GError **err)
{
    /*
     * File assumed to contain data in XML format. All data
     * is within <libosinfo>...</libosinfo>. The following steps are taken
     * to process the data within the file:
     *
     * Advance till we are at opening <libosinfo> tag.
     * While true:
     *   Advance tag
     *   If closing libosinfo tag, break
     *   If non element tag, continue
     *   If element tag, and element is not os, platform, device,
     *   datamap, deployment or install-script, error
     *   Else, switch on tag type and handle reading in data
     * After loop, return success if no error
     * If there was an error, clean up lib data acquired so far
     */
    xmlNodePtr it;

    if (!xmlStrEqual(root->name, BAD_CAST "libosinfo")) {
        OSINFO_LOADER_SET_ERROR(err, _("Incorrect root element"));
        return;
    }

    for (it = root->children; it; it = it->next) {
        xmlNodePtr saved;

        if (it->type != XML_ELEMENT_NODE)
            continue;

        saved = ctxt->node;
        ctxt->node = it;

        if (xmlStrEqual(it->name, BAD_CAST "device"))
            osinfo_loader_device(loader, relpath, ctxt, it, err);

        else if (xmlStrEqual(it->name, BAD_CAST "platform"))
            osinfo_loader_platform(loader, relpath, ctxt, it, err);

        else if (xmlStrEqual(it->name, BAD_CAST "os"))
            osinfo_loader_os(loader, relpath, ctxt, it, err);

        else if (xmlStrEqual(it->name, BAD_CAST "deployment"))
            osinfo_loader_deployment(loader, relpath, ctxt, it, err);

        else if (xmlStrEqual(it->name, BAD_CAST "install-script"))
            osinfo_loader_install_script(loader, relpath, ctxt, it, err);

        else if (xmlStrEqual(it->name, BAD_CAST "datamap"))
            osinfo_loader_datamap(loader, relpath, ctxt, it, err);

        ctxt->node = saved;

        if (error_is_set(err))
            return;
    }
}

static void
catchXMLError(void *ctx, const char *msg ATTRIBUTE_UNUSED, ...)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;

    if (ctxt && ctxt->_private) {
        GError **err = ctxt->_private;
        if (!error_is_set(err)) {
            gchar *xmlmsg;
            if (ctxt->lastError.file) {
                xmlmsg = g_strdup_printf("%s:%d: %s",
                                         ctxt->lastError.file,
                                         ctxt->lastError.line,
                                         ctxt->lastError.message);
            } else {
                xmlmsg = g_strdup_printf("at line %d: %s",
                                         ctxt->lastError.line,
                                         ctxt->lastError.message);
            }
            OSINFO_LOADER_SET_ERROR(ctxt->_private, xmlmsg);
            g_free(xmlmsg);
        }
    }
}

static void osinfo_loader_process_xml(OsinfoLoader *loader,
                                      const gchar *relpath,
                                      const gchar *xmlStr,
                                      const gchar *src,
                                      GError **err)
{
    xmlParserCtxtPtr pctxt;
    xmlXPathContextPtr ctxt = NULL;
    xmlDocPtr xml = NULL;
    xmlNodePtr root;

    /* Set up a parser context so we can catch the details of XML errors. */
    pctxt = xmlNewParserCtxt();
    if (!pctxt || !pctxt->sax) {
        OSINFO_LOADER_SET_ERROR(err, _("Unable to construct parser context"));
        goto cleanup;
    }

    pctxt->_private = err;
    pctxt->sax->error = catchXMLError;

    xml = xmlCtxtReadDoc(pctxt, BAD_CAST xmlStr, src, NULL,
                         XML_PARSE_NONET |
                         XML_PARSE_NOWARNING);
    if (!xml)
        goto cleanup;

    /* catchXMLError may be called for non-fatal errors. If that happens,
     * xml will be non-NULL but err may be set. This will cause warnings if
     * we try to set err later on, so clear it first.
     */
    g_clear_error(err);

    root = xmlDocGetRootElement(xml);

    if (!root) {
        OSINFO_LOADER_SET_ERROR(err, _("Missing root XML element"));
        goto cleanup;
    }

    ctxt = xmlXPathNewContext(xml);
    if (!ctxt)
        goto cleanup;

    ctxt->node = root;

    osinfo_loader_root(loader, relpath, ctxt, root, err);

 cleanup:
    xmlXPathFreeContext(ctxt);
    xmlFreeDoc(xml);
    xmlFreeParserCtxt(pctxt);
}

static void
osinfo_loader_process_file_reg_ids(OsinfoLoader *loader,
                                   GFile *file,
                                   GHashTable *allentries,
                                   gboolean withSubsys,
                                   const char *baseURI,
                                   const char *busType,
                                   GError **err)
{
    GFileInputStream *is;
    GDataInputStream *dis;

    gchar *data;
    gsize datalen;

    gchar *vendor_buf = NULL;
    gchar *vendor = NULL;
    gchar *vendor_id = NULL;
    gchar *device_buf = NULL;
    gchar *device = NULL;
    gchar *device_id = NULL;
    gchar *subsystem = NULL;
    gchar *subvendor_id = NULL;
    gchar *subdevice_id = NULL;

    is = g_file_read(file, NULL, err);
    if (error_is_set(err))
        return;

    dis = g_data_input_stream_new(G_INPUT_STREAM(is));

    while ((data = g_data_input_stream_read_line(dis, &datalen, NULL, err)) != NULL) {
        char *tmp = data;
        gsize offset = 0;

        if (data[0] == '#')
            goto done;

#define GOT_TAB() \
        (offset < datalen && tmp[offset] == '\t')

#define GOT_ID()                             \
        (((offset + 5) < datalen) &&         \
         g_ascii_isxdigit(tmp[offset])    && \
         g_ascii_isxdigit(tmp[offset+1])  && \
         g_ascii_isxdigit(tmp[offset+2])  && \
         g_ascii_isxdigit(tmp[offset+3])  && \
         g_ascii_isspace(tmp[offset+4]))

#define FREE_BUF(var) \
        g_free(var);  \
        (var) = NULL

#define SAVE_BUF(var) \
        (var) = data; \
        data = NULL

#define WANT_ID(var)                            \
        if (GOT_ID()) {                         \
            (var) = tmp+offset;                 \
            offset += 4;                        \
            tmp[offset] = '\0';                 \
            offset += 1;                        \
        } else {                                \
            goto done;                          \
        }
#define WANT_REST(var)                          \
        (var) = tmp+offset;                     \
        while (*(var) == ' ')                   \
            (var)++

        if (GOT_TAB()) {
            offset++;
            if (!vendor_id)
                goto done;
            if (GOT_TAB()) {
                offset++;
                WANT_ID(subvendor_id);
                WANT_ID(subdevice_id);
                WANT_REST(subsystem);
                ignore_value(subvendor_id);
                ignore_value(subdevice_id);
                ignore_value(subsystem);
            } else {
                gchar *id;
                gchar *key;
                OsinfoLoaderEntityFiles *files;
                OsinfoDevice *dev;
                OsinfoEntity *entity;

                FREE_BUF(device_buf);
                WANT_ID(device_id);
                WANT_REST(device);
                SAVE_BUF(device_buf);

                id = g_strdup_printf("%s/%s/%s/%s",
                                            baseURI, busType, vendor_id, device_id);
                key = g_strdup_printf("/device/%s/%s-%s-%s",
                                              baseURI + 7, busType, vendor_id, device_id);
                files = g_hash_table_lookup(allentries, key);
                g_free(key);
                if (files && files->master) {
                    /* Native database has a matching entry that completely
                     * replaces the external record */
                    continue;
                }

                dev = osinfo_loader_get_device(loader, id);
                g_hash_table_remove(loader->priv->entity_refs, id);
                entity = OSINFO_ENTITY(dev);
                osinfo_entity_set_param(entity,
                                        OSINFO_DEVICE_PROP_VENDOR_ID,
                                        vendor_id);
                osinfo_entity_set_param(entity,
                                        OSINFO_DEVICE_PROP_VENDOR,
                                        vendor);
                osinfo_entity_set_param(entity,
                                        OSINFO_DEVICE_PROP_PRODUCT_ID,
                                        device_id);
                osinfo_entity_set_param(entity,
                                        OSINFO_DEVICE_PROP_PRODUCT,
                                        device);
                osinfo_entity_set_param(entity,
                                        OSINFO_DEVICE_PROP_BUS_TYPE,
                                        busType);
                g_free(id);
            }
        } else {
            FREE_BUF(vendor_buf);
            WANT_ID(vendor_id);
            WANT_REST(vendor);
            SAVE_BUF(vendor_buf);
        }

    done:
        g_free(data);
        if (error_is_set(err))
            break;
    }

    FREE_BUF(device_buf);
    FREE_BUF(vendor_buf);
    g_object_unref(dis);
    g_object_unref(is);
}

static void
osinfo_loader_process_file_reg_usb(OsinfoLoader *loader,
                                   GFile *file,
                                   GHashTable *allentries,
                                   GError **err)
{
    osinfo_loader_process_file_reg_ids(loader,
                                       file,
                                       allentries,
                                       FALSE,
                                       "http://usb.org",
                                       "usb",
                                       err);
}

static void
osinfo_loader_process_file_reg_pci(OsinfoLoader *loader,
                                   GFile *file,
                                   GHashTable *allentries,
                                   GError **err)
{
    osinfo_loader_process_file_reg_ids(loader,
                                       file,
                                       allentries,
                                       TRUE,
                                       "http://pcisig.com",
                                       "pci",
                                       err);
}

static void
osinfo_loader_process_file_reg_xml(OsinfoLoader *loader,
                                   GFile *base,
                                   GFile *file,
                                   GError **err)
{
    gchar *xml = NULL;
    gsize xmlLen;
    gchar *relpath = NULL;
    gchar *uri = NULL;

    g_file_load_contents(file, NULL, &xml, &xmlLen, NULL, err);
    if (error_is_set(err))
        return;

    if (xmlLen == 0)
        return;
    if (base) {
        relpath = g_file_get_relative_path(base, file);
        if (relpath == NULL) {
            relpath = g_file_get_path(file);
            g_warning("File %s does not have expected prefix", relpath);
        }
    }
    uri = g_file_get_uri(file);
    osinfo_loader_process_xml(loader,
                              relpath,
                              xml,
                              uri,
                              err);
    g_free(uri);
    g_free(xml);
    g_free(relpath);
}


static void osinfo_loader_entity_files_free(OsinfoLoaderEntityFiles *files)
{
    if (!files)
        return;
    g_list_free_full(files->extensions, g_object_unref);
    if (files->master)
        g_object_unref(files->master);
    g_free(files);
}


static void osinfo_loader_entity_files_add_path(GHashTable *entries,
                                                GFile *base,
                                                GFile *ent)
{
    /*
     * We have paths which are either:
     *
     *   $DB_ROOT/os/fedoraproject.org/fedora-19.xml
     *   $DB_ROOT/os/fedoraproject.org/fedora-19.d/fragment.xml
     *
     * And need to extract the prefix
     *
     *   os/fedoraproject.org/fedora-19
     *
     * We assume that no domain names end with '.d'
     */
    gchar *path = g_file_get_path(ent);
    const gchar *relpath = path;
    gchar *dirname;
    gchar *key = NULL;
    gboolean extension = FALSE;
    OsinfoLoaderEntityFiles *entry;
    gchar *basepath = NULL;

    if (base) {
        basepath = g_file_get_path(base);

        g_object_set_data(G_OBJECT(ent), "base", base);

        if (g_str_has_prefix(path, basepath))
            relpath += strlen(basepath);
    }

    dirname = g_path_get_dirname(relpath);

    if (g_str_has_suffix(dirname, ".d")) {
        key = g_strndup(dirname, strlen(dirname) - 2);
        extension = TRUE;
    } else if (g_str_has_suffix(relpath, ".xml")) {
        key = g_strndup(relpath, strlen(relpath) - 4);
    } else {
        /* This should not be reached, since we already
         * filtered to only have files in .xml
         */
        goto error;
    }

    entry = g_hash_table_lookup(entries, key);
    if (!entry) {
        entry = g_new0(OsinfoLoaderEntityFiles, 1);
        g_hash_table_insert(entries, g_strdup(key), entry);
    }
    g_object_ref(ent);
    if (extension) {
        entry->extensions = g_list_append(entry->extensions, ent);
    } else {
        if (entry->master) {
            g_warning("Unexpected duplicate master file %s", path);
        }
        entry->master = ent;
    }

 error:
    g_free(key);
    g_free(dirname);
    g_free(path);
    g_free(basepath);
}

static gint osinfo_sort_files(gconstpointer a, gconstpointer b)
{
    GFileInfo *ai = (GFileInfo *)a;
    GFileInfo *bi = (GFileInfo *)b;
    const gchar *an = g_file_info_get_name(ai);
    const gchar *bn = g_file_info_get_name(bi);

    return strcmp(an, bn);
}

static void osinfo_loader_find_files(OsinfoLoader *loader,
                                     GFile *base,
                                     GFile *file,
                                     GHashTable *entries,
                                     gboolean skipMissing,
                                     GError **err)
{
    GError *error = NULL;
    GFileInfo *info;
    GFileType type;

    info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (error) {
        if (error->code == G_IO_ERROR_NOT_FOUND && skipMissing) {
            g_error_free(error);
            return;
        }
        g_propagate_error(err, error);
        return;
    }
    type = g_file_info_get_attribute_uint32(info,
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE);
    g_object_unref(info);
    if (type == G_FILE_TYPE_REGULAR) {
        char *path = g_file_get_path(file);
        g_warning("Using a file (%s) as a database location is deprecated, use a directory instead",
                  path);
        g_free(path);
        osinfo_loader_entity_files_add_path(entries, NULL, file);
    } else if (type == G_FILE_TYPE_DIRECTORY) {
        GFileEnumerator *ents;
        GList *children = NULL, *tmp;
        ents = g_file_enumerate_children(file,
                                         "standard::*",
                                         G_FILE_QUERY_INFO_NONE,
                                         NULL,
                                         &error);
        if (error) {
            if (error->code == G_IO_ERROR_NOT_FOUND) {
                g_error_free(error);
                return;
            }
            g_propagate_error(err, error);
            return;
        }

        while ((info = g_file_enumerator_next_file(ents, NULL, err)) != NULL) {
            children = g_list_append(children, info);
        }

        children = g_list_sort(children, osinfo_sort_files);
        tmp = children;
        while (tmp) {
            const gchar *name;
            GFile *ent;

            info = tmp->data;
            name = g_file_info_get_name(info);
            ent = g_file_get_child(file, name);
            type = g_file_info_get_attribute_uint32(info,
                                                    G_FILE_ATTRIBUTE_STANDARD_TYPE);
            if (type == G_FILE_TYPE_DIRECTORY) {
                if (!g_str_equal(name, "schema"))
                    osinfo_loader_find_files(loader, base, ent, entries, FALSE, &error);
            } else {
                if (g_str_has_suffix(name, ".xml")) {
                    osinfo_loader_entity_files_add_path(entries, base, ent);
                } else if (!g_str_equal(name, "LICENSE") &&
                           !g_str_equal(name, "VERSION") &&
                           !g_str_has_suffix(name, "~") &&
                           !g_str_has_suffix(name, ".bak")) {
                    g_autofree gchar *path = g_file_get_path(ent);
                    g_printerr("Ignoring %s with missing '.xml' extension\n", path);
                }
            }
            g_object_unref(ent);
            g_object_unref(info);

            if (error) {
                g_propagate_error(err, error);
                break;
            }

            tmp = tmp->next;
        }
        g_object_unref(ents);
        g_list_free(children);
    } else if (type == G_FILE_TYPE_UNKNOWN) {
        g_autofree gchar *path = g_file_get_path(file);
        g_autofree gchar *msg = g_strdup_printf("Can't read path %s", path);
        if (skipMissing) {
            /* This is a work-around for
             * <https://gitlab.gnome.org/GNOME/glib/-/issues/1237>. If the
             * lstat() call underlying our g_file_query_info() call at the top
             * of this function fails for "path" with EACCES, then
             * g_file_query_info() should fail, and the "skipMissing" branch up
             * there should suppress the error and return cleanly.
             * Unfortunately, _g_local_file_info_get() masks the lstat()
             * failure, g_file_info_get_attribute_uint32() is reached above,
             * and returns G_FILE_TYPE_UNKNOWN for the file that could never be
             * accessed. So we need to consider "skipMissing" here too.
             */
            g_warning("%s", msg);
            return;
        }
        OSINFO_LOADER_SET_ERROR(&error, msg);
        g_propagate_error(err, error);
    } else {
        OSINFO_LOADER_SET_ERROR(&error, "Unexpected file type");
        g_propagate_error(err, error);
    }
}


typedef enum {
    OSINFO_DATA_FORMAT_NATIVE,
    OSINFO_DATA_FORMAT_PCI_IDS,
    OSINFO_DATA_FORMAT_USB_IDS,
} OsinfoLoaderDataFormat;

static void osinfo_loader_process_list(OsinfoLoader *loader,
                                       GFile **dirs,
                                       gboolean skipMissing,
                                       GError **err)
{
    GError *lerr = NULL;
    GFile **tmp;
    GHashTable *allentries = g_hash_table_new_full(g_str_hash,
                                                   g_str_equal,
                                                   g_free,
                                                   (GDestroyNotify)osinfo_loader_entity_files_free);
    GHashTableIter iter;
    gpointer key, value;

    /* Phase 1: gather the files in each native format location */
    tmp = dirs;
    while (tmp && *tmp) {
        OsinfoLoaderDataFormat fmt = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(*tmp), "data-format"));
        GHashTable *entries;

        if (fmt != OSINFO_DATA_FORMAT_NATIVE) {
            tmp++;
            continue;
        }

        entries = g_hash_table_new_full(g_str_hash,
                                        g_str_equal,
                                        g_free,
                                        (GDestroyNotify)osinfo_loader_entity_files_free);

        osinfo_loader_find_files(loader, *tmp, *tmp, entries, skipMissing, &lerr);
        if (lerr) {
            g_propagate_error(err, lerr);
            g_hash_table_unref(entries);
            goto cleanup;
        }

        /* 'entries' contains a list of files from this location, which
         * we need to merge with any previously gathered files.
         *
         * If 'allentries' already contains an entry with the matching key
         *
         *  => If 'entries' has the master file present, this completely
         *     replaces the data in 'allentries.'
         *  => Else we just augment the extensions in 'allentries'
         */
        g_hash_table_iter_init(&iter, entries);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            const gchar *path = key;
            OsinfoLoaderEntityFiles *newfiles = value;
            OsinfoLoaderEntityFiles *oldfiles;

            oldfiles = g_hash_table_lookup(allentries, key);
            if (!oldfiles || newfiles->master) {
                /* Completely new, or replacing master */
                oldfiles = g_new0(OsinfoLoaderEntityFiles, 1);
                oldfiles->master = newfiles->master;
                newfiles->master = NULL;
                oldfiles->extensions = newfiles->extensions;
                newfiles->extensions = NULL;
                g_hash_table_insert(allentries, g_strdup(path), oldfiles);
            } else {
                /* Just augmenting the extensions */
                oldfiles->extensions = g_list_concat(oldfiles->extensions,
                                                     newfiles->extensions);
                newfiles->extensions = NULL;
            }
        }

        g_hash_table_unref(entries);

        tmp++;
    }

    /* Phase 2: load data from non-native locations, filtering based
     * on overrides from native locations */
    tmp = dirs;
    while (tmp && *tmp) {
        OsinfoLoaderDataFormat fmt = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(*tmp), "data-format"));

        switch (fmt) {
        case OSINFO_DATA_FORMAT_NATIVE:
            /* nada */
            break;

        case OSINFO_DATA_FORMAT_PCI_IDS:
            osinfo_loader_process_file_reg_pci(loader, *tmp, allentries, &lerr);
            break;

        case OSINFO_DATA_FORMAT_USB_IDS:
            osinfo_loader_process_file_reg_usb(loader, *tmp, allentries, &lerr);
            break;

        default:
            g_warn_if_reached();
            break;
        }

        if (lerr) {
            g_propagate_error(err, lerr);
            goto cleanup;
        }

        tmp++;
    }

    /* Phase 3: load combined set of files from native locations */
    g_hash_table_iter_init(&iter, allentries);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        OsinfoLoaderEntityFiles *files = value;
        GList *tmpl;
        if (files->master) {
            osinfo_loader_process_file_reg_xml(loader,
                                               g_object_get_data(G_OBJECT(files->master), "base"),
                                               files->master, &lerr);
            if (lerr) {
                g_propagate_error(err, lerr);
                goto cleanup;
            }
        }

        tmpl = files->extensions;
        while (tmpl) {
            GFile *file = tmpl->data;
            osinfo_loader_process_file_reg_xml(loader,
                                               g_object_get_data(G_OBJECT(file), "base"),
                                               file,
                                               &lerr);
            if (lerr) {
                g_propagate_error(err, lerr);
                goto cleanup;
            }

            tmpl = tmpl->next;
        }
    }

    g_hash_table_iter_init(&iter, loader->priv->entity_refs);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        g_warning("Entity %s referenced but not defined", (const char *)key);
    }

 cleanup:
    g_hash_table_unref(allentries);
    g_hash_table_remove_all(loader->priv->entity_refs);
}


/**
 * osinfo_loader_get_db:
 * @loader: the loader object
 *
 * Retrieves the database being populated
 *
 * Returns: (transfer none): the database
 */
OsinfoDb *osinfo_loader_get_db(OsinfoLoader *loader)
{
    g_return_val_if_fail(OSINFO_IS_LOADER(loader), NULL);

    return loader->priv->db;
}

/**
 * osinfo_loader_process_path:
 * @loader: the loader object
 * @path: the fully qualified path
 * @err: (out): filled with error information upon failure
 *
 * Loads data from the specified path. If the path
 * points to a file, that will be loaded as XML
 * Otherwise it can point to a directory which will
 * be recursively traversed, loading all files as XML.
 */
void osinfo_loader_process_path(OsinfoLoader *loader,
                                const gchar *path,
                                GError **err)
{
    GFile *dirs[] = {
        g_file_new_for_path(path),
        NULL,
    };
    g_object_set_data(G_OBJECT(dirs[0]), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_NATIVE));
    osinfo_loader_process_list(loader, dirs, FALSE, err);
    g_object_unref(dirs[0]);
}

/**
 * osinfo_loader_process_uri:
 * @loader: the loader object
 * @uri: the data source URI
 * @err: (out): filled with error information upon failure
 *
 * Loads data from the specified URI. If the URI
 * points to a file, that will be loaded as XML
 * Otherwise it can point to a directory which will
 * be recursively traversed, loading all files as XML.
 */
void osinfo_loader_process_uri(OsinfoLoader *loader,
                               const gchar *uri,
                               GError **err)
{
    GFile *dirs[] = {
        g_file_new_for_uri(uri),
        NULL,
    };
    g_object_set_data(G_OBJECT(dirs[0]), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_NATIVE));
    osinfo_loader_process_list(loader, dirs, FALSE, err);
    g_object_unref(dirs[0]);
}


static GFile *osinfo_loader_get_pci_path(void)
{
    GFile *ids = g_file_new_for_path(PCI_IDS);
    g_object_set_data(G_OBJECT(ids), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_PCI_IDS));
    return ids;
}


static GFile *osinfo_loader_get_usb_path(void)
{
    GFile *ids = g_file_new_for_path(USB_IDS);
    g_object_set_data(G_OBJECT(ids), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_USB_IDS));
    return ids;
}


static GFile *osinfo_loader_get_system_path(void)
{
    GFile *file;
    const gchar *path;

    path = g_getenv("OSINFO_DATA_DIR");
    if (path) {
        char *dbpath;
        static gboolean warned = FALSE;
        if (!warned) {
            g_printerr(_("$OSINFO_DATA_DIR is deprecated, please "
                         "use $OSINFO_SYSTEM_DIR instead. Support "
                         "for $OSINFO_DATA_DIR will be removed "
                         "in a future release\n"));
            warned = TRUE;
        }

        dbpath = g_strdup_printf("%s/db", path);
        file = g_file_new_for_path(path);
        g_free(dbpath);
    } else {
        path = g_getenv("OSINFO_SYSTEM_DIR");
        if (!path)
            path = DATA_DIR "/osinfo";

        file = g_file_new_for_path(path);
    }
    g_object_set_data(G_OBJECT(file), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_NATIVE));
    return file;
}

static GFile *osinfo_loader_get_local_path(void)
{
    GFile *file;
    const gchar *path = g_getenv("OSINFO_LOCAL_DIR");

    if (!path) {
        path = SYS_CONF_DIR "/osinfo";

        if (!g_file_test(path, G_FILE_TEST_IS_DIR)) {
            const gchar *oldpath = SYS_CONF_DIR "/libosinfo/db";
            if (g_file_test(oldpath, G_FILE_TEST_IS_DIR)) {
                static gboolean warned = FALSE;

                if (!warned) {
                    g_printerr(_("%s is deprecated, please use %s instead. "
                                 "Support for %s will be removed in a future "
                                 "release\n"),
                               oldpath, path, oldpath);
                    warned = TRUE;
                }
                path = oldpath;
            }
        }
    }

    file = g_file_new_for_path(path);
    g_object_set_data(G_OBJECT(file), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_NATIVE));
    return file;
}

static GFile *osinfo_loader_get_user_path(void)
{
    GFile *file;
    const gchar *path = g_getenv("OSINFO_USER_DIR");
    const gchar *configdir = g_get_user_config_dir();

    if (path) {
        file = g_file_new_for_path(path);
    } else {
        gchar *dbdir = g_strdup_printf("%s/osinfo", configdir);
        if (!g_file_test(dbdir, G_FILE_TEST_IS_DIR)) {
            static gboolean warned = FALSE;
            gchar *olddir = g_strdup_printf("%s/libosinfo/db", configdir);
            if (g_file_test(olddir, G_FILE_TEST_IS_DIR)) {
                if (!warned) {
                    g_printerr(_("%s is deprecated, please use %s instead. "
                                 "Support for %s will be removed in a future "
                                 "release\n"),
                               olddir, dbdir, olddir);
                    warned = TRUE;
                }
                g_free(dbdir);
                dbdir = olddir;
            } else {
                g_free(olddir);
            }
        }
        file = g_file_new_for_path(dbdir);
        g_free(dbdir);
    }

    g_object_set_data(G_OBJECT(file), "data-format",
                      GINT_TO_POINTER(OSINFO_DATA_FORMAT_NATIVE));
    return file;
}

void osinfo_loader_process_default_path(OsinfoLoader *loader, GError **err)
{
    GFile *dirs[] = {
        osinfo_loader_get_pci_path(),
        osinfo_loader_get_usb_path(),
        osinfo_loader_get_system_path(),
        osinfo_loader_get_local_path(),
        osinfo_loader_get_user_path(),
        NULL,
    };

    osinfo_loader_process_list(loader, dirs, TRUE, err);
    g_object_unref(dirs[0]);
    g_object_unref(dirs[1]);
    g_object_unref(dirs[2]);
    g_object_unref(dirs[3]);
    g_object_unref(dirs[4]);
}

/**
 * osinfo_loader_process_system_path:
 * @loader: the loader object
 * @err: (out): filled with error information upon failure
 *
 * Loads data from the system path.
 *
 * Since: 0.2.8
 */
void osinfo_loader_process_system_path(OsinfoLoader *loader,
                                       GError **err)
{
    GFile *dirs[] = {
        osinfo_loader_get_pci_path(),
        osinfo_loader_get_usb_path(),
        osinfo_loader_get_system_path(),
        NULL,
    };

    osinfo_loader_process_list(loader, dirs, FALSE, err);
    g_object_unref(dirs[0]);
    g_object_unref(dirs[1]);
    g_object_unref(dirs[2]);
}

/**
 * osinfo_loader_process_local_path:
 * @loader: the loader object
 * @err: (out): filled with error information upen failures
 *
 * Loads data from the local path.
 *
 * Since: 0.2.8
 */
void osinfo_loader_process_local_path(OsinfoLoader *loader, GError **err)
{
    GFile *dirs[] = {
        osinfo_loader_get_local_path(),
        NULL,
    };

    osinfo_loader_process_list(loader, dirs, TRUE, err);
    g_object_unref(dirs[0]);
}

/**
 * osinfo_loader_process_user_path:
 * @loader: the loader object
 * @err: (out): filled with error information upen failures
 *
 * Loads data from user path.
 *
 * Since: 0.2.8
 */
void osinfo_loader_process_user_path(OsinfoLoader *loader, GError **err)
{
    GFile *dirs[] = {
        osinfo_loader_get_user_path(),
        NULL,
    };

    osinfo_loader_process_list(loader, dirs, TRUE, err);
    g_object_unref(dirs[0]);
}
