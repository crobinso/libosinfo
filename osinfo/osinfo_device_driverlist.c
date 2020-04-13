/*
 * libosinfo: Device driver list
 *
 * Copyright (C) 2009-2020 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <osinfo/osinfo.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_device_driverlist
 * @short_description: A list of device drivers
 * @see_also: #OsinfoList, #OsinfoDeviceDriver
 *
 * #OsinfoDeviceDriverList is a list specialization that stores only
 * #OsinfoDeviceDriver objects.
 */

struct _OsinfoDeviceDriverListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDeviceDriverList, osinfo_device_driverlist, OSINFO_TYPE_LIST);

static void
osinfo_device_driverlist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_device_driverlist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_device_driverlist_class_init(OsinfoDeviceDriverListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_device_driverlist_finalize;
}

static void
osinfo_device_driverlist_init(OsinfoDeviceDriverList *list)
{
    list->priv = osinfo_device_driverlist_get_instance_private(list);
}

/**
 * osinfo_device_driverlist_new:
 *
 * Construct a new device driver list that is initially empty.
 *
 * Returns: (transfer full): an empty device driver list
 *
 * Since: 0.2.2
 */
OsinfoDeviceDriverList *osinfo_device_driverlist_new(void)
{
    return g_object_new(OSINFO_TYPE_DEVICE_DRIVERLIST,
                        "element-type", OSINFO_TYPE_DEVICE_DRIVER,
                        NULL);
}
