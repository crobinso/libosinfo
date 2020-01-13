/*
 * libosinfo: a list of pre-installed images
 *
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
 * SECTION:osinfo_imagelist
 * @short_description: A list of pre-installated images
 * @see_also: #OsinfoList, #OsinfoImage
 *
 * #OsinfoImageList is a list specialization that stores
 * only #OsinfoImage objects.
 */

struct _OsinfoImageListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoImageList, osinfo_imagelist, OSINFO_TYPE_LIST);

static void
osinfo_imagelist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_imagelist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_imagelist_class_init(OsinfoImageListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_imagelist_finalize;
}

static void
osinfo_imagelist_init(OsinfoImageList *list)
{
    list->priv = osinfo_imagelist_get_instance_private(list);
}

/**
 * osinfo_imagelist_new:
 *
 * Construct a new image list that is initially empty.
 *
 * Returns: (transfer full): an empty image list
 *
 * Since: 1.3.0
 */
OsinfoImageList *osinfo_imagelist_new(void)
{
    return g_object_new(OSINFO_TYPE_IMAGELIST,
                        "element-type", OSINFO_TYPE_IMAGE,
                        NULL);
}
