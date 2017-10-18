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



static void
test_basic(void)
{
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoDevice *dev = osinfo_device_new("e1000");

    g_assert_true(OSINFO_IS_FILTER(filter));
    g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    osinfo_filter_add_constraint(filter, "class", "network");
    GList *keys = osinfo_filter_get_constraint_keys(filter);
    g_assert_nonnull(keys);
    g_assert_cmpstr(keys->data, ==, "class");
    g_assert_null(keys->next);
    g_list_free(keys);

    osinfo_filter_add_constraint(filter, "class", "audio");
    keys = osinfo_filter_get_constraint_keys(filter);
    g_assert_nonnull(keys);
    g_assert_cmpstr(keys->data, ==, "class");
    g_assert_null(keys->next);
    g_list_free(keys);

    osinfo_filter_add_constraint(filter, "bus", "pci");
    keys = osinfo_filter_get_constraint_keys(filter);
    g_assert_nonnull(keys);
    g_assert_nonnull(keys->next);
    g_assert_cmpstr(keys->data, ==, "bus");
    g_assert_cmpstr(keys->next->data, ==, "class");
    g_assert_null(keys->next->next);
    g_list_free(keys);

    GList *values = osinfo_filter_get_constraint_values(filter, "bus");
    g_assert_nonnull(values);
    g_assert_cmpstr(values->data, ==, "pci");
    g_assert_null(values->next);
    g_list_free(values);

    values = osinfo_filter_get_constraint_values(filter, "class");
    g_assert_nonnull(values);
    g_assert_nonnull(values->next);
    g_assert_cmpstr(values->data, ==, "audio");
    g_assert_cmpstr(values->next->data, ==, "network");
    g_assert_null(values->next->next);
    g_list_free(values);

    g_object_unref(dev);
    g_object_unref(filter);
}

static void
test_filter_single(void)
{
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoDevice *dev = osinfo_device_new("e1000");

    osinfo_entity_add_param(OSINFO_ENTITY(dev), "bus", "pci");

    osinfo_filter_add_constraint(filter, "class", "network");
    g_assert_false(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    osinfo_entity_add_param(OSINFO_ENTITY(dev), "class", "network");
    g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    osinfo_filter_clear_constraint(filter, "class");
    osinfo_filter_add_constraint(filter, "class", "audio");
    g_assert_false(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    g_object_unref(dev);
    g_object_unref(filter);
}

static void
test_filter_multi(void)
{
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoDevice *dev = osinfo_device_new("e1000");

    osinfo_entity_add_param(OSINFO_ENTITY(dev), "bus", "pci");

    osinfo_filter_add_constraint(filter, "bus", "isa");
    g_assert_false(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    osinfo_filter_add_constraint(filter, "bus", "pci");
    /* XXX is this right ?  Multiple values for a filter constraint
     * is treated as requiring all constraint values to match, not
     * required any to match */
    //g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));
    g_assert_false(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    osinfo_filter_clear_constraints(filter);
    osinfo_filter_add_constraint(filter, "bus", "pci");
    g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev)));

    g_object_unref(dev);
    g_object_unref(filter);
}


static void
test_filter_combine(void)
{
    OsinfoFilter *filter = osinfo_filter_new();
    OsinfoDevice *dev1 = osinfo_device_new("e1000");
    OsinfoDevice *dev2 = osinfo_device_new("ne2k");

    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "bus", "pci");
    osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");

    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "bus", "isa");
    osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "network");

    osinfo_filter_add_constraint(filter, "class", "network");
    g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev1)));
    g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev2)));

    osinfo_filter_add_constraint(filter, "bus", "isa");
    g_assert_false(osinfo_filter_matches(filter, OSINFO_ENTITY(dev1)));
    g_assert_true(osinfo_filter_matches(filter, OSINFO_ENTITY(dev2)));

    g_object_unref(dev1);
    g_object_unref(dev2);
    g_object_unref(filter);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/filter/basic", test_basic);
    g_test_add_func("/filter/filter_single", test_filter_single);
    g_test_add_func("/filter/filter_multi", test_filter_multi);
    g_test_add_func("/filter/filter_combine", test_filter_combine);

    /* Make sure we catch unexpected g_warning() */
    g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);

    /* Upfront so we don't confuse valgrind */
    osinfo_device_get_type();
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
