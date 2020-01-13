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

/**
 * SECTION:osinfo_datamaplist
 * @short_description: A list of datamaps
 * @see_also: #OsinfoList, #OsinfoDatamap
 *
 * #OsinfoDatamapList is a list specialization that stores
 * only #OsinfoDatamap objects.
 */

struct _OsinfoDatamapListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDatamapList, osinfo_datamaplist, OSINFO_TYPE_LIST);

static void
osinfo_datamaplist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_datamaplist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_datamaplist_class_init(OsinfoDatamapListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_datamaplist_finalize;
}

static void
osinfo_datamaplist_init(OsinfoDatamapList *list)
{
    list->priv = osinfo_datamaplist_get_instance_private(list);
}

/**
 * osinfo_datamaplist_new:
 *
 * Construct a new install_datamap list that is initially empty.
 *
 * Returns: (transfer full): an empty install_datamap list
 *
 * Since: 0.2.3
 */
OsinfoDatamapList *osinfo_datamaplist_new(void)
{
    return g_object_new(OSINFO_TYPE_DATAMAPLIST,
                        "element-type", OSINFO_TYPE_DATAMAP,
                        NULL);
}
