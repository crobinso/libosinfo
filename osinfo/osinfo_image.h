/*
 * libosinfo: An installed image of a (guest) OS
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

#pragma once

#include <glib-object.h>
#include <gio/gio.h>

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_IMAGE (osinfo_image_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoImage,
                                           osinfo_image,
                                           OSINFO,
                                           IMAGE,
                                           OsinfoEntity)

#define OSINFO_IMAGE_PROP_ARCHITECTURE      "architecture"
#define OSINFO_IMAGE_PROP_FORMAT            "format"
#define OSINFO_IMAGE_PROP_URL               "url"
#define OSINFO_IMAGE_PROP_CLOUD_INIT        "cloud-init"
#define OSINFO_IMAGE_PROP_VARIANT           "variant"

OsinfoImage *osinfo_image_new(const gchar *id, const gchar *architecture, const gchar *format);
const gchar *osinfo_image_get_architecture(OsinfoImage *image);
const gchar *osinfo_image_get_format(OsinfoImage *image);
const gchar *osinfo_image_get_url(OsinfoImage *image);
gboolean osinfo_image_get_cloud_init(OsinfoImage *image);
OsinfoOs *osinfo_image_get_os(OsinfoImage *image);
void osinfo_image_set_os(OsinfoImage *image, OsinfoOs *os);
OsinfoOsVariantList *osinfo_image_get_os_variants(OsinfoImage *image);
