/*
 * Copyright (C) 2009-2012, 2014 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <osinfo/osinfo.h>

struct ISOInfo {
    gchar *shortid;
    gchar *filename;
    OsinfoMedia *media;
    GHashTable *langs;
};

static void free_iso(struct ISOInfo *info)
{
    if (!info)
        return;

    g_free(info->filename);
    g_free(info->shortid);
    if (info->media)
        g_object_unref(info->media);
    if (info->langs)
        g_hash_table_unref(info->langs);
    g_free(info);
}

/* void* wrapper for free_iso, so it can be used where a void* parameter
 * is required (e.g. g_list_free_full), with no need for casts.
 */
static void free_iso_void(void *info)
{
    free_iso((struct ISOInfo *)info);
}

static gboolean load_langs(GFile *file, struct ISOInfo *info, GError **error)
{
    char *path;
    GKeyFile *lang_file;
    GStrv langs;
    GStrv it;

    path = g_file_get_path(file);
    if (path == NULL)
        return FALSE;
    if (!g_str_has_suffix(path, ".txt")) {
        g_free(path);
        return FALSE;
    }
    strcpy(&path[strlen(path) - sizeof(".txt") + 1], ".lng");
    lang_file = g_key_file_new();
    if (!g_key_file_load_from_file(lang_file, path, G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(lang_file);
        g_free(path);
        return FALSE;
    }
    langs = g_key_file_get_string_list(lang_file, "general", "l10n-language",
                                       NULL, NULL);
    if (langs == NULL) {
        g_key_file_free(lang_file);
        g_free(path);
        return FALSE;
    }

    for (it = langs; (it != NULL) && (*it != NULL); it++) {
        g_hash_table_add(info->langs, g_strdup(*it));
    }

    g_strfreev(langs);
    g_key_file_free(lang_file);
    g_free(path);

    return TRUE;
}

static struct ISOInfo *load_iso(GFile *file, const gchar *shortid, const gchar *name, GError **error)
{
    struct ISOInfo *info = g_new0(struct ISOInfo, 1);
    GFileInputStream *fis = NULL;
    GDataInputStream *dis = NULL;
    gchar *line;
    const gchar *arch;
    gint64 vol_size = -1, blk_size = -1;

    if (!(fis = g_file_read(file, NULL, error)))
        goto error;

    if (!(dis = g_data_input_stream_new(G_INPUT_STREAM(fis))))
        goto error;

    info->filename = g_strdup(name);
    info->shortid = g_strdup(shortid);
    info->langs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    if (strstr(name, "amd64") ||
             strstr(name, "x64") ||
             strstr(name, "X64") ||
             strstr(name, "x86_64") ||
             strstr(name, "64bit") ||
             strstr(name, "64-bit"))
        arch = "x86_64";
    else if (strstr(name, "i386") ||
             strstr(name, "i586") ||
             strstr(name, "i686") ||
             strstr(name, "x86") ||
             strstr(name, "X86") ||
             strstr(name, "32bit") ||
             strstr(name, "32-bit"))
        arch = "i386";
    else if (strstr(name, "ppc") ||
             strstr(name, "powerpc"))
        arch = "ppc";
    else if (strstr(name, "hppa"))
        arch = "hppa";
    else if (strstr(name, "sparc"))
        arch = "sparc";
    else if (strstr(name, "ia64"))
        arch = "ia64";
    else if (strstr(name, "s390x"))
        arch = "s390x";
    else if (strstr(name, "aarch64") ||
             strstr(name, "arm64"))
        arch = "aarch64";
    else if (strstr(name, "armv7l") ||
             strstr(name, "armv7hl") ||
             strstr(name, "armhf"))
        arch = "armv7l";
    else {
        g_debug("Unknown arch in %s", name);
        arch = "i386";
    }

    info->media = osinfo_media_new(name, arch);

    while ((line = g_data_input_stream_read_line(dis, NULL, NULL, error)) != NULL) {
        const gchar *key = NULL;
        char *value = NULL;

        if (g_str_has_prefix(line, "Volume id: ")) {
            key = OSINFO_MEDIA_PROP_VOLUME_ID;
            value = line + strlen("Volume id: ");
        } else if (g_str_has_prefix(line, "Publisher id: ")) {
            key = OSINFO_MEDIA_PROP_PUBLISHER_ID;
            value = line + strlen("Publisher id: ");
        } else if (g_str_has_prefix(line, "System id: ")) {
            key = OSINFO_MEDIA_PROP_SYSTEM_ID;
            value = line + strlen("System id: ");
        } else if (g_str_has_prefix(line, "Application id: ")) {
            key = OSINFO_MEDIA_PROP_APPLICATION_ID;
            value = line + strlen("Application id: ");
        } else if (g_str_has_prefix(line, "Logical block size is: ")) {
            blk_size = (gint64) atoll(line + strlen("Logical block size is: "));
        } else if (g_str_has_prefix(line, "Volume size is: ")) {
            vol_size = atoll(line + strlen("Volume size is: "));
        }

        if (key != NULL && value != NULL && value[0] != '\0')
            osinfo_entity_set_param(OSINFO_ENTITY(info->media), key, value);

        g_free(line);
    }

    if (vol_size > 0 && blk_size > 0)
        osinfo_entity_set_param_int64(OSINFO_ENTITY(info->media),
                                      OSINFO_MEDIA_PROP_VOLUME_SIZE,
                                      vol_size * blk_size);

    if (*error)
        goto error;

    load_langs(file, info, error);

 cleanup:
    if (fis)
        g_object_unref(fis);

    if (dis)
        g_object_unref(dis);

    return info;

 error:
    free_iso(info);
    info = NULL;
    goto cleanup;
}


static GList *load_distro(GFile *dir, const gchar *shortid, GError **error) {
    GFileEnumerator *children = g_file_enumerate_children(dir,
                                                          "standard::*",
                                                          G_FILE_QUERY_INFO_NONE,
                                                          NULL,
                                                          error);
    GFileInfo *childinfo;
    GList *ret = NULL;

    if (!children)
        return NULL;

    while ((childinfo = g_file_enumerator_next_file(children, NULL, error)) != NULL) {
        if (g_file_info_get_file_type(childinfo) != G_FILE_TYPE_REGULAR) {
            g_object_unref(childinfo);
            continue;
        }

        if (!g_str_has_suffix(g_file_info_get_name(childinfo), ".txt")) {
            g_object_unref(childinfo);
            continue;
        }

        GFile *child = g_file_get_child(dir, g_file_info_get_name(childinfo));
        struct ISOInfo *iso = load_iso(child,
                                       shortid,
                                       g_file_info_get_name(childinfo),
                                       error);
        g_object_unref(child);
        g_object_unref(childinfo);

        if (!iso)
            goto error;

        ret = g_list_append(ret, iso);
    }

    if (*error)
        goto error;

 cleanup:
    g_object_unref(children);

    return ret;

 error:
    g_list_free_full(ret, free_iso_void);
    ret = NULL;
    goto cleanup;
}


static GList *load_distros(GFile *dir, GError **error)
{
    GFileEnumerator *children = g_file_enumerate_children(dir,
                                                          "standard::*",
                                                          G_FILE_QUERY_INFO_NONE,
                                                          NULL,
                                                          error);
    GFileInfo *childinfo;
    GList *ret = NULL;

    if (!children)
        return NULL;

    while ((childinfo = g_file_enumerator_next_file(children, NULL, error)) != NULL) {
        if (g_file_info_get_file_type(childinfo) != G_FILE_TYPE_DIRECTORY) {
            g_object_unref(childinfo);
            continue;
        }

        GFile *child = g_file_get_child(dir, g_file_info_get_name(childinfo));
        GList *isos = load_distro(child, g_file_info_get_name(childinfo), error);

        g_object_unref(child);
        g_object_unref(childinfo);

        if (!isos && *error)
            goto error;
        ret = g_list_concat(ret, isos);
    }

    if (*error)
        goto error;

 cleanup:
    g_object_unref(children);

    return ret;

 error:
    g_list_free_full(ret, free_iso_void);
    ret = NULL;
    goto cleanup;
}


static GList *load_isos(const gchar *vendor, GError **error)
{
    gchar *path = g_strdup_printf(SRCDIR "/tests/isodata/%s", vendor);
    GList *ret = NULL;
    GFile *f = g_file_new_for_path(path);

    ret = load_distros(f, error);

    g_object_unref(f);
    g_free(path);
    return ret;
}

static GList *load_vendors(GError **error)
{
    GFile *dir = g_file_new_for_path(SRCDIR "/tests/isodata");
    GFileEnumerator *children = g_file_enumerate_children(dir,
                                                          "standard::*",
                                                          G_FILE_QUERY_INFO_NONE,
                                                          NULL,
                                                          error);
    GFileInfo *childinfo;
    GList *ret = NULL;

    if (!children)
        return NULL;

    while ((childinfo = g_file_enumerator_next_file(children, NULL, error)) != NULL) {
        if (g_file_info_get_file_type(childinfo) != G_FILE_TYPE_DIRECTORY) {
            g_object_unref(childinfo);
            continue;
        }

        ret = g_list_prepend(ret, g_strdup(g_file_info_get_name(childinfo)));

        g_object_unref(childinfo);
    }

    if (error && *error)
        goto error;

 cleanup:
    g_object_unref(children);
    g_object_unref(dir);

    return ret;

 error:
    g_list_free_full(ret, g_free);
    ret = NULL;
    goto cleanup;
}


static void test_langs(struct ISOInfo *info)
{
    GList *langs;
    GList *it;

    /* exit early if there was no associated .lng file */
    if (g_hash_table_size(info->langs) == 0)
        return;

    langs = osinfo_media_get_languages(info->media);

    for (it = langs; it != NULL; it = it->next) {
        g_test_message("checking ISO %s, language %s",
                       info->filename, (const char *)it->data);
        g_assert_true(g_hash_table_contains(info->langs, it->data));
        g_hash_table_remove(info->langs, it->data);
    }
    g_list_free(langs);
    g_assert_cmpint(g_hash_table_size(info->langs), ==, 0);
}

static void test_one(const gchar *vendor)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    GList *isos = NULL;
    GList *tmp;
    GError *error = NULL;

    g_assert_true(OSINFO_IS_LOADER(loader));
    g_assert_true(OSINFO_IS_DB(db));

    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);

    isos = load_isos(vendor, &error);

    g_assert_nonnull(isos);

    for (tmp = isos; tmp; tmp = tmp->next) {
        struct ISOInfo *info  = tmp->data;
        gboolean matched = osinfo_db_identify_media(db, info->media);
        OsinfoOs *os;

        g_test_message("checking OS %s for ISO %s",
                       info->shortid, info->filename);
        if (!matched) {
            g_printerr("ISO %s was not matched by OS %s\n/isodetect/%s: ",
                       info->filename, info->shortid, vendor);
            g_test_fail();
            continue;
        }

        g_object_get(info->media, "os", &os, NULL);
        const gchar *shortid = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
        g_assert_cmpstr(shortid, ==, info->shortid);
        g_object_unref(G_OBJECT(os));
        test_langs(info);
    }

    g_list_free_full(isos, free_iso_void);

    g_object_unref(loader);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();

    GList *vendors = load_vendors(NULL);
    GList *it;
    for (it = vendors; it != NULL; it = it->next) {
        char *vendor = (char *)it->data;
        char *test_path = g_strdup_printf("/isodetect/%s", vendor);

        g_test_add_data_func_full(test_path, vendor, (GTestDataFunc)test_one, g_free);

        g_free(test_path);

    }
    g_list_free(vendors);

    /* Upfront so we don't confuse valgrind */
    osinfo_entity_get_type();
    osinfo_db_get_type();
    osinfo_device_get_type();
    osinfo_platform_get_type();
    osinfo_os_get_type();
    osinfo_list_get_type();
    osinfo_devicelist_get_type();
    osinfo_platformlist_get_type();
    osinfo_oslist_get_type();
    osinfo_filter_get_type();

    return g_test_run();
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
