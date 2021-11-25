/*
 * libosinfo: A firmware representation for a (guest) OS
 *
 * Copyright (C) 2019-2020 Red Hat, Inc.
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
#include <gio/gio.h>
#include <osinfo/osinfo_entity.h>

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_FIRMWARE (osinfo_firmware_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoFirmware,
                                           osinfo_firmware,
                                           OSINFO,
                                           FIRMWARE,
                                           OsinfoEntity)

#define OSINFO_FIRMWARE_PROP_ARCHITECTURE   "architecture"
#define OSINFO_FIRMWARE_PROP_TYPE           "type"
#define OSINFO_FIRMWARE_PROP_SUPPORTED      "supported"

OsinfoFirmware *osinfo_firmware_new(const gchar *id, const gchar *architecture, const gchar *type);
const gchar *osinfo_firmware_get_architecture(OsinfoFirmware *firmware);
const gchar *osinfo_firmware_get_firmware_type(OsinfoFirmware *firmware);
gboolean osinfo_firmware_is_supported(OsinfoFirmware *firmware);
