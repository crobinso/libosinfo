/*
 * libosinfo: An installation media for a (guest) OS
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
#include <osinfo/osinfo_os_variantlist.h>

#include <osinfo/osinfo_macros.h>

GQuark
osinfo_media_error_quark (void) G_GNUC_CONST;

#define OSINFO_MEDIA_ERROR (osinfo_media_error_quark ())

/**
 * OsinfoMediaError:
 * @OSINFO_MEDIA_ERROR_NO_DESCRIPTORS: No descriptors.
 * @OSINFO_MEDIA_ERROR_INSUFFICIENT_METADATA: Not enough metadata.
 * @OSINFO_MEDIA_ERROR_NOT_BOOTABLE: Install media not bootable.
 * @OSINFO_MEDIA_ERROR_NO_PVD: No Primary volume descriptor.
 * @OSINFO_MEDIA_ERROR_NO_SVD: No supplementary volume descriptor.
 * @OSINFO_MEDIA_ERROR_NO_DIRECTORY_RECORD_EXTENT: No directory record extent
 * found.
 *
 * #GError codes used for errors in the #OSINFO_MEDIA_ERROR domain, during
 * reading of data from install media location.
 */
typedef enum {
    OSINFO_MEDIA_ERROR_NO_DESCRIPTORS,
    OSINFO_MEDIA_ERROR_NO_PVD,
    OSINFO_MEDIA_ERROR_NO_SVD,
    OSINFO_MEDIA_ERROR_INSUFFICIENT_METADATA,
    OSINFO_MEDIA_ERROR_NOT_BOOTABLE,
    OSINFO_MEDIA_ERROR_NO_DIRECTORY_RECORD_EXTENT
} OsinfoMediaError;

/**
 * OsinfoMediaDetectFlags
 * OSINFO_MEDIA_DETECT_REQUIRE_BOOTABLE: Requires a media to be bootable.
 *
 * Flags used for detecting a media.
 *
 * Since: 1.6.0
 */
typedef enum {
    OSINFO_MEDIA_DETECT_REQUIRE_BOOTABLE = 1 << 0,
} OsinfoMediaDetectFlags;

#define OSINFO_TYPE_MEDIA (osinfo_media_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoMedia,
                                           osinfo_media,
                                           OSINFO,
                                           MEDIA,
                                           OsinfoEntity)

#define OSINFO_MEDIA_PROP_ARCHITECTURE   "architecture"
#define OSINFO_MEDIA_PROP_URL            "url"
#define OSINFO_MEDIA_PROP_VOLUME_ID      "volume-id"
#define OSINFO_MEDIA_PROP_SYSTEM_ID      "system-id"
#define OSINFO_MEDIA_PROP_PUBLISHER_ID   "publisher-id"
#define OSINFO_MEDIA_PROP_APPLICATION_ID "application-id"
#define OSINFO_MEDIA_PROP_KERNEL         "kernel"
#define OSINFO_MEDIA_PROP_INITRD         "initrd"
#define OSINFO_MEDIA_PROP_LIVE           "live"
#define OSINFO_MEDIA_PROP_INSTALLER      "installer"
#define OSINFO_MEDIA_PROP_INSTALLER_REBOOTS "installer-reboots"
#define OSINFO_MEDIA_PROP_LANG           "l10n-language"
#define OSINFO_MEDIA_PROP_LANG_REGEX     "l10n-language-regex"
#define OSINFO_MEDIA_PROP_LANG_MAP       "l10n-language-map"
#define OSINFO_MEDIA_PROP_VARIANT        "variant"
#define OSINFO_MEDIA_PROP_VOLUME_SIZE    "volume-size"
#define OSINFO_MEDIA_PROP_EJECT_AFTER_INSTALL "eject-after-install"
#define OSINFO_MEDIA_PROP_INSTALLER_SCRIPT "installer-script"
#define OSINFO_MEDIA_PROP_BOOTABLE        "bootable"

OsinfoMedia *osinfo_media_new(const gchar *id, const gchar *architecture);
OsinfoMedia *osinfo_media_create_from_location(const gchar *location,
                                               GCancellable *cancellable,
                                               GError **error);
void osinfo_media_create_from_location_async(const gchar *location,
                                             gint priority,
                                             GCancellable *cancellable,
                                             GAsyncReadyCallback callback,
                                             gpointer user_data);
OsinfoMedia *osinfo_media_create_from_location_finish(GAsyncResult *res,
                                                      GError **error);

OsinfoMedia *osinfo_media_create_from_location_with_flags(const gchar *location,
                                                          GCancellable *cancellable,
                                                          guint flags,
                                                          GError **error);
void osinfo_media_create_from_location_with_flags_async(const gchar *location,
                                                        gint priority,
                                                        GCancellable *cancellable,
                                                        GAsyncReadyCallback callback,
                                                        guint flags,
                                                        gpointer user_data);
OsinfoMedia *osinfo_media_create_from_location_with_flags_finish(GAsyncResult *res,
                                                                 GError **error);

const gchar *osinfo_media_get_architecture(OsinfoMedia *media);
const gchar *osinfo_media_get_url(OsinfoMedia *media);
const gchar *osinfo_media_get_volume_id(OsinfoMedia *media);
const gchar *osinfo_media_get_system_id(OsinfoMedia *media);
const gchar *osinfo_media_get_publisher_id(OsinfoMedia *media);
const gchar *osinfo_media_get_application_id(OsinfoMedia *media);
const gchar *osinfo_media_get_kernel_path(OsinfoMedia *media);
const gchar *osinfo_media_get_initrd_path(OsinfoMedia *media);
gboolean osinfo_media_is_bootable(OsinfoMedia *media);
OsinfoOs *osinfo_media_get_os(OsinfoMedia *media);
OsinfoOsVariantList *osinfo_media_get_os_variants(OsinfoMedia *media);
GList *osinfo_media_get_languages(OsinfoMedia *media);
gboolean osinfo_media_get_installer(OsinfoMedia *media);
gboolean osinfo_media_get_live(OsinfoMedia *media);
gint osinfo_media_get_installer_reboots(OsinfoMedia *media);
gint64 osinfo_media_get_volume_size(OsinfoMedia *media);
gboolean osinfo_media_get_eject_after_install(OsinfoMedia *media);
gboolean osinfo_media_supports_installer_script(OsinfoMedia *media);
void osinfo_media_add_install_script(OsinfoMedia *media, OsinfoInstallScript *script);
OsinfoInstallScriptList *osinfo_media_get_install_script_list(OsinfoMedia *media);
