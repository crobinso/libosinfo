/*
 * libosinfo: a list of OS variants
 *
 * Copyright (C) 2013-2020 Red Hat, Inc.
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
 * SECTION:osinfo_os_variantlist
 * @short_description: A list of OS variants
 * @see_also: #OsinfoList, #OsinfoOsVariant
 *
 * #OsinfoOsVariantList is a list specialization that stores
 * only #OsinfoOsVariant objects.
 */

struct _OsinfoOsVariantListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoOsVariantList, osinfo_os_variantlist, OSINFO_TYPE_LIST);

/* Init functions */
static void
osinfo_os_variantlist_class_init(OsinfoOsVariantListClass *klass)
{
}

static void
osinfo_os_variantlist_init(OsinfoOsVariantList *list)
{
    list->priv = osinfo_os_variantlist_get_instance_private(list);
}

/**
 * osinfo_os_variantlist_new:
 *
 * Construct a new install_variant list that is initially empty.
 *
 * Returns: (transfer full): an empty install_variant list
 *
 * Since: 0.2.9
 */
OsinfoOsVariantList *osinfo_os_variantlist_new(void)
{
    return g_object_new(OSINFO_TYPE_OS_VARIANTLIST,
                        "element-type", OSINFO_TYPE_OS_VARIANT,
                        NULL);
}
