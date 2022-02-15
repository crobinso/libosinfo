/*
 * libosinfo:
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

#include <osinfo/osinfo.h>
#include "osinfo_media_private.h"
#include "osinfo/osinfo_product_private.h"
#include "osinfo/osinfo_resources_private.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_os
 * @short_description: An operating system
 * @see_also: #OsinfoOs, #OsinfoDeployment
 *
 * #OsinfoOs is an entity representing an operating system.
 * Operating systems have a list of supported devices.
 * There are relationships amongst operating systems to
 * declare which are newest releases, which are clones
 * and which are derived from a common ancestry.
 */

struct _OsinfoOsPrivate
{
    // Value: List of device_link structs
    GList *deviceLinks;

    OsinfoFirmwareList *firmwares;
    OsinfoMediaList *medias;
    OsinfoTreeList *trees;
    OsinfoImageList *images;
    OsinfoOsVariantList *variants;
    OsinfoResourcesList *network_install;
    OsinfoResourcesList *minimum;
    OsinfoResourcesList *recommended;
    OsinfoResourcesList *maximum;

    OsinfoInstallScriptList *scripts;

    OsinfoDeviceDriverList *device_drivers;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoOs, osinfo_os, OSINFO_TYPE_PRODUCT);

struct _OsinfoOsDeviceLink {
    OsinfoDevice *dev;
    gchar *driver;
};

enum {
    PROP_0,

    PROP_FAMILY,
    PROP_DISTRO,
    PROP_KERNEL_URL_ARGUMENT,
    PROP_CLOUD_IMAGE_USERNAME,
    LAST_PROP
};
static GParamSpec *properties[LAST_PROP];

static void osinfo_os_finalize(GObject *object);

static void
osinfo_os_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    OsinfoEntity *entity = OSINFO_ENTITY(object);

    switch (property_id)
        {
        case PROP_FAMILY:
            g_value_set_string(value,
                                osinfo_entity_get_param_value(entity,
                                                               "family"));
            break;
        case PROP_DISTRO:
            g_value_set_string(value,
                                osinfo_entity_get_param_value(entity,
                                                               "distro"));
            break;
        case PROP_KERNEL_URL_ARGUMENT:
            g_value_set_string(value,
                               osinfo_entity_get_param_value(entity,
                                                             OSINFO_OS_PROP_KERNEL_URL_ARGUMENT));
            break;
        case PROP_CLOUD_IMAGE_USERNAME:
            g_value_set_string(value,
                               osinfo_entity_get_param_value(entity,
                                                             OSINFO_OS_PROP_CLOUD_IMAGE_USERNAME));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}

static void
osinfo_os_finalize(GObject *object)
{
    OsinfoOs *os = OSINFO_OS(object);

    g_list_free_full(os->priv->deviceLinks, g_object_unref);
    g_object_unref(os->priv->firmwares);
    g_object_unref(os->priv->medias);
    g_object_unref(os->priv->trees);
    g_object_unref(os->priv->images);
    g_object_unref(os->priv->variants);
    g_object_unref(os->priv->network_install);
    g_object_unref(os->priv->minimum);
    g_object_unref(os->priv->recommended);
    g_object_unref(os->priv->maximum);

    g_object_unref(os->priv->scripts);

    g_object_unref(os->priv->device_drivers);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_os_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_os_class_init(OsinfoOsClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->get_property = osinfo_os_get_property;
    g_klass->finalize = osinfo_os_finalize;

    /**
     * OsinfoOs:family:
     *
     * The generic family this OS belongs to, based upon its kernel,
     * for example linux, winnt, solaris, freebsd etc.
     */
    properties[PROP_FAMILY] = g_param_spec_string("family",
                                                  "Family",
                                                  _("Generic Family"),
                                                  NULL /* default value */,
                                                  G_PARAM_READABLE |
                                                  G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoOs:distro:
     *
     * The generic distro this OS belongs to, for example fedora, windows,
     * solaris, freebsd etc.
     */
    properties[PROP_DISTRO] = g_param_spec_string("distro",
                                                  "Distro",
                                                  _("Generic Distro"),
                                                  NULL /* default value */,
                                                  G_PARAM_READABLE |
                                                  G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoOs:kernel-url-argument:
     *
     * The argument to be passed to kernel command line when performing a
     * tree based installation of this OS.
     */
    properties[PROP_KERNEL_URL_ARGUMENT] = g_param_spec_string("kernel-url-argument",
                                                               "KernelURLArgument",
                                                               _("Kernel URL Argument"),
                                                               NULL /* default value */,
                                                               G_PARAM_READABLE |
                                                               G_PARAM_STATIC_STRINGS);

    /**
     * OsinfoOs:cloud-image-username:
     *
     * The username to be passed to the cloud-init program.
     */
    properties[PROP_CLOUD_IMAGE_USERNAME] = g_param_spec_string("cloud-image-username",
                                                                "CloudImageUsername",
                                                                _("Cloud image username"),
                                                                NULL /* default value */,
                                                                G_PARAM_READABLE |
                                                                G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(g_klass, LAST_PROP, properties);
}

static void
osinfo_os_init(OsinfoOs *os)
{
    os->priv = osinfo_os_get_instance_private(os);

    os->priv->deviceLinks = NULL;
    os->priv->firmwares = osinfo_firmwarelist_new();
    os->priv->medias = osinfo_medialist_new();
    os->priv->trees = osinfo_treelist_new();
    os->priv->images = osinfo_imagelist_new();
    os->priv->variants = osinfo_os_variantlist_new();
    os->priv->network_install = osinfo_resourceslist_new();
    os->priv->minimum = osinfo_resourceslist_new();
    os->priv->recommended = osinfo_resourceslist_new();
    os->priv->maximum = osinfo_resourceslist_new();
    os->priv->scripts = osinfo_install_scriptlist_new();
    os->priv->device_drivers = osinfo_device_driverlist_new();
}

/**
 * osinfo_os_new:
 * @id: a unique identifier
 *
 * Create a new operating system entity
 *
 * Returns: (transfer full): a new operating system entity
 */
OsinfoOs *osinfo_os_new(const gchar *id)
{
    return g_object_new(OSINFO_TYPE_OS,
                        "id", id,
                        NULL);
}


static gboolean
add_entity_to_list_check(OsinfoEntity *ent1, /* OsinfoDeviceLink */
                         OsinfoEntity *ent2, /* OsinfoDevice or OsinfoDevice Link */
                         OsinfoFilter *filter,
                         gboolean include_unsupported)
{
    gboolean ret = FALSE;
    gboolean unsupported = FALSE;

    if (filter == NULL || osinfo_filter_matches(filter, ent2))
        ret = TRUE;

    if (!osinfo_entity_get_param_value_boolean_with_default(ent1,
                                                            OSINFO_DEVICELINK_PROP_SUPPORTED,
                                                            TRUE))
        unsupported = TRUE;

    if (ret && unsupported && !include_unsupported)
        ret = FALSE;

    return ret;
}


static OsinfoDeviceList *
osinfo_os_get_devices_internal(OsinfoOs *os,
                               OsinfoFilter *filter,
                               gboolean include_unsupported)
{
    OsinfoDeviceList *newList;
    GList *tmp = NULL;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    newList = osinfo_devicelist_new();
    tmp = os->priv->deviceLinks;

    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);
        OsinfoDevice *dev = osinfo_devicelink_get_target(devlink);

        if (add_entity_to_list_check(OSINFO_ENTITY(devlink),
                                     OSINFO_ENTITY(dev),
                                     filter,
                                     include_unsupported))
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(dev));

        tmp = tmp->next;
    }

    return newList;
}


/**
 * osinfo_os_get_devices:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter
 *
 * Returns: (transfer full): A list of devices
 */
OsinfoDeviceList *osinfo_os_get_devices(OsinfoOs *os, OsinfoFilter *filter)
{
    return osinfo_os_get_devices_internal(os, filter, FALSE);
}

struct GetAllDevicesData {
    OsinfoFilter *filter;
    OsinfoDeviceList *devices;
};

static void get_all_devices_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoDeviceList *devices;
    OsinfoList *tmp_list;
    struct GetAllDevicesData *foreach_data = (struct GetAllDevicesData *)user_data;

    g_return_if_fail(OSINFO_IS_OS(product));

    devices = osinfo_os_get_devices_internal(OSINFO_OS(product),
                                             foreach_data->filter,
                                             TRUE);
    tmp_list = osinfo_list_new_union(OSINFO_LIST(foreach_data->devices),
                                     OSINFO_LIST(devices));
    g_object_unref(foreach_data->devices);
    g_object_unref(devices);
    foreach_data->devices = OSINFO_DEVICELIST(tmp_list);
}


static OsinfoDeviceLinkList *
osinfo_os_get_all_device_links_internal(OsinfoOs *os,
                                        OsinfoFilter *filter,
                                        gboolean include_unsupported);


/**
 * osinfo_os_get_all_devices:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter but unlike osinfo_os_get_devices
 * this function also retrieves devices from all derived and cloned operating
 * systems.
 *
 * Returns: (transfer full): A list of devices
 *
 * Since: 0.0.5
 */
OsinfoDeviceList *osinfo_os_get_all_devices(OsinfoOs *os, OsinfoFilter *filter)
{
    struct GetAllDevicesData foreach_data = {
        .filter = filter,
        .devices = osinfo_devicelist_new()
    };
    OsinfoDeviceLinkList *devlinks;
    OsinfoDeviceLinkList *unsupported_devlinks;
    OsinfoDeviceList *unsupported_devs;
    OsinfoDeviceList *new_list;
    OsinfoFilter *unsupported_filter;
    GList *list, *unsupported_list;
    GList *it;

    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_all_devices_cb,
                                   &foreach_data);

    devlinks = osinfo_os_get_all_device_links_internal(os, filter, TRUE);

    unsupported_filter = osinfo_filter_new();
    osinfo_filter_add_constraint(unsupported_filter,
                                 OSINFO_DEVICELINK_PROP_SUPPORTED,
                                 "false");

    unsupported_devlinks = OSINFO_DEVICELINKLIST
        (osinfo_list_new_filtered(OSINFO_LIST(devlinks), unsupported_filter));

    unsupported_devs = osinfo_devicelinklist_get_devices(unsupported_devlinks, NULL);

    list = osinfo_list_get_elements(OSINFO_LIST(foreach_data.devices));
    unsupported_list = osinfo_list_get_elements(OSINFO_LIST(unsupported_devs));

    new_list = osinfo_devicelist_new();
    for (it = list; it != NULL; it = it->next) {
        OsinfoDevice *dev = OSINFO_DEVICE(it->data);
        if (g_list_find(unsupported_list, dev))
            continue;

        osinfo_list_add(OSINFO_LIST(new_list), OSINFO_ENTITY(dev));
    }

    g_list_free(list);
    g_list_free(unsupported_list);
    g_object_unref(devlinks);
    g_object_unref(unsupported_devlinks);
    g_object_unref(unsupported_devs);
    g_object_unref(unsupported_filter);
    g_object_unref(foreach_data.devices);

    return new_list;
}

/**
 * osinfo_os_get_devices_by_property:
 * @os: an operating system
 * @property: the property of interest
 * @value: the required value of property @property
 * @inherited: Should devices from inherited and cloned OSs be included in the
 * search.
 *
 * A utility function that gets devices found from the list of devices
 * @os supports, for which the value of @property is @value.
 *
 * Returns: (transfer full): The found devices
 *
 * Since: 0.0.6
 */
OsinfoDeviceList *osinfo_os_get_devices_by_property(OsinfoOs *os,
                                                    const gchar *property,
                                                    const gchar *value,
                                                    gboolean inherited)
{
    OsinfoDeviceList *devices;
    OsinfoFilter *filter;

    filter = osinfo_filter_new();
    osinfo_filter_add_constraint(filter, property, value);

    if (inherited)
        devices = osinfo_os_get_all_devices(os, filter);
    else
        devices = osinfo_os_get_devices(os, filter);
    g_object_unref(filter);

    return devices;
}


static OsinfoDeviceLinkList *
osinfo_os_get_device_links_internal(OsinfoOs *os,
                                    OsinfoFilter *filter,
                                    gboolean include_unsupported)
{
    OsinfoDeviceLinkList *newList;
    GList *tmp = NULL;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    newList = osinfo_devicelinklist_new();
    tmp = os->priv->deviceLinks;

    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);

        if (add_entity_to_list_check(OSINFO_ENTITY(devlink),
                                     OSINFO_ENTITY(devlink),
                                     filter,
                                     include_unsupported))
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(devlink));

        tmp = tmp->next;
    }

    return newList;
}


/**
 * osinfo_os_get_device_links:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devices matching a given filter. The filter
 * matches against the links, not the devices.
 *
 * Returns: (transfer full): A list of device links
 */
OsinfoDeviceLinkList *osinfo_os_get_device_links(OsinfoOs *os, OsinfoFilter *filter)
{
    return osinfo_os_get_device_links_internal(os, filter, FALSE);
}


struct GetAllDeviceLinksData {
    OsinfoFilter *filter;
    OsinfoDeviceLinkList *device_links;
};

static void get_all_device_links_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoDeviceLinkList *device_links;
    OsinfoList *tmp_list;
    struct GetAllDeviceLinksData *foreach_data;

    g_return_if_fail(OSINFO_IS_OS(product));

    foreach_data = (struct GetAllDeviceLinksData *)user_data;
    device_links = osinfo_os_get_device_links_internal(OSINFO_OS(product),
                                                       foreach_data->filter,
                                                       TRUE);
    tmp_list = osinfo_list_new_union(OSINFO_LIST(foreach_data->device_links),
                                     OSINFO_LIST(device_links));
    g_object_unref(foreach_data->device_links);
    g_object_unref(device_links);
    foreach_data->device_links = OSINFO_DEVICELINKLIST(tmp_list);
}

static OsinfoDeviceLinkList *
osinfo_os_get_all_device_links_internal(OsinfoOs *os,
                                        OsinfoFilter *filter,
                                        gboolean include_unsupported)
{
    struct GetAllDeviceLinksData foreach_data = {
        .filter = filter,
        .device_links = osinfo_devicelinklist_new()
    };
    OsinfoDeviceLinkList *devlinks;
    GList *list, *it;

    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_all_device_links_cb,
                                   &foreach_data);

    if (include_unsupported)
        return foreach_data.device_links;

    devlinks = osinfo_devicelinklist_new();

    list = osinfo_list_get_elements(OSINFO_LIST(foreach_data.device_links));
    for (it = list; it != NULL; it = it->next) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(it->data);

        if (!osinfo_entity_get_param_value_boolean_with_default(OSINFO_ENTITY(devlink),
                                                                OSINFO_DEVICELINK_PROP_SUPPORTED,
                                                                TRUE))
            continue;

        osinfo_list_add(OSINFO_LIST(devlinks), OSINFO_ENTITY(devlink));
    }

    g_object_unref(foreach_data.device_links);
    g_list_free(list);

    return devlinks;
}

/**
 * osinfo_os_get_all_device_links:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional device property filter
 *
 * Get all devicelinks matching a given filter but unlike
 * osinfo_os_get_device_links this function also retrieves devices from all
 * derived and cloned operating systems.
 *
 * Returns: (transfer full): A list of OsinfoDeviceLink
 *
 * Since: 1.3.0
 */
OsinfoDeviceLinkList *osinfo_os_get_all_device_links(OsinfoOs *os, OsinfoFilter *filter)
{
    return osinfo_os_get_all_device_links_internal(os, filter, FALSE);
}

/**
 * osinfo_os_add_device:
 * @os: an operating system
 * @dev: (transfer none): the device to associate with
 *
 * Associated a device with an operating system.  The
 * returned #OsinfoDeviceLink can be used to record
 * extra metadata against the link
 *
 * Returns: (transfer none): the device association
 */
OsinfoDeviceLink *osinfo_os_add_device(OsinfoOs *os, OsinfoDevice *dev)
{
    OsinfoDeviceLink *devlink;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(OSINFO_IS_DEVICE(dev), NULL);

    devlink = osinfo_devicelink_new(dev);
    os->priv->deviceLinks = g_list_append(os->priv->deviceLinks, devlink);

    return devlink;
}

/**
 * osinfo_os_get_family:
 * @os: an #OsinfoOs
 *
 * Retrieves the generic family the OS @os belongs to, based upon its kernel,
 * for example linux, winnt, solaris, freebsd etc.
 *
 * Returns: (transfer none): the family of this os
 */
const gchar *osinfo_os_get_family(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    return osinfo_entity_get_param_value(OSINFO_ENTITY(os), "family");
}

/**
 * osinfo_os_get_distro:
 * @os: an #OsinfoOs
 *
 * Retrieves the generic family the OS @os belongs to, for example fedora,
 * ubuntu, windows, solaris, freebsd etc.
 *
 * Returns: (transfer none): the distro of this os
 */
const gchar *osinfo_os_get_distro(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    return osinfo_entity_get_param_value(OSINFO_ENTITY(os), "distro");
}

/**
 * osinfo_os_get_release_status:
 * @os: an #OsinfoOs
 *
 * Use this to determine the release status of the @os.
 *
 * Returns: (type OsinfoReleaseStatus): release status of @os.
 *
 * Since: 0.2.9
 */
int osinfo_os_get_release_status(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), OSINFO_RELEASE_STATUS_RELEASED);

    return osinfo_entity_get_param_value_enum
        (OSINFO_ENTITY(os),
         OSINFO_OS_PROP_RELEASE_STATUS,
         OSINFO_TYPE_RELEASE_STATUS,
         OSINFO_RELEASE_STATUS_RELEASED);
}

/**
 * osinfo_os_get_media_list:
 * @os: an operating system
 *
 * Get all installation medias associated with operating system @os.
 *
 * Returns: (transfer full): A list of medias
 */
OsinfoMediaList *osinfo_os_get_media_list(OsinfoOs *os)
{
    OsinfoMediaList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_medialist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->medias));

    return newList;
}

/**
 * osinfo_os_add_media:
 * @os: an operating system
 * @media: (transfer none): the media to add
 *
 * Adds installation media @media to operating system @os.
 */
void osinfo_os_add_media(OsinfoOs *os, OsinfoMedia *media)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_MEDIA(media));

    osinfo_list_add(OSINFO_LIST(os->priv->medias), OSINFO_ENTITY(media));
    osinfo_media_set_os(media, os);
}

/**
 * osinfo_os_get_tree_list:
 * @os: an operating system
 *
 * Get all installation trees associated with operating system @os.
 *
 * Returns: (transfer full): A list of trees
 */
OsinfoTreeList *osinfo_os_get_tree_list(OsinfoOs *os)
{
    OsinfoTreeList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_treelist_new();
    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->trees));

    return newList;
}

/**
 * osinfo_os_add_tree:
 * @os: an operating system
 * @tree: (transfer none): the tree to add
 *
 * Adds installation tree @tree to operating system @os.
 */
void osinfo_os_add_tree(OsinfoOs *os, OsinfoTree *tree)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_TREE(tree));

    osinfo_list_add(OSINFO_LIST(os->priv->trees), OSINFO_ENTITY(tree));
    osinfo_tree_set_os(tree, os);
}

/**
 * osinfo_os_get_image_list:
 * @os: an operating system
 *
 * Get all installed images associated with operating system @os.
 *
 * Returns: (transfer full): A list of images
 *
 * Since: 1.3.0
 */
OsinfoImageList *osinfo_os_get_image_list(OsinfoOs *os)
{
    OsinfoImageList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_imagelist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->images));

    return newList;
}

/**
 * osinfo_os_add_image:
 * @os: an operating system
 * @image: (transfer none): the image to add
 *
 * Adds an installed image @image to operating system @os.
 *
 * Since: 1.3.0
 */
void osinfo_os_add_image(OsinfoOs *os, OsinfoImage *image)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_IMAGE(image));

    osinfo_list_add(OSINFO_LIST(os->priv->images), OSINFO_ENTITY(image));
    osinfo_image_set_os(image, os);
}

/**
 * osinfo_os_get_variant_list:
 * @os: an operating system
 *
 * Gets all known variants of operating system @os.
 *
 * Returns: (transfer full): A list of variants
 *
 * Since: 0.2.9
 */
OsinfoOsVariantList *osinfo_os_get_variant_list(OsinfoOs *os)
{
    OsinfoOsVariantList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_os_variantlist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->variants));

    return newList;
}

/**
 * osinfo_os_add_variant:
 * @os: an operating system
 * @variant: (transfer none): the variant to add
 *
 * Adds a variant @variant to operating system @os.
 *
 * Since: 0.2.9
 */
void osinfo_os_add_variant(OsinfoOs *os, OsinfoOsVariant *variant)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_OS_VARIANT(variant));

    osinfo_list_add(OSINFO_LIST(os->priv->variants), OSINFO_ENTITY(variant));
}

struct GetAllResourcesData {
    OsinfoOs *os;
    OsinfoResourcesList *resourceslist;
    OsinfoResourcesList *(*get_resourceslist)(OsinfoOs *);
};

static void get_all_resources_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoResourcesList *resourceslist;
    struct GetAllResourcesData *foreach_data = (struct GetAllResourcesData *)user_data;
    gint original_resourceslist_len;
    gint resourceslist_len;
    gint i;

    g_return_if_fail(OSINFO_IS_OS(product));

    if (OSINFO_OS(product) == foreach_data->os)
        return;

    original_resourceslist_len = osinfo_list_get_length(OSINFO_LIST(foreach_data->resourceslist));

    resourceslist = foreach_data->get_resourceslist(OSINFO_OS(product));
    resourceslist_len = osinfo_list_get_length(OSINFO_LIST(resourceslist));

    for (i = 0; i < original_resourceslist_len; i++) {
        OsinfoResources *original_resources;
        const gchar *original_arch;
        gint original_n_cpus;
        gint64 original_cpu;
        gint64 original_ram;
        gint64 original_storage;
        gint j;

        original_resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(foreach_data->resourceslist), i));

        if (!osinfo_resources_get_inherit(original_resources))
            continue;

        original_arch = osinfo_resources_get_architecture(original_resources);
        original_n_cpus = osinfo_resources_get_n_cpus(original_resources);
        original_cpu = osinfo_resources_get_cpu(original_resources);
        original_ram = osinfo_resources_get_ram(original_resources);
        original_storage = osinfo_resources_get_storage(original_resources);

        for (j = 0; j < resourceslist_len; j++) {
            OsinfoResources *resources;
            const gchar *arch;
            gint n_cpus;
            gint64 cpu;
            gint64 ram;
            gint64 storage;

            resources = OSINFO_RESOURCES(osinfo_list_get_nth(OSINFO_LIST(resourceslist), j));
            arch = osinfo_resources_get_architecture(resources);
            n_cpus = osinfo_resources_get_n_cpus(resources);
            cpu = osinfo_resources_get_cpu(resources);
            ram = osinfo_resources_get_ram(resources);
            storage = osinfo_resources_get_storage(resources);

            if (!g_str_equal(original_arch, arch))
                continue;

            if (original_n_cpus == -1)
                osinfo_resources_set_n_cpus(original_resources, n_cpus);

            if (original_cpu == -1)
                osinfo_resources_set_cpu(original_resources, cpu);

            if (original_ram == -1)
                osinfo_resources_set_ram(original_resources, ram);

            if (original_storage == -1)
                osinfo_resources_set_storage(original_resources, storage);
        }
    }

    g_object_unref(resourceslist);
}


static OsinfoResourcesList *
osinfo_os_get_resources_internal(OsinfoOs *os,
                                 OsinfoResourcesList *(*get_resourceslist)(OsinfoOs *))
{
    struct GetAllResourcesData foreach_data = {
        .os = os,
        .resourceslist = get_resourceslist(os),
        .get_resourceslist = get_resourceslist
    };

    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_all_resources_cb,
                                   &foreach_data);

    return foreach_data.resourceslist;
}

/**
 * osinfo_os_get_minimum_resources_without_inheritance:
 * @os: an operating system
 *
 * Get the list of minimum required resources for the operating system @os.
 *
 * Mind that this method is *private*!
 *
 * Returns: (transfer full): A list of resources
 */
static OsinfoResourcesList *
osinfo_os_get_minimum_resources_without_inheritance(OsinfoOs *os)
{
    OsinfoResourcesList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_resourceslist_new();

    osinfo_list_add_all(OSINFO_LIST(newList), OSINFO_LIST(os->priv->minimum));

    return newList;
}

/**
 * osinfo_os_get_minimum_resources:
 * @os: an operating system
 *
 * Get the list of minimum required resources for the operating system @os.
 *
 * Returns: (transfer full): A list of resources
 */
OsinfoResourcesList *osinfo_os_get_minimum_resources(OsinfoOs *os)
{
    return osinfo_os_get_resources_internal
            (os, osinfo_os_get_minimum_resources_without_inheritance);
}

/**
 * osinfo_os_get_maximum_resources_without_inheritance:
 * @os: an operating system
 *
 * Get the list of maximum resources for the operating system @os.
 *
 * Mind that this method is *private*!
 *
 * Returns: (transfer full): A list of resources
 */
static OsinfoResourcesList *
osinfo_os_get_maximum_resources_without_inheritance(OsinfoOs *os)
{
    OsinfoResourcesList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_resourceslist_new();

    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(os->priv->maximum));

    return newList;
}

/**
 * osinfo_os_get_maximum_resources:
 * @os: an operating system
 *
 * Get the list of maximum resources for the operating system @os.
 *
 * Returns: (transfer full): A list of resources
 *
 * Since: 1.3.0
 */
OsinfoResourcesList *osinfo_os_get_maximum_resources(OsinfoOs *os)
{
    return osinfo_os_get_resources_internal
            (os, osinfo_os_get_maximum_resources_without_inheritance);
}

/**
 * osinfo_os_get_recommended_resources_without_inheritance:
 * @os: an operating system
 *
 * Get the list of recommended resources for the operating system @os.
 *
 * Mind that this method is *private*!
 *
 * Returns: (transfer full): A list of resources
 */
static OsinfoResourcesList *
osinfo_os_get_recommended_resources_without_inheritance(OsinfoOs *os)
{
    OsinfoResourcesList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_resourceslist_new();

    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(os->priv->recommended));

    return newList;
}

/**
 * osinfo_os_get_recommended_resources:
 * @os: an operating system
 *
 * Get the list of recommended resources for the operating system @os.
 *
 * Returns: (transfer full): A list of resources
 */
OsinfoResourcesList *osinfo_os_get_recommended_resources(OsinfoOs *os)
{
    return osinfo_os_get_resources_internal
            (os, osinfo_os_get_recommended_resources_without_inheritance);
}

/**
 * osinfo_os_get_network_install_resources_without_inheritance:
 * @os: an operating system
 *
 * Get the list of resources needed for network installing an operating system
 * @os.
 *
 * Mind that this method is *private*!
 *
 * Returns: (transfer full): A list of resources
 */
static OsinfoResourcesList *
osinfo_os_get_network_install_resources_without_inheritance(OsinfoOs *os)
{
    OsinfoResourcesList *newList;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    newList = osinfo_resourceslist_new();

    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(os->priv->network_install));

    return newList;
}

/**
 * osinfo_os_get_network_install_resources:
 * @os: an operating system
 *
 * Get the list of resources needed for network installing an operating system
 * @os.
 *
 * Returns: (transfer full): A list of resources
 *
 * Since: 1.3.0
 */
OsinfoResourcesList *osinfo_os_get_network_install_resources(OsinfoOs *os)
{
    return osinfo_os_get_resources_internal
            (os, osinfo_os_get_network_install_resources_without_inheritance);
}

/**
 * osinfo_os_add_minimum_resources:
 * @os: an operating system
 * @resources: (transfer none): the resources to add
 *
 * Adds @resources to list of minimum resources of operating system @os.
 */
void osinfo_os_add_minimum_resources(OsinfoOs *os, OsinfoResources *resources)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_RESOURCES(resources));

    osinfo_list_add(OSINFO_LIST(os->priv->minimum), OSINFO_ENTITY(resources));
}

/**
 * osinfo_os_add_recommended_resources:
 * @os: an operating system
 * @resources: (transfer none): the resources to add
 *
 * Adds @resources to list of recommended resources of operating system @os.
 */
void osinfo_os_add_recommended_resources(OsinfoOs *os,
                                         OsinfoResources *resources)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_RESOURCES(resources));

    osinfo_list_add(OSINFO_LIST(os->priv->recommended),
                    OSINFO_ENTITY(resources));
}

/**
 * osinfo_os_add_maximum_resources:
 * @os: an operating system
 * @resources: (transfer none): the resources to add
 *
 * Adds @resources to list of maximum resources of operating system @os.
 *
 * Since: 1.3.0
 */
void osinfo_os_add_maximum_resources(OsinfoOs *os,
                                     OsinfoResources *resources)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_RESOURCES(resources));

    osinfo_list_add(OSINFO_LIST(os->priv->maximum),
                    OSINFO_ENTITY(resources));
}

/**
 * osinfo_os_add_network_install_resources:
 * @os: an operating system
 * @resources: (transfer none): the resources to add
 *
 * Adds @resources to list of resources needed for network installing an
 * operating system @os.
 *
 * Since: 1.3.0
 */
void osinfo_os_add_network_install_resources(OsinfoOs *os,
                                     OsinfoResources *resources)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_RESOURCES(resources));

    osinfo_list_add(OSINFO_LIST(os->priv->network_install),
                    OSINFO_ENTITY(resources));
}

/**
 * osinfo_os_find_install_script:
 * @os:      an operating system
 * @profile: the install script profile that must be either
 * OSINFO_INSTALL_SCRIPT_PROFILE_DESKTOP or OSINFO_INSTALL_SCRIPT_PROFILE_JEOS
 *
 * Returns: (transfer none): A new #OsinfoInstallScript for the @os @profile
 *
 * Since: 0.2.0
 */
OsinfoInstallScript *osinfo_os_find_install_script(OsinfoOs *os, const gchar *profile)
{
    GList *scripts;
    GList *tmp;
    OsinfoInstallScript *script = NULL;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(profile != NULL, NULL);

    scripts = osinfo_list_get_elements(OSINFO_LIST(os->priv->scripts));
    tmp = scripts;

    while (tmp) {
        script = tmp->data;
        if (g_str_equal(profile, osinfo_install_script_get_profile(script)))
            break;

        tmp = tmp->next;
        script = NULL;
    }

    g_list_free(scripts);

    return script;
}


/**
 * osinfo_os_get_install_script_list:
 * @os: an operating system
 *
 * Returns: (transfer full): a list of the install scripts for the specified os
 *
 * Since: 0.2.0
 */
OsinfoInstallScriptList *osinfo_os_get_install_script_list(OsinfoOs *os)
{
    OsinfoList *new_list;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    new_list = osinfo_list_new_copy(OSINFO_LIST(os->priv->scripts));

    return OSINFO_INSTALL_SCRIPTLIST(new_list);
}


/**
 * osinfo_os_add_install_script:
 * @os: an operating system
 * @script: (transfer none): the install script to add
 *
 * Adds @script to the list of scripts of operating system @os.
 *
 * Since: 0.2.0
 */
void osinfo_os_add_install_script(OsinfoOs *os, OsinfoInstallScript *script)
{
    g_return_if_fail(OSINFO_IS_OS(os));

    osinfo_list_add(OSINFO_LIST(os->priv->scripts), OSINFO_ENTITY(script));
}

/**
 * osinfo_os_get_device_drivers:
 * @os: an operating system
 *
 * Gets list of all available device drivers for OS @os.
 *
 * Returns: (transfer none): A list of device drivers
 *
 * Since: 0.2.2
 */
OsinfoDeviceDriverList *osinfo_os_get_device_drivers(OsinfoOs *os)
{
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    return os->priv->device_drivers;
}

static gint osinfo_device_drivers_sort_by_priority(gconstpointer a,
                                                   gconstpointer b)
{
    OsinfoDeviceDriver *adriver = OSINFO_DEVICE_DRIVER((gpointer) a);
    OsinfoDeviceDriver *bdriver = OSINFO_DEVICE_DRIVER((gpointer) b);

    return osinfo_device_driver_get_priority(bdriver) - osinfo_device_driver_get_priority(adriver);
}

/**
 * osinfo_os_get_device_drivers_prioritized:
 * @os: an operating system
 *
 * Gets list of the highest priority device drivers for OS @os.
 *
 * Returns: (transfer full): A list of device drivers
 *
 * Since: 1.7.0
 */
OsinfoDeviceDriverList *osinfo_os_get_device_drivers_prioritized(OsinfoOs *os)
{
    OsinfoDeviceDriverList *device_drivers;
    GList *list, *sorted, *l;
    gint priority = -1;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    list = osinfo_list_get_elements(OSINFO_LIST(os->priv->device_drivers));
    sorted = g_list_sort(list, osinfo_device_drivers_sort_by_priority);

    device_drivers = osinfo_device_driverlist_new();
    for (l = sorted; l != NULL; l = l->next) {
        if (priority == -1)
            priority = osinfo_device_driver_get_priority(OSINFO_DEVICE_DRIVER(l->data));

        if (priority != osinfo_device_driver_get_priority(OSINFO_DEVICE_DRIVER(l->data)))
            break;

        osinfo_list_add(OSINFO_LIST(device_drivers), OSINFO_ENTITY(l->data));
    }
    g_list_free(sorted);

    return device_drivers;
}

/**
 * osinfo_os_add_device_driver:
 * @os: an operating system
 * @driver: (transfer none): the device driver to add
 *
 * Adds @driver to the list of device drivers of operating system @os.
 *
 * Since: 0.2.2
 */
void osinfo_os_add_device_driver(OsinfoOs *os, OsinfoDeviceDriver *driver)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_DEVICE_DRIVER(driver));

    osinfo_list_add(OSINFO_LIST(os->priv->device_drivers),
                    OSINFO_ENTITY(driver));
}

struct GetKernelURLArgumentData {
    const gchar *kernel_url_arg;
};

static void get_kernel_url_argument_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoOs *os = OSINFO_OS(product);
    struct GetKernelURLArgumentData *foreach_data = user_data;

    g_return_if_fail(OSINFO_IS_OS(os));

    if (foreach_data->kernel_url_arg != NULL)
        return;

    foreach_data->kernel_url_arg =
            osinfo_entity_get_param_value(OSINFO_ENTITY(os),
                                          OSINFO_OS_PROP_KERNEL_URL_ARGUMENT);
}

/**
 * osinfo_os_get_kernel_url_argument:
 * @os: an operating system
 *
 * Gets the argument expected to be passed to the kernel command line when
 * performing a tree based installation.
 *
 * Returns: (transfer none): the kernel url argument, if present. Otherwise,
 * NULL.
 */
const gchar *osinfo_os_get_kernel_url_argument(OsinfoOs *os)
{
    struct GetKernelURLArgumentData foreach_data = {
        .kernel_url_arg = NULL
    };
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);


    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_kernel_url_argument_cb,
                                   &foreach_data);

    return foreach_data.kernel_url_arg;
}

struct GetImageUsernameData {
    const gchar *username;
};

static void get_cloud_image_username_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoOs *os = OSINFO_OS(product);
    struct GetImageUsernameData *foreach_data = user_data;

    g_return_if_fail(OSINFO_IS_OS(os));

    if (foreach_data->username != NULL)
        return;

    foreach_data->username =
            osinfo_entity_get_param_value(OSINFO_ENTITY(os),
                                          OSINFO_OS_PROP_CLOUD_IMAGE_USERNAME);
}

/**
 * osinfo_os_get_cloud_image_username:
 * @os: an operating system
 *
 * Gets the username expected to be passed to the cloud image when performing
 * installation.
 *
 * Returns: (transfer none): the username, if present. Otherwise, NULL.
 */
const gchar *osinfo_os_get_cloud_image_username(OsinfoOs *os)
{
    struct GetImageUsernameData foreach_data = {
        .username = NULL
    };
    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);

    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_cloud_image_username_cb,
                                   &foreach_data);

    return foreach_data.username;
}

struct GetAllFirmwaresData {
    OsinfoFilter *filter;
    OsinfoFilter *unsupported_filter;
    OsinfoFirmwareList *firmwares;
    OsinfoFirmwareList *unsupported_firmwares;
};

static OsinfoList *
get_all_unsupported_firmwares(OsinfoList *firmwares,
                              OsinfoFilter *unsupported_filter,
                              OsinfoList *unsupported_firmwares)
{
    OsinfoList *list;
    OsinfoList *union_list;

    list = osinfo_list_new_filtered(firmwares, unsupported_filter);
    union_list = osinfo_list_new_union(list, unsupported_firmwares);
    g_object_unref(list);

    return union_list;
}

static void
filter_out_unsupported_firmwares(OsinfoList *firmwares,
                                 OsinfoFilter *filter,
                                 OsinfoList *unsupported_firmwares,
                                 OsinfoList *out)
{
    gsize len, i;
    gsize unsupported_len, j;

    len = osinfo_list_get_length(firmwares);
    unsupported_len = osinfo_list_get_length(unsupported_firmwares);
    for (i = 0; i < len; i++) {
        OsinfoFirmware *firmware;
        const gchar *arch;
        const gchar *type;
        gboolean requested_by_caller;
        gboolean supported;

        firmware = OSINFO_FIRMWARE(osinfo_list_get_nth(firmwares, i));
        arch = osinfo_firmware_get_architecture(firmware);
        type = osinfo_firmware_get_firmware_type(firmware);
        requested_by_caller = TRUE;
        supported = TRUE;

        /*
         * In case there's no filter, the firmware can be considered as
         * requested_by_caller.
         *
         * In case there's a filter, let's ensure the firmware matches the
         * filter before adding it to the final firmwares' list.
         */
        if (filter != NULL &&
            !osinfo_filter_matches(filter, OSINFO_ENTITY(firmware)))
            requested_by_caller = FALSE;

        /*
         * In case the firmware is valid to be added, let's ensure the firmware
         * is supported by the OS.
         */
        if (requested_by_caller) {
            for (j = 0; j < unsupported_len; j++) {
                OsinfoFirmware *unsupported;
                const gchar *unsupported_arch;
                const gchar *unsupported_type;

                unsupported = OSINFO_FIRMWARE(
                        osinfo_list_get_nth(unsupported_firmwares, j));
                unsupported_arch = osinfo_firmware_get_architecture(unsupported);
                unsupported_type = osinfo_firmware_get_firmware_type(unsupported);


                if (g_str_equal(arch, unsupported_arch) &&
                    g_str_equal(type, unsupported_type)) {
                    supported = FALSE;
                    break;
                }
            }
        }

        /*
         * Only add the firmware to the final list of firmwares in case it is
         * requested by the caller and supported
         */
        if (requested_by_caller &&
            supported)
            osinfo_list_add(out, OSINFO_ENTITY(firmware));
    }
}

static void get_all_supported_firmwares_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoFirmwareList *unsupported_firmwares;
    OsinfoOs *os = OSINFO_OS(product);
    struct GetAllFirmwaresData *foreach_data = user_data;

    g_return_if_fail(OSINFO_IS_OS(os));

    unsupported_firmwares = OSINFO_FIRMWARELIST(
            get_all_unsupported_firmwares(OSINFO_LIST(os->priv->firmwares),
                                          foreach_data->unsupported_filter,
                                          OSINFO_LIST(foreach_data->unsupported_firmwares)));

    filter_out_unsupported_firmwares(OSINFO_LIST(os->priv->firmwares),
                                     foreach_data->filter,
                                     OSINFO_LIST(unsupported_firmwares),
                                     OSINFO_LIST(foreach_data->firmwares));

    g_object_unref(foreach_data->unsupported_firmwares);
    foreach_data->unsupported_firmwares = unsupported_firmwares;
}

/**
 * osinfo_os_get_firmware_list:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional firmware property filter
 *
 * Get all the supported firmwares matching a given filter
 *
 * Returns: (transfer full): A list of firmwares
 *
 * Since: 1.7.0
 */
OsinfoFirmwareList *osinfo_os_get_firmware_list(OsinfoOs *os, OsinfoFilter *filter)
{
    struct GetAllFirmwaresData foreach_data;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    foreach_data.filter = filter;
    foreach_data.unsupported_filter = NULL;
    foreach_data.firmwares = osinfo_firmwarelist_new();
    foreach_data.unsupported_firmwares = osinfo_firmwarelist_new();
    foreach_data.unsupported_filter = osinfo_filter_new();

    osinfo_filter_add_constraint(foreach_data.unsupported_filter,
                                 OSINFO_FIRMWARE_PROP_SUPPORTED,
                                 "false");

    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_all_supported_firmwares_cb,
                                   &foreach_data);

    g_object_unref(foreach_data.unsupported_filter);
    g_object_unref(foreach_data.unsupported_firmwares);

    return foreach_data.firmwares;
}

static void get_all_firmwares_cb(OsinfoProduct *product, gpointer user_data)
{
    OsinfoFirmwareList *out;
    OsinfoOs *os = OSINFO_OS(product);
    gsize len, i;
    gsize current_list_len, j;
    struct GetAllFirmwaresData *foreach_data = user_data;

    g_return_if_fail(OSINFO_IS_OS(os));

    out = OSINFO_FIRMWARELIST(osinfo_list_new_copy(OSINFO_LIST(foreach_data->firmwares)));

    len = osinfo_list_get_length(OSINFO_LIST(os->priv->firmwares));
    current_list_len = osinfo_list_get_length(OSINFO_LIST(foreach_data->firmwares));
    for (i = 0; i < len; i++) {
        OsinfoFirmware *firmware;
        const gchar *arch;
        const gchar *type;
        gboolean requested_by_caller;
        gboolean already_in = FALSE;

        firmware = OSINFO_FIRMWARE(osinfo_list_get_nth(OSINFO_LIST(os->priv->firmwares), i));
        arch = osinfo_firmware_get_architecture(firmware);
        type = osinfo_firmware_get_firmware_type(firmware);
        requested_by_caller = TRUE;

        /*
         * In case there's no filter, the firmware can be considered as
         * requested_by_caller.
         *
         * In case there's a filter, let's ensure the firmware matches the
         * filter before adding it to the final firmwares' list.
         */
        if (foreach_data->filter != NULL &&
            !osinfo_filter_matches(foreach_data->filter, OSINFO_ENTITY(firmware)))
            requested_by_caller = FALSE;

        /*
         * In case the firmware is valid to be added, let's ensure the firmware
         * is not already part of the firmware list.
         *
         * We can safely do this check here as the _foreach() function ensures
         * we iterate through the latest child all the way back to the parent.
         */
        if (requested_by_caller) {
            for (j = 0; j < current_list_len; j++) {
                OsinfoFirmware *in;
                const gchar *in_arch;
                const gchar *in_type;

                in = OSINFO_FIRMWARE(
                        osinfo_list_get_nth(OSINFO_LIST(foreach_data->firmwares), j));
                in_arch = osinfo_firmware_get_architecture(in);
                in_type = osinfo_firmware_get_firmware_type(in);

                if (g_str_equal(arch, in_arch) &&
                    g_str_equal(type, in_type)) {
                    already_in = TRUE;
                    break;
                }
            }
        }

        /*
         * Only add the firmware to the final list of firmwares in case it is
         * requested by the caller and a newer entry is not yet part of the
         * list.
         */
        if (requested_by_caller && !already_in)
            osinfo_list_add(OSINFO_LIST(out), OSINFO_ENTITY(firmware));
    }

    g_object_unref(foreach_data->firmwares);
    foreach_data->firmwares = out;
}

/**
 * osinfo_os_get_complete_firmware_list:
 * @os: an operating system
 * @filter: (allow-none)(transfer none): an optional firmware property filter
 *
 * Get the complete firmwares matching a given filter, including the non-supported ones.
 *
 * Returns: (transfer full): A list of firmwares
 *
 * Since: 1.10.0
 */
OsinfoFirmwareList *osinfo_os_get_complete_firmware_list(OsinfoOs *os, OsinfoFilter *filter)
{
    struct GetAllFirmwaresData foreach_data;

    g_return_val_if_fail(OSINFO_IS_OS(os), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    foreach_data.filter = filter;
    foreach_data.firmwares = osinfo_firmwarelist_new();

    osinfo_product_foreach_related(OSINFO_PRODUCT(os),
                                   OSINFO_PRODUCT_FOREACH_FLAG_DERIVES_FROM |
                                   OSINFO_PRODUCT_FOREACH_FLAG_CLONES,
                                   get_all_firmwares_cb,
                                   &foreach_data);

    return foreach_data.firmwares;
}

/**
 * osinfo_os_add_firmware:
 * @os: an operating system
 * @firmware: (transfer none): the firmware to add
 *
 * Adds @firmware to the list of firmwares of operating system @os.
 *
 * Since: 1.7.0
 */
void osinfo_os_add_firmware(OsinfoOs *os, OsinfoFirmware *firmware)
{
    g_return_if_fail(OSINFO_IS_OS(os));
    g_return_if_fail(OSINFO_IS_FIRMWARE(firmware));

    osinfo_list_add(OSINFO_LIST(os->priv->firmwares), OSINFO_ENTITY(firmware));
}
