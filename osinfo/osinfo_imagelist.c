/*
 * libosinfo: a list of pre-installed images
 *
 * Copyright (C) 2018 Red Hat, Inc.
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
 *
 * Authors:
 *   Fabiano Fidêncio <fidencio@redhat.com>
 */

#include <config.h>

#include <osinfo/osinfo.h>
#include <glib/gi18n-lib.h>

G_DEFINE_TYPE(OsinfoImageList, osinfo_imagelist, OSINFO_TYPE_LIST);

#define OSINFO_IMAGELIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OSINFO_TYPE_IMAGELIST, OsinfoImageListPrivate))

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
    g_type_class_add_private(klass, sizeof(OsinfoImageListPrivate));
}

static void
osinfo_imagelist_init(OsinfoImageList *list)
{
    list->priv = OSINFO_IMAGELIST_GET_PRIVATE(list);
}

/**
 * osinfo_imagelist_new:
 *
 * Construct a new image list that is initially empty.
 *
 * Returns: (transfer full): an empty image list
 */
OsinfoImageList *osinfo_imagelist_new(void)
{
    return g_object_new(OSINFO_TYPE_IMAGELIST,
                        "element-type", OSINFO_TYPE_IMAGE,
                        NULL);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
