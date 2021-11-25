/*
 * libosinfo: an object with a set of parameters
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

#pragma once

#include <glib-object.h>

#include <osinfo/osinfo_macros.h>

#define OSINFO_TYPE_ENTITY (osinfo_entity_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoEntity,
                                           osinfo_entity,
                                           OSINFO,
                                           ENTITY,
                                           GObject)

#define OSINFO_ENTITY_PROP_ID "id"

const gchar *osinfo_entity_get_id(OsinfoEntity *entity);

GList *osinfo_entity_get_param_keys(OsinfoEntity *entity);
const gchar *osinfo_entity_get_param_value(OsinfoEntity *entity, const gchar *key);
gboolean osinfo_entity_get_param_value_boolean(OsinfoEntity *entity, const gchar *key);
gboolean osinfo_entity_get_param_value_boolean_with_default(OsinfoEntity *entity,
                                                            const char *key,
                                                            gboolean default_value);
gint osinfo_entity_get_param_value_enum(OsinfoEntity *entity,
                                        const char *key,
                                        GType enum_type,
                                        gint default_value);
gint64 osinfo_entity_get_param_value_int64(OsinfoEntity *entity, const gchar *key);
gint64 osinfo_entity_get_param_value_int64_with_default(OsinfoEntity *entity,
                                                        const gchar *key,
                                                        gint64 default_value);
GList *osinfo_entity_get_param_value_list(OsinfoEntity *entity, const gchar *key);
void osinfo_entity_set_param(OsinfoEntity *entity, const gchar *key, const gchar *value);
void osinfo_entity_set_param_boolean(OsinfoEntity *entity, const gchar *key, gboolean value);
void osinfo_entity_set_param_int64(OsinfoEntity *entity, const gchar *key, gint64 value);
void osinfo_entity_set_param_enum(OsinfoEntity *entity, const gchar *key, gint value, GType enum_type);
void osinfo_entity_add_param(OsinfoEntity *entity, const gchar *key, const gchar *value);
void osinfo_entity_clear_param(OsinfoEntity *entity, const gchar *key);
