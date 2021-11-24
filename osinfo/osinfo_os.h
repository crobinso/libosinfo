/*
 * libosinfo: an operating system
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
#include <osinfo/osinfo_product.h>
#include <osinfo/osinfo_device.h>
#include <osinfo/osinfo_devicelist.h>
#include <osinfo/osinfo_medialist.h>
#include <osinfo/osinfo_media.h>
#include <osinfo/osinfo_treelist.h>
#include <osinfo/osinfo_os_variant.h>
#include <osinfo/osinfo_os_variantlist.h>
#include <osinfo/osinfo_tree.h>
#include <osinfo/osinfo_resources.h>
#include <osinfo/osinfo_resourceslist.h>
#include <osinfo/osinfo_image.h>
#include <osinfo/osinfo_imagelist.h>

#ifndef __OSINFO_OS_H__
# define __OSINFO_OS_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_OS (osinfo_os_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoOs,
                                           osinfo_os,
                                           OSINFO,
                                           OS,
                                           OsinfoProduct)

# define OSINFO_OS_PROP_FAMILY                 "family"
# define OSINFO_OS_PROP_DISTRO                 "distro"
# define OSINFO_OS_PROP_RELEASE_STATUS         "release-status"
# define OSINFO_OS_PROP_KERNEL_URL_ARGUMENT    "kernel-url-argument"
# define OSINFO_OS_PROP_CLOUD_IMAGE_USERNAME   "cloud-image-username"

/**
 * OsinfoReleaseStatus:
 * @OSINFO_RELEASE_STATUS_RELEASED: A released OS. (Since: 0.2.9)
 * @OSINFO_RELEASE_STATUS_SNAPSHOT: A continuous integration snapshot and not
 * an actual released product yet. For example, gnome-continuous images for
 * development snapshots of GNOME and nightly build ISOs of Fedora etc.
 * (Since: 0.2.9)
 * @OSINFO_RELEASE_STATUS_PRERELEASE: A pre-release. For example, alpha and
 * beta pre-releases of Fedora etc. (Since: 0.2.9)
 * @OSINFO_RELEASE_STATUS_ROLLING: A rolling version of an OS and thus with no
 * actual releases. For example, Gentoo, Arch Linux, etc. (Since: 1.4.0)
 */
typedef enum {
    OSINFO_RELEASE_STATUS_RELEASED,
    OSINFO_RELEASE_STATUS_SNAPSHOT,
    OSINFO_RELEASE_STATUS_PRERELEASE,
    OSINFO_RELEASE_STATUS_ROLLING
} OsinfoReleaseStatus;

OsinfoOs *osinfo_os_new(const gchar *id);

OsinfoDeviceList *osinfo_os_get_devices(OsinfoOs *os, OsinfoFilter *filter);
OsinfoDeviceList *osinfo_os_get_all_devices(OsinfoOs *os, OsinfoFilter *filter);
OsinfoDeviceList *osinfo_os_get_devices_by_property(OsinfoOs *os,
                                                    const char *property,
                                                    const char *value,
                                                    gboolean inherited);
OsinfoDeviceLinkList *osinfo_os_get_device_links(OsinfoOs *os, OsinfoFilter *filter);
OsinfoDeviceLinkList *osinfo_os_get_all_device_links(OsinfoOs *os, OsinfoFilter *filter);

OsinfoDeviceLink *osinfo_os_add_device(OsinfoOs *os, OsinfoDevice *dev);
const gchar *osinfo_os_get_family(OsinfoOs *os);
const gchar *osinfo_os_get_distro(OsinfoOs *os);
int osinfo_os_get_release_status(OsinfoOs *os);
OsinfoMediaList *osinfo_os_get_media_list(OsinfoOs *os);
void osinfo_os_add_media(OsinfoOs *os, OsinfoMedia *media);
OsinfoTreeList *osinfo_os_get_tree_list(OsinfoOs *os);
void osinfo_os_add_tree(OsinfoOs *os, OsinfoTree *tree);
OsinfoImageList *osinfo_os_get_image_list(OsinfoOs *os);
void osinfo_os_add_image(OsinfoOs *os, OsinfoImage *image);
OsinfoOsVariantList *osinfo_os_get_variant_list(OsinfoOs *os);
void osinfo_os_add_variant(OsinfoOs *os, OsinfoOsVariant *variant);
OsinfoResourcesList *osinfo_os_get_network_install_resources(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_minimum_resources(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_recommended_resources(OsinfoOs *os);
OsinfoResourcesList *osinfo_os_get_maximum_resources(OsinfoOs *os);
void osinfo_os_add_network_install_resources(OsinfoOs *os, OsinfoResources *resources);
void osinfo_os_add_minimum_resources(OsinfoOs *os, OsinfoResources *resources);
void osinfo_os_add_recommended_resources(OsinfoOs *os, OsinfoResources *resources);
void osinfo_os_add_maximum_resources(OsinfoOs *os, OsinfoResources *resources);

OsinfoInstallScript *osinfo_os_find_install_script(OsinfoOs *os, const gchar *profile);
OsinfoInstallScriptList *osinfo_os_get_install_script_list(OsinfoOs *os);
void osinfo_os_add_install_script(OsinfoOs *os, OsinfoInstallScript *script);

OsinfoDeviceDriverList *osinfo_os_get_device_drivers(OsinfoOs *os);
OsinfoDeviceDriverList *osinfo_os_get_device_drivers_prioritized(OsinfoOs *os);
void osinfo_os_add_device_driver(OsinfoOs *os, OsinfoDeviceDriver *driver);

const gchar *osinfo_os_get_kernel_url_argument(OsinfoOs *os);

OsinfoFirmwareList *osinfo_os_get_firmware_list(OsinfoOs *os, OsinfoFilter *filter);
OsinfoFirmwareList *osinfo_os_get_complete_firmware_list(OsinfoOs *os, OsinfoFilter *filter);
void osinfo_os_add_firmware(OsinfoOs *os, OsinfoFirmware *firmware);

const gchar *osinfo_os_get_cloud_image_username(OsinfoOs *os);

#endif /* __OSINFO_OS_H__ */
