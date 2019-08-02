/*
 * libosinfo: An installation tree for a (guest) OS
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
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Zeeshan Ali <zeenix@redhat.com>
 *   Arjun Roy <arroy@redhat.com>
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <glib-object.h>
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>

#ifndef __OSINFO_TREE_H__
#define __OSINFO_TREE_H__

GQuark
osinfo_tree_error_quark (void) G_GNUC_CONST;

#define OSINFO_TREE_ERROR (osinfo_tree_error_quark ())

/**
 * OsinfoTreeError:
 * Since: 1.6.0
 * @OSINFO_TREE_ERROR_NO_TREEINFO: No treeinfo found;
 * @OSINFO_TREE_ERROR_NOT_SUPPORTED_PROTOCOL: The URL protocol is not supported.
 *
 * #GError codes used for errors in the #OSINFO_TREE_ERROR domain, during
 * reading the treeinfo from a URI.
 */
typedef enum {
    OSINFO_TREE_ERROR_NO_TREEINFO,
    OSINFO_TREE_ERROR_NOT_SUPPORTED_PROTOCOL
} OsinfoTreeError;


/*
 * Type macros.
 */
#define OSINFO_TYPE_TREE                  (osinfo_tree_get_type ())
#define OSINFO_TREE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_TREE, OsinfoTree))
#define OSINFO_IS_TREE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_TREE))
#define OSINFO_TREE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_TREE, OsinfoTreeClass))
#define OSINFO_IS_TREE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_TREE))
#define OSINFO_TREE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_TREE, OsinfoTreeClass))

typedef struct _OsinfoTree        OsinfoTree;

typedef struct _OsinfoTreeClass   OsinfoTreeClass;

typedef struct _OsinfoTreePrivate OsinfoTreePrivate;

#define OSINFO_TREE_PROP_ARCHITECTURE     "architecture"
#define OSINFO_TREE_PROP_URL              "url"
#define OSINFO_TREE_PROP_TREEINFO_FAMILY  "treeinfo-family"
#define OSINFO_TREE_PROP_TREEINFO_VARIANT "treeinfo-variant"
#define OSINFO_TREE_PROP_TREEINFO_VERSION "treeinfo-version"
#define OSINFO_TREE_PROP_TREEINFO_ARCH    "treeinfo-arch"
#define OSINFO_TREE_PROP_BOOT_ISO         "boot-iso"
#define OSINFO_TREE_PROP_KERNEL           "kernel"
#define OSINFO_TREE_PROP_INITRD           "initrd"
#define OSINFO_TREE_PROP_HAS_TREEINFO     "has-treeinfo"
#define OSINFO_TREE_PROP_VARIANT          "variant"


/* object */
struct _OsinfoTree
{
    OsinfoEntity parent_instance;

    /* public */

    /* private */
    OsinfoTreePrivate *priv;
};

/* class */
struct _OsinfoTreeClass
{
    /*< private >*/
    OsinfoEntityClass parent_class;

    /* class members */
};

GType osinfo_tree_get_type(void);

OsinfoTree *osinfo_tree_new(const gchar *id, const gchar *architecture);
OsinfoTree *osinfo_tree_create_from_location(const gchar *location,
                                             GCancellable *cancellable,
                                             GError **error);
void osinfo_tree_create_from_location_async(const gchar *location,
                                            gint priority,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer user_data);
OsinfoTree *osinfo_tree_create_from_location_finish(GAsyncResult *res,
                                                    GError **error);

OsinfoTree *osinfo_tree_create_from_treeinfo(const gchar *treeinfo,
                                             const gchar *location,
                                             GError **error);

const gchar *osinfo_tree_get_architecture(OsinfoTree *tree);
const gchar *osinfo_tree_get_url(OsinfoTree *tree);
gboolean osinfo_tree_has_treeinfo(OsinfoTree *tree);
const gchar *osinfo_tree_get_treeinfo_family(OsinfoTree *tree);
const gchar *osinfo_tree_get_treeinfo_variant(OsinfoTree *tree);
const gchar *osinfo_tree_get_treeinfo_version(OsinfoTree *tree);
const gchar *osinfo_tree_get_treeinfo_arch(OsinfoTree *tree);
/* XXX list of files to probe for in absence of treeinfo ?  see virtinst isValidStore */
const gchar *osinfo_tree_get_boot_iso_path(OsinfoTree *tree);
const gchar *osinfo_tree_get_kernel_path(OsinfoTree *tree);
const gchar *osinfo_tree_get_initrd_path(OsinfoTree *tree);
OsinfoOs *osinfo_tree_get_os(OsinfoTree *tree);
void osinfo_tree_set_os(OsinfoTree *tree, OsinfoOs *os);
OsinfoOsVariantList *osinfo_tree_get_os_variants(OsinfoTree *tree);
/* XXX Xen kernel/initrd paths ? */

#endif /* __OSINFO_TREE_H__ */
