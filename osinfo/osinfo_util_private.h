/*
 * libosinfo: A collection of utilities used for medias & trees
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

#include <glib.h>

gboolean osinfo_util_requires_soup(const gchar *location);

#if SOUP_MAJOR_VERSION < 3
# define soup_message_get_status(message) message->status_code
# define soup_message_get_response_headers(message) message->response_headers
#endif
