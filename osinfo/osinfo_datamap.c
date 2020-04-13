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
#include <string.h>
#include <libxml/tree.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xsltInternals.h>

/**
 * SECTION:osinfo_datamap
 * @short_descripion: OS  datamap
 * @see_also: #OsinfoDatamap
 *
 * #OsinfoDatamap is an object for representing OS
 * datamaps. It is to translate generic osinfo values to OS
 * specific data.
 */

struct _OsinfoDatamapPrivate
{
    GHashTable *map;
    GHashTable *reverse_map;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDatamap, osinfo_datamap, OSINFO_TYPE_ENTITY);

static void
osinfo_datamap_finalize(GObject *object)
{
    OsinfoDatamap *map = OSINFO_DATAMAP(object);

    g_hash_table_unref(map->priv->map);
    g_hash_table_unref(map->priv->reverse_map);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_datamap_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_datamap_class_init(OsinfoDatamapClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_datamap_finalize;
}

static void
osinfo_datamap_init(OsinfoDatamap *map)
{
    map->priv = osinfo_datamap_get_instance_private(map);
    map->priv->map = g_hash_table_new_full(g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            g_free);
    map->priv->reverse_map = g_hash_table_new(g_str_hash, g_str_equal);
}


/**
 * osinfo_datamap_new:
 * @id: the unique identifier
 *
 * Construct a new datamapa that is initially empty.
 *
 * Returns: (transfer full): an empty datamap
 *
 * Since: 0.2.3
 */
OsinfoDatamap *osinfo_datamap_new(const gchar *id)
{
    return g_object_new(OSINFO_TYPE_DATAMAP,
                        "id", id,
                        NULL);
}


/**
 * osinfo_datamap_insert:
 * @map: the OS datamap
 * @inval: the input value
 * @outval: the output value
 *
 * Adds the input value and the output value associated to it to
 * the @map.
 *
 * Since: 0.2.3
 */
void osinfo_datamap_insert(OsinfoDatamap *map,
                           const gchar *inval,
                           const gchar *outval)
{
    gchar *dup_inval;
    gchar *dup_outval;
    g_return_if_fail(OSINFO_IS_DATAMAP(map));
    g_return_if_fail(inval != NULL);

    dup_inval = g_strdup(inval);
    dup_outval = g_strdup(outval);
    g_hash_table_insert(map->priv->map, dup_inval, dup_outval);
    g_hash_table_insert(map->priv->reverse_map, dup_outval, dup_inval);
}

/**
 * osinfo_datamap_lookup:
 * @map: the OS datamap
 * @inval: the input value
 *
 * Returns the output value with which @inval is associated to.
 *
 * Since: 0.2.3
 */
const gchar *osinfo_datamap_lookup(OsinfoDatamap *map,
                                   const gchar *inval)
{
    return g_hash_table_lookup(map->priv->map, inval);
}

/**
 * osinfo_datamap_reverse_lookup:
 * @map: the OS datamap
 * @outval: the output value
 *
 * Returns the input value with which @outval is associated to.
 *
 * Since: 0.2.3
 */
const gchar *osinfo_datamap_reverse_lookup(OsinfoDatamap *map,
                                           const gchar *outval)
{
    return g_hash_table_lookup(map->priv->reverse_map, outval);
}
