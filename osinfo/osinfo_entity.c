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
 * SECTION:osinfo_entity
 * @short_description: Abstract base class for metadata objects
 * @see_also: #OsinfoList, #OsinfoDb
 *
 * #OsinfoEntity is an abstract base class for all objects against which
 * metadata needs to be recorded. Every object has a unique identifier,
 * which is recommended to be in URI format. Named, multi-valued data
 * parameters can be associated with each entity. When filtering lists
 * of entities, the parameter values can be used for matching.
 */

struct _OsinfoEntityPrivate
{
    gchar *id;

    // Key: gchar*
    // Value: GList of gchar* values
    GHashTable *params;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(OsinfoEntity, osinfo_entity, G_TYPE_OBJECT);

static void osinfo_entity_finalize(GObject *object);

enum {
    PROP_0,

    PROP_ID,

    LAST_PROP
};
static GParamSpec *properties[LAST_PROP];

static void
osinfo_entity_set_property(GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    OsinfoEntity *entity = OSINFO_ENTITY(object);

    switch (property_id)
        {
        case PROP_ID:
            g_free(entity->priv->id);
            entity->priv->id = g_value_dup_string(value);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}

static void
osinfo_entity_get_property(GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    OsinfoEntity *entity = OSINFO_ENTITY(object);

    switch (property_id)
        {
        case PROP_ID:
            g_value_set_string(value, entity->priv->id);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}

static void
osinfo_entity_finalize(GObject *object)
{
    OsinfoEntity *entity = OSINFO_ENTITY(object);

    g_free(entity->priv->id);
    g_hash_table_destroy(entity->priv->params);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_entity_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_entity_class_init(OsinfoEntityClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->set_property = osinfo_entity_set_property;
    g_klass->get_property = osinfo_entity_get_property;
    g_klass->finalize = osinfo_entity_finalize;

    /**
     * OsinfoEntity:id:
     *
     * The unique identifier for the entity The format of identifiers
     * is undefined, but the recommended practice is to use a URI.
     * This parameter must be set at time of construction as no
     * default value is provided.
     */
    properties[PROP_ID] = g_param_spec_string("id",
                                              "ID",
                                              _("Unique identifier"),
                                              NULL /* default value */,
                                              G_PARAM_CONSTRUCT |
                                              G_PARAM_READWRITE |
                                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(g_klass, LAST_PROP, properties);
}


static void osinfo_entity_param_values_free(gpointer values)
{
    g_list_free_full(values, g_free);
}


static void
osinfo_entity_init(OsinfoEntity *entity)
{
    entity->priv = osinfo_entity_get_instance_private(entity);
    entity->priv->params = g_hash_table_new_full(g_str_hash,
                                               g_str_equal,
                                               g_free,
                                               osinfo_entity_param_values_free);
}


/**
 * osinfo_entity_set_param:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @value: the data to associated with that key
 *
 * Sets a new parameter against the entity. If the key already
 * has a value associated with it, the existing value will be
 * cleared.
 */
void osinfo_entity_set_param(OsinfoEntity *entity, const gchar *key, const gchar *value)
{
    GList *values = NULL;

    g_return_if_fail(OSINFO_IS_ENTITY(entity));
    g_return_if_fail(key != NULL);
    g_return_if_fail(value != NULL);

    values = g_list_append(values, g_strdup(value));
    g_hash_table_replace(entity->priv->params, g_strdup(key), values);
}


/**
 * osinfo_entity_set_param_boolean:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @value: the boolean value to be associated with that key
 *
 * Sets a new parameter against the entity. If the key already
 * has a value associated with it, the existing value will be
 * cleared.
 *
 * Since: 0.2.0
 */
void osinfo_entity_set_param_boolean(OsinfoEntity *entity, const gchar *key, gboolean value)
{
    osinfo_entity_set_param(entity, key, value ? "true" : "false");
}

/**
 * osinfo_entity_set_param_int64:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @value: the int64 value to be associated with that key
 *
 * Sets a new parameter against the entity. If the key already
 * has a value associated with it, the existing value will be
 * cleared.
 *
 * Since: 0.2.1
 */
void osinfo_entity_set_param_int64(OsinfoEntity *entity, const gchar *key, gint64 value)
{
    gchar *str;

    str = g_strdup_printf("%"G_GINT64_FORMAT, value);
    osinfo_entity_set_param(entity, key, str);
    g_free(str);
}

/**
 * osinfo_entity_set_param_enum:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @value: the enum value to be associated with that key
 * @enum_type: the enum type
 *
 * Sets a new parameter against the entity. If the key already
 * has a value associated with it, the existing value will be
 * cleared.
 *
 * Since: 0.2.2
 */
void osinfo_entity_set_param_enum(OsinfoEntity *entity, const gchar *key, gint value, GType enum_type)
{
    GEnumClass *enum_class;
    GEnumValue *enum_value;

    g_return_if_fail(G_TYPE_IS_ENUM(enum_type));

    enum_class = g_type_class_ref(enum_type);
    enum_value = g_enum_get_value(enum_class, value);
    g_type_class_unref(enum_class);
    g_return_if_fail(enum_value != NULL);

    osinfo_entity_set_param(entity, key, enum_value->value_nick);
}

/**
 * osinfo_entity_add_param:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @value: the data to associated with that key
 *
 * Adds a new parameter against the entity. A key can have multiple
 * values associated. Thus repeated calls with the same key will
 * build up a list of possible values.
 */
void osinfo_entity_add_param(OsinfoEntity *entity, const gchar *key, const gchar *value)
{
    GList *values = NULL;
    gpointer origKey = NULL;
    gpointer foundValue = NULL;
    gboolean found;

    g_return_if_fail(OSINFO_IS_ENTITY(entity));
    g_return_if_fail(key != NULL);
    g_return_if_fail(value != NULL);

    // First check if there exists an existing array of entries for this key
    // If not, create a ptrarray of strings for this key and insert into map
    found = g_hash_table_lookup_extended(entity->priv->params, key, &origKey, &foundValue);
    if (found) {
        g_hash_table_steal(entity->priv->params, key);
        g_free(origKey);
        values = foundValue;
    }

    values = g_list_append(values, g_strdup(value));
    g_hash_table_insert(entity->priv->params, g_strdup(key), values);
}


/**
 * osinfo_entity_clear_param:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 *
 * Remove all values associated with a key
 */
void osinfo_entity_clear_param(OsinfoEntity *entity, const gchar *key)
{
    g_return_if_fail(OSINFO_IS_ENTITY(entity));

    g_hash_table_remove(entity->priv->params, key);
}

/**
 * osinfo_entity_get_id:
 * @entity: an #OsinfoEntity
 *
 * Retrieves the unique key for the entity. The format of identifiers
 * is undefined, but the recommended practice is to use a URI.
 *
 * Returns: (transfer none): the unique key for the entity
 */
const gchar *osinfo_entity_get_id(OsinfoEntity *entity)
{
    g_return_val_if_fail(OSINFO_IS_ENTITY(entity), NULL);

    return entity->priv->id;
}


/**
 * osinfo_entity_get_param_keys:
 * @entity: an #OsinfoEntity containing the parameters
 *
 * Retrieve all the known parameter keys associated with
 * the entity
 *
 * Returns: (transfer container) (element-type utf8): The list of string parameters
 */
GList *osinfo_entity_get_param_keys(OsinfoEntity *entity)
{
    GList *keys;

    g_return_val_if_fail(OSINFO_IS_ENTITY(entity), NULL);

    keys = g_hash_table_get_keys(entity->priv->params);
    keys = g_list_append(keys, (char *)"id");

    return keys;
}


/**
 * osinfo_entity_get_param_value:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 *
 * Retrieve the parameter value associated with a named key. If
 * multiple values are stored against the key, only the first
 * value is returned. If no value is associated, NULL is returned
 *
 * Returns: (transfer none): the value associated with the key, or NULL
 */
const gchar *osinfo_entity_get_param_value(OsinfoEntity *entity, const gchar *key)
{
    GList *values;

    g_return_val_if_fail(OSINFO_IS_ENTITY(entity), NULL);
    g_return_val_if_fail(key != NULL, NULL);

    if (g_str_equal(key, OSINFO_ENTITY_PROP_ID))
        return entity->priv->id;

    values = g_hash_table_lookup(entity->priv->params, key);

    if (values)
        return values->data;
    return NULL;
}

static gboolean str_to_bool(const char *str)
{
    return (g_strcmp0("true", str) == 0 || g_strcmp0("yes", str) == 0);
}

/**
 * osinfo_entity_get_param_value_boolean:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 *
 * Retrieve the parameter value associated with a named key as a
 * boolean. If multiple values are stored against the key, only the
 * first value is returned. If no value is associated, FALSE is returned
 *
 * Returns: the value associated with the key as a boolean, or FALSE
 *
 * Since: 0.2.0
 */
gboolean osinfo_entity_get_param_value_boolean(OsinfoEntity *entity, const gchar *key)
{
    const gchar *value = osinfo_entity_get_param_value(entity, key);

    return value && str_to_bool(value);
}

/**
 * osinfo_entity_get_param_value_boolean_with_default:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @default_value: the value to be returned in case there's no value
 *                 associated with the @key
 *
 * Retrieve the parameter value associated with a named key as a
 * boolean. If multiple values are stored against the key, only the
 * first value is returned. If no value is associated, @default_value
 * is returned.
 *
 * Returns: the value associated with the key as a boolean, or
 * @default_value
 *
 * Since: 0.2.1
 */
gboolean osinfo_entity_get_param_value_boolean_with_default(OsinfoEntity *entity,
                                                            const char *key,
                                                            gboolean default_value)
{
    const gchar *value;

    value = osinfo_entity_get_param_value(entity, key);
    if (value == NULL)
        return default_value;
    else
        return str_to_bool(value);
}

/**
 * osinfo_entity_get_param_value_int64:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 *
 * Retrieve the parameter value associated with a named key as an
 * int64. If multiple values are stored against the key, only the
 * first value is returned. If no value is associated, -1 is returned.
 *
 * Returns: the value associated with the key as an int64, or -1.
 *
 * Since: 0.2.1
 */
gint64 osinfo_entity_get_param_value_int64(OsinfoEntity *entity,
                                           const gchar *key)
{
    return osinfo_entity_get_param_value_int64_with_default(entity, key, -1);
}

/**
 * osinfo_entity_get_param_value_int64_with_default:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @default_value: the value to be returned in case there's no value
 *                 associated with the @key
 *
 * Retrieve the parameter value associated with a named key as an
 * int64. If multiple values are stored against the key, only the
 * first value is returned. If no value is associated, @default_value
 * is returned.
 *
 * Returns: the value associated with the key as an int64, or
 * @default_value
 *
 * Since: 0.2.1
 */
gint64 osinfo_entity_get_param_value_int64_with_default(OsinfoEntity *entity,
                                                        const gchar *key,
                                                        gint64 default_value)
{
    const gchar *str;

    str = osinfo_entity_get_param_value(entity, key);

    if (str == NULL)
        return default_value;

    return g_ascii_strtoll(str, NULL, 0);
}

/**
 * osinfo_entity_get_param_value_enum:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 * @enum_type: the enum type
 * @default_value: the default value to be used, in case there's
 *                 no value associated with the key
 *
 * Retrieve the parameter value associated with a named key as an
 * enum value. If multiple values are stored against the key, only
 * the first value is returned. If no value is associated, the
 * @default_value is returned.
 *
 * Returns: the enum value associated with the key, or @default_value.
 *
 * Since: 0.2.2
 */
gint osinfo_entity_get_param_value_enum(OsinfoEntity *entity,
                                        const char *key,
                                        GType enum_type,
                                        gint default_value)
{
    const gchar *nick;
    GEnumClass *enum_class;
    GEnumValue *enum_value;

    g_return_val_if_fail(G_TYPE_IS_ENUM(enum_type), default_value);

    nick = osinfo_entity_get_param_value(entity, key);
    if (nick == NULL)
        return default_value;

    enum_class = g_type_class_ref(enum_type);
    enum_value = g_enum_get_value_by_nick(enum_class, nick);
    g_type_class_unref(enum_class);

    if (enum_value != NULL)
        return enum_value->value;

    g_return_val_if_reached(default_value);
}

/**
 * osinfo_entity_get_param_value_list:
 * @entity: an #OsinfoEntity containing the parameters
 * @key: the name of the key
 *
 * Retrieve all the parameter values associated with a named
 * key. If no values are associated, NULL is returned
 *
 * Returns: (transfer container) (element-type utf8): the values associated with the key
 */
GList *osinfo_entity_get_param_value_list(OsinfoEntity *entity, const gchar *key)
{
    GList *values;

    g_return_val_if_fail(OSINFO_IS_ENTITY(entity), NULL);
    g_return_val_if_fail(key != NULL, NULL);

    if (g_str_equal(key, OSINFO_ENTITY_PROP_ID))
        return g_list_append(NULL, entity->priv->id);

    values = g_hash_table_lookup(entity->priv->params, key);

    return g_list_copy(values);
}
