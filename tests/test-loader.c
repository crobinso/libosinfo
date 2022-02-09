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
 */

#include <osinfo/osinfo.h>
#include <glib/gstdio.h>
#include <unistd.h>

static void
test_basic(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    GError *error = NULL;

    g_assert_true(OSINFO_IS_LOADER(loader));

    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);

    g_object_unref(loader);
}

typedef struct {
    gchar *tmp_parent;
    gchar *tmp_child;
    gchar *orig_userdir;
    gchar *expected_warning;
} TestEaccesFixture;

static void
eacces_fixture_setup(TestEaccesFixture *fixture, gconstpointer user_data)
{
    gpointer rp;
    gint ri;
    gboolean rb;

    /* create a temporary directory with permissions 0700 */
    fixture->tmp_parent = g_strdup_printf("%s/%s", g_get_tmp_dir(),
                                          "test_eacces.XXXXXX");
    rp = g_mkdtemp_full(fixture->tmp_parent, 0700);
    g_assert_nonnull(rp);

    /* create a child directory called "osinfo" in it, with permissions 0700 */
    fixture->tmp_child = g_strdup_printf("%s/osinfo", fixture->tmp_parent);
    ri = g_mkdir(fixture->tmp_child, 0700);
    g_assert_cmpint(ri, ==, 0);

    /* revoke the search permission (0100) from the parent */
    ri = g_chmod(fixture->tmp_parent, 0600);
    g_assert_cmpint(ri, ==, 0);

    /* stash the current value of OSINFO_USER_DIR */
    fixture->orig_userdir = g_strdup(g_getenv("OSINFO_USER_DIR"));

    /* point osinfo_loader_get_user_path() inside
     * osinfo_loader_process_default_path() to the child directory
     */
    rb = g_setenv("OSINFO_USER_DIR", fixture->tmp_child, TRUE);
    g_assert_true(rb);

    /* format the pattern for the warning expected later on */
    fixture->expected_warning = g_strdup_printf("Can't read path %s",
                                                fixture->tmp_child);
}

static void
eacces_fixture_teardown(TestEaccesFixture *fixture, gconstpointer user_data)
{
    gboolean rb;
    gint ri;

    /* free the expected warning pattern */
    g_free(fixture->expected_warning);

    /* restore the OSINFO_USER_DIR variable */
    if (fixture->orig_userdir) {
        rb = g_setenv("OSINFO_USER_DIR", fixture->orig_userdir, TRUE);
        g_assert_true(rb);
        g_free(fixture->orig_userdir);
    } else {
        g_unsetenv("OSINFO_USER_DIR");
    }

    /* restore search permission on the parent */
    ri = g_chmod(fixture->tmp_parent, 0700);
    g_assert_cmpint(ri, ==, 0);

    /* remove both directories */
    ri = g_rmdir(fixture->tmp_child);
    g_assert_cmpint(ri, ==, 0);
    g_free(fixture->tmp_child);

    ri = g_rmdir(fixture->tmp_parent);
    g_assert_cmpint(ri, ==, 0);
    g_free(fixture->tmp_parent);
}

static void
test_eacces(TestEaccesFixture *fixture, gconstpointer user_data)
{
    OsinfoLoader *loader = osinfo_loader_new();
    GError *error = NULL;

    g_assert_true(OSINFO_IS_LOADER(loader));

    /* this should trigger an EACCES in glib's lstat(), but not break db
     * loading; also we expect the warning here
     */
    g_test_expect_message(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
                          fixture->expected_warning);
    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);
    g_test_assert_expected_messages();

    g_object_unref(loader);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/loader/basic", test_basic);

    /* the following test depends on a directory with file mode bits 0600 being
     * unsearchable for the owner, so skip it if the test is running as root
     */
    if (geteuid() != 0) {
        g_test_add("/loader/eacces", TestEaccesFixture, NULL,
                   eacces_fixture_setup, test_eacces, eacces_fixture_teardown);
    }

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
    osinfo_loader_get_type();

    return g_test_run();
}
