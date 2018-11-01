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
 *   Fabiano Fidênciog <fidencio@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>

#define ID "foo"
#define ARCH "x86_64"
#define FORMAT "qcow2"
#define URL "https://foo.bar/foobar.img"

static void
test_basic(void)
{
    OsinfoImage *image = osinfo_image_new(ID, ARCH, FORMAT);
    osinfo_entity_set_param(OSINFO_ENTITY(image),
                            OSINFO_IMAGE_PROP_URL,
                            URL);

    g_assert_true(OSINFO_IS_IMAGE(image));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(image)), ==, ID);
    g_assert_cmpstr(osinfo_image_get_architecture(image), ==, ARCH);
    g_assert_cmpstr(osinfo_image_get_format(image), ==, FORMAT);
    g_assert_cmpstr(osinfo_image_get_url(image), ==, URL);

    g_object_unref(image);
}


int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/image/basic", test_basic);

    /* Upfront so we don't confuse valgrind */
    osinfo_image_get_type();

    return g_test_run();
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
