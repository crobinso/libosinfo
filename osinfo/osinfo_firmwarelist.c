/*
 * libosinfo: a list of firmwares
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
 * SECTION:osinfo_firmwarelist
 * @short_description: A list of firmwares
 * @see_also: #OsinfoList, #OsinfoFirmware
 *
 * #OsinfoFirmwareList is a list specialization that stores
 * only #OsinfoFirmware objects.
 */

struct _OsinfoFirmwareListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoFirmwareList, osinfo_firmwarelist, OSINFO_TYPE_LIST);

static void
osinfo_firmwarelist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_firmwarelist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_firmwarelist_class_init(OsinfoFirmwareListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_firmwarelist_finalize;
}

static void
osinfo_firmwarelist_init(OsinfoFirmwareList *list)
{
    list->priv = osinfo_firmwarelist_get_instance_private(list);
}

/**
 * osinfo_firmwarelist_new:
 *
 * Construct a new firmware list that is initially empty.
 *
 * Returns: (transfer full): an empty firmware list
 *
 * Since: 1.7.0
 */
OsinfoFirmwareList *osinfo_firmwarelist_new(void)
{
    return g_object_new(OSINFO_TYPE_FIRMWARELIST,
                        "element-type", OSINFO_TYPE_FIRMWARE,
                        NULL);
}
