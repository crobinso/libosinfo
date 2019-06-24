/*
 * libosinfo: An installation media for a (guest) OS
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
#include "osinfo_media_private.h"
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

#define MAX_VOLUME 32
#define MAX_SYSTEM 32
#define MAX_PUBLISHER 128
#define MAX_APPLICATION 128

#define PVD_OFFSET 0x00008000
#define BOOTABLE_TAG "EL TORITO SPECIFICATION"
#define PPC_BOOTINFO "/ppc/bootinfo.txt"

enum {
    DIRECTORY_RECORD_FLAG_EXISTENCE       = 1 << 0,
    DIRECTORY_RECORD_FLAG_DIRECTORY       = 1 << 1,
    DIRECTORY_RECORD_FLAG_ASSOCIATED_FILE = 1 << 2,
    DIRECTORY_RECORD_FLAG_RECORD          = 1 << 3,
    DIRECTORY_RECORD_FLAG_PROTECTION      = 1 << 4,
    DIRECTORY_RECORD_FLAG_RESERVED5       = 1 << 5,
    DIRECTORY_RECORD_FLAG_RESERVED6       = 1 << 6,
    DIRECTORY_RECORD_FLAG_MULTIEXTENT     = 1 << 7
};

typedef struct _DirectoryRecord DirectoryRecord;

struct __attribute__((packed)) _DirectoryRecord {
    guint8 length;
    guint8 ignored;
    guint32 extent_location[2];
    guint32 extent_size[2];
    guint8 ignored2[7];
    guint8 flags;
    guint8 ignored3[6];
    guint8 filename_length;
    gchar filename[1];
};

G_STATIC_ASSERT(sizeof(struct _DirectoryRecord) == 34);

typedef struct _PrimaryVolumeDescriptor PrimaryVolumeDescriptor;

struct _PrimaryVolumeDescriptor {
    guint8 ignored[8];
    gchar  system[MAX_SYSTEM];       /* System ID */
    gchar  volume[MAX_VOLUME];       /* Volume ID */
    guint8 ignored2[8];
    guint32 volume_space_size[2];
    guint8 ignored3[40];
    guint16 logical_blk_size[2];
    guint8 ignored4[24];
    guint8 root_directory_entry[33];
    guint8 ignored5[129];
    gchar  publisher[MAX_PUBLISHER]; /* Publisher ID */
    guint8 ignored6[128];
    gchar  application[MAX_APPLICATION]; /* Application ID */
    guint8 ignored7[1346];
};

/* the PrimaryVolumeDescriptor struct must exactly 2048 bytes long
 * since we expect the supplementary volume descriptor to be right
 * after it.
 */
G_STATIC_ASSERT(sizeof(struct _PrimaryVolumeDescriptor) == 2048);

typedef struct _SupplementaryVolumeDescriptor SupplementaryVolumeDescriptor;

struct _SupplementaryVolumeDescriptor {
    guint8 ignored[7];
    gchar  system[MAX_SYSTEM]; /* System ID */
};

typedef struct _SearchPPCBootinfoAsyncData SearchPPCBootinfoAsyncData;
struct _SearchPPCBootinfoAsyncData {
    GTask *res;

    PrimaryVolumeDescriptor *pvd;
    guint8 *extent;

    gchar **filepath;
    gsize filepath_index;
    gsize filepath_index_max;

    gsize offset;
    gsize length;
};

static void search_ppc_bootinfo_async_data_free(SearchPPCBootinfoAsyncData *data)
{
    g_object_unref(data->res);

    g_strfreev(data->filepath);
    g_free(data->extent);

    g_slice_free(SearchPPCBootinfoAsyncData, data);

}

typedef struct _CreateFromLocationAsyncData CreateFromLocationAsyncData;
struct _CreateFromLocationAsyncData {
    GFile *file;

    GTask *res;

    PrimaryVolumeDescriptor pvd;
    SupplementaryVolumeDescriptor svd;

    gsize offset;
    gsize length;

    gchar *volume;
    gchar *system;
    gchar *application;
    gchar *publisher;

    guint flags;
};

static void create_from_location_async_data_free
                                (CreateFromLocationAsyncData *data)
{
   g_object_unref(data->file);
   g_object_unref(data->res);
   g_free(data->volume);
   g_free(data->system);
   g_free(data->application);
   g_free(data->publisher);

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
osinfo_media_error_quark(void)
{
    static GQuark quark = 0;

    if (!quark)
        quark = g_quark_from_static_string("osinfo-media-error");

    return quark;
}

G_DEFINE_TYPE(OsinfoMedia, osinfo_media, OSINFO_TYPE_ENTITY);

#define OSINFO_MEDIA_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
                                     OSINFO_TYPE_MEDIA,         \
                                     OsinfoMediaPrivate))

/**
 * SECTION:osinfo_media
 * @short_description: An installation media for a (guest) OS
 * @see_also: #OsinfoOs
 *
 * #OsinfoMedia is an entity representing an installation media
 * a (guest) operating system.
 */

struct _OsinfoMediaPrivate
{
    GWeakRef os;
    OsinfoInstallScriptList *scripts;
};

enum {
    PROP_0,

    PROP_ARCHITECTURE,
    PROP_URL,
    PROP_VOLUME_ID,
    PROP_PUBLISHER_ID,
    PROP_APPLICATION_ID,
    PROP_SYSTEM_ID,
    PROP_KERNEL_PATH,
    PROP_INITRD_PATH,
    PROP_INSTALLER,
    PROP_LIVE,
    PROP_INSTALLER_REBOOTS,
    PROP_OS,
    PROP_LANGUAGES,
    PROP_VOLUME_SIZE,
    PROP_EJECT_AFTER_INSTALL,
    PROP_INSTALLER_SCRIPT
};

static void
osinfo_media_get_property(GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    OsinfoMedia *media = OSINFO_MEDIA(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        g_value_set_string(value,
                           osinfo_media_get_architecture(media));
        break;

    case PROP_URL:
        g_value_set_string(value,
                           osinfo_media_get_url(media));
        break;

    case PROP_VOLUME_ID:
        g_value_set_string(value,
                           osinfo_media_get_volume_id(media));
        break;

    case PROP_PUBLISHER_ID:
        g_value_set_string(value,
                           osinfo_media_get_publisher_id(media));
        break;

    case PROP_APPLICATION_ID:
        g_value_set_string(value,
                           osinfo_media_get_application_id(media));
        break;

    case PROP_SYSTEM_ID:
        g_value_set_string(value,
                           osinfo_media_get_system_id(media));
        break;

    case PROP_KERNEL_PATH:
        g_value_set_string(value,
                           osinfo_media_get_kernel_path(media));
        break;

    case PROP_INITRD_PATH:
        g_value_set_string(value,
                           osinfo_media_get_initrd_path(media));
        break;

    case PROP_INSTALLER:
        g_value_set_boolean(value,
                            osinfo_media_get_installer(media));
        break;

    case PROP_LIVE:
        g_value_set_boolean(value,
                            osinfo_media_get_live(media));
        break;

    case PROP_INSTALLER_REBOOTS:
        g_value_set_int(value,
                        osinfo_media_get_installer_reboots(media));
        break;

    case PROP_OS:
        g_value_take_object(value, osinfo_media_get_os(media));
        break;

    case PROP_LANGUAGES:
        g_value_set_pointer(value, osinfo_media_get_languages(media));
        break;

    case PROP_VOLUME_SIZE:
        g_value_set_int64(value,
                          osinfo_media_get_volume_size(media));
        break;

    case PROP_EJECT_AFTER_INSTALL:
        g_value_set_boolean(value,
                            osinfo_media_get_eject_after_install(media));
        break;

    case PROP_INSTALLER_SCRIPT:
        g_value_set_boolean(value,
                            osinfo_media_supports_installer_script(media));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_media_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    OsinfoMedia *media = OSINFO_MEDIA(object);

    switch (property_id) {
    case PROP_ARCHITECTURE:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_ARCHITECTURE,
                                g_value_get_string(value));
        break;

    case PROP_URL:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_URL,
                                g_value_get_string(value));
        break;

    case PROP_VOLUME_ID:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_VOLUME_ID,
                                g_value_get_string(value));
        break;

    case PROP_PUBLISHER_ID:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_PUBLISHER_ID,
                                g_value_get_string(value));
        break;

    case PROP_APPLICATION_ID:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_APPLICATION_ID,
                                g_value_get_string(value));
        break;

    case PROP_SYSTEM_ID:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_SYSTEM_ID,
                                g_value_get_string(value));
        break;

    case PROP_KERNEL_PATH:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_KERNEL,
                                g_value_get_string(value));
        break;

    case PROP_INITRD_PATH:
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_INITRD,
                                g_value_get_string(value));
        break;

    case PROP_LIVE:
        osinfo_entity_set_param_boolean(OSINFO_ENTITY(media),
                                        OSINFO_MEDIA_PROP_LIVE,
                                        g_value_get_boolean(value));
        break;

    case PROP_INSTALLER:
        osinfo_entity_set_param_boolean(OSINFO_ENTITY(media),
                                        OSINFO_MEDIA_PROP_INSTALLER,
                                        g_value_get_boolean(value));
        break;

    case PROP_INSTALLER_REBOOTS:
        osinfo_entity_set_param_int64(OSINFO_ENTITY(media),
                                      OSINFO_MEDIA_PROP_INSTALLER_REBOOTS,
                                      g_value_get_int(value));
        break;

    case PROP_OS:
        osinfo_media_set_os(media, g_value_get_object(value));
        break;

    case PROP_LANGUAGES:
        osinfo_media_set_languages(media, g_value_get_pointer(value));
        break;

    case PROP_VOLUME_SIZE:
        osinfo_entity_set_param_int64(OSINFO_ENTITY(media),
                                      OSINFO_MEDIA_PROP_VOLUME_SIZE,
                                      g_value_get_int64(value));
        break;

    case PROP_EJECT_AFTER_INSTALL:
        osinfo_entity_set_param_boolean(OSINFO_ENTITY(media),
                                        OSINFO_MEDIA_PROP_EJECT_AFTER_INSTALL,
                                        g_value_get_boolean(value));
        break;

    case PROP_INSTALLER_SCRIPT:
        osinfo_entity_set_param_boolean(OSINFO_ENTITY(media),
                                        OSINFO_MEDIA_PROP_INSTALLER_SCRIPT,
                                        g_value_get_boolean(value));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osinfo_media_finalize(GObject *object)
{
    OsinfoMedia *media = OSINFO_MEDIA(object);

    g_object_unref(media->priv->scripts);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_media_parent_class)->finalize(object);
}

static void osinfo_media_dispose(GObject *obj)
{
    OsinfoMedia *media = OSINFO_MEDIA(obj);

    g_weak_ref_clear(&media->priv->os);

    G_OBJECT_CLASS(osinfo_media_parent_class)->dispose(obj);
}


/* Init functions */
static void
osinfo_media_class_init(OsinfoMediaClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);
    GParamSpec *pspec;

    g_klass->dispose = osinfo_media_dispose;
    g_klass->finalize = osinfo_media_finalize;
    g_klass->get_property = osinfo_media_get_property;
    g_klass->set_property = osinfo_media_set_property;
    g_type_class_add_private(klass, sizeof(OsinfoMediaPrivate));

    /**
     * OsinfoMedia:architecture:
     *
     * The target hardware architecture of this media.
     */
    pspec = g_param_spec_string("architecture",
                                "ARCHITECTURE",
                                _("CPU Architecture"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_ARCHITECTURE, pspec);

    /**
     * OsinfoMedia:url:
     *
     * The URL to this media.
     */
    pspec = g_param_spec_string("url",
                                "URL",
                                _("The URL to this media"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_URL, pspec);

    /**
     * OsinfoMedia:volume-id:
     *
     * Expected volume ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("volume-id",
                                "VolumeID",
                                _("The expected ISO9660 volume ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_VOLUME_ID, pspec);

    /**
     * OsinfoMedia:publisher-id:
     *
     * Expected publisher ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("publisher-id",
                                "PublisherID",
                                _("The expected ISO9660 publisher ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_PUBLISHER_ID, pspec);

    /**
     * OsinfoMedia:application-id:
     *
     * Expected application ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("application-id",
                                "ApplicationID",
                                _("The expected ISO9660 application ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_APPLICATION_ID, pspec);

    /**
     * OsinfoMedia:system-id:
     *
     * Expected system ID (regular expression) for ISO9660 image/device.
     */
    pspec = g_param_spec_string("system-id",
                                "SystemID",
                                _("The expected ISO9660 system ID"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_SYSTEM_ID, pspec);

    /**
     * OsinfoMedia:kernel-path:
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
     * OsinfoMedia:initrd-path:
     *
     * The path to the initrd image in the install tree.
     */
    pspec = g_param_spec_string("initrd-path",
                                "InitrdPath",
                                _("The path to the initrd image"),
                                NULL /* default value */,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_INITRD_PATH, pspec);

    /**
     * OsinfoMedia:installer:
     *
     * Whether media provides an installer for an OS.
     */
    pspec = g_param_spec_boolean("installer",
                                 "Installer",
                                 _("Media provides an installer"),
                                 TRUE /* default value */,
                                 G_PARAM_READWRITE |
                                 G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_INSTALLER, pspec);

    /**
     * OsinfoMedia:live:
     *
     * Whether media can boot directly an OS without any installations.
     */
    pspec = g_param_spec_boolean("live",
                                 "Live",
                                 _("Media can boot directly w/o installation"),
                                 FALSE /* default value */,
                                 G_PARAM_READWRITE |
                                 G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_LIVE, pspec);

    /**
     * OsinfoMedia:installer-reboots:
     *
     * If media is an installer, this property indicates the number of reboots
     * the installer takes before installation is complete.
     *
     * This property is not applicable to media that has no installer. You can
     * use #osinfo_media_get_installer(or OsinfoMedia::installer) to check
     * that.
     *
     * Warning: Some media allow you to install from live sessions, in which
     * case number of reboots *alone* is not a reliable method for tracking
     * installation.
     */
    pspec = g_param_spec_int("installer-reboots",
                             "InstallerReboots",
                             _("Number of installer reboots"),
                             G_MININT,
                             G_MAXINT,
                             1 /* default value */,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_INSTALLER_REBOOTS, pspec);

    /**
     * OsinfoMedia:os:
     *
     * Os information for the current media. For media stored in an
     * #OsinfoDB, it will be filled when the database is loaded, otherwise
     * the property will be filled after a successful call to
     * osinfo_db_identify_media().
     */
    pspec = g_param_spec_object("os",
                                "Os",
                                _("Information about the operating system on this media"),
                                OSINFO_TYPE_OS,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_OS, pspec);

    /**
     * OsinfoMedia:languages:
     *
     * If media is an installer, this property indicates the languages that
     * can be used during automatic installations.
     *
     * On media that are not installers, this property will indicate the
     * languages that the user interface can be displayed in.
     * Use #osinfo_media_get_installer(or OsinfoMedia::installer) to know
     * if the media is an installer or not.
     *
     * Type: GLib.List(utf8)
     * Transfer: container
     */
    pspec = g_param_spec_pointer("languages",
                                 "Languages",
                                 _("Supported languages"),
                                 G_PARAM_READABLE |
                                 G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_LANGUAGES, pspec);

    /**
     * OsinfoMedia:volume-size:
     *
     * Expected volume size, in bytes for ISO9660 image/device.
     */
    pspec = g_param_spec_int64("volume-size",
                               "VolumeSize",
                               _("Expected ISO9660 volume size, in bytes"),
                               G_MININT,
                               G_MAXINT64,
                               -1 /* default value */,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_VOLUME_SIZE, pspec);

    /**
     * OsinfoMedia:eject-after-install:
     *
     * Whether the media should be ejected after the installation process.
     *
     * Some distros need their media to not be ejected after the final reboot
     * during its installation process as some packages are installed after the
     * reboot (which may cause the media to be ejected, depending on the
     * application).
     */
    pspec = g_param_spec_boolean("eject-after-install",
                                 "EjectAfterInstall",
                                 _("Whether the media should be ejected after the installtion process"),
                                 TRUE /* default value */,
                                 G_PARAM_READWRITE |
                                 G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_EJECT_AFTER_INSTALL, pspec);

    /**
     * OsinfoMedia:installer-script:
     *
     * Whether the media supports installation via an install-script.
     *
     * Some distros provide a few different medias and not all the medias support
     * installation via an install script.
     */
    pspec = g_param_spec_boolean("installer-script",
                                 "InstallerScript",
                                 _("Whether the media should be used for an installation using install scripts"),
                                 TRUE /* default value */,
                                 G_PARAM_READWRITE |
                                 G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_klass, PROP_INSTALLER_SCRIPT, pspec);
}

static void
osinfo_media_init(OsinfoMedia *media)
{
    media->priv = OSINFO_MEDIA_GET_PRIVATE(media);
    g_weak_ref_init(&media->priv->os, NULL);
    media->priv->scripts = osinfo_install_scriptlist_new();
}

OsinfoMedia *osinfo_media_new(const gchar *id,
                              const gchar *architecture)
{
    OsinfoMedia *media;

    media = g_object_new(OSINFO_TYPE_MEDIA,
                         "id", id,
                         NULL);

    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_ARCHITECTURE,
                            architecture);

    return media;
}

static void on_media_create_from_location_ready(GObject *source_object,
                                                 GAsyncResult *res,
                                                 gpointer user_data)
{
    CreateFromLocationData *data = (CreateFromLocationData *)user_data;

    data->res = g_object_ref(res);

    g_main_loop_quit(data->main_loop);
}

/**
 * osinfo_media_create_from_location:
 * @location: the location of an installation media
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 *
 * Creates a new #OsinfoMedia for installation media at @location. The @location
 * could be any URI that GIO can handle or a local path.
 *
 * NOTE: Currently this only works for ISO images/devices.
 *
 * Returns: (transfer full): a new #OsinfoMedia , or NULL on error
 */
OsinfoMedia *osinfo_media_create_from_location(const gchar *location,
                                               GCancellable *cancellable,
                                               GError **error)
{
    CreateFromLocationData *data;
    OsinfoMedia *ret;

    data = g_slice_new0(CreateFromLocationData);
    data->main_loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);

    osinfo_media_create_from_location_async(location,
                                            G_PRIORITY_DEFAULT,
                                            cancellable,
                                            on_media_create_from_location_ready,
                                            data);

    /* Loop till we get a reply (or time out) */
    g_main_loop_run(data->main_loop);

    ret = osinfo_media_create_from_location_finish(data->res, error);
    create_from_location_data_free(data);

    return ret;
}

/**
 * osinfo_media_create_from_location_with_flags:
 * @location: the location of an installation media
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: The location where to store any error, or %NULL
 * @flags: An #OsinfoMediaDetectFlag, or 0.
 *
 * Creates a new #OsinfoMedia for installation media at @location. The @location
 * could be any URI that GIO can handle or a local path.
 *
 * NOTE: Currently this only works for ISO images/devices.
 *
 * Returns: (transfer full): a new #OsinfoMedia , or NULL on error
 */
OsinfoMedia *osinfo_media_create_from_location_with_flags(const gchar *location,
                                                          GCancellable *cancellable,
                                                          guint flags,
                                                          GError **error)
{
    CreateFromLocationData *data;
    OsinfoMedia *ret;

    data = g_slice_new0(CreateFromLocationData);
    data->main_loop = g_main_loop_new(g_main_context_get_thread_default(),
                                      FALSE);

    osinfo_media_create_from_location_with_flags_async(location,
                                                       G_PRIORITY_DEFAULT,
                                                       cancellable,
                                                       on_media_create_from_location_ready,
                                                       flags,
                                                       data);

    /* Loop till we get a reply (or time out) */
    g_main_loop_run(data->main_loop);

    ret = osinfo_media_create_from_location_with_flags_finish(data->res, error);
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

static void set_non_bootable_media_error(GError **error)
{
    g_set_error(error,
                OSINFO_MEDIA_ERROR,
                OSINFO_MEDIA_ERROR_NOT_BOOTABLE,
                _("Install media is not bootable"));
}

static OsinfoMedia *
create_from_location_async_data(CreateFromLocationAsyncData *data)
{
    OsinfoMedia *media;
    gchar *uri;
    guint64 vol_size;
    guint8 index;

    uri = g_file_get_uri(data->file);
    media = g_object_new(OSINFO_TYPE_MEDIA,
                         "id", uri,
                         NULL);
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_URL,
                            uri);
    g_free(uri);
    if (!is_str_empty(data->volume))
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_VOLUME_ID,
                                data->volume);
    if (!is_str_empty(data->system))
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_SYSTEM_ID,
                                data->system);
    if (!is_str_empty(data->publisher))
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_PUBLISHER_ID,
                                data->publisher);
    if (!is_str_empty(data->application))
        osinfo_entity_set_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_APPLICATION_ID,
                                data->application);

    index = (G_BYTE_ORDER == G_LITTLE_ENDIAN) ? 0 : 1;
    vol_size = ((gint64) data->pvd.volume_space_size[index]) *
               data->pvd.logical_blk_size[index];
    osinfo_entity_set_param_int64(OSINFO_ENTITY(media),
                                  OSINFO_MEDIA_PROP_VOLUME_SIZE,
                                  vol_size);

    return media;
}

static gboolean check_directory_record_entry_flags(guint8 flags,
                                                   gboolean is_dir)
{
    if (is_dir)
        return (flags & DIRECTORY_RECORD_FLAG_DIRECTORY) != 0;

    return (flags & DIRECTORY_RECORD_FLAG_DIRECTORY) == 0;
}

static void on_directory_record_extent_read(GObject *source,
                                            GAsyncResult *res,
                                            gpointer user_data)
{
    GInputStream *stream = G_INPUT_STREAM(source);
    SearchPPCBootinfoAsyncData *data;
    DirectoryRecord *dr;
    gsize offset;
    gssize ret;
    gboolean is_dir;
    guint8 index = (G_BYTE_ORDER == G_LITTLE_ENDIAN) ? 0 : 1;
    GError *error = NULL;

    data = (SearchPPCBootinfoAsyncData *)user_data;

    ret = g_input_stream_read_finish(stream, res, &error);
    if (ret < 0) {
        g_prefix_error(&error,
                       _("Failed to read \"%s\" directory record extent: "),
                       data->filepath[data->filepath_index]);
        goto cleanup;
    }

    if (ret == 0) {
        g_set_error(&error,
                    OSINFO_MEDIA_ERROR,
                    OSINFO_MEDIA_ERROR_NO_DIRECTORY_RECORD_EXTENT,
                    _("No \"%s\" directory record extent"),
                    data->filepath[data->filepath_index]);
        goto cleanup;
    }

    data->offset += ret;
    if (data->offset < data->length) {
        g_input_stream_read_async(stream,
                                  ((gchar *)data->extent + data->offset),
                                  data->length - data->offset,
                                  g_task_get_priority(data->res),
                                  g_task_get_cancellable(data->res),
                                  on_directory_record_extent_read,
                                  data);
        return;
    }


    is_dir = data->filepath_index < data->filepath_index_max - 1;
    offset = 0;

    do {
        gboolean check;

        dr = (DirectoryRecord *)&data->extent[offset];
        if (dr->length == 0) {
            offset++;
            continue;
        }

        check = check_directory_record_entry_flags(dr->flags, is_dir);
        if (check &&
            g_ascii_strncasecmp(data->filepath[data->filepath_index], dr->filename, strlen(data->filepath[data->filepath_index])) == 0) {
            data->filepath_index++;
            break;
        }

        offset += dr->length;
    } while (offset < data->length);

    if (offset >= data->length) {
        set_non_bootable_media_error(&error);
        goto cleanup;
    }

    /* It just means that we walked through all the filepath entries and we
     * found the file we're looking for! Just return TRUE! */
    if (data->filepath_index == data->filepath_index_max)
        goto cleanup;

    if (!g_seekable_seek(G_SEEKABLE(stream),
                         dr->extent_location[index]  * data->pvd->logical_blk_size[index],
                         G_SEEK_SET,
                         g_task_get_cancellable(data->res),
                         &error))
        goto cleanup;

    data->offset = 0;
    data->length = dr->extent_size[index];

    g_free(data->extent);
    data->extent = g_malloc0(data->length);
    g_input_stream_read_async(stream,
                              data->extent,
                              data->length,
                              g_task_get_priority(data->res),
                              g_task_get_cancellable(data->res),
                              on_directory_record_extent_read,
                              data);
    return;

 cleanup:
    if (error != NULL)
        g_task_return_error(data->res, error);
    else
        g_task_return_boolean(data->res, TRUE);

    search_ppc_bootinfo_async_data_free(data);
}

static void search_ppc_bootinfo_async(GInputStream *stream,
                                      PrimaryVolumeDescriptor *pvd,
                                      gint priority,
                                      GCancellable *cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data)
{
    SearchPPCBootinfoAsyncData *data;
    DirectoryRecord *root_directory_entry = NULL;
    GError *error = NULL;
    guint8 index = (G_BYTE_ORDER == G_LITTLE_ENDIAN) ? 0 : 1;

    g_return_if_fail(G_IS_INPUT_STREAM(stream));
    g_return_if_fail(pvd != NULL);

    data = g_slice_new0(SearchPPCBootinfoAsyncData);
    data->pvd = pvd;
    data->res = g_task_new(stream,
                           cancellable,
                           callback,
                           user_data);
    g_task_set_priority(data->res, priority);

    root_directory_entry = (DirectoryRecord *)&data->pvd->root_directory_entry;

    if (!g_seekable_seek(G_SEEKABLE(stream),
                         root_directory_entry->extent_location[index] * data->pvd->logical_blk_size[index],
                         G_SEEK_SET,
                         g_task_get_cancellable(data->res),
                         &error))
        goto cleanup;

    data->offset = 0;
    data->length = root_directory_entry->extent_size[index];
    data->extent = g_malloc0(root_directory_entry->extent_size[index]);

    data->filepath = g_strsplit(PPC_BOOTINFO, "/", -1);
    /* As the path starts with "/", we can just ignore the first element of the
     * split entry. */
    data->filepath_index = 1;
    data->filepath_index_max = g_strv_length(data->filepath);

    g_input_stream_read_async(stream,
                              data->extent,
                              data->length,
                              g_task_get_priority(data->res),
                              g_task_get_cancellable(data->res),
                              on_directory_record_extent_read,
                              data);
    return;

 cleanup:
    g_task_return_error(data->res, error);
    search_ppc_bootinfo_async_data_free(data);
}

static gboolean search_ppc_bootinfo_finish(GAsyncResult *res,
                                           GError **error)
{
    GTask *task = G_TASK(res);

    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    return g_task_propagate_boolean(task, error);
}

static void search_ppc_bootinfo_callback(GObject *source,
                                         GAsyncResult *res,
                                         gpointer user_data)
{
    OsinfoMedia *media = NULL;
    GInputStream *stream = G_INPUT_STREAM(source);
    GError *error = NULL;
    CreateFromLocationAsyncData *data;
    gboolean ret;

    data = (CreateFromLocationAsyncData *)user_data;

    ret = search_ppc_bootinfo_finish(res, &error);
    if (!ret)
        goto cleanup;

    media = create_from_location_async_data(data);

 cleanup:
    if (error != NULL)
        g_task_return_error(data->res, error);
    else
        g_task_return_pointer(data->res, media, g_object_unref);

    g_object_unref(stream);
    create_from_location_async_data_free(data);
}

static void on_svd_read(GObject *source,
                         GAsyncResult *res,
                         gpointer user_data)
{
    OsinfoMedia *media = NULL;
    GInputStream *stream = G_INPUT_STREAM(source);
    GError *error = NULL;
    CreateFromLocationAsyncData *data;
    gssize ret;

    data = (CreateFromLocationAsyncData *)user_data;

    ret = g_input_stream_read_finish(stream,
                                     res,
                                     &error);
    if (ret < 0) {
        g_prefix_error(&error,
                       _("Failed to read supplementary volume descriptor: "));
        goto cleanup;
    }
    if (ret == 0) {
        g_set_error(&error,
                    OSINFO_MEDIA_ERROR,
                    OSINFO_MEDIA_ERROR_NO_SVD,
                    _("Supplementary volume descriptor was truncated"));
        goto cleanup;
    }

    data->offset += ret;
    if (data->offset < data->length) {
        g_input_stream_read_async(stream,
                                  ((gchar *)&data->svd + data->offset),
                                  data->length - data->offset,
                                  g_task_get_priority(data->res),
                                  g_task_get_cancellable(data->res),
                                  on_svd_read,
                                  data);
        return;
    }


    data->svd.system[MAX_SYSTEM - 1] = 0;
    g_strchomp(data->svd.system);

    if (strncmp(BOOTABLE_TAG, data->svd.system, sizeof(BOOTABLE_TAG)) != 0) {
        /*
         * In case we reached this point, there are basically 2 alternatives:
         * - the media is a PPC media and we should check for the existence of
         *   "/ppc/bootinfo.txt" file
         * - the media is not bootable.
         *
         * Let's check for the existence of the "/ppc/bootinfo.txt" file and,
         * only after that, return whether the media is bootable or not.
         */
        search_ppc_bootinfo_async(stream,
                                  &data->pvd,
                                  g_task_get_priority(data->res),
                                  g_task_get_cancellable(data->res),
                                  search_ppc_bootinfo_callback,
                                  data);
        return;
    }

    media = create_from_location_async_data(data);

 cleanup:
    if (error != NULL)
        g_task_return_error(data->res, error);
    else
        g_task_return_pointer(data->res, media, g_object_unref);

    g_object_unref(stream);
    create_from_location_async_data_free(data);
}

static void on_pvd_read(GObject *source,
                         GAsyncResult *res,
                         gpointer user_data)
{
    GInputStream *stream = G_INPUT_STREAM(source);
    CreateFromLocationAsyncData *data;
    GError *error = NULL;
    gssize ret;

    data = (CreateFromLocationAsyncData *)user_data;

    ret = g_input_stream_read_finish(stream,
                                     res,
                                     &error);
    if (ret < 0) {
        g_prefix_error(&error, _("Failed to read primary volume descriptor: "));
        goto error;
    }
    if (ret == 0) {
        g_set_error(&error,
                    OSINFO_MEDIA_ERROR,
                    OSINFO_MEDIA_ERROR_NO_PVD,
                    _("Primary volume descriptor was truncated"));
        goto error;
    }

    data->offset += ret;
    if (data->offset < data->length) {
        g_input_stream_read_async(stream,
                                  ((gchar*)&data->pvd) + data->offset,
                                  data->length - data->offset,
                                  g_task_get_priority(data->res),
                                  g_task_get_cancellable(data->res),
                                  on_pvd_read,
                                  data);
        return;
    }

    data->volume = g_strndup(data->pvd.volume, MAX_VOLUME);
    g_strchomp(data->volume);

    data->system = g_strndup(data->pvd.system, MAX_SYSTEM);
    g_strchomp(data->system);

    data->publisher = g_strndup(data->pvd.publisher, MAX_PUBLISHER);
    g_strchomp(data->publisher);

    data->application = g_strndup(data->pvd.application, MAX_APPLICATION);
    g_strchomp(data->application);

    if (is_str_empty(data->volume)) {
        g_set_error(&error,
                    OSINFO_MEDIA_ERROR,
                    OSINFO_MEDIA_ERROR_INSUFFICIENT_METADATA,
                    _("Insufficient metadata on installation media"));

        goto error;
    }

    data->offset = 0;
    data->length = sizeof(data->svd);

    g_input_stream_read_async(stream,
                              (gchar *)&data->svd,
                              data->length,
                              g_task_get_priority(data->res),
                              g_task_get_cancellable(data->res),
                              on_svd_read,
                              data);
    return;

 error:
    g_object_unref(stream);
    g_task_return_error(data->res, error);
    create_from_location_async_data_free(data);
}

static void on_location_skipped(GObject *source,
                                GAsyncResult *res,
                                gpointer user_data)
{
    GInputStream *stream = G_INPUT_STREAM(source);
    CreateFromLocationAsyncData *data;
    GError *error = NULL;

    data = (CreateFromLocationAsyncData *)user_data;

    if (g_input_stream_skip_finish(stream, res, &error) < PVD_OFFSET) {
        if (error)
            g_prefix_error(&error, _("Failed to skip %d bytes"), PVD_OFFSET);
        else
            g_set_error(&error,
                        OSINFO_MEDIA_ERROR,
                        OSINFO_MEDIA_ERROR_NO_DESCRIPTORS,
                        _("No volume descriptors"));
        g_object_unref(stream);
        g_task_return_error(data->res, error);
        create_from_location_async_data_free(data);

        return;
    }

    data->offset = 0;
    data->length = sizeof(data->pvd);

    g_input_stream_read_async(stream,
                              (gchar *)&data->pvd,
                              data->length,
                              g_task_get_priority(data->res),
                              g_task_get_cancellable(data->res),
                              on_pvd_read,
                              data);
}

static void on_location_read(GObject *source,
                             GAsyncResult *res,
                             gpointer user_data)
{
    GFileInputStream *stream;
    CreateFromLocationAsyncData *data;
    GError *error = NULL;

    data = (CreateFromLocationAsyncData *)user_data;

    stream = g_file_read_finish(G_FILE(source), res, &error);
    if (error != NULL) {
        g_prefix_error(&error, _("Failed to open file"));
        g_task_return_error(data->res, error);
        create_from_location_async_data_free(data);

        return;
    }

    g_input_stream_skip_async(G_INPUT_STREAM(stream),
                              PVD_OFFSET,
                              g_task_get_priority(data->res),
                              g_task_get_cancellable(data->res),
                              on_location_skipped,
                              data);
}

/**
 * osinfo_media_create_from_location_async:
 * @location: the location of an installation media
 * @priority: the I/O priority of the request
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_media_create_from_location.
 */
void osinfo_media_create_from_location_async(const gchar *location,
                                             gint priority,
                                             GCancellable *cancellable,
                                             GAsyncReadyCallback callback,
                                             gpointer user_data)
{
    CreateFromLocationAsyncData *data;

    g_return_if_fail(location != NULL);

    data = g_slice_new0(CreateFromLocationAsyncData);
    data->res = g_task_new(NULL,
                           cancellable,
                           callback,
                           user_data);
    g_task_set_priority(data->res, priority);

    data->file = g_file_new_for_commandline_arg(location);
    g_file_read_async(data->file,
                      priority,
                      cancellable,
                      on_location_read,
                      data);
}

/**
 * osinfo_media_create_from_location_finish:
 * @res: a #GAsyncResult
 * @error: The location where to store any error, or %NULL
 *
 * Finishes an asynchronous media object creation process started with
 * #osinfo_media_create_from_location_async.
 *
 * Returns: (transfer full): a new #OsinfoMedia , or NULL on error
 */
OsinfoMedia *osinfo_media_create_from_location_finish(GAsyncResult *res,
                                                      GError **error)
{
    GTask *task = G_TASK(res);

    g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    return g_task_propagate_pointer(task, error);
}

/**
 * osinfo_media_create_from_location_with_flags_async:
 * @location: the location of an installation media
 * @priority: the I/O priority of the request
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: Function to call when result of this call is ready
 * @flags: An #OsinfoMediaDetectFlag, or 0.
 * @user_data: The user data to pass to @callback, or %NULL
 *
 * Asynchronous variant of #osinfo_media_create_from_location.
 */
void osinfo_media_create_from_location_with_flags_async(const gchar *location,
                                                        gint priority,
                                                        GCancellable *cancellable,
                                                        GAsyncReadyCallback callback,
                                                        guint flags,
                                                        gpointer user_data)
{
    CreateFromLocationAsyncData *data;

    g_return_if_fail(location != NULL);

    data = g_slice_new0(CreateFromLocationAsyncData);
    data->res = g_task_new(NULL,
                           cancellable,
                           callback,
                           user_data);
    g_task_set_priority(data->res, priority);
    data->flags = flags;

    data->file = g_file_new_for_commandline_arg(location);
    g_file_read_async(data->file,
                      priority,
                      cancellable,
                      on_location_read,
                      data);
}

/**
 * osinfo_media_create_from_location_with_flags_finish:
 * @res: a #GAsyncResult
 * @error: The location where to store any error, or %NULL
 *
 * Finishes an asynchronous media object creation process started with
 * #osinfo_media_create_from_location_async.
 *
 * Returns: (transfer full): a new #OsinfoMedia , or NULL on error
 */
OsinfoMedia *osinfo_media_create_from_location_with_flags_finish(GAsyncResult *res,
                                                                 GError **error)
{
    GTask *task = G_TASK(res);

    g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    return g_task_propagate_pointer(task, error);
}

/**
 * osinfo_media_get_architecture:
 * @media: an #OsinfoMedia instance
 *
 * Retrieves the target hardware architecture of the OS @media provides.
 *
 * Returns: (transfer none): the hardware architecture, or NULL
 */
const gchar *osinfo_media_get_architecture(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_ARCHITECTURE);
}

/**
 * osinfo_media_get_url:
 * @media: an #OsinfoMedia instance
 *
 * The URL to the @media
 *
 * Returns: (transfer none): the URL, or NULL
 */
const gchar *osinfo_media_get_url(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_URL);
}

/**
 * osinfo_media_get_volume_id:
 * @media: an #OsinfoMedia instance
 *
 * If @media is an ISO9660 image/device, this function retrieves the expected
 * volume ID.
 *
 * Note: In practice, this will usually not be the exact copy of the volume ID
 * string on the ISO image/device but rather a regular expression that matches
 * it.
 *
 * Returns: (transfer none): the volume id, or NULL
 */
const gchar *osinfo_media_get_volume_id(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_VOLUME_ID);
}

/**
 * osinfo_media_get_system_id:
 * @media: an #OsinfoMedia instance
 *
 * If @media is an ISO9660 image/device, this function retrieves the expected
 * system ID.
 *
 * Note: In practice, this will usually not be the exact copy of the system ID
 * string on the ISO image/device but rather a regular expression that matches
 * it.
 *
 * Returns: (transfer none): the system id, or NULL
 */
const gchar *osinfo_media_get_system_id(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_SYSTEM_ID);
}

/**
 * osinfo_media_get_publisher_id:
 * @media: an #OsinfoMedia instance
 *
 * If @media is an ISO9660 image/device, this function retrieves the expected
 * publisher ID.
 *
 * Note: In practice, this will usually not be the exact copy of the publisher
 * ID string on the ISO image/device but rather a regular expression that
 * matches it.
 *
 * Returns: (transfer none): the publisher id, or NULL
 */
const gchar *osinfo_media_get_publisher_id(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_PUBLISHER_ID);
}

/**
 * osinfo_media_get_application_id:
 * @media: an #OsinfoMedia instance
 *
 * If @media is an ISO9660 image/device, this function retrieves the expected
 * application ID.
 *
 * Note: In practice, this will usually not be the exact copy of the application
 * ID string on the ISO image/device but rather a regular expression that
 * matches it.
 *
 * Returns: (transfer none): the application id, or NULL
 */
const gchar *osinfo_media_get_application_id(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_APPLICATION_ID);
}

/**
 * osinfo_media_get_kernel_path:
 * @media: an #OsinfoMedia instance
 *
 * Retrieves the path to the kernel image in the install tree.
 *
 * Note: This only applies to installer medias of 'linux' OS family.
 *
 * Returns: (transfer none): the path to kernel image, or NULL
 */
const gchar *osinfo_media_get_kernel_path(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_KERNEL);
}

/**
 * osinfo_media_get_initrd_path:
 * @media: an #OsinfoMedia instance
 *
 * Retrieves the path to the initrd image in the install tree.
 *
 * Note: This only applies to installer medias of 'linux' OS family.
 *
 * Returns: (transfer none): the path to initrd image, or NULL
 */
const gchar *osinfo_media_get_initrd_path(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(media),
                                         OSINFO_MEDIA_PROP_INITRD);
}

/**
 * osinfo_media_get_installer:
 * @media: an #OsinfoMedia instance
 *
 * Whether @media provides an installer for an OS.
 *
 * Returns: #TRUE if media is installer, #FALSE otherwise
 */
gboolean osinfo_media_get_installer(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value_boolean_with_default
            (OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_INSTALLER, TRUE);
}

/**
 * osinfo_media_get_live:
 * @media: an #OsinfoMedia instance
 *
 * Whether @media can boot directly an OS without any installations.
 *
 * Returns: #TRUE if media is live, #FALSE otherwise
 */
gboolean osinfo_media_get_live(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value_boolean_with_default
            (OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_LIVE, FALSE);
}

/**
 * osinfo_media_get_installer_reboots:
 * @media: an #OsinfoMedia instance
 *
 * If media is an installer, this method retrieves the number of reboots the
 * installer takes before installation is complete.
 *
 * This function is not supposed to be called on media that has no installer.
 * You can use #osinfo_media_get_installer(or OsinfoMedia::installer) to check
 * that.
 *
 * Warning: Some media allow you to install from live sessions, in which case
 * number of reboots *alone* is not a reliable method for tracking installation.
 *
 * Returns: (transfer none): the number of installer reboots or -1 if media is
 * not an installer
 */
gint osinfo_media_get_installer_reboots(OsinfoMedia *media)
{
    g_return_val_if_fail(OSINFO_IS_MEDIA(media), -1);
    g_return_val_if_fail(osinfo_media_get_installer(media), -1);

    return (gint) osinfo_entity_get_param_value_int64_with_default
            (OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_INSTALLER_REBOOTS, 1);
}

/**
 * osinfo_media_get_os:
 * @media: an #OsinfoMedia instance
 *
 * Returns: (transfer full): the operating system, or NULL
 */
OsinfoOs *osinfo_media_get_os(OsinfoMedia *media)
{
    g_return_val_if_fail(OSINFO_IS_MEDIA(media), NULL);

    return g_weak_ref_get(&media->priv->os);
}

void osinfo_media_set_os(OsinfoMedia *media, OsinfoOs *os)
{
    g_return_if_fail(OSINFO_IS_MEDIA(media));

    g_object_ref(os);
    g_weak_ref_set(&media->priv->os, os);
    g_object_unref(os);
}

/**
 * osinfo_media_get_os_variants:
 * @media: an #OsinfoMedia instance
 *
 * Gets the variants of the associated operating system.
 *
 * Returns: (transfer full): the operating system variant, or NULL
 */
OsinfoOsVariantList *osinfo_media_get_os_variants(OsinfoMedia *media)
{
    OsinfoOs *os;
    OsinfoOsVariantList *os_variants;
    OsinfoOsVariantList *media_variants;
    GList *ids, *node;
    OsinfoFilter *filter;

    g_return_val_if_fail(OSINFO_IS_MEDIA(media), NULL);

    os = g_weak_ref_get(&media->priv->os);
    if (os == NULL)
        return NULL;

    os_variants = osinfo_os_get_variant_list(os);
    g_object_unref(os);

    ids = osinfo_entity_get_param_value_list(OSINFO_ENTITY(media),
                                             OSINFO_MEDIA_PROP_VARIANT);
    filter = osinfo_filter_new();
    media_variants = osinfo_os_variantlist_new();
    for (node = ids; node != NULL; node = node->next) {
        osinfo_filter_clear_constraints(filter);
        osinfo_filter_add_constraint(filter,
                                     OSINFO_ENTITY_PROP_ID,
                                     (const char *) node->data);
        osinfo_list_add_filtered(OSINFO_LIST(media_variants),
                                 OSINFO_LIST(os_variants),
                                 filter);
    }
    g_object_unref(os_variants);

    return media_variants;
}

/**
 * osinfo_media_get_languages:
 * @media: an #OsinfoMedia instance
 *
 * If media is an installer, this property indicates the languages that
 * can be used during automatic installations.
 *
 * On media that are not installers, this property will indicate the
 * languages that the user interface can be displayed in.
 * Use #osinfo_media_get_installer(or OsinfoMedia::installer) to know
 * if the media is an installer or not.
 *
 * Returns: (transfer container) (element-type utf8): a #GList
 * containing the list of the UI languages this media supports. The list
 * must be freed with g_list_free() when no longer needed. If the
 * supported languages are unknown, NULL will be returned.
 */
GList *osinfo_media_get_languages(OsinfoMedia *media)
{
    g_return_val_if_fail(OSINFO_IS_MEDIA(media), NULL);
    return osinfo_entity_get_param_value_list(OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_LANG);
}

/**
 * osinfo_media_set_languages:
 * @media: an #OsinfoMedia instance
 * @languages: (element-type utf8): a #GList containing the list of the UI
 * languages this media supports.
 *
 * Sets the #OSINFO_MEDIA_PROP_LANG parameter
 */
void osinfo_media_set_languages(OsinfoMedia *media, GList *languages)
{
    GList *it;

    g_return_if_fail(OSINFO_IS_MEDIA(media));

    osinfo_entity_clear_param(OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_LANG);
    for (it = languages; it != NULL; it = it->next)
        osinfo_entity_add_param(OSINFO_ENTITY(media),
                                OSINFO_MEDIA_PROP_LANG,
                                it->data);
}

/**
 * osinfo_media_get_volume_size:
 * @media: an #OsinfoMedia instance
 *
 * Returns: (transfer none): the ISO9660 volume size, in bytes or -1 if size is
 * unknown or media is not an ISO9660 device/image.
 */
gint64 osinfo_media_get_volume_size(OsinfoMedia *media)
{
    g_return_val_if_fail(OSINFO_IS_MEDIA(media), -1);

    return osinfo_entity_get_param_value_int64_with_default
        (OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_VOLUME_SIZE, -1);
}

/**
 * osinfo_media_get_eject_after_install:
 * @media: an #OsinfoMedia instance
 *
 * Whether @media should ejected after the installation procces.
 *
 * Returns: #TRUE if media should be ejected, #FALSE otherwise
 */
gboolean osinfo_media_get_eject_after_install(OsinfoMedia *media)
{
    return osinfo_entity_get_param_value_boolean_with_default
        (OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_EJECT_AFTER_INSTALL, TRUE);
}

/**
 * osinfo_media_supports_installer_script:
 * @media: an #OsinfoMedia instance
 *
 * Whether @media supports installation using install scripts.
 *
 * Returns: #TRUE if install-scripts are supported by the media,
 * #FALSE otherwise
 */
gboolean osinfo_media_supports_installer_script(OsinfoMedia *media)
{
    OsinfoOs *os;
    OsinfoInstallScriptList *list;
    gboolean ret;

    g_return_val_if_fail(OSINFO_IS_MEDIA(media), FALSE);

    os = osinfo_media_get_os(media);
    list = osinfo_os_get_install_script_list(os);

    if (osinfo_list_get_length(OSINFO_LIST(list)) == 0 &&
        osinfo_list_get_length(OSINFO_LIST(media->priv->scripts)) == 0) {
        ret = FALSE;
        goto cleanup;
    }

    ret = osinfo_entity_get_param_value_boolean_with_default
            (OSINFO_ENTITY(media), OSINFO_MEDIA_PROP_INSTALLER_SCRIPT, TRUE);

 cleanup:
    g_object_unref(list);
    g_object_unref(os);

    return ret;
}

/**
 * osinfo_media_add_install_script:
 * @media: an #OsinfoMedia instance
 * @script: an #OsinfoInstallScript instance
 *
 * Adds an @script to the specified @media
 */
void osinfo_media_add_install_script(OsinfoMedia *media, OsinfoInstallScript *script)
{
    g_return_if_fail(OSINFO_IS_MEDIA(media));

    osinfo_list_add(OSINFO_LIST(media->priv->scripts), OSINFO_ENTITY(script));
}

/**
 * osinfo_media_get_install_script_list:
 * @media: an #OsinfoMedia instance
 *
 * Returns: (transfer full): a list of the install scripts for the specified media
 */
OsinfoInstallScriptList *osinfo_media_get_install_script_list(OsinfoMedia *media)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_MEDIA(media), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(media->priv->scripts));

    return OSINFO_INSTALL_SCRIPTLIST(new_list);
}
