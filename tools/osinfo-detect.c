/*
 * Copyright (C) 2011, 2014 Red Hat, Inc.
 *
 * osinfo-detect: Given a path to a ISO9660 image/device, detects if media is
 *                bootable and the relevant OS if media is an installer for it.
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
 *   Zeeshan Ali <zeenix@redhat.com>
 */

#include <osinfo/osinfo.h>
#include <string.h>
#include <locale.h>
#include <glib/gi18n.h>

#define FORMAT_STR_PLAIN "plain"

typedef enum {
    OUTPUT_FORMAT_PLAIN,
} OutputFormat;

static OutputFormat format = OUTPUT_FORMAT_PLAIN;

#define TYPE_STR_MEDIA "media"
#define TYPE_STR_TREE "tree"

typedef enum {
    URL_TYPE_MEDIA,
    URL_TYPE_TREE
} OutputType;

static OutputType type = URL_TYPE_MEDIA;

static gboolean parse_format_str(const gchar *option_name,
                                 const gchar *value,
                                 gpointer data,
                                 GError **error)
{
    if (g_str_equal(value, FORMAT_STR_PLAIN))
        format = OUTPUT_FORMAT_PLAIN;
    else {
        g_set_error(error,
                    G_OPTION_ERROR,
                    G_OPTION_ERROR_FAILED,
                    _("Invalid value '%s'"), value);

        return FALSE;
    }

    return TRUE;
}

static gboolean parse_type_str(const gchar *option_name,
                               const gchar *value,
                               gpointer data,
                               GError **error)
{
    if (g_str_equal(value, TYPE_STR_MEDIA))
        type = URL_TYPE_MEDIA;
    else if (g_str_equal(value, TYPE_STR_TREE))
        type = URL_TYPE_TREE;
    else {
        g_set_error(error,
                    G_OPTION_ERROR,
                    G_OPTION_ERROR_FAILED,
                    _("Invalid value '%s'"), value);

        return FALSE;
    }

    return TRUE;
}

static GOptionEntry entries[] =
{
    { "format", 'f', 0,
      G_OPTION_ARG_CALLBACK, parse_format_str,
      N_("Output format. Default: plain"),
      N_("plain.") },
    { "type", 't', 0,
      G_OPTION_ARG_CALLBACK, parse_type_str,
      N_("The type to be used. Default: media"),
      N_("media|tree.") },
    { 0 }
};

static void print_bootable(gboolean bootable)
{
    if (bootable)
        g_print(_("Media is bootable.\n"));
    else
        g_print(_("Media is not bootable.\n"));
}

static void print_media(OsinfoMedia *media)
{
    OsinfoOs *os;
    OsinfoOsVariantList *variants;
    const gchar *name;
    const gchar *arch;
    guint num_variants = 0;

    g_object_get(G_OBJECT(media), "os", &os, NULL);
    if (os == NULL)
        return;

    variants = osinfo_media_get_os_variants(media);
    if (variants != NULL)
        num_variants = osinfo_list_get_length(OSINFO_LIST(variants));

    if (num_variants == 1) {
        OsinfoEntity *variant;

        variant = osinfo_list_get_nth(OSINFO_LIST(variants), 0);
        name = osinfo_os_variant_get_name(OSINFO_OS_VARIANT(variant));
    } else {
        name = osinfo_product_get_name(OSINFO_PRODUCT(os));
    }

    arch = osinfo_media_get_architecture(media);

    if (osinfo_media_get_installer(media))
        g_print(_("Media is an installer for OS '%s (%s)'\n"), name, arch);
    if (osinfo_media_get_live(media))
        g_print(_("Media is live media for OS '%s (%s)'\n"), name, arch);

    if (num_variants > 1) {
        guint i;

        g_print(_("Available OS variants on media:\n"));
        for (i = 0; i < num_variants; i++) {
            OsinfoEntity *variant;

            variant = osinfo_list_get_nth(OSINFO_LIST(variants), i);
            name = osinfo_os_variant_get_name(OSINFO_OS_VARIANT(variant));
            g_print("%s\n", name);
        }
    }

    g_clear_object(&variants);
    g_object_unref(os);
}

static void print_tree(OsinfoTree *tree)
{
    OsinfoOs *os;
    OsinfoOsVariantList *variants;
    const gchar *name;
    const gchar *arch;
    guint num_variants = 0;


    g_object_get(G_OBJECT(tree), "os", &os, NULL);
    if (os == NULL)
        return;


    variants = osinfo_tree_get_os_variants(tree);
    if (variants != NULL)
        num_variants = osinfo_list_get_length(OSINFO_LIST(variants));

    if (num_variants == 1) {
        OsinfoEntity *variant;

        variant = osinfo_list_get_nth(OSINFO_LIST(variants), 0);
        name = osinfo_os_variant_get_name(OSINFO_OS_VARIANT(variant));
    } else {
        name = osinfo_product_get_name(OSINFO_PRODUCT(os));
    }

    arch = osinfo_tree_get_architecture(tree);
    g_print(_("Tree is an installer for OS '%s (%s)'\n"), name, arch);

    if (num_variants > 1) {
        guint i;

        g_print(_("Available OS variants on tree:\n"));
        for (i = 0; i < num_variants; i++) {
            OsinfoEntity *variant;

            variant = osinfo_list_get_nth(OSINFO_LIST(variants), i);
            name = osinfo_os_variant_get_name(OSINFO_OS_VARIANT(variant));
            g_print("%s\n", name);
        }
    }

    g_clear_object(&variants);
    g_object_unref(os);
}

gint main(gint argc, gchar **argv)
{
    GOptionContext *context;
    GError *error = NULL;
    OsinfoLoader *loader = NULL;
    OsinfoDb *db = NULL;
    gint ret = 0;

    setlocale(LC_ALL, "");
    textdomain(GETTEXT_PACKAGE);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    context = g_option_context_new(_("- Detect if media is bootable " \
                                     "and the relevant OS and distribution."));
    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("Error while parsing commandline options: %s\n"),
                   error->message);
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));

        ret = -1;
        goto EXIT;
    }

    if (argc < 2) {
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));

        ret = -2;
        goto EXIT;
    }

    loader = osinfo_loader_new();
    osinfo_loader_process_default_path(loader, &error);
    if (error != NULL) {
        g_printerr(_("Error loading OS data: %s\n"), error->message);
        /* errors loading the osinfo database are not fatal as this can
         * happen when the user has an invalid file in
         * ~/.local/share/libosinfo for example. Let's report but ignore
         * them
         */
        g_clear_error(&error);
    }

    db = osinfo_loader_get_db(loader);

    if (type == URL_TYPE_MEDIA) {
        OsinfoMedia *media = NULL;
        media = osinfo_media_create_from_location(argv[1], NULL, &error);
        if (error != NULL) {
            if (error->code != OSINFO_MEDIA_ERROR_NOT_BOOTABLE) {
                g_printerr(_("Error parsing media: %s\n"), error->message);

                ret = -3;
            } else {
                print_bootable(FALSE);
            }
            goto EXIT;
        } else {
            print_bootable(TRUE);
        }
        osinfo_db_identify_media(db, media);
        print_media(media);
    } else if (type == URL_TYPE_TREE) {
        OsinfoTree *tree = NULL;
        tree = osinfo_tree_create_from_location(argv[1], NULL, &error);
        if (error != NULL) {
            g_printerr(_("Error parsing installer tree: %s\n"), error->message);

            ret = -3;
            goto EXIT;
        }
        osinfo_db_identify_tree(db, tree);
        print_tree(tree);
    }


EXIT:
    g_clear_error(&error);
    g_clear_object(&loader);
    g_option_context_free(context);

    return ret;
}

/*
=pod

=head1 NAME

osinfo-detect - Detect the operating system on installable media or trees

=head1 SYNOPSIS

osinfo-detect [OPTIONS...] PATH|URI

=head1 DESCRIPTION

Examine the C<PATH> or C<URI> to determine what (if any) operating
system it is for, and whether it is installable or is a Live image.
By default C<PATH> or C<URI> will be interpreted as pointing to
ISO media. To request examination of an install tree instead, the
option C<--type=tree> should be given.

The output information is formatted for humans;

=head1 OPTIONS

=over 8

=item B<--format=plain>

=item B<--type=media|tree>

Switch between looking for CD/DVD ISO media (B<media>, the default) or
install trees (B<tree>)

=back

=head1 EXIT STATUS

The exit status will be 0 if an operating system was detected,
or 1 if none was found.

=head1 AUTHORS

Zeeshan Ali (Khattak) <zeeshanak@gnome.org>, Daniel P. Berrange <berrange@redhat.com>

=head1 COPYRIGHT

Copyright (C) 2011-2012 Red Hat, Inc.

=head1 LICENSE

C<osinfo-detect> is distributed under the termsof the GNU LGPL v2
license. This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE

=cut
*/
