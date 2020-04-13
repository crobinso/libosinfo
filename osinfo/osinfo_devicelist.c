/*
 * libosinfo:
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
 * SECTION:osinfo_devicelist
 * @short_description: A list of hardware device
 * @see_also: #OsinfoList, #OsinfoDevice
 *
 * #OsinfoDeviceList is a list specialization that stores
 * only #OsinfoDevice objects.
 */

struct _OsinfoDeviceListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDeviceList, osinfo_devicelist, OSINFO_TYPE_LIST);

static void
osinfo_devicelist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_devicelist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_devicelist_class_init(OsinfoDeviceListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_devicelist_finalize;
}

static void
osinfo_devicelist_init(OsinfoDeviceList *list)
{
    list->priv = osinfo_devicelist_get_instance_private(list);
}


/**
 * osinfo_devicelist_new:
 *
 * Construct a new device list that is initially empty.
 *
 * Returns: (transfer full): an empty device list
 */
OsinfoDeviceList *osinfo_devicelist_new(void)
{
    return g_object_new(OSINFO_TYPE_DEVICELIST,
                        "element-type", OSINFO_TYPE_DEVICE,
                        NULL);
}


/**
 * osinfo_devicelist_new_copy:
 * @source: the device list to copy
 *
 * Construct a new device list that is filled with devices
 * from @source
 *
 * Returns: (transfer full): a copy of the device list
 * Deprecated: 0.2.2: Use osinfo_list_new_copy() instead.
 */
OsinfoDeviceList *osinfo_devicelist_new_copy(OsinfoDeviceList *source)
{
    OsinfoDeviceList *newList = osinfo_devicelist_new();
    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(source));
    return newList;
}


/**
 * osinfo_devicelist_new_filtered:
 * @source: the device list to copy
 * @filter: the filter to apply
 *
 * Construct a new device list that is filled with devices
 * from @source that match @filter
 *
 * Returns: (transfer full): a filtered copy of the device list
 * Deprecated: 0.2.2: Use osinfo_list_new_filtered() instead.
 */
OsinfoDeviceList *osinfo_devicelist_new_filtered(OsinfoDeviceList *source, OsinfoFilter *filter)
{
    OsinfoDeviceList *newList = osinfo_devicelist_new();
    osinfo_list_add_filtered(OSINFO_LIST(newList),
                             OSINFO_LIST(source),
                             filter);
    return newList;
}

/**
 * osinfo_devicelist_new_intersection:
 * @sourceOne: the first device list to copy
 * @sourceTwo: the second device list to copy
 *
 * Construct a new device list that is filled with only the
 * devices that are present in both @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): an intersection of the two device lists
 * Deprecated: 0.2.2: Use osinfo_list_new_intersection() instead.
 */
OsinfoDeviceList *osinfo_devicelist_new_intersection(OsinfoDeviceList *sourceOne, OsinfoDeviceList *sourceTwo)
{
    OsinfoDeviceList *newList = osinfo_devicelist_new();
    osinfo_list_add_intersection(OSINFO_LIST(newList),
                                 OSINFO_LIST(sourceOne),
                                 OSINFO_LIST(sourceTwo));
    return newList;
}

/**
 * osinfo_devicelist_new_union:
 * @sourceOne: the first device list to copy
 * @sourceTwo: the second device list to copy
 *
 * Construct a new device list that is filled with all the
 * devices that are present in either @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): a union of the two device lists
 * Deprecated: 0.2.2: Use osinfo_list_new_union() instead.
 */
OsinfoDeviceList *osinfo_devicelist_new_union(OsinfoDeviceList *sourceOne, OsinfoDeviceList *sourceTwo)
{
    OsinfoDeviceList *newList = osinfo_devicelist_new();
    osinfo_list_add_union(OSINFO_LIST(newList),
                          OSINFO_LIST(sourceOne),
                          OSINFO_LIST(sourceTwo));
    return newList;
}
