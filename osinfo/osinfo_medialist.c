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
 * SECTION:osinfo_medialist
 * @short_description: A list of installation media
 * @see_also: #OsinfoList, #OsinfoMedia
 *
 * #OsinfoMediaList is a list specialization that stores
 * only #OsinfoMedia objects.
 */

struct _OsinfoMediaListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoMediaList, osinfo_medialist, OSINFO_TYPE_LIST);

static void
osinfo_medialist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_medialist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_medialist_class_init(OsinfoMediaListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_medialist_finalize;
}

static void
osinfo_medialist_init(OsinfoMediaList *list)
{
    list->priv = osinfo_medialist_get_instance_private(list);
}

/**
 * osinfo_medialist_new:
 *
 * Construct a new media list that is initially empty.
 *
 * Returns: (transfer full): an empty media list
 */
OsinfoMediaList *osinfo_medialist_new(void)
{
    return g_object_new(OSINFO_TYPE_MEDIALIST,
                        "element-type", OSINFO_TYPE_MEDIA,
                        NULL);
}

/**
 * osinfo_medialist_new_copy:
 * @source: the media list to copy
 *
 * Construct a new media list that is filled with medias
 * from @source
 *
 * Returns: (transfer full): a copy of the media list
 * Deprecated: 0.2.2: Use osinfo_list_new_copy() instead.
 */
OsinfoMediaList *osinfo_medialist_new_copy(OsinfoMediaList *source)
{
    OsinfoMediaList *newList = osinfo_medialist_new();
    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(source));
    return newList;
}

/**
 * osinfo_medialist_new_filtered:
 * @source: the media list to copy
 * @filter: the filter to apply
 *
 * Construct a new media list that is filled with medias
 * from @source that match @filter
 *
 * Returns: (transfer full): a filtered copy of the media list
 * Deprecated: 0.2.2: Use osinfo_list_new_filtered() instead.
 */
OsinfoMediaList *osinfo_medialist_new_filtered(OsinfoMediaList *source,
                                               OsinfoFilter *filter)
{
    OsinfoMediaList *newList = osinfo_medialist_new();
    osinfo_list_add_filtered(OSINFO_LIST(newList),
                             OSINFO_LIST(source),
                             filter);
    return newList;
}

/**
 * osinfo_medialist_new_intersection:
 * @sourceOne: the first media list to copy
 * @sourceTwo: the second media list to copy
 *
 * Construct a new media list that is filled with only the
 * medias that are present in both @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): an intersection of the two media lists
 * Deprecated: 0.2.2: Use osinfo_list_new_intersection() instead.
 */
OsinfoMediaList *osinfo_medialist_new_intersection(OsinfoMediaList *sourceOne,
                                                   OsinfoMediaList *sourceTwo)
{
    OsinfoMediaList *newList = osinfo_medialist_new();
    osinfo_list_add_intersection(OSINFO_LIST(newList),
                                 OSINFO_LIST(sourceOne),
                                 OSINFO_LIST(sourceTwo));
    return newList;
}

/**
 * osinfo_medialist_new_union:
 * @sourceOne: the first media list to copy
 * @sourceTwo: the second media list to copy
 *
 * Construct a new media list that is filled with all the
 * medias that are present in either @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): a union of the two media lists
 * Deprecated: 0.2.2: Use osinfo_list_new_union() instead.
 */
OsinfoMediaList *osinfo_medialist_new_union(OsinfoMediaList *sourceOne,
                                            OsinfoMediaList *sourceTwo)
{
    OsinfoMediaList *newList = osinfo_medialist_new();
    osinfo_list_add_union(OSINFO_LIST(newList),
                          OSINFO_LIST(sourceOne),
                          OSINFO_LIST(sourceTwo));
    return newList;
}
