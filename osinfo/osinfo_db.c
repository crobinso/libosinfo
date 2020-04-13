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
#include "osinfo_media_private.h"
#include <gio/gio.h>
#include <string.h>
#include <glib/gi18n-lib.h>

#define match_regex(pattern, str)                                       \
    (((pattern) == NULL) ||                                             \
     (((str) != NULL) &&                                                \
      g_regex_match_simple((pattern), (str), 0, 0)))

static gchar *get_raw_lang(const char *volume_id, const gchar *regex_str)
{
    GRegex *regex;
    GMatchInfo *match;
    gboolean matched;
    gchar *raw_lang = NULL;

    regex = g_regex_new(regex_str, G_REGEX_ANCHORED,
                        G_REGEX_MATCH_ANCHORED, NULL);
    if (regex == NULL)
        return NULL;

    matched = g_regex_match(regex, volume_id, G_REGEX_MATCH_ANCHORED, &match);
    if (!matched || !g_match_info_matches(match))
        goto end;
    raw_lang = g_match_info_fetch(match, 1);
    if (raw_lang == NULL)
        goto end;

end:
    g_match_info_unref(match);
    g_regex_unref(regex);

    return raw_lang;
}

static const char *language_code_from_raw(OsinfoDatamap *lang_map,
                                          const char *raw_lang)
{
    const char *lang;

    if (lang_map == NULL)
        return raw_lang;

    lang = osinfo_datamap_lookup(lang_map, raw_lang);
    if (lang == NULL)
        return raw_lang;

    return lang;
}

static void set_languages_for_media(OsinfoDb *db, OsinfoMedia *media,
                                    OsinfoMedia *db_media)
{
    const gchar *volume_id;
    const gchar *regex;
    const gchar *lang_map_id;
    OsinfoDatamap *lang_map;
    gchar *raw_lang;
    GList *languages;
    const char *lang;

    g_return_if_fail(OSINFO_IS_MEDIA(media));
    g_return_if_fail(OSINFO_IS_MEDIA(db_media));

    regex = osinfo_entity_get_param_value(OSINFO_ENTITY(db_media),
                                          OSINFO_MEDIA_PROP_LANG_REGEX);
    if (regex == NULL)
        return;

    volume_id = osinfo_media_get_volume_id(media);
    if (volume_id == NULL)
        return;

    lang_map_id = osinfo_entity_get_param_value(OSINFO_ENTITY(db_media),
                                                OSINFO_MEDIA_PROP_LANG_MAP);
    if (lang_map_id != NULL) {
        lang_map = osinfo_db_get_datamap(db, lang_map_id);
    } else {
        lang_map = NULL;
    }

    raw_lang = get_raw_lang(volume_id, regex);
    lang = language_code_from_raw(lang_map, raw_lang);
    languages = g_list_append(NULL, (gpointer)lang);
    osinfo_media_set_languages(media, languages);
    g_list_free(languages);
    g_free(raw_lang);
}


/**
 * SECTION:osinfo_db
 * @short_description: Database of all entities
 * @see_also: #OsinfoList, #OsinfoEntity
 *
 * #OsinfoDb is a database tracking all entity instances against which
 * metadata is recorded.
 */

struct _OsinfoDbPrivate
{
    OsinfoDeviceList *devices;
    OsinfoPlatformList *platforms;
    OsinfoOsList *oses;
    OsinfoDeploymentList *deployments;
    OsinfoDatamapList *datamaps;
    OsinfoInstallScriptList *scripts;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDb, osinfo_db, G_TYPE_OBJECT);

static void
osinfo_db_finalize(GObject *object)
{
    OsinfoDb *db = OSINFO_DB(object);

    g_object_unref(db->priv->devices);
    g_object_unref(db->priv->platforms);
    g_object_unref(db->priv->oses);
    g_object_unref(db->priv->deployments);
    g_object_unref(db->priv->datamaps);
    g_object_unref(db->priv->scripts);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_db_parent_class)->finalize(object);
}


/* Init functions */
static void
osinfo_db_class_init(OsinfoDbClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_db_finalize;
}


static void
osinfo_db_init(OsinfoDb *db)
{
    db->priv = osinfo_db_get_instance_private(db);
    db->priv->devices = osinfo_devicelist_new();
    db->priv->platforms = osinfo_platformlist_new();
    db->priv->oses = osinfo_oslist_new();
    db->priv->deployments = osinfo_deploymentlist_new();
    db->priv->datamaps = osinfo_datamaplist_new();
    db->priv->scripts = osinfo_install_scriptlist_new();
}

/**
 * osinfo_db_new:
 *
 * Returns: (transfer full): the new database
 */
OsinfoDb *osinfo_db_new(void)
{
    return g_object_new(OSINFO_TYPE_DB, NULL);
}

/**
 * osinfo_db_get_platform:
 * @db: the database
 * @id: the unique platform identifier
 *
 * Returns: (transfer none): the platform, or NULL if none is found
 */
OsinfoPlatform *osinfo_db_get_platform(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_PLATFORM(osinfo_list_find_by_id(OSINFO_LIST(db->priv->platforms), id));
}

/**
 * osinfo_db_get_device:
 * @db: the database
 * @id: the unique device identifier
 *
 * Returns: (transfer none): the device, or NULL if none is found
 */
OsinfoDevice *osinfo_db_get_device(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_DEVICE(osinfo_list_find_by_id(OSINFO_LIST(db->priv->devices), id));
}

/**
 * osinfo_db_get_os:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the operating system, or NULL if none is found
 */
OsinfoOs *osinfo_db_get_os(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_OS(osinfo_list_find_by_id(OSINFO_LIST(db->priv->oses), id));
}

/**
 * osinfo_db_get_deployment:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the operating system, or NULL if none is found
 */
OsinfoDeployment *osinfo_db_get_deployment(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_DEPLOYMENT(osinfo_list_find_by_id(OSINFO_LIST(db->priv->deployments), id));
}

/**
 * osinfo_db_get_datamap:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the install datamap, or NULL if none is found
 *
 * Since: 0.2.3
 */
OsinfoDatamap *osinfo_db_get_datamap(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_DATAMAP(osinfo_list_find_by_id(OSINFO_LIST(db->priv->datamaps), id));
}


/**
 * osinfo_db_get_install_script:
 * @db: the database
 * @id: the unique operating system identifier
 *
 * Returns: (transfer none): the install script, or NULL if none is found
 *
 * Since: 0.2.0
 */
OsinfoInstallScript *osinfo_db_get_install_script(OsinfoDb *db, const gchar *id)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(id != NULL, NULL);

    return OSINFO_INSTALL_SCRIPT(osinfo_list_find_by_id(OSINFO_LIST(db->priv->scripts), id));
}


/**
 * osinfo_db_find_deployment:
 * @db: the database
 * @os: the operating system to find
 * @platform: the virtualization platform
 *
 * Find the deployment for @os on @platform, if any.
 *
 * Returns: (transfer none): the deployment, or NULL
 */
OsinfoDeployment *osinfo_db_find_deployment(OsinfoDb *db,
                                            OsinfoOs *os,
                                            OsinfoPlatform *platform)
{
    GList *deployments;
    GList *tmp;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(OSINFO_IS_PLATFORM(platform), NULL);

    deployments = osinfo_list_get_elements(OSINFO_LIST(db->priv->deployments));
    tmp = deployments;

    while (tmp) {
        OsinfoDeployment *deployment = OSINFO_DEPLOYMENT(tmp->data);
        OsinfoOs *otheros = osinfo_deployment_get_os(deployment);
        OsinfoPlatform *otherplatform = osinfo_deployment_get_platform(deployment);

        if (g_strcmp0(osinfo_entity_get_id(OSINFO_ENTITY(os)),
                      osinfo_entity_get_id(OSINFO_ENTITY(otheros))) == 0 &&
            g_strcmp0(osinfo_entity_get_id(OSINFO_ENTITY(platform)),
                      osinfo_entity_get_id(OSINFO_ENTITY(otherplatform))) == 0) {
            g_list_free(deployments);
            return deployment;
        }

        tmp = tmp->next;
    }

    g_list_free(deployments);
    return NULL;
}


/**
 * osinfo_db_get_os_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of operating systems
 */
OsinfoOsList *osinfo_db_get_os_list(OsinfoDb *db)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(db->priv->oses));

    return OSINFO_OSLIST(new_list);
}

/**
 * osinfo_db_get_platform_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of platforms
 */
OsinfoPlatformList *osinfo_db_get_platform_list(OsinfoDb *db)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(db->priv->platforms));

    return OSINFO_PLATFORMLIST(new_list);
}

/**
 * osinfo_db_get_device_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of devices
 */
OsinfoDeviceList *osinfo_db_get_device_list(OsinfoDb *db)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(db->priv->devices));

    return OSINFO_DEVICELIST(new_list);
}


/**
 * osinfo_db_get_deployment_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of deployments
 */
OsinfoDeploymentList *osinfo_db_get_deployment_list(OsinfoDb *db)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(db->priv->deployments));

    return OSINFO_DEPLOYMENTLIST(new_list);
}


/**
 * osinfo_db_get_datamap_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of install datamaps
 *
 * Since: 0.2.3
 */
OsinfoDatamapList *osinfo_db_get_datamap_list(OsinfoDb *db)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(db->priv->datamaps));

    return OSINFO_DATAMAPLIST(new_list);
}


/**
 * osinfo_db_get_install_script_list:
 * @db: the database
 *
 * Returns: (transfer full): the list of install scripts
 *
 * Since: 0.2.0
 */
OsinfoInstallScriptList *osinfo_db_get_install_script_list(OsinfoDb *db)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(db->priv->scripts));

    return OSINFO_INSTALL_SCRIPTLIST(new_list);
}


/**
 * osinfo_db_add_os:
 * @db: the database
 * @os: (transfer none): an operating system
 *
 */
void osinfo_db_add_os(OsinfoDb *db, OsinfoOs *os)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_OS(os));

    osinfo_list_add(OSINFO_LIST(db->priv->oses), OSINFO_ENTITY(os));
}


/**
 * osinfo_db_add_platform:
 * @db: the database
 * @platform: (transfer none): an platform
 *
 */
void osinfo_db_add_platform(OsinfoDb *db, OsinfoPlatform *platform)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_PLATFORM(platform));

    osinfo_list_add(OSINFO_LIST(db->priv->platforms), OSINFO_ENTITY(platform));
}


/**
 * osinfo_db_add_device:
 * @db: the database
 * @device: (transfer none): a device
 *
 */
void osinfo_db_add_device(OsinfoDb *db, OsinfoDevice *device)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_DEVICE(device));

    osinfo_list_add(OSINFO_LIST(db->priv->devices), OSINFO_ENTITY(device));
}


/**
 * osinfo_db_add_deployment:
 * @db: the database
 * @deployment: (transfer none): a deployment
 *
 */
void osinfo_db_add_deployment(OsinfoDb *db, OsinfoDeployment *deployment)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_DEPLOYMENT(deployment));

    osinfo_list_add(OSINFO_LIST(db->priv->deployments), OSINFO_ENTITY(deployment));
}


/**
 * osinfo_db_add_datamap:
 * @db: the database
 * @datamap: (transfer none): an install datamap
 *
 * Since: 0.2.3
 */
void osinfo_db_add_datamap(OsinfoDb *db, OsinfoDatamap *datamap)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_DATAMAP(datamap));

    osinfo_list_add(OSINFO_LIST(db->priv->datamaps), OSINFO_ENTITY(datamap));
}


/**
 * osinfo_db_add_install_script:
 * @db: the database
 * @script: (transfer none): an install script
 *
 * Since: 0.2.0
 */
void osinfo_db_add_install_script(OsinfoDb *db, OsinfoInstallScript *script)
{
    g_return_if_fail(OSINFO_IS_DB(db));
    g_return_if_fail(OSINFO_IS_INSTALL_SCRIPT(script));

    osinfo_list_add(OSINFO_LIST(db->priv->scripts), OSINFO_ENTITY(script));
}


static gint media_volume_compare(gconstpointer a, gconstpointer b)
{
    OsinfoMedia *media_a = OSINFO_MEDIA((gpointer) a);
    OsinfoMedia *media_b = OSINFO_MEDIA((gpointer) b);
    const gchar *volume_a = osinfo_media_get_volume_id(media_a);
    const gchar *volume_b = osinfo_media_get_volume_id(media_b);

    if (volume_a == NULL || volume_b == NULL)
        /* Order doesn't matter then */
        return 0;

    if (strstr(volume_a, volume_b) != NULL) {
        gint64 volume_size_a = osinfo_media_get_volume_size(media_a);
        gint64 volume_size_b = osinfo_media_get_volume_size(media_b);

        if (volume_size_a != -1 && volume_size_b == -1)
            return -1;

        if (volume_size_b != -1 && volume_size_a == -1)
            return 1;

        return -1;
    } else {
        /* Sub-string comes later */
        return 1;
    }
}

static gboolean compare_media(OsinfoMedia *media,
                              GList *oss,
                              OsinfoOs **ret_os,
                              OsinfoMedia **matched,
                              GList **fallback_oss)
{
    GList *os_iter;
    const gchar *media_volume;
    const gchar *media_system;
    const gchar *media_publisher;
    const gchar *media_application;
    gint64 media_vol_size;

    media_volume = osinfo_media_get_volume_id(media);
    media_system = osinfo_media_get_system_id(media);
    media_publisher = osinfo_media_get_publisher_id(media);
    media_application = osinfo_media_get_application_id(media);
    media_vol_size = osinfo_media_get_volume_size(media);

    for (os_iter = oss; os_iter; os_iter = os_iter->next) {
        OsinfoOs *os = OSINFO_OS(os_iter->data);
        OsinfoReleaseStatus release_status = osinfo_os_get_release_status(os);
        OsinfoMediaList *media_list = osinfo_os_get_media_list(os);
        GList *medias = osinfo_list_get_elements(OSINFO_LIST(media_list));
        GList *media_iter;

        medias = g_list_sort(medias, media_volume_compare);

        for (media_iter = medias; media_iter; media_iter = media_iter->next) {
            OsinfoMedia *os_media = OSINFO_MEDIA(media_iter->data);
            const gchar *os_arch = osinfo_media_get_architecture(os_media);
            const gchar *os_volume = osinfo_media_get_volume_id(os_media);
            const gchar *os_system = osinfo_media_get_system_id(os_media);
            const gchar *os_publisher = osinfo_media_get_publisher_id(os_media);
            const gchar *os_application = osinfo_media_get_application_id(os_media);
            gint64 os_vol_size = osinfo_media_get_volume_size(os_media);

            if (os_volume == NULL &&
                os_system == NULL &&
                os_publisher == NULL &&
                os_application == NULL &&
                os_vol_size <= 0)
                continue;

            if (fallback_oss != NULL) {
                if (g_str_equal(os_arch, "all")) {
                    *fallback_oss = g_list_prepend(*fallback_oss, os);
                    continue;
                }

                if (release_status == OSINFO_RELEASE_STATUS_ROLLING) {
                    *fallback_oss = g_list_prepend(*fallback_oss, os);
                    continue;
                }
            }

            if (os_vol_size <= 0)
                os_vol_size = media_vol_size;

            if (match_regex(os_volume, media_volume) &&
                match_regex(os_application, media_application) &&
                match_regex(os_system, media_system) &&
                match_regex(os_publisher, media_publisher) &&
                os_vol_size == media_vol_size) {
                *ret_os = os;
                if (matched != NULL)
                    *matched = os_media;
                break;
            }
        }

        g_list_free(medias);
        g_object_unref(media_list);

        if (*ret_os)
            return TRUE;
    }

    return FALSE;
}

static OsinfoOs *
osinfo_db_guess_os_from_media_internal(OsinfoDb *db,
                                       OsinfoMedia *media,
                                       OsinfoMedia **matched_media)
{
    OsinfoOs *ret = NULL;
    GList *oss = NULL;
    GList *fallback_oss = NULL;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(media != NULL, NULL);

    oss = osinfo_list_get_elements(OSINFO_LIST(db->priv->oses));
    if (compare_media(media, oss, &ret, matched_media, &fallback_oss))
        goto end;

    compare_media(media, fallback_oss, &ret, matched_media, NULL);

 end:
    g_list_free(oss);
    g_list_free(fallback_oss);

    return ret;
}
/**
 * osinfo_db_guess_os_from_media:
 * @db: the database
 * @media: the installation media
 * @matched_media: (out) (transfer none) (allow-none): the matched operating
 * system media
 *
 * Guess operating system given an #OsinfoMedia object.
 *
 * Returns: (transfer none): the operating system, or NULL if guessing failed
 * Deprecated: 0.2.3: Use osinfo_db_identify_media() instead.
 */
OsinfoOs *osinfo_db_guess_os_from_media(OsinfoDb *db,
                                        OsinfoMedia *media,
                                        OsinfoMedia **matched_media)
{
    return osinfo_db_guess_os_from_media_internal(db, media, matched_media);
}

static void fill_media(OsinfoDb *db, OsinfoMedia *media,
                        OsinfoMedia *matched_media,
                        OsinfoOs *os)
{
    gboolean is_installer;
    gboolean is_live;
    gboolean eject_after_install;
    gint reboots;
    const gchar *id;
    const gchar *kernel_path;
    const gchar *initrd_path;
    const gchar *arch;
    const gchar *url;
    gint i;
    gboolean installer_script;
    OsinfoInstallScriptList *install_script_list;
    GList *variants, *node;

    set_languages_for_media(db, media, matched_media);

    id = osinfo_entity_get_id(OSINFO_ENTITY(matched_media));
    g_object_set(G_OBJECT(media), "id", id, NULL);

    arch = osinfo_media_get_architecture(matched_media);
    if (arch != NULL)
        g_object_set(G_OBJECT(media), "architecture", arch, NULL);
    url = osinfo_media_get_url(matched_media);
    if (url != NULL)
        g_object_set(G_OBJECT(media), "url", url, NULL);
    variants = osinfo_entity_get_param_value_list(OSINFO_ENTITY(matched_media),
                                                  "variant");
    for (node = variants; node != NULL; node = node->next)
        osinfo_entity_add_param(OSINFO_ENTITY(media),
                                "variant",
                                (gchar *) node->data);
    g_list_free(variants);
    kernel_path = osinfo_media_get_kernel_path(matched_media);
    if (kernel_path != NULL)
        g_object_set(G_OBJECT(media), "kernel_path", kernel_path, NULL);

    initrd_path = osinfo_media_get_initrd_path(matched_media);
    if (initrd_path != NULL)
        g_object_set(G_OBJECT(media), "initrd_path", initrd_path, NULL);
    is_installer = osinfo_media_get_installer(matched_media);
    is_live = osinfo_media_get_live(matched_media);
    g_object_set(G_OBJECT(media),
                 "installer", is_installer,
                 "live", is_live,
                 NULL);
    if (is_installer) {
        reboots = osinfo_media_get_installer_reboots(matched_media);
        eject_after_install =
            osinfo_media_get_eject_after_install(matched_media);

        g_object_set(G_OBJECT(media),
                     "installer-reboots", reboots,
                     "eject-after-install", eject_after_install,
                     NULL);
    }
    installer_script = osinfo_entity_get_param_value_boolean_with_default(OSINFO_ENTITY(matched_media),
                                                                          OSINFO_MEDIA_PROP_INSTALLER_SCRIPT,
                                                                          TRUE);
    g_object_set(G_OBJECT(media),
                 "installer-script", installer_script,
                 NULL);
    install_script_list = osinfo_media_get_install_script_list(matched_media);
    if (install_script_list != NULL &&
        osinfo_list_get_length(OSINFO_LIST(install_script_list)) > 0) {
        for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(install_script_list)); i++) {
            OsinfoInstallScript *script;

            script = OSINFO_INSTALL_SCRIPT(osinfo_list_get_nth(OSINFO_LIST(install_script_list), i));
            osinfo_media_add_install_script(media, script);
        }
    }

    if (os != NULL)
        osinfo_media_set_os(media, os);
}

/**
 * osinfo_db_identify_media:
 * @db: an #OsinfoDb database
 * @media: the installation media
 * data
 *
 * Try to match a newly created @media with a media description from @db.
 * If found, @media will be filled with the corresponding information
 * stored in @db. In particular, after a call to osinfo_db_identify_media(), if
 * the media could be identified, its OsinfoEntify::id and OsinfoMedia::os
 * properties will be set.
 *
 * Returns: TRUE if @media was found in @db, FALSE otherwise
 *
 * Since: 0.2.3
 */
gboolean osinfo_db_identify_media(OsinfoDb *db, OsinfoMedia *media)
{
    OsinfoMedia *matched_media;
    OsinfoOs *matched_os;

    g_return_val_if_fail(OSINFO_IS_MEDIA(media), FALSE);
    g_return_val_if_fail(OSINFO_IS_DB(db), FALSE);

    matched_os = osinfo_db_guess_os_from_media_internal(db, media,
                                                        &matched_media);
    if (matched_os == NULL) {
        return FALSE;
    }

    fill_media(db, media, matched_media, matched_os);

    return TRUE;
}

static gboolean compare_tree(OsinfoTree *tree,
                             GList *oss,
                             OsinfoOs **ret_os,
                             OsinfoTree **matched,
                             GList **fallback_oss)
{
    GList *os_iter;
    const gchar *treeinfo_family;
    const gchar *treeinfo_variant;
    const gchar *treeinfo_version;
    const gchar *treeinfo_arch;

    treeinfo_family = osinfo_tree_get_treeinfo_family(tree);
    treeinfo_variant = osinfo_tree_get_treeinfo_variant(tree);
    treeinfo_version = osinfo_tree_get_treeinfo_version(tree);
    treeinfo_arch = osinfo_tree_get_treeinfo_arch(tree);

    for (os_iter = oss; os_iter; os_iter = os_iter->next) {
        OsinfoOs *os = OSINFO_OS(os_iter->data);
        OsinfoTreeList *tree_list = osinfo_os_get_tree_list(os);
        GList *trees = osinfo_list_get_elements(OSINFO_LIST(tree_list));
        GList *tree_iter;
        gboolean found = FALSE;

        for (tree_iter = trees; tree_iter; tree_iter = tree_iter->next) {
            OsinfoTree *os_tree = OSINFO_TREE(tree_iter->data);
            const gchar *os_tree_arch = NULL;
            const gchar *os_treeinfo_family;
            const gchar *os_treeinfo_variant;
            const gchar *os_treeinfo_version;
            const gchar *os_treeinfo_arch;

            if (!osinfo_tree_has_treeinfo(os_tree))
                continue;

            os_tree_arch = osinfo_tree_get_architecture(os_tree);
            if (fallback_oss != NULL) {
                if (g_str_equal(os_tree_arch, "all")) {
                    *fallback_oss = g_list_prepend(*fallback_oss, os);
                    continue;
                }
            }

            os_treeinfo_family = osinfo_tree_get_treeinfo_family(os_tree);
            os_treeinfo_variant = osinfo_tree_get_treeinfo_variant(os_tree);
            os_treeinfo_version = osinfo_tree_get_treeinfo_version(os_tree);
            os_treeinfo_arch = osinfo_tree_get_treeinfo_arch(os_tree);

            if (match_regex(os_treeinfo_family, treeinfo_family) &&
                match_regex(os_treeinfo_variant, treeinfo_variant) &&
                match_regex(os_treeinfo_version, treeinfo_version) &&
                match_regex(os_treeinfo_arch, treeinfo_arch)) {
                *ret_os = os;
                if (matched != NULL) {
                    *matched = os_tree;
                    osinfo_tree_set_os(*matched, *ret_os);
                    found = TRUE;
                }
                break;
            }
        }

        g_list_free(trees);
        g_object_unref(tree_list);

        if (found)
            return TRUE;
    }

    return FALSE;
}

static OsinfoOs *
osinfo_db_guess_os_from_tree_internal(OsinfoDb *db,
                                      OsinfoTree *tree,
                                      OsinfoTree **matched_tree)
{
    OsinfoOs *ret = NULL;
    GList *oss = NULL;
    GList *fallback_oss = NULL;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(tree != NULL, NULL);

    oss = osinfo_list_get_elements(OSINFO_LIST(db->priv->oses));
    if (compare_tree(tree, oss, &ret, matched_tree, &fallback_oss))
        goto end;

    compare_tree(tree, fallback_oss, &ret, matched_tree, NULL);

 end:
    g_list_free(oss);
    g_list_free(fallback_oss);

    return ret;
}

/**
 * osinfo_db_guess_os_from_tree:
 * @db: the database
 * @tree: the installation tree
 * @matched_tree: (out) (transfer none) (allow-none): the matched operating
 * system tree
 *
 * Guess operating system given an #OsinfoTree object.
 *
 * Returns: (transfer none): the operating system, or NULL if guessing failed
 * Deprecated: 1.6.0: Use osinfo_db_identify_tree() instead.
 */
OsinfoOs *osinfo_db_guess_os_from_tree(OsinfoDb *db,
                                       OsinfoTree *tree,
                                       OsinfoTree **matched_tree)
{
    return osinfo_db_guess_os_from_tree_internal(db, tree, matched_tree);
}

static void fill_tree(OsinfoDb *db, OsinfoTree *tree,
                      OsinfoTree *matched_tree,
                      OsinfoOs *os)
{
    const gchar *id;
    const gchar *kernel_path;
    const gchar *initrd_path;
    const gchar *boot_iso_path;
    const gchar *arch;
    const gchar *url;
    gboolean has_treeinfo;
    GList *variants, *node;

    id = osinfo_entity_get_id(OSINFO_ENTITY(matched_tree));
    g_object_set(G_OBJECT(tree), "id", id, NULL);

    arch = osinfo_tree_get_architecture(matched_tree);
    if (arch != NULL)
        g_object_set(G_OBJECT(tree), "architecture", arch, NULL);
    url = osinfo_tree_get_url(matched_tree);
    if (url != NULL)
        g_object_set(G_OBJECT(tree), "url", url, NULL);
    variants = osinfo_entity_get_param_value_list(OSINFO_ENTITY(matched_tree),
                                                  "variant");
    for (node = variants; node != NULL; node = node->next)
        osinfo_entity_add_param(OSINFO_ENTITY(tree),
                                "variant",
                                (gchar *) node->data);
    g_list_free(variants);
    kernel_path = osinfo_tree_get_kernel_path(matched_tree);
    if (kernel_path != NULL)
        g_object_set(G_OBJECT(tree), "kernel", kernel_path, NULL);

    initrd_path = osinfo_tree_get_initrd_path(matched_tree);
    if (initrd_path != NULL)
        g_object_set(G_OBJECT(tree), "initrd", initrd_path, NULL);

    boot_iso_path = osinfo_tree_get_boot_iso_path(matched_tree);
    if (boot_iso_path != NULL)
        g_object_set(G_OBJECT(tree), "boot-iso", boot_iso_path, NULL);

    has_treeinfo = osinfo_tree_has_treeinfo(matched_tree);
    if (has_treeinfo) {
        const gchar *treeinfo_family;
        const gchar *treeinfo_variant;
        const gchar *treeinfo_version;
        const gchar *treeinfo_arch;

        treeinfo_family = osinfo_tree_get_treeinfo_family(matched_tree);
        if (treeinfo_family != NULL)
            g_object_set(G_OBJECT(tree), "treeinfo-family", treeinfo_family, NULL);

        treeinfo_variant = osinfo_tree_get_treeinfo_variant(matched_tree);
        if (treeinfo_variant != NULL)
            g_object_set(G_OBJECT(tree), "treeinfo-variant", treeinfo_variant, NULL);

        treeinfo_version = osinfo_tree_get_treeinfo_version(matched_tree);
        if (treeinfo_version != NULL)
            g_object_set(G_OBJECT(tree), "treeinfo-version", treeinfo_version, NULL);

        treeinfo_arch = osinfo_tree_get_treeinfo_arch(matched_tree);
        if (treeinfo_arch != NULL)
            g_object_set(G_OBJECT(tree), "treeinfo-arch", treeinfo_arch, NULL);

        g_object_set(G_OBJECT(tree), "has-treeinfo", TRUE, NULL);
    }

    if (os != NULL)
        osinfo_tree_set_os(tree, os);
}

/**
 * osinfo_db_identify_tree:
 * @db: an #OsinfoDb database
 * @tree: the installation tree
 * data
 *
 * Try to match a newly created @tree with a tree description from @db.
 * If found, @tree will be filled with the corresponding information
 * stored in @db. In particular, after a call to osinfo_db_identify_tree(), if
 * the tree could be identified, its OsinfoEntify::id and OsinfoMedia::os
 * properties will be set.
 *
 * Returns: TRUE if @tree was found in @db, FALSE otherwise
 *
 * Since: 1.6.0
 */
gboolean osinfo_db_identify_tree(OsinfoDb *db,
                                 OsinfoTree *tree)
{
    OsinfoTree *matched_tree;
    OsinfoOs *matched_os;

    g_return_val_if_fail(OSINFO_IS_TREE(tree), FALSE);
    g_return_val_if_fail(OSINFO_IS_DB(db), FALSE);

    matched_os = osinfo_db_guess_os_from_tree_internal(db, tree,
                                                       &matched_tree);
    if (matched_os == NULL) {
        return FALSE;
    }

    fill_tree(db, tree, matched_tree, matched_os);

    return TRUE;
}

struct osinfo_db_populate_values_args {
    GHashTable *values;
    const gchar *property;
};

static void osinfo_db_get_property_values_in_entity(gpointer data, gpointer opaque)
{
    OsinfoEntity *entity = OSINFO_ENTITY(data);
    struct osinfo_db_populate_values_args *args = opaque;
    GHashTable *newValues = args->values;
    const gchar *property = args->property;
    GList *values = osinfo_entity_get_param_value_list(entity, property);
    GList *tmp = values;

    while (tmp) {
        gchar *value = tmp->data;

        if (!g_hash_table_lookup(newValues, value)) {
            g_hash_table_insert(newValues,
                                value,
                                GINT_TO_POINTER(1));
        }

        tmp = tmp->next;
    }

    g_list_free(values);
}

static GList *osinfo_db_unique_values_for_property_in_entity(OsinfoList *entities, const gchar *propName)
{
    /* Delibrately no free func for key, since we return those to caller */
    GHashTable *values = g_hash_table_new(g_str_hash, g_str_equal);
    GList *ret;
    struct osinfo_db_populate_values_args args = { values, propName };
    GList *elements;

    elements = osinfo_list_get_elements(entities);
    g_list_foreach(elements, osinfo_db_get_property_values_in_entity, &args);
    g_list_free(elements);

    ret = g_hash_table_get_keys(values);
    g_hash_table_unref(values);
    return ret;
}

/**
 * osinfo_db_unique_values_for_property_in_os:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * operating systems in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_os(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->oses), propName);
}


/**
 * osinfo_db_unique_values_for_property_in_platform:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * platforms in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_platform(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->platforms), propName);
}


/**
 * osinfo_db_unique_values_for_property_in_device:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * devices in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_device(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->devices), propName);
}

/**
 * osinfo_db_unique_values_for_property_in_deployment:
 * @db: the database
 * @propName: a property name
 *
 * Get all unique values for a named property amongst all
 * deployments in the database
 *
 * Returns: (transfer container)(element-type utf8): a list of strings
 */
GList *osinfo_db_unique_values_for_property_in_deployment(OsinfoDb *db, const gchar *propName)
{
    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);
    g_return_val_if_fail(propName != NULL, NULL);

    return osinfo_db_unique_values_for_property_in_entity(OSINFO_LIST(db->priv->deployments), propName);
}

struct __osinfoProductCheckRelationshipArgs {
    OsinfoList *list;
    OsinfoProductRelationship relshp;
};


static void __osinfoAddProductIfRelationship(gpointer data, gpointer opaque)
{
    struct __osinfoProductCheckRelationshipArgs *args = opaque;
    OsinfoProduct *product = OSINFO_PRODUCT(data);
    OsinfoList *newList = args->list;
    OsinfoProductList *thisList = osinfo_product_get_related(product, args->relshp);
    int i;

    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(thisList)); i++) {
        OsinfoEntity *entity = osinfo_list_get_nth(OSINFO_LIST(thisList), i);
        osinfo_list_add(newList, entity);
    }

    g_object_unref(thisList);
}

/**
 * osinfo_db_unique_values_for_os_relationship:
 * @db: the database
 * @relshp: the product relationship
 *
 * Get all operating systems that are the referee
 * in an operating system relationship.
 *
 * Returns: (transfer full): a list of operating systems
 */
OsinfoOsList *osinfo_db_unique_values_for_os_relationship(OsinfoDb *db, OsinfoProductRelationship relshp)
{
    OsinfoOsList *newList;
    struct __osinfoProductCheckRelationshipArgs args;
    GList *entities;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    newList = osinfo_oslist_new();

    args.list = OSINFO_LIST(newList);
    args.relshp = relshp;

    entities = osinfo_list_get_elements(OSINFO_LIST(db->priv->oses));

    g_list_foreach(entities, __osinfoAddProductIfRelationship, &args);
    g_list_free(entities);

    return newList;
}


/**
 * osinfo_db_unique_values_for_platform_relationship:
 * @db: the database
 * @relshp: the product relationship
 *
 * Get all platforms that are the referee
 * in an platform relationship.
 *
 * Returns: (transfer full): a list of virtualization platforms
 */
OsinfoPlatformList *osinfo_db_unique_values_for_platform_relationship(OsinfoDb *db, OsinfoProductRelationship relshp)
{
    OsinfoPlatformList *newList;
    struct __osinfoProductCheckRelationshipArgs args;
    GList *entities;

    g_return_val_if_fail(OSINFO_IS_DB(db), NULL);

    newList = osinfo_platformlist_new();

    args.list = OSINFO_LIST(newList);
    args.relshp = relshp;

    entities = osinfo_list_get_elements(OSINFO_LIST(db->priv->platforms));

    g_list_foreach(entities, __osinfoAddProductIfRelationship, &args);
    g_list_free(entities);

    return newList;
}
