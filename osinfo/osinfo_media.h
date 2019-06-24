/*
 * libosinfo: An installation media for a (guest) OS
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
#include <osinfo/osinfo_os_variantlist.h>

#ifndef __OSINFO_MEDIA_H__
#define __OSINFO_MEDIA_H__

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
 */
typedef enum {
    OSINFO_MEDIA_DETECT_REQUIRE_BOOTABLE = 1 << 0,
} OsinfoMediaDetectFlags;

/*
 * Type macros.
 */
#define OSINFO_TYPE_MEDIA                  (osinfo_media_get_type ())
#define OSINFO_MEDIA(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_MEDIA, OsinfoMedia))
#define OSINFO_IS_MEDIA(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_MEDIA))
#define OSINFO_MEDIA_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_MEDIA, OsinfoMediaClass))
#define OSINFO_IS_MEDIA_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_MEDIA))
#define OSINFO_MEDIA_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_MEDIA, OsinfoMediaClass))

/*
 * Forward declared in osinfo_install_script.h
 *
 * typedef struct _OsinfoMedia        OsinfoMedia;
 */
typedef struct _OsinfoMediaClass   OsinfoMediaClass;

typedef struct _OsinfoMediaPrivate OsinfoMediaPrivate;

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

/* object */
struct _OsinfoMedia
{
    OsinfoEntity parent_instance;

    /* public */

    /* private */
    OsinfoMediaPrivate *priv;
};

/* class */
struct _OsinfoMediaClass
{
    /*< private >*/
    OsinfoEntityClass parent_class;

    /* class members */
};

GType osinfo_media_get_type(void);

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

#endif /* __OSINFO_MEDIA_H__ */
