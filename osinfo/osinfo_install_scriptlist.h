/*
 * libosinfo: a list of installation install_script
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
#include <osinfo/osinfo_list.h>

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_INSTALL_SCRIPTLIST (osinfo_install_scriptlist_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoInstallScriptList,
                                           osinfo_install_scriptlist,
                                           OSINFO,
                                           INSTALL_SCRIPTLIST,
                                           OsinfoList)

OsinfoInstallScriptList *osinfo_install_scriptlist_new(void);
G_DEPRECATED_FOR(osinfo_list_new_copy)
OsinfoInstallScriptList *osinfo_install_scriptlist_new_copy(OsinfoInstallScriptList *source);
G_DEPRECATED_FOR(osinfo_list_new_filtered)
OsinfoInstallScriptList *osinfo_install_scriptlist_new_filtered(OsinfoInstallScriptList *source, OsinfoFilter *filter);
G_DEPRECATED_FOR(osinfo_list_new_intersection)
OsinfoInstallScriptList *osinfo_install_scriptlist_new_intersection(OsinfoInstallScriptList *sourceOne, OsinfoInstallScriptList *sourceTwo);
G_DEPRECATED_FOR(osinfo_list_new_union)
OsinfoInstallScriptList *osinfo_install_scriptlist_new_union(OsinfoInstallScriptList *sourceOne, OsinfoInstallScriptList *sourceTwo);
