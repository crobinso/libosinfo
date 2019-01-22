/*
 * libosinfo: an operating system
 *
 * Copyright (C) 2018 Red Hat, Inc.
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
 *
 * Authors:
 *   Fabiano Fidêncio <fidencio@redhat.com>
 */

#include <osinfo/osinfo_os.h>

#ifndef __OSINFO_OS_PRIVATE_H__
#define __OSINFO_OS_PRIVATE_H__

OsinfoResourcesList *osinfo_os_get_network_install_resources_without_inheritance(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_minimum_resources_without_inheritance(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_recommended_resources_without_inheritance(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_maximum_resources_without_inheritance(OsinfoOs *os);

#endif /* __OSINFO_OS_PRIVATE_H__ */
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
