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
 * SECTION:osinfo_devicelinklist
 * @short_description: A list of hardware devicelink
 * @see_also: #OsinfoList, #OsinfoDeviceLink
 *
 * #OsinfoDeviceLinkList is a list specialization that stores
 * only #OsinfoDeviceLink objects.
 */

struct _OsinfoDeviceLinkListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDeviceLinkList, osinfo_devicelinklist, OSINFO_TYPE_LIST);

static void
osinfo_devicelinklist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_devicelinklist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_devicelinklist_class_init(OsinfoDeviceLinkListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_devicelinklist_finalize;
}

static void
osinfo_devicelinklist_init(OsinfoDeviceLinkList *list)
{
    list->priv = osinfo_devicelinklist_get_instance_private(list);
}


/**
 * osinfo_devicelinklist_new:
 *
 * Construct a new devicelink list that is initially empty.
 *
 * Returns: (transfer full): an empty devicelink list
 */
OsinfoDeviceLinkList *osinfo_devicelinklist_new(void)
{
    return g_object_new(OSINFO_TYPE_DEVICELINKLIST,
                        "element-type", OSINFO_TYPE_DEVICELINK,
                        NULL);
}


/**
 * osinfo_devicelinklist_new_copy:
 * @source: the devicelink list to copy
 *
 * Construct a new devicelink list that is filled with devicelinks
 * from @source
 *
 * Returns: (transfer full): a copy of the devicelink list
 * Deprecated: 0.2.2: Use osinfo_list_new_copy() instead.
 */
OsinfoDeviceLinkList *osinfo_devicelinklist_new_copy(OsinfoDeviceLinkList *source)
{
    OsinfoDeviceLinkList *newList = osinfo_devicelinklist_new();
    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(source));
    return newList;
}


/**
 * osinfo_devicelinklist_new_filtered:
 * @source: the devicelink list to copy
 * @filter: the filter to apply
 *
 * Construct a new devicelink list that is filled with devicelinks
 * from @source that match @filter
 *
 * Returns: (transfer full): a filtered copy of the devicelink list
 * Deprecated: 0.2.2: Use osinfo_list_new_filtered() instead.
 */
OsinfoDeviceLinkList *osinfo_devicelinklist_new_filtered(OsinfoDeviceLinkList *source, OsinfoFilter *filter)
{
    OsinfoDeviceLinkList *newList = osinfo_devicelinklist_new();
    osinfo_list_add_filtered(OSINFO_LIST(newList),
                             OSINFO_LIST(source),
                             filter);
    return newList;
}

/**
 * osinfo_devicelinklist_new_intersection:
 * @sourceOne: the first devicelink list to copy
 * @sourceTwo: the second devicelink list to copy
 *
 * Construct a new devicelink list that is filled with only the
 * devicelinks that are present in both @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): an intersection of the two devicelink lists
 * Deprecated: 0.2.2: Use osinfo_list_new_intersection() instead.
 */
OsinfoDeviceLinkList *osinfo_devicelinklist_new_intersection(OsinfoDeviceLinkList *sourceOne, OsinfoDeviceLinkList *sourceTwo)
{
    OsinfoDeviceLinkList *newList = osinfo_devicelinklist_new();
    osinfo_list_add_intersection(OSINFO_LIST(newList),
                                 OSINFO_LIST(sourceOne),
                                 OSINFO_LIST(sourceTwo));
    return newList;
}

/**
 * osinfo_devicelinklist_new_union:
 * @sourceOne: the first devicelink list to copy
 * @sourceTwo: the second devicelink list to copy
 *
 * Construct a new devicelink list that is filled with all the
 * devicelinks that are present in either @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): a union of the two devicelink lists
 * Deprecated: 0.2.2: Use osinfo_list_new_union() instead.
 */
OsinfoDeviceLinkList *osinfo_devicelinklist_new_union(OsinfoDeviceLinkList *sourceOne, OsinfoDeviceLinkList *sourceTwo)
{
    OsinfoDeviceLinkList *newList = osinfo_devicelinklist_new();
    osinfo_list_add_union(OSINFO_LIST(newList),
                          OSINFO_LIST(sourceOne),
                          OSINFO_LIST(sourceTwo));
    return newList;
}


/**
 * osinfo_devicelinklist_get_devices:
 * @list: an device link list
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter
 *
 * Returns: (transfer full): A list of devices
 */
OsinfoDeviceList *osinfo_devicelinklist_get_devices(OsinfoDeviceLinkList *list,
                                                    OsinfoFilter *filter)
{
    OsinfoDeviceList *newList = osinfo_devicelist_new();
    int i;
    for (i = 0; i < osinfo_list_get_length(OSINFO_LIST(list)); i++) {
        OsinfoEntity *ent = osinfo_list_get_nth(OSINFO_LIST(list), i);
        if (!filter || osinfo_filter_matches(filter, ent)) {
            OsinfoDevice *dev = osinfo_devicelink_get_target(OSINFO_DEVICELINK(ent));
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(dev));
        }
    }
    return newList;
}
