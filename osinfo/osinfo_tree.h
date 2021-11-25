/*
 * libosinfo: An installation tree for a (guest) OS
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
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>

#include "osinfo/osinfo_macros.h"

GQuark
osinfo_tree_error_quark (void) G_GNUC_CONST;

#define OSINFO_TREE_ERROR (osinfo_tree_error_quark ())

/**
 * OsinfoTreeError:
 * @OSINFO_TREE_ERROR_NO_TREEINFO: No treeinfo found;
 * @OSINFO_TREE_ERROR_NOT_SUPPORTED_PROTOCOL: The URL protocol is not supported.
 *
 * #GError codes used for errors in the #OSINFO_TREE_ERROR domain, during
 * reading the treeinfo from a URI.
 *
 * Since: 1.6.0
 */
typedef enum {
    OSINFO_TREE_ERROR_NO_TREEINFO,
    OSINFO_TREE_ERROR_NOT_SUPPORTED_PROTOCOL
} OsinfoTreeError;

#define OSINFO_TYPE_TREE (osinfo_tree_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoTree,
                                           osinfo_tree,
                                           OSINFO,
                                           TREE,
                                           OsinfoEntity)

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
