/*
 * Copyright (C) 2017 Fabiano Fidêncio
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
 *   Fabiano Fidêncio <fabiano@fidencio.org>
 */

#include <config.h>

#include <stdlib.h>
#include <osinfo/osinfo.h>


static void test_n_cpus(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint minimum_cpus, recommended_cpus;

    minimum_cpus = osinfo_resources_get_n_cpus(minimum);
    recommended_cpus = osinfo_resources_get_n_cpus(recommended);

    if (recommended_cpus >= 0 && minimum_cpus >= 0)
        g_assert_true(recommended_cpus >= minimum_cpus);
}


static void test_cpu(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint64 minimum_cpu, recommended_cpu;

    minimum_cpu = osinfo_resources_get_cpu(minimum);
    recommended_cpu = osinfo_resources_get_cpu(recommended);

    if (recommended_cpu >= 0 && minimum_cpu >= 0)
        g_assert_true(recommended_cpu >= minimum_cpu);
}


static void test_ram(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint64 minimum_ram, recommended_ram;

    minimum_ram = osinfo_resources_get_ram(minimum);
    recommended_ram = osinfo_resources_get_ram(recommended);

    if (recommended_ram >= 0 && minimum_ram >= 0)
        g_assert_true(recommended_ram >= minimum_ram);
}


static void test_storage(OsinfoResources *minimum, OsinfoResources *recommended)
{
    gint64 minimum_storage, recommended_storage;

    minimum_storage = osinfo_resources_get_storage(minimum);
    recommended_storage = osinfo_resources_get_storage(recommended);

    if (recommended_storage >= 0 && minimum_storage >= 0)
        g_assert_true(recommended_storage >= minimum_storage);
}

static void
test_minimum_recommended_resources(void)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    OsinfoOsList *oslist;
    GList *oses;
    GList *oses_it;
    GError *error = NULL;

    g_assert_true(OSINFO_IS_LOADER(loader));
    g_assert_true(OSINFO_IS_DB(db));

    osinfo_loader_process_default_path(loader, &error);
    g_assert_no_error(error);

    oslist = osinfo_db_get_os_list(db);
    oses = osinfo_list_get_elements(OSINFO_LIST(oslist));

    for (oses_it = oses; oses_it != NULL; oses_it = oses_it->next) {
        OsinfoOs *os = oses_it->data;
        OsinfoResourcesList *minimum_list, *recommended_list;
        GList *minimum_resources, *recommended_resources;
        GList *resources_it;
        const gchar *minimum_arch, *recommended_arch;

        minimum_list = osinfo_os_get_minimum_resources(os);
        minimum_resources = osinfo_list_get_elements(OSINFO_LIST(minimum_list));

        recommended_list = osinfo_os_get_recommended_resources(os);
        recommended_resources = osinfo_list_get_elements(OSINFO_LIST(recommended_list));

        /* That's fine as not all OSes have those fields filled */
        if (minimum_resources == NULL || recommended_resources == NULL)
            goto next;

        for (resources_it = minimum_resources; resources_it != NULL; resources_it = resources_it->next) {
            OsinfoResources *minimum = resources_it->data;
            GList *tmp = recommended_resources;

            minimum_arch = osinfo_resources_get_architecture(minimum);

            for (; tmp != NULL; tmp = tmp->next) {
                OsinfoResources *recommended = tmp->data;

                recommended_arch = osinfo_resources_get_architecture(recommended);

                 if (g_str_equal(minimum_arch, recommended_arch)) {
                    const gchar *name;

                    name = osinfo_product_get_name(OSINFO_PRODUCT(os));

                    g_test_message("checking %s (architecture: %s)",
                                   name, minimum_arch);

                    test_n_cpus(minimum, recommended);
                    test_cpu(minimum, recommended);
                    test_ram(minimum, recommended);
                    test_storage(minimum, recommended);
                    break;
                }
            }
        }

next:
        g_list_free(minimum_resources);
        g_list_free(recommended_resources);
        g_object_unref(minimum_list);
        g_object_unref(recommended_list);
    }

    g_object_unref(oslist);
    g_list_free(oses);

    g_object_unref(loader);
}


int
main(int argc, char *argv[])
{
    int ret;

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/os/resources/minimum_recommended", test_minimum_recommended_resources);

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

    ret = g_test_run();

    return ret;
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
