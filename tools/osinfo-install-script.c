/*
 * Copyright (C) 2011, 2014 Red Hat, Inc.
 *
 * osinfo-install-script: generate an install script
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
 */

#include <osinfo/osinfo.h>
#include <string.h>
#include <locale.h>
#include <glib/gi18n.h>

static const gchar *profile = "jeos";
static const gchar *output_dir;
static const gchar *prefix;
static const gchar *source = "media";

static gboolean list_config = FALSE;
static gboolean list_profile = FALSE;
static gboolean list_inj_method = FALSE;
static gboolean quiet = FALSE;

static const gchar *configs[] = {
    OSINFO_INSTALL_CONFIG_PROP_HARDWARE_ARCH,
    OSINFO_INSTALL_CONFIG_PROP_L10N_TIMEZONE,
    OSINFO_INSTALL_CONFIG_PROP_L10N_LANGUAGE,
    OSINFO_INSTALL_CONFIG_PROP_L10N_KEYBOARD,
    OSINFO_INSTALL_CONFIG_PROP_ADMIN_PASSWORD,
    OSINFO_INSTALL_CONFIG_PROP_USER_PASSWORD,
    OSINFO_INSTALL_CONFIG_PROP_USER_LOGIN,
    OSINFO_INSTALL_CONFIG_PROP_USER_REALNAME,
    OSINFO_INSTALL_CONFIG_PROP_USER_AUTOLOGIN,
    OSINFO_INSTALL_CONFIG_PROP_USER_ADMIN,
    OSINFO_INSTALL_CONFIG_PROP_REG_LOGIN,
    OSINFO_INSTALL_CONFIG_PROP_REG_PASSWORD,
    OSINFO_INSTALL_CONFIG_PROP_REG_PRODUCTKEY,
    OSINFO_INSTALL_CONFIG_PROP_HOSTNAME,
    OSINFO_INSTALL_CONFIG_PROP_TARGET_DISK,
    OSINFO_INSTALL_CONFIG_PROP_SCRIPT_DISK,
    OSINFO_INSTALL_CONFIG_PROP_AVATAR_LOCATION,
    OSINFO_INSTALL_CONFIG_PROP_AVATAR_DISK,
    OSINFO_INSTALL_CONFIG_PROP_PRE_INSTALL_DRIVERS_DISK,
    OSINFO_INSTALL_CONFIG_PROP_PRE_INSTALL_DRIVERS_LOCATION,
    OSINFO_INSTALL_CONFIG_PROP_POST_INSTALL_DRIVERS_DISK,
    OSINFO_INSTALL_CONFIG_PROP_POST_INSTALL_DRIVERS_LOCATION,
    OSINFO_INSTALL_CONFIG_PROP_DRIVER_SIGNING,
    OSINFO_INSTALL_CONFIG_PROP_INSTALLATION_URL,
    NULL
};

static OsinfoInstallConfig *config;

static gboolean handle_config(const gchar *option_name G_GNUC_UNUSED,
                          const gchar *value,
                          gpointer data G_GNUC_UNUSED,
                          GError **error)
{
    const gchar *val;
    gchar *key;
    gsize len;

    if (!(val = strchr(value, '='))) {
        g_set_error(error, OSINFO_ERROR, 0,
                    _("Expected configuration key=value"));
        return FALSE;
    }
    len = val - value;
    val++;
    key = g_strndup(value, len);

    if (g_str_equal(key, OSINFO_INSTALL_CONFIG_PROP_USER_PASSWORD) ||
        g_str_equal(key, OSINFO_INSTALL_CONFIG_PROP_ADMIN_PASSWORD)) {
        g_warning("When setting user or admin password, use --config-file "
                  "instead.\n");
    }

    osinfo_entity_set_param(OSINFO_ENTITY(config),
                            key,
                            val);
    g_free(key);
    return TRUE;
}


static gboolean handle_config_file(const gchar *option_name G_GNUC_UNUSED,
                                   const gchar *value,
                                   gpointer data G_GNUC_UNUSED,
                                   GError **error)
{
    GKeyFile *key_file = NULL;
    gchar *val = NULL;
    gsize i;
    gboolean ret = FALSE;

    key_file = g_key_file_new();
    if (!g_key_file_load_from_file(key_file, value, G_KEY_FILE_NONE, error))
        goto error;

    for (i = 0; configs[i] != NULL; i++) {
        val = g_key_file_get_string(key_file, "install-script", configs[i], error);
        if (val == NULL) {
            if (g_error_matches(*error, G_KEY_FILE_ERROR,
                                G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
                g_clear_error(error);
                continue;
            }

            goto error;
        }

        osinfo_entity_set_param(OSINFO_ENTITY(config),
                                configs[i],
                                val);
        g_free(val);
    }

    ret = TRUE;

error:
    g_key_file_unref(key_file);

    return ret;
}


static GOptionEntry entries[] =
{
    { "profile", 'p', 0, G_OPTION_ARG_STRING, (void*)&profile,
      N_("Install script profile"), NULL, },
    { "output-dir", 'd', 0, G_OPTION_ARG_STRING, (void*)&output_dir,
      N_("Install script output directory"), NULL, },
    { "prefix", 'P', 0, G_OPTION_ARG_STRING, (void*)&prefix,
      N_("The output filename prefix"), NULL, },
    { "installation-source", 's', 0, G_OPTION_ARG_STRING, (void*)&source,
      N_("The installation source to be used with the script"), NULL, },
    { "config", 'c', 0, G_OPTION_ARG_CALLBACK,
      handle_config,
      N_("Set configuration parameter"), "key=value" },
    { "config-file", 'f', 0, G_OPTION_ARG_CALLBACK,
      handle_config_file,
      N_("Set configuration parameters"), "file:///path/to/config/file" },
    { "list-config", '\0', 0, G_OPTION_ARG_NONE, (void*)&list_config,
      N_("List configuration parameters"), NULL },
    { "list-profiles", '\0', 0, G_OPTION_ARG_NONE, (void*)&list_profile,
      N_("List install script profiles"), NULL },
    { "list-injection-methods", '\0', 0, G_OPTION_ARG_NONE,
      (void*)&list_inj_method, N_("List supported injection methods"), NULL },
    { "quiet", 'q', 0, G_OPTION_ARG_NONE, (void*)&quiet,
      N_("Do not display output filenames"), NULL },
    { 0 }
};


static OsinfoOs *find_os(OsinfoDb *db,
                         const char *idoruri)
{
    OsinfoOsList *oslist;
    OsinfoOsList *filteredList;
    OsinfoFilter *filter;
    OsinfoOs *os;

    os = osinfo_db_get_os(db, idoruri);

    if (os)
        return g_object_ref(os);

    oslist = osinfo_db_get_os_list(db);
    filter = osinfo_filter_new();
    osinfo_filter_add_constraint(filter,
                                 OSINFO_PRODUCT_PROP_SHORT_ID,
                                 idoruri);

    filteredList = OSINFO_OSLIST(osinfo_list_new_filtered(OSINFO_LIST(oslist),
                                                          filter));

    if (osinfo_list_get_length(OSINFO_LIST(filteredList)) > 0) {
        os = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(filteredList), 0));
        g_object_ref(os);
    }

    g_object_unref(oslist);
    g_object_unref(filteredList);
    g_object_unref(filter);

    return os;
}


static gboolean list_script_config(OsinfoOs *os)
{
    OsinfoInstallScriptList *scripts = osinfo_os_get_install_script_list(os);
    OsinfoInstallScriptList *profile_scripts;
    OsinfoFilter *filter;
    GList *l, *tmp;
    gboolean ret = FALSE;

    filter = osinfo_filter_new();
    osinfo_filter_add_constraint(filter,
                                 OSINFO_INSTALL_SCRIPT_PROP_PROFILE,
                                 profile ? profile :
                                 OSINFO_INSTALL_SCRIPT_PROFILE_JEOS);
    profile_scripts = OSINFO_INSTALL_SCRIPTLIST(osinfo_list_new_filtered(OSINFO_LIST(scripts),
                                                                         filter));
    l = osinfo_list_get_elements(OSINFO_LIST(profile_scripts));
    if (!l) {
        g_printerr(_("No install script for profile '%s' and OS '%s'"),
                   profile, osinfo_product_get_name(OSINFO_PRODUCT(os)));
        goto cleanup;
    }

    for (tmp = l; tmp != NULL; tmp = tmp->next) {
        OsinfoInstallScript *script = tmp->data;
        GList *params = osinfo_install_script_get_config_param_list(script);
        GList *tmp2;

        for (tmp2 = params; tmp2 != NULL; tmp2 = tmp2->next) {
            OsinfoInstallConfigParam *param = OSINFO_INSTALL_CONFIG_PARAM(tmp2->data);

            g_print("%s: %s\n",
                    osinfo_install_config_param_get_name(param),
                    osinfo_install_config_param_is_required(param) ?
                    _("required") : _("optional"));
        }
    }
    ret = TRUE;

 cleanup:
    g_list_free(l);
    g_object_unref(scripts);
    g_object_unref(filter);
    g_object_unref(profile_scripts);
    return ret;
}


static gboolean list_script_profile(OsinfoOs *os)
{
    OsinfoInstallScriptList *scripts = osinfo_os_get_install_script_list(os);
    GList *l, *tmp;
    gboolean ret = FALSE;

    l = osinfo_list_get_elements(OSINFO_LIST(scripts));

    for (tmp = l; tmp != NULL; tmp = tmp->next) {
        OsinfoInstallScript *script = tmp->data;

        g_print("%s: %s\n",
                osinfo_install_script_get_profile(script),
                osinfo_install_script_get_expected_filename(script));
    }
    ret = TRUE;

    g_list_free(l);
    g_object_unref(scripts);
    return ret;
}

static gboolean list_script_inj_method(OsinfoOs *os)
{
    OsinfoInstallScriptList *scripts = osinfo_os_get_install_script_list(os);
    GList *l, *tmp;
    gboolean ret = FALSE;
    GFlagsClass *f;

    f = g_type_class_ref(OSINFO_TYPE_INSTALL_SCRIPT_INJECTION_METHOD);
    l = osinfo_list_get_elements(OSINFO_LIST(scripts));

    for (tmp = l; tmp != NULL; tmp = tmp->next) {
        OsinfoInstallScript *script = tmp->data;
        unsigned int methods, i;
        methods = osinfo_install_script_get_injection_methods(script);

        g_print("%s:", osinfo_install_script_get_profile(script));
        for (i = 0; i < f->n_values; i++)
            if (f->values[i].value & methods)
                g_print(" %s", f->values[i].value_nick);
        g_print("\n");
    }
    ret = TRUE;

    g_list_free(l);
    g_type_class_unref(f);
    g_object_unref(scripts);
    return ret;
}


static gboolean generate_script(OsinfoOs *os, OsinfoMedia *media)
{
    OsinfoInstallScriptList *scripts = osinfo_os_get_install_script_list(os);
    OsinfoInstallScriptList *profile_scripts;
    OsinfoFilter *filter;
    GFile *dir = NULL;
    GList *l, *tmp;
    gboolean ret = FALSE;
    GError *error = NULL;

    filter = osinfo_filter_new();
    osinfo_filter_add_constraint(filter,
                                 OSINFO_INSTALL_SCRIPT_PROP_PROFILE,
                                 profile ? profile :
                                 OSINFO_INSTALL_SCRIPT_PROFILE_JEOS);
    profile_scripts = OSINFO_INSTALL_SCRIPTLIST(osinfo_list_new_filtered(OSINFO_LIST(scripts),
                                                                         filter));
    l = osinfo_list_get_elements(OSINFO_LIST(profile_scripts));

    if (!l) {
        g_printerr(_("No install script for profile '%s' and OS '%s'\n"),
                   profile, osinfo_product_get_name(OSINFO_PRODUCT(os)));
        goto cleanup;
    }

    dir = g_file_new_for_commandline_arg(output_dir ? output_dir : ".");

    for (tmp = l; tmp != NULL; tmp = tmp->next) {
        OsinfoInstallScript *script = tmp->data;
        OsinfoInstallScriptInstallationSource installation_source;

        installation_source = g_str_equal(source, "network") ?
                OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_NETWORK :
                OSINFO_INSTALL_SCRIPT_INSTALLATION_SOURCE_MEDIA;
        osinfo_install_script_set_installation_source(script,
                                                      installation_source);

        if (prefix)
            osinfo_install_script_set_output_prefix(script, prefix);

        if (media != NULL) {
            osinfo_install_script_generate_output_for_media(script,
                                                            media,
                                                            config,
                                                            dir,
                                                            NULL,
                                                            &error);
        } else {
            osinfo_install_script_generate_output(script,
                                                  os,
                                                  config,
                                                  dir,
                                                  NULL,
                                                  &error);
        }
        if (error != NULL) {
            g_printerr(_("Unable to generate install script: %s\n"),
                       error->message ? error->message : "unknown");
            goto cleanup;
        }
        if (!quiet)
           g_print("%s\n", osinfo_install_script_get_output_filename(script));
    }
    ret = TRUE;

 cleanup:
    g_list_free(l);
    g_object_unref(scripts);
    g_object_unref(filter);
    g_object_unref(profile_scripts);
    if (dir)
        g_object_unref(dir);
    return ret;
}


gint main(gint argc, gchar **argv)
{
    GOptionContext *context;
    GError *error = NULL;
    OsinfoLoader *loader = NULL;
    OsinfoDb *db = NULL;
    OsinfoOs *os = NULL;
    OsinfoMedia *media = NULL;
    gint ret = 0;

    setlocale(LC_ALL, "");
    textdomain(GETTEXT_PACKAGE);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    config = osinfo_install_config_new("https://libosinfo.org/config");

    context = g_option_context_new(_("- Generate an OS install script"));
    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("Error while parsing options: %s\n"), error->message);
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));

        ret = -1;
        goto EXIT;
    }

    if (argc < 2) {
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));

        ret = -2;
        goto EXIT;
    }

    if ((list_profile     ? 1 : 0) +
        (list_config      ? 1 : 0) +
        (list_inj_method  ? 1 : 0) > 1) {
        g_printerr("%s",
                   _("Only one of --list-profile, --list-config and "
                     "--list-injection-methods can be requested"));
        ret = -2;
        goto EXIT;
    }


    loader = osinfo_loader_new();
    osinfo_loader_process_default_path(loader, &error);
    if (error != NULL) {
        g_printerr(_("Error loading OS data: %s\n"), error->message);

        ret = -3;
        goto EXIT;
    }

    db = osinfo_loader_get_db(loader);

    /* First assume it is a path to a media that can be identified */
    media = osinfo_media_create_from_location(argv[1], NULL, NULL);
    if (media != NULL)
        if (!osinfo_db_identify_media(db, media)) {
            g_object_unref(media);
            media = NULL;
        }

    if (media == NULL) {
        os = find_os(db, argv[1]);
        if (!os) {
            g_printerr(_("Error finding OS: %s\n"), argv[1]);
            ret = -4;
            goto EXIT;
        }
    } else {
        os = osinfo_media_get_os(media);
    }

    if (list_config) {
        if (!list_script_config(os)) {
            ret = -5;
            goto EXIT;
        }
    } else if (list_profile) {
        if (!list_script_profile(os)) {
            ret = -5;
            goto EXIT;
        }
    } else if (list_inj_method) {
        if (!list_script_inj_method(os)) {
            ret = -5;
            goto EXIT;
        }
    } else {
        if (!generate_script(os, media)) {
            ret = -5;
            goto EXIT;
        }
    }

EXIT:
    if (config)
        g_object_unref(config);
    if (media != NULL)
        g_object_unref(media);
    g_clear_error(&error);
    g_clear_object(&os);
    g_clear_object(&loader);
    g_option_context_free(context);

    return ret;
}

/*
=pod

=head1 NAME

osinfo-install-script - generate a script for automated installation

=head1 SYNOPSIS

osinfo-install-script [OPTIONS...] MEDIA-FILE|OS-ID

=head1 DESCRIPTION

Generate a script suitable for performing an automated installation
of C<MEDIA_FILE> or C<OS-ID>. C<MEDIA_FILE> should be a path to an
installer or live media (typically an ISO file). C<OS-ID> should be
a URI identifying the operating system, or its short ID.

By default a script will be generated for a C<JEOS> style install.

=head1 OPTIONS

=over 8

=item B<--profile=NAME>

Choose the installation script profile. Defaults to C<jeos>, but
can also be C<desktop>, or a site specific profile name

=item B<--installation-source=NAME>

Set the installation source to be used with the installation
script. Defaults to C<media>, but can also be C<network>.

=item B<--config=key=value>

Set the configuration parameter C<key> to C<value>.

=item B<--config-file=config-file>

Set the configurations parameters according to the config-file passed.

Note that use of B<--config-file> is strongly recommended if the user
or admin passwords need to be set. Providing passwords directly using
B<--config=> is insecure as the password is visible to all processes
and users on the same host.

=back

=head1 CONFIGURATION KEYS

The following configuration keys are available

=over 8

=item C<hardware-arch>

The hardware architecture

=item C<l10n-timezone>

The local timezone

=item C<l10n-keyboard>

The local keyboard layout

=item C<l10n-language>

The local language

=item C<admin-password>

The administrator password
This option has been deprecated, use B<--config-file>
for setting the admin password.

=item C<user-password>

The user password
This option has been deprecated, use B<--config-file>
for setting the user password.

=item C<user-login>

The user login name

=item C<user-realname>

The user real name

=item C<user-autologin>

Whether to automatically login the user

=item C<user-admin>

Whether to give the user administrative privileges

=item C<reg-product-key>

The software registration key

=item C<reg-login>

The software registration user login

=item C<reg-password>

The software registration user password

=back

=head1 CONFIGURATION FILE FORMAT

The configuration file must consist in a file which contains a
C<install-script> group and, under this group, C<key>=C<value>
pairs, as shown below:

  [install-script]
  l10n-timezone=GMT
  l10n-keyboard=uk
  l10n-language=en_GB
  admin-password=123456
  user-login=berrange
  user-password=123456
  user-realname="Daniel P Berrange"

=head1 EXAMPLE USAGE

The following usages generate a Fedora 30 kickstart script:

  # osinfo-install-script \
         --profile jeos \
         --config-file /path/to/config/file \
         fedora30

  # osinfo-install-script \
         --profile jeos \
         --config l10n-timezone=GMT \
         --config l10n-keyboard=uk \
         --config l10n-language=en_GB \
         --config admin-password=123456 \
         --config user-login=berrange \
         --config user-password=123456 \
         --config user-realname="Daniel P Berrange" \
         fedora30

=head1 EXIT STATUS

The exit status will be 0 if an install script is generated,
or 1 on error

=head1 AUTHORS

Daniel P. Berrange <berrange@redhat.com>

=head1 COPYRIGHT

Copyright (C) 2012 Red Hat, Inc.

=head1 LICENSE

C<osinfo-install-script> is distributed under the termsof the GNU LGPL v2
license. This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE

=cut
*/
