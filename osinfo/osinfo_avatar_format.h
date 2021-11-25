/*
 * libosinfo: OS installation avatar information
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
 * <http://www.gnu.org/licenses/>
 */

#pragma once

#include <glib-object.h>

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_AVATAR_FORMAT (osinfo_avatar_format_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoAvatarFormat,
                                           osinfo_avatar_format,
                                           OSINFO,
                                           AVATAR_FORMAT,
                                           OsinfoEntity)

#define OSINFO_AVATAR_FORMAT_PROP_MIME_TYPE "mime-type"
#define OSINFO_AVATAR_FORMAT_PROP_WIDTH     "width"
#define OSINFO_AVATAR_FORMAT_PROP_HEIGHT    "height"
#define OSINFO_AVATAR_FORMAT_PROP_ALPHA     "alpha"

OsinfoAvatarFormat *osinfo_avatar_format_new(void);

GList *osinfo_avatar_format_get_mime_types(OsinfoAvatarFormat *avatar);
gint osinfo_avatar_format_get_width(OsinfoAvatarFormat *avatar);
gint osinfo_avatar_format_get_height(OsinfoAvatarFormat *avatar);
gboolean osinfo_avatar_format_get_alpha(OsinfoAvatarFormat *avatar);
