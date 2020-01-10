/*
 * libosinfo: Database loader
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

#include <glib-object.h>
#include <osinfo/osinfo_db.h>

#ifndef __OSINFO_LOADER_H__
# define __OSINFO_LOADER_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_LOADER (osinfo_loader_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoLoader,
                                           osinfo_loader,
                                           OSINFO,
                                           LOADER,
                                           GObject)

/**
 * OsinfoError:
 * @OSINFO_ERROR_GENERIC: Generic Osinfo error;
 *
 * #GError codes used for errors in #OSINFO_ERROR domain.
 * Since: 1.8.0
 */
typedef enum {
    OSINFO_ERROR_GENERIC = 0,
} OsinfoError;

/**
 * OSINFO_ERROR:
 *
 * Error domain for generic libosinfo errors
 */
# define OSINFO_ERROR osinfo_error_quark()
GQuark osinfo_error_quark(void);

OsinfoLoader *osinfo_loader_new(void);

OsinfoDb *osinfo_loader_get_db(OsinfoLoader *loader);

void osinfo_loader_process_path(OsinfoLoader *loader, const gchar *path, GError **err);
void osinfo_loader_process_uri(OsinfoLoader *loader, const gchar *uri, GError **err);
void osinfo_loader_process_default_path(OsinfoLoader *loader, GError **err);
void osinfo_loader_process_system_path(OsinfoLoader *loader, GError **err);
void osinfo_loader_process_local_path(OsinfoLoader *loader, GError **err);
void osinfo_loader_process_user_path(OsinfoLoader *loader, GError **err);


#endif /* __OSINFO_LOADER_H__ */
