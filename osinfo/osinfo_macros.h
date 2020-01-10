/*
 * libosinfo:
 *
 * Copyright (C) 2020 Red Hat, Inc.
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

#ifndef __OSINFO_MACROS_H__
# define __OSINFO_MACROS_H__

# include <glib-object.h>

# define OSINFO_DECLARE_TYPE(ModuleObjName, module_obj_name, MODULE, OBJ_NAME, ParentName)                \
    G_DECLARE_DERIVABLE_TYPE(ModuleObjName, module_obj_name, MODULE, OBJ_NAME, ParentName)

# define OSINFO_DECLARE_TYPE_WITH_PRIVATE(ModuleObjName, module_obj_name, MODULE, OBJ_NAME, ParentName)  \
  GType module_obj_name##_get_type (void);                                                               \
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS                                                                       \
  typedef struct _##ModuleObjName ModuleObjName;                                                         \
  typedef struct _##ModuleObjName##Class ModuleObjName##Class;                                           \
  typedef struct _##ModuleObjName##Private ModuleObjName##Private;                                       \
  struct _##ModuleObjName { ParentName parent_instance; ModuleObjName##Private *priv; };                 \
                                                                                                         \
  _GLIB_DEFINE_AUTOPTR_CHAINUP (ModuleObjName, ParentName)                                               \
                                                                                                         \
  static inline ModuleObjName * MODULE##_##OBJ_NAME (gpointer ptr) {                                     \
    return G_TYPE_CHECK_INSTANCE_CAST (ptr, module_obj_name##_get_type (), ModuleObjName); }             \
  static inline ModuleObjName##Class * MODULE##_##OBJ_NAME##_CLASS (gpointer ptr) {                      \
    return G_TYPE_CHECK_CLASS_CAST (ptr, module_obj_name##_get_type (), ModuleObjName##Class); }         \
  static inline gboolean MODULE##_IS_##OBJ_NAME (gpointer ptr) {                                         \
    return G_TYPE_CHECK_INSTANCE_TYPE (ptr, module_obj_name##_get_type ()); }                            \
  static inline gboolean MODULE##_IS_##OBJ_NAME##_CLASS (gpointer ptr) {                                 \
    return G_TYPE_CHECK_CLASS_TYPE (ptr, module_obj_name##_get_type ()); }                               \
  static inline ModuleObjName##Class * MODULE##_##OBJ_NAME##_GET_CLASS (gpointer ptr) {                  \
    return G_TYPE_INSTANCE_GET_CLASS (ptr, module_obj_name##_get_type (), ModuleObjName##Class); }       \
  G_GNUC_END_IGNORE_DEPRECATIONS

# define OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(ModuleObjName, module_obj_name, MODULE, OBJ_NAME, ParentName) \
  GType module_obj_name##_get_type (void);                                                                        \
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS                                                                                \
  typedef struct _##ModuleObjName ModuleObjName;                                                                  \
  typedef struct _##ModuleObjName##Class ModuleObjName##Class;                                                    \
  typedef struct _##ModuleObjName##Private ModuleObjName##Private;                                                \
  struct _##ModuleObjName { ParentName parent_instance; ModuleObjName##Private *priv; };                          \
  struct _##ModuleObjName##Class { ParentName##Class parent_class ; };                                            \
                                                                                                                  \
  _GLIB_DEFINE_AUTOPTR_CHAINUP (ModuleObjName, ParentName)                                                        \
                                                                                                                  \
  static inline ModuleObjName * MODULE##_##OBJ_NAME (gpointer ptr) {                                              \
    return G_TYPE_CHECK_INSTANCE_CAST (ptr, module_obj_name##_get_type (), ModuleObjName); }                      \
  static inline ModuleObjName##Class * MODULE##_##OBJ_NAME##_CLASS (gpointer ptr) {                               \
    return G_TYPE_CHECK_CLASS_CAST (ptr, module_obj_name##_get_type (), ModuleObjName##Class); }                  \
  static inline gboolean MODULE##_IS_##OBJ_NAME (gpointer ptr) {                                                  \
    return G_TYPE_CHECK_INSTANCE_TYPE (ptr, module_obj_name##_get_type ()); }                                     \
  static inline gboolean MODULE##_IS_##OBJ_NAME##_CLASS (gpointer ptr) {                                          \
    return G_TYPE_CHECK_CLASS_TYPE (ptr, module_obj_name##_get_type ()); }                                        \
  static inline ModuleObjName##Class * MODULE##_##OBJ_NAME##_GET_CLASS (gpointer ptr) {                           \
    return G_TYPE_INSTANCE_GET_CLASS (ptr, module_obj_name##_get_type (), ModuleObjName##Class); }                \
  G_GNUC_END_IGNORE_DEPRECATIONS

#endif /* __OSINFO_MACROS_H__ */
