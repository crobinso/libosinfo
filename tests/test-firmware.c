/*
 * Copyright (C) 2019 Red Hat, Inc.
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

#include <config.h>

#include <osinfo/osinfo.h>

#define ID "foo:0"
#define ARCH "x86_64"
#define TYPE "efi"

static void
test_basic(void)
{
    OsinfoFirmware *firmware = osinfo_firmware_new(ID, ARCH, TYPE);

    g_assert_true(OSINFO_IS_FIRMWARE(firmware));
    g_assert_cmpstr(osinfo_entity_get_id(OSINFO_ENTITY(firmware)), ==, ID);
    g_assert_cmpstr(osinfo_firmware_get_architecture(firmware), ==, ARCH);
    g_assert_cmpstr(osinfo_firmware_get_firmware_type(firmware), ==, TYPE);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/firmware/basic", test_basic);

    /* Upfront so we don't confuse valgrind */
    osinfo_firmware_get_type();

    return g_test_run();
}
