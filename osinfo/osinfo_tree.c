/*
 * libosinfo: An installation tree for a (guest) OS
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
 *   Zeeshan Ali <zeenix@redhat.com>
 *   Arjun Roy <arroy@redhat.com>
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

typedef struct _CreateFromLocationAsyncData CreateFromLocationAsyncData;
struct _CreateFromLocationAsyncData {
    GFile *file;
    gchar *location;
    gchar *treeinfo;

    GTask *res;

    OsinfoTree *tree;
};

static void create_from_location_async_data_free(CreateFromLocationAsyncData *data)
{
    if (data->tree)
        g_object_unref(data->tree);
    g_object_unref(data->file);
    g_object_unref(data->res);

    g_slice_free(CreateFromLocationAsyncData, data);
}

typedef struct _CreateFromLocationData CreateFromLocationData;
struct _CreateFromLocationData {
    GMainLoop *main_loop;

    GAsyncResult *res;
};

static void create_from_location_data_free(CreateFromLocationData *data)
{
    g_object_unref(data->res);
    g_main_loop_unref(data->main_loop);

    g_slice_free(CreateFromLocationData, data);
}

GQuark
osinfo_tree_error_quark(void)
{
    static GQuark quark = 0;

    if (!quark)
        quark = g_quark_from_static_string("osinfo-tree-error");

    return quark;
}

G_DEFINE_TYPE(OsinfoTree, osinfo_tree, OSINFO_TYPE_ENTITY);

#define OSINFO_TREE_GET_PRIVATE(obj)                    \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj),                \
                                  OSINFO_TYPE_TREE,     \
                                  OsinfoTreePrivate))

/**
 * SECTION:osinfo_tree
 * @short_description: An installation tree for a (guest) OS
 * @see_also: #OsinfoOs
 *
 * #OsinfoTree is an entity representing an installation tree
 * a (guest) operating system.
 */

struct _OsinfoTreePrivate
{
    GWeakRef os;
};

enum {
    PROP_0,

    PROP_ARCHITECTURE,
    PROP_URL,
    PROP_TREEINFO_FAMILY,
    PROP_TREEINFO_VARIANT,
    PROP_TREEINFO_VERSION,
    PROP_TREEINFO_ARCH,
    PROP_KERNEL_PATH,
    PROP_INITRD_PATH,
    PROP_BOOT_ISO_PATH,
    PROP_HAS_TREEINFO,
};

static void
osinfo_tree_get_property(GObject *object,
                         guint property_id,
                         GValue *value,
                         GParamSpec *pspec)
{
    OsinfoTree *tree = OSINFO_TREE(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        g_value_set_string(value,
                           osinfo_tree_get_architecture(tree));
        break;

    case PROP_URL:
        g_value_set_string(value,
                           osinfo_tree_get_url(tree));
        break;

    case PROP_TREEINFO_FAMILY:
        g_value_set_string(value,
                           osinfo_tree_get_treeinfo_family(tree));
        break;

    case PROP_TREEINFO_VARIANT:
        g_value_set_string(value,
                           osinfo_tree_get_treeinfo_variant(tree));
        break;

    case PROP_TREEINFO_VERSION:
        g_value_set_string(value,
                           osinfo_tree_get_treeinfo_version(tree));
        break;

    case PROP_TREEINFO_ARCH:
        g_value_set_string(value,
                           osinfo_tree_get_treeinfo_arch(tree));
        break;

    case PROP_KERNEL_PATH:
        g_value_set_string(value,
                           osinfo_tree_get_kernel_path(tree));
        break;

    case PROP_INITRD_PATH:
        g_value_set_string(value,
                           osinfo_tree_get_initrd_path(tree));
        break;

    case PROP_BOOT_ISO_PATH:
        g_value_set_string(value,
                           osinfo_tree_get_boot_iso_path(tree));
        break;

    case PROP_HAS_TREEINFO:
        g_value_set_boolean(value,
                            osinfo_tree_has_treeinfo(tree));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}


static void
osinfo_tree_set_property(GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    OsinfoTree *tree = OSINFO_TREE(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_ARCHITECTURE,
                                g_value_get_string(value));
        break;

    case PROP_URL:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_URL,
                                g_value_get_string(value));
        break;

    case PROP_TREEINFO_FAMILY:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_FAMILY,
                                g_value_get_string(value));
        break;

    case PROP_TREEINFO_VARIANT:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_VARIANT,
                                g_value_get_string(value));
        break;

    case PROP_TREEINFO_VERSION:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_VERSION,
                                g_value_get_string(value));
        break;

    case PROP_TREEINFO_ARCH:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_ARCH,
                                g_value_get_string(value));
        break;

    case PROP_KERNEL_PATH:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_KERNEL,
                                g_value_get_string(value));
        break;

    case PROP_INITRD_PATH:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_INITRD,
                                g_value_get_string(value));
        break;

    case PROP_BOOT_ISO_PATH:
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_BOOT_ISO,
                                g_value_get_string(value));
        break;

    case PROP_HAS_TREEINFO:
        osinfo_entity_set_param_boolean(OSINFO_ENTITY(tree),
                                        OSINFO_TREE_PROP_HAS_TREEINFO,
                                        g_value_get_boolean(value));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_tree_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_tree_parent_class)->finalize(object);
}

static void osinfo_tree_dispose(GObject *obj)
{
    OsinfoTree *tree = OSINFO_TREE(obj);

    g_weak_ref_clear(&tree->priv->os);

    G_OBJECT_CLASS(osinfo_tree_parent_class)->dispose(obj);
}

/* Init functions */
static void
osinfo_tree_class_init(OsinfoTreeClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);
    GParamSpec *pspec;

    g_klass->dispose = osinfo_tree_dispose;
    g_klass->finalize = osinfo_tree_finalize;
    g_klass->get_property = osinfo_tree_get_property;
    g_klass->set_property = osinfo_tree_set_property;
    g_type_class_add_private(klass, sizeof(OsinfoTreePrivate));

    /**
     * OsinfoTree:architecture:
     *
     * The target hardware architecture of this tree.
     */
    pspec = g_param_spec_string("architecture",
                                "ARCHITECTURE",
                                _("CPU Architecture"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_ARCHITECTURE, pspec);

    /**
     * OsinfoTree:url:
     *
     * The URL to this tree.
     */
    pspec = g_param_spec_string("url",
                                "URL",
                                _("The URL to this tree"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_URL, pspec);

    /**
     * OsinfoTree:volume-id:
     *
     * Expected volume ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("volume-id",
                                "VolumeID",
                                _("The expected ISO9660 volume ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_TREEINFO_FAMILY, pspec);

    /**
     * OsinfoTree:publisher-id:
     *
     * Expected publisher ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("publisher-id",
                                "PublisherID",
                                _("The expected ISO9660 publisher ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_TREEINFO_VARIANT, pspec);

    /**
     * OsinfoTree:application-id:
     *
     * Expected application ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("application-id",
                                "ApplicationID",
                                _("The expected ISO9660 application ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_TREEINFO_VERSION, pspec);

    /**
     * OsinfoTree:system-id:
     *
     * Expected system ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("system-id",
                                "SystemID",
                                _("The expected ISO9660 system ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_TREEINFO_ARCH, pspec);

    /**
     * OsinfoTree:kernel-path:
     *
     * The path to the kernel image in the install tree.
     */
    pspec = g_param_spec_string("kernel-path",
                                "KernelPath",
                                _("The path to the kernel image"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_KERNEL_PATH, pspec);

    /**
     * OsinfoTree:initrd-path:
     *
     * The path to the initrd image in the install tree.
     */
    pspec = g_param_spec_string("initrd-path",
                                "InitrdPath",
                                _("The path to the inirtd image"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_INITRD_PATH, pspec);

    /**
     * OsinfoTree:boot-iso-path:
     *
     * The path to the boot ISO in the install tree
     */
    pspec = g_param_spec_string("boot-iso-path",
                                "BootISOPath",
                                _("The path to the bootable ISO image"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_BOOT_ISO_PATH, pspec);

    /**
     * OsinfoTree:has-treeinfo
     *
     * Whether the tree has treeinfo or not
     */
    pspec = g_param_spec_boolean("has-treeinfo",
                                 "HasTreeinfo",
                                 _("Whether the tree has treeinfo"),
                                 TRUE /* default value */,
                                 G_PARAM_READWRITE |
                                 G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_HAS_TREEINFO, pspec);
}

static void
osinfo_tree_init(OsinfoTree *tree)
{
    tree->priv = OSINFO_TREE_GET_PRIVATE(tree);

    g_weak_ref_init(&tree->priv->os, NULL);
}

OsinfoTree *osinfo_tree_new(const gchar *id,
                            const gchar *architecture)
{
    OsinfoTree *tree;

    tree = g_object_new(OSINFO_TYPE_TREE,
                        "id", id,
                        NULL);

    osinfo_entity_set_param(OSINFO_ENTITY(tree),
                            OSINFO_TREE_PROP_ARCHITECTURE,
                            architecture);

    return tree;
}

static void on_tree_create_from_location_ready(GObject *source_object,
                                               GAsyncResult *res,
                                               gpointer user_data)
{
    CreateFromLocationData *data = (CreateFromLocationData *)user_data;

    data->res = g_object_ref(res);

    g_main_loop_quit(data->main_loop);
}

/**
 * osinfo_tree_create_from_location:
 * @location: the location of an installation tree
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates a new #OsinfoTree for installation tree at @location. The @location
 * could be any URI that GIO can handle or a local path.
 *
 * NOTE: Currently this only works for trees with a .treeinfo file
 *
 * Returns: (transfer full): a new #OsinfoTree , or NULL on error
 */
OsinfoTree *osinfo_tree_create_from_location(const gchar *location,
                                             GCancellable *cancellable,
                                             GError **error)
{
    CreateFromLocationData *data;
    OsinfoTree *ret;

    data = g_slice_new0(CreateFromLocationData);
    data->main_loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);

    osinfo_tree_create_from_location_async(location,
                                           G_PRIORITY_DEFAULT,
                                           cancellable,
                                           on_tree_create_from_location_ready,
                                           data);

    /* Loop till we get a reply (or time out) */
    g_main_loop_run(data->main_loop);

    ret = osinfo_tree_create_from_location_finish(data->res, error);
    create_from_location_data_free(data);

    return ret;
}

static gboolean is_str_empty(const gchar *str) {
    guint8 i;
    gboolean ret = TRUE;

    if ((str == NULL) || (*str == 0))
        return TRUE;

    for (i = 0; i < strlen(str); i++)
        if (!g_ascii_isspace(str[i])) {
            ret = FALSE;

            break;
        }

    return ret;
}

static gboolean is_unknown_group_or_key_error(const GError *error)
{
    return (g_error_matches(error, G_KEY_FILE_ERROR,
                            G_KEY_FILE_ERROR_KEY_NOT_FOUND) ||
            g_error_matches(error, G_KEY_FILE_ERROR,
                            G_KEY_FILE_ERROR_GROUP_NOT_FOUND));
}

static OsinfoTree *load_keyinfo(const gchar *location,
                                const gchar *content,
                                gsize length,
                                GError **error)
{
    GKeyFile *file = g_key_file_new();
    OsinfoTree *tree = NULL;
    gchar *family = NULL;
    gchar *variant = NULL;
    gchar *version = NULL;
    gchar *arch = NULL;
    gchar *kernel = NULL;
    gchar *initrd = NULL;
    gchar *bootiso = NULL;
    gchar *group = NULL;

    if (!g_key_file_load_from_data(file, content, length,
                                   G_KEY_FILE_NONE, error))
        goto cleanup;

    if (!(family = g_key_file_get_string(file, "general", "family", error))) {
        if (!is_unknown_group_or_key_error(*error))
            goto cleanup;
        g_clear_error(error);
    }

    if (!(variant = g_key_file_get_string(file, "general", "variant", error))) {
        if (!is_unknown_group_or_key_error(*error))
            goto cleanup;
        g_clear_error(error);
    }

    if (!(version = g_key_file_get_string(file, "general", "version", error))) {
        if (!is_unknown_group_or_key_error(*error))
            goto cleanup;
        g_clear_error(error);
    }

    if (!(arch = g_key_file_get_string(file, "general", "arch", error))) {
        if (!is_unknown_group_or_key_error(*error))
            goto cleanup;
        g_clear_error(error);
    }


    if (arch) {
        group = g_strdup_printf("images-%s", arch);

        if (!(kernel = g_key_file_get_string(file, group, "kernel", error))) {
            if (!is_unknown_group_or_key_error(*error))
                goto cleanup;
            g_clear_error(error);
        }

        if (!(initrd = g_key_file_get_string(file, group, "initrd", error))) {
            if (!is_unknown_group_or_key_error(*error))
                goto cleanup;
            g_clear_error(error);
        }

        if (!(bootiso = g_key_file_get_string(file, group, "boot.iso", error))) {
            if (!is_unknown_group_or_key_error(*error))
                goto cleanup;
            g_clear_error(error);
        }
    }

    tree = osinfo_tree_new(location, arch ? arch : "i386");

    osinfo_entity_set_param(OSINFO_ENTITY(tree),
                            OSINFO_TREE_PROP_URL,
                            location);

    if (!is_str_empty(family))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_FAMILY,
                                family);
    if (!is_str_empty(variant))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_VARIANT,
                                variant);
    if (!is_str_empty(version))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_VERSION,
                                version);
    if (!is_str_empty(arch))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_TREEINFO_ARCH,
                                arch);
    if (!is_str_empty(kernel))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_KERNEL,
                                kernel);
    if (!is_str_empty(initrd))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_INITRD,
                                initrd);
    if (!is_str_empty(bootiso))
        osinfo_entity_set_param(OSINFO_ENTITY(tree),
                                OSINFO_TREE_PROP_BOOT_ISO,
                                bootiso);

 cleanup:
    g_free(family);
    g_free(variant);
    g_free(version);
    g_free(arch);
    g_free(kernel);
    g_free(initrd);
    g_free(bootiso);
    g_key_file_free(file);
    return tree;
}

static void
osinfo_tree_create_from_location_async_helper(const gchar *url,
                                              const gchar *treeinfo,
                                              GCancellable *cancellable,
                                              CreateFromLocationAsyncData *data);

static void on_location_read(GObject *source,
                             GAsyncResult *res,
                             gpointer user_data)
{
    CreateFromLocationAsyncData *data;
    GError *error = NULL;
    gchar *content = NULL;
    gsize length = 0;
    OsinfoTree *ret = NULL;

    data = (CreateFromLocationAsyncData *)user_data;

    if (!g_file_load_contents_finish(G_FILE(source),
                                     res,
                                     &content,
                                     &length,
                                     NULL,
                                     &error)) {
        /* It means no ".treeinfo" file has been found. Try again, this time
         * looking for a "treeinfo" file. */
        if (g_str_equal(data->treeinfo, ".treeinfo")) {
            osinfo_tree_create_from_location_async_helper(data->location,
                                                          "treeinfo",
                                                          g_task_get_cancellable(data->res),
                                                          data);
            return;
        }

        g_prefix_error(&error, _("Failed to load .treeinfo|treeinfo file: "));
        g_task_return_error(data->res, error);
        create_from_location_async_data_free(data);
        return;
    }

    if (!(ret = load_keyinfo(data->location,
                             content,
                             length,
                             &error))) {
        g_prefix_error(&error, _("Failed to process keyinfo file: "));
        g_task_return_error(data->res, error);
        goto cleanup;
    }

    g_task_return_pointer(data->res, ret, g_object_unref);

 cleanup:
    create_from_location_async_data_free(data);
    g_free(content);
}

static void
osinfo_tree_create_from_location_async_helper(const gchar *url,
                                              const gchar *treeinfo,
                                              GCancellable *cancellable,
                                              CreateFromLocationAsyncData *data)
{
    gchar *location;

    g_return_if_fail(url != NULL);
    g_return_if_fail(treeinfo != NULL);

    location = g_strdup_printf("%s/%s", url, treeinfo);

    g_clear_object(&data->file);
    data->file = g_file_new_for_uri(location);

    g_free(data->treeinfo);
    data->treeinfo = g_strdup(treeinfo);

    g_file_load_contents_async(data->file,
                               cancellable,
                               on_location_read,
                               data);
    g_free(location);
}

/**
 * osinfo_tree_create_from_location_async:
 * @location: the location of an installation tree
 * @priority: the I/O priority of the request
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_tree_create_from_location.
 */
void osinfo_tree_create_from_location_async(const gchar *location,
                                            gint priority,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer user_data)
{
    CreateFromLocationAsyncData *data;

    data = g_slice_new0(CreateFromLocationAsyncData);
    data->res = g_task_new(NULL,
                           cancellable,
                           callback,
                           user_data);
    g_task_set_priority(data->res, priority);

    data->location = g_strdup(location);

    osinfo_tree_create_from_location_async_helper(location,
                                                  ".treeinfo",
                                                  cancellable,
                                                  data);
}


/**
 * osinfo_tree_create_from_location_finish:
 * @res: a #GAsyncResult
 * @error: The location where to store any error, or %NULL
 *
 * Finishes an asynchronous tree object creation process started with
 * #osinfo_tree_create_from_location_async.
 *
 * Returns: (transfer full): a new #OsinfoTree , or NULL on error
 */
OsinfoTree *osinfo_tree_create_from_location_finish(GAsyncResult *res,
                                                    GError **error)
{
    GTask *task = G_TASK(res);

    g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    return g_task_propagate_pointer(task, error);
}

/**
 * osinfo_tree_get_architecture:
 * @tree: an #OsinfoTree instance
 *
 * Retrieves the target hardware architecture of the OS @tree provides.
 *
 * Returns: (transfer none): the hardware architecture, or NULL
 */
const gchar *osinfo_tree_get_architecture(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_ARCHITECTURE);
}

/**
 * osinfo_tree_get_url:
 * @tree: an #OsinfoTree instance
 *
 * The URL to the @tree
 *
 * Returns: (transfer none): the URL, or NULL
 */
const gchar *osinfo_tree_get_url(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_URL);
}

/**
 * osinfo_tree_get_treeinfo_family:
 * @tree: an #OsinfoTree instance
 *
 * If @tree is an ISO9660 image/device, this function retrieves the expected
 * volume ID.
 *
 * Note: In practice, this will usually not be the exact copy of the volume ID
 * string on the ISO image/device but rather a regular expression that matches
 * it.
 *
 * Returns: (transfer none): the volume id, or NULL
 */
const gchar *osinfo_tree_get_treeinfo_family(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_TREEINFO_FAMILY);
}

/**
 * osinfo_tree_get_treeinfo_arch:
 * @tree: an #OsinfoTree instance
 *
 * If @tree is an ISO9660 image/device, this function retrieves the expected
 * system ID.
 *
 * Note: In practice, this will usually not be the exact copy of the system ID
 * string on the ISO image/device but rather a regular expression that matches
 * it.
 *
 * Returns: (transfer none): the system id, or NULL
 */
const gchar *osinfo_tree_get_treeinfo_arch(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_TREEINFO_ARCH);
}

/**
 * osinfo_tree_get_treeinfo_variant:
 * @tree: an #OsinfoTree instance
 *
 * If @tree is an ISO9660 image/device, this function retrieves the expected
 * publisher ID.
 *
 * Note: In practice, this will usually not be the exact copy of the publisher
 * ID string on the ISO image/device but rather a regular expression that
 * matches it.
 *
 * Returns: (transfer none): the publisher id, or NULL
 */
const gchar *osinfo_tree_get_treeinfo_variant(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_TREEINFO_VARIANT);
}

/**
 * osinfo_tree_get_treeinfo_version:
 * @tree: an #OsinfoTree instance
 *
 * If @tree is an ISO9660 image/device, this function retrieves the expected
 * application ID.
 *
 * Note: In practice, this will usually not be the exact copy of the application
 * ID string on the ISO image/device but rather a regular expression that
 * matches it.
 *
 * Returns: (transfer none): the application id, or NULL
 */
const gchar *osinfo_tree_get_treeinfo_version(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_TREEINFO_VERSION);
}

/**
 * osinfo_tree_get_boot_iso_path:
 * @tree: an #OsinfoTree instance
 *
 * Retrieves the path to the boot_iso image in the install tree.
 *
 * Returns: (transfer none): the path to boot_iso image, or NULL
 */
const gchar *osinfo_tree_get_boot_iso_path(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_BOOT_ISO);
}

/**
 * osinfo_tree_get_kernel_path:
 * @tree: an #OsinfoTree instance
 *
 * Retrieves the path to the kernel image in the install tree.
 *
 * Note: This only applies to installer trees of 'linux' OS family.
 *
 * Returns: (transfer none): the path to kernel image, or NULL
 */
const gchar *osinfo_tree_get_kernel_path(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_KERNEL);
}

/**
 * osinfo_tree_get_initrd_path:
 * @tree: an #OsinfoTree instance
 *
 * Retrieves the path to the initrd image in the install tree.
 *
 * Note: This only applies to installer trees of 'linux' OS family.
 *
 * Returns: (transfer none): the path to initrd image, or NULL
 */
const gchar *osinfo_tree_get_initrd_path(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(tree),
                                         OSINFO_TREE_PROP_INITRD);
}

/**
 * osinfo_tree_has_treeinfo:
 * @tree: an #OsinfoTree instance
 *
 * Return whether a tree has treeinfo or not.
 *
 * Returns: TRUE if the tree has treeinfo. FALSE otherwise.
 */
gboolean osinfo_tree_has_treeinfo(OsinfoTree *tree)
{
    return osinfo_entity_get_param_value_boolean(OSINFO_ENTITY(tree),
                                                 OSINFO_TREE_PROP_HAS_TREEINFO);
}

/**
 * osinfo_tree_get_os:
 * @tree: an #OsinfoTree instance
 *
 * Returns: (transfer full): the operating system, or NULL
 */
OsinfoOs *osinfo_tree_get_os(OsinfoTree *tree)
{
    g_return_val_if_fail(OSINFO_IS_TREE(tree), NULL);

    return g_weak_ref_get(&tree->priv->os);
}


/**
 * osinfo_tree_set_os
 * @tree: an #OsinfoTree instance
 * @os: an #OsinfoOs instance
 *
 * Sets the #OsinfoOs associated to the #OsinfoTree instance.
 */
void osinfo_tree_set_os(OsinfoTree *tree, OsinfoOs *os)
{
    g_return_if_fail(OSINFO_IS_TREE(tree));

    g_object_ref(os);
    g_weak_ref_set(&tree->priv->os, os);
    g_object_unref(os);
}

/**
 * osinfo_tree_get_os_variants:
 * @tree: an #OsinfoTree instance
 *
 * Gets the variants of the associated operating system.
 *
 * Returns: (transfer full): the operating system variant, or NULL
 */
OsinfoOsVariantList *osinfo_tree_get_os_variants(OsinfoTree *tree)
{
    OsinfoOs *os;
    OsinfoOsVariantList *os_variants;
    OsinfoOsVariantList *tree_variants;
    GList *ids, *node;
    OsinfoFilter *filter;

    g_return_val_if_fail(OSINFO_IS_TREE(tree), NULL);

    os = osinfo_tree_get_os(tree);
    if (os == NULL)
        return NULL;

    os_variants = osinfo_os_get_variant_list(os);
    g_object_unref(os);

    ids = osinfo_entity_get_param_value_list(OSINFO_ENTITY(tree),
                                             OSINFO_TREE_PROP_VARIANT);
    filter = osinfo_filter_new();
    tree_variants = osinfo_os_variantlist_new();
    for (node = ids; node != NULL; node = node->next) {
        osinfo_filter_clear_constraints(filter);
        osinfo_filter_add_constraint(filter,
                                     OSINFO_ENTITY_PROP_ID,
                                     (const char *) node->data);
        osinfo_list_add_filtered(OSINFO_LIST(tree_variants),
                                 OSINFO_LIST(os_variants),
                                 filter);
    }
    g_object_unref(os_variants);

    return tree_variants;
}
