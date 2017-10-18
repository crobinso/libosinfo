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
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>

static GError *error = NULL;
static gchar *actualData = NULL;
static const gchar *expectData =                                        \
    "\n"                                                                \
    "# OS id=http://fedoraproject.org/fedora/16\n"                      \
    "# Media id=http://fedoraproject.org/fedora/16:0\n"                 \
    "# Profile: jeos\n"                                                 \
    "install\n"                                                         \
    "text\n"                                                            \
    "keyboard uk\n"                                                     \
    "lang en_GB.UTF-8\n"                                                \
    "skipx\n"                                                           \
    "network --device eth0 --bootproto dhcp\n"                          \
    "rootpw 123456\n"                                                   \
    "timezone --utc Europe/London\n"                                    \
    "bootloader --location=mbr\n"                                       \
    "zerombr\n"                                                         \
    "\n"                                                                \
    "part biosboot --fstype=biosboot --size=1\n"                        \
    "part pv.2 --size=1 --grow --ondisk=vda\n"                          \
    "volgroup VolGroup00 --pesize=32768 pv.2\n"                         \
    "logvol / --fstype ext4 --name=LogVol00 --vgname=VolGroup00 --size=1024 --grow\n" \
    "reboot\n"                                                          \
    "\n"                                                                \
    "%packages\n"                                                       \
    "@base\n"                                                           \
    "@core\n"                                                           \
    "@hardware-support\n"                                               \
    "\n"                                                                \
    "%end\n"                                                            \
    "  ";

static const gchar *expectData2 =                                       \
    "\n"                                                                \
    "# OS id=http://fedoraproject.org/fedora/16\n"                      \
    "# Unknown media\n"                                                 \
    "# Profile: jeos\n"                                                 \
    "keyboard FOOBAR\n"                                                 \
    "lang French\n"                                                     \
    "timezone --utc Europe/Paris";

static void test_generate_for_media_finish(GObject *src,
                                           GAsyncResult *res,
                                           gpointer user_data)
{
    GMainLoop *loop = user_data;

    actualData = osinfo_install_script_generate_for_media_finish(OSINFO_INSTALL_SCRIPT(src),
                                                                 res,
                                                                 &error);

    g_main_loop_quit(loop);
}

static void test_generate_finish(GObject *src,
                                 GAsyncResult *res,
                                 gpointer user_data)
{
    GMainLoop *loop = user_data;

    actualData = osinfo_install_script_generate_finish(OSINFO_INSTALL_SCRIPT(src),
                                                       res,
                                                       &error);

    g_main_loop_quit(loop);
}

static OsinfoInstallConfig *test_get_config(void)
{
    OsinfoInstallConfig *config;

    config = osinfo_install_config_new("http://example.com");

    osinfo_install_config_set_l10n_keyboard(config, "uk");
    osinfo_install_config_set_l10n_language(config, "en_GB.UTF-8");
    osinfo_install_config_set_l10n_timezone(config, "Europe/London");

    osinfo_install_config_set_admin_password(config, "123456");

    osinfo_install_config_set_user_password(config, "123456");
    osinfo_install_config_set_user_login(config, "fred");
    osinfo_install_config_set_user_realname(config, "Fred Blogs");
    osinfo_install_config_set_user_autologin(config, TRUE);
    osinfo_install_config_set_user_administrator(config, TRUE);

    return config;
}

static OsinfoMedia *create_media(void)
{
    OsinfoMedia *media = osinfo_media_new("http://fedoraproject.org/fedora/16:0",
                                          "i686");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_VOLUME_ID,
                            "Fedora 16 i386 DVD");
    osinfo_entity_set_param(OSINFO_ENTITY(media),
                            OSINFO_MEDIA_PROP_SYSTEM_ID,
                            "LINUX");
    osinfo_entity_set_param_int64(OSINFO_ENTITY(media),
                                  OSINFO_MEDIA_PROP_VOLUME_SIZE,
                                  3806375936);
    return media;
}

static void
test_script_file(void)
{
    OsinfoInstallScript *script;
    OsinfoInstallConfig *config = test_get_config();
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoMedia *media;
    GMainLoop *loop;

    script = osinfo_install_script_new_uri("http://example.com",
                                           "jeos",
                                           "file://" SRCDIR "/tests/install-script.xsl");

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    loop = g_main_loop_new(g_main_context_get_thread_default(),
                           TRUE);

    media = create_media();
    g_assert_true(osinfo_db_identify_media(db, media));

    osinfo_install_script_generate_for_media_async(script,
                                                   media,
                                                   config,
                                                   NULL,
                                                   test_generate_for_media_finish,
                                                   loop);

    if (g_main_loop_is_running(loop))
        g_main_loop_run(loop);

    unlink(BUILDDIR "/tests/install-script-actual.txt");
    g_assert_no_error(error);

    g_assert_cmpstr(actualData, ==, expectData);

    g_free(actualData);
    g_object_unref(media);
    g_object_unref(db);
    g_object_unref(config);
    g_object_unref(script);
    g_main_loop_unref(loop);
}



static void
test_script_data(void)
{
    OsinfoInstallScript *script;
    OsinfoInstallConfig *config = test_get_config();
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoMedia *media;
    GMainLoop *loop;
    GFile *file = g_file_new_for_uri("file://" SRCDIR "/tests/install-script.xsl");
    gchar *data;

    g_file_load_contents(file, NULL, &data, NULL, NULL, &error);
    g_assert_no_error(error);
    g_object_unref(file);

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    media = create_media();
    g_assert_true(osinfo_db_identify_media(db, media));

    script = osinfo_install_script_new_data("http://example.com",
                                            "jeos",
                                            data);

    loop = g_main_loop_new(g_main_context_get_thread_default(),
                           TRUE);

    osinfo_install_script_generate_for_media_async(script,
                                                   media,
                                                   config,
                                                   NULL,
                                                   test_generate_for_media_finish,
                                                   loop);

    if (g_main_loop_is_running(loop))
        g_main_loop_run(loop);

    unlink(BUILDDIR "/tests/install-script-actual.txt");
    g_assert_no_error(error);

    g_free(data);
    g_free(actualData);
    g_object_unref(media);
    g_object_unref(db);
    g_object_unref(config);
    g_object_unref(script);
    g_main_loop_unref(loop);
}

static void
test_script_datamap(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db;
    OsinfoOs *os;
    OsinfoInstallScript *script;
    OsinfoInstallConfig *config;
    GMainLoop *loop;

    osinfo_loader_process_path(loader, SRCDIR "/tests/dbdata", &error);
    g_assert_no_error(error);
    db = g_object_ref(osinfo_loader_get_db(loader));
    g_object_unref(loader);

    g_assert_nonnull(osinfo_db_get_datamap(db, "http://example.com/libosinfo/test-datamap"));
    g_assert_nonnull(osinfo_db_get_datamap(db, "http://example.com/libosinfo/test-datamap2"));
    script = osinfo_db_get_install_script(db, "http://example.com/libosinfo/test-install-script");
    g_assert_nonnull(script);

    config = osinfo_install_config_new("http://example.com");

    osinfo_install_config_set_l10n_keyboard(config, "unknown");
    g_assert_cmpstr(osinfo_install_config_get_l10n_keyboard(config), ==, "unknown");

    osinfo_install_config_set_l10n_keyboard(config, "val1");
    g_assert_cmpstr(osinfo_install_config_get_l10n_keyboard(config), ==, "val1");

    osinfo_install_config_set_l10n_keyboard(config, "val2");
    g_assert_cmpstr(osinfo_install_config_get_l10n_keyboard(config), ==, "val2");

    osinfo_install_config_set_l10n_keyboard(config, "val3");
    g_assert_cmpstr(osinfo_install_config_get_l10n_keyboard(config), ==, "val3");

    osinfo_install_config_set_l10n_keyboard(config, "VAL1");
    g_assert_cmpstr(osinfo_install_config_get_l10n_keyboard(config), ==, "VAL1");

    osinfo_install_config_set_l10n_language(config, "en_EN");
    g_assert_cmpstr(osinfo_install_config_get_l10n_language(config), ==, "en_EN");
    osinfo_install_config_set_l10n_language(config, "fr_FR");
    osinfo_install_config_set_l10n_timezone(config, "Europe/Paris");

    os = osinfo_os_new("http://fedoraproject.org/fedora/16");
    osinfo_entity_set_param(OSINFO_ENTITY(os),
                            OSINFO_PRODUCT_PROP_SHORT_ID,
                            "fedora16");

    loop = g_main_loop_new(g_main_context_get_thread_default(),
                           TRUE);

    osinfo_install_script_generate_async(script,
                                         os,
                                         config,
                                         NULL,
                                         test_generate_finish,
                                         loop);

    if (g_main_loop_is_running(loop))
        g_main_loop_run(loop);

    unlink(BUILDDIR "/tests/install-script-actual.txt");
    g_assert_no_error(error);

    g_assert_cmpstr(actualData, ==, expectData2);

    g_free(actualData);
    g_object_unref(db);
    g_object_unref(os);
    g_object_unref(config);
    g_main_loop_unref(loop);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/install-script/script_file", test_script_file);
    g_test_add_func("/install-script/script_data", test_script_data);
    g_test_add_func("/install-script/script_datamap", test_script_datamap);

    /* Make sure we catch unexpected g_warning() */
    g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);

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
