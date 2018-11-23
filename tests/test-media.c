/*
 * Copyright (C) 2018 Red Hat, Inc.
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
 *   Fabiano Fidêncio <fidencio@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>



static void
test_basic(void)
{
    OsinfoMedia *media = osinfo_media_new("foo", "x86_64");

    g_assert_true(OSINFO_IS_MEDIA(media));
    g_assert_cmpstr((const char *)osinfo_entity_get_id(OSINFO_ENTITY(media)), ==, "foo");
    g_assert_cmpstr((const char *)osinfo_media_get_architecture(media), ==, "x86_64");

    g_object_unref(media);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/media/basic", test_basic);

    /* Upfront so we don't confuse valgrind */
    osinfo_media_get_type();

    return g_test_run();
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
