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
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_deployment
 * @short_description: An virtualization deployment
 * @see_also: #OsinfoOs, #OsinfoDeployment
 *
 * #OsinfoDeployment is an entity representing an virtualization
 * deployment. Deployments have a list of supported devices
 */

struct _OsinfoDeploymentPrivate
{
    // Value: List of device_link structs
    GList *deviceLinks;

    OsinfoOs *os;
    OsinfoPlatform *platform;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDeployment, osinfo_deployment, OSINFO_TYPE_ENTITY);

enum {
    PROP_0,

    PROP_OS,
    PROP_PLATFORM,

    LAST_PROP
};
static GParamSpec *properties[LAST_PROP];

static void
osinfo_deployment_set_property(GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
    OsinfoDeployment *deployment = OSINFO_DEPLOYMENT(object);

    switch (property_id)
        {
        case PROP_OS:
            if (deployment->priv->os)
                g_object_unref(deployment->priv->os);
            deployment->priv->os = g_value_get_object(value);
            if (deployment->priv->os)
                g_object_ref(deployment->priv->os);
            break;
        case PROP_PLATFORM:
            if (deployment->priv->platform)
                g_object_unref(deployment->priv->platform);
            deployment->priv->platform = g_value_get_object(value);
            if (deployment->priv->platform)
                g_object_ref(deployment->priv->platform);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}

static void
osinfo_deployment_get_property(GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec)
{
    OsinfoDeployment *deployment = OSINFO_DEPLOYMENT(object);

    switch (property_id)
        {
        case PROP_OS:
            g_value_set_object(value, deployment->priv->os);
            break;
        case PROP_PLATFORM:
            g_value_set_object(value, deployment->priv->platform);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}



static void
osinfo_deployment_finalize(GObject *object)
{
    OsinfoDeployment *deployment = OSINFO_DEPLOYMENT(object);

    g_list_free_full(deployment->priv->deviceLinks, g_object_unref);

    g_object_unref(deployment->priv->os);
    g_object_unref(deployment->priv->platform);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_deployment_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_deployment_class_init(OsinfoDeploymentClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->set_property = osinfo_deployment_set_property;
    g_klass->get_property = osinfo_deployment_get_property;
    g_klass->finalize = osinfo_deployment_finalize;

    /**
     * OsinfoDeployment:os:
     *
     * The operating system to be deployed
     */
    properties[PROP_OS] = g_param_spec_object("os",
                                              "Os",
                                              _("Operating system"),
                                              OSINFO_TYPE_OS,
                                              G_PARAM_CONSTRUCT_ONLY |
                                              G_PARAM_READWRITE |
                                              G_PARAM_STATIC_STRINGS);
    /**
     * OsinfoDeployment:platform:
     *
     * The platform to deploy on
     */
    properties[PROP_PLATFORM] = g_param_spec_object("platform",
                                                    "Platform",
                                                    _("Virtualization platform"),
                                                    OSINFO_TYPE_PLATFORM,
                                                    G_PARAM_CONSTRUCT_ONLY |
                                                    G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(g_klass, LAST_PROP, properties);
}

static void
osinfo_deployment_init(OsinfoDeployment *deployment)
{
    deployment->priv = osinfo_deployment_get_instance_private(deployment);
    deployment->priv->deviceLinks = NULL;
}


/**
 * osinfo_deployment_new:
 * @id: the unique identifier
 * @os: the operating system to deploy
 * @platform: the platform to deploy on
 *
 * Create a new deployment entity
 *
 * Returns: (transfer full): A deployment entity
 */
OsinfoDeployment *osinfo_deployment_new(const gchar *id,
                                        OsinfoOs *os,
                                        OsinfoPlatform *platform)
{
    return g_object_new(OSINFO_TYPE_DEPLOYMENT,
                        "id", id,
                        "os", os,
                        "platform", platform,
                        NULL);
}


/**
 * osinfo_deployment_get_os:
 * @deployment: the deployment entity
 *
 * Get the operating system for the deployment
 *
 * Returns: (transfer none): an OS, or NULL
 */
OsinfoOs *osinfo_deployment_get_os(OsinfoDeployment *deployment)
{
    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);

    return deployment->priv->os;
}


/**
 * osinfo_deployment_get_platform:
 * @deployment: the deployment entity
 *
 * Get the platform for the deployment
 *
 * Returns: (transfer none): a platform, or NULL
 */
OsinfoPlatform *osinfo_deployment_get_platform(OsinfoDeployment *deployment)
{
    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);

    return deployment->priv->platform;
}


/**
 * osinfo_deployment_get_preferred_device:
 * @deployment: the deployment entity
 * @filter: (transfer none)(allow-none): a device metadata filter
 *
 * Get the preferred device matching a given filter
 *
 * Returns: (transfer none): a device, or NULL
 */
OsinfoDevice *osinfo_deployment_get_preferred_device(OsinfoDeployment *deployment, OsinfoFilter *filter)
{
    OsinfoDeviceLinkFilter *linkfilter;
    OsinfoDeviceLink *devlink;

    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);
    g_return_val_if_fail(OSINFO_IS_FILTER(filter), NULL);

    linkfilter = osinfo_devicelinkfilter_new(filter);
    devlink = osinfo_deployment_get_preferred_device_link(deployment, OSINFO_FILTER(linkfilter));
    if (devlink)
        return osinfo_devicelink_get_target(devlink);
    return NULL;
}


/**
 * osinfo_deployment_get_preferred_device_link:
 * @deployment: the deployment entity
 * @filter: (transfer none)(allow-none): a device metadata filter
 *
 * Get the preferred device link matching a given filter and platform.
 * The filter matches against attributes on the link, not the device.
 *
 * Returns: (transfer none): a device, or NULL
 */
OsinfoDeviceLink *osinfo_deployment_get_preferred_device_link(OsinfoDeployment *deployment, OsinfoFilter *filter)
{
    GList *tmp;

    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);
    g_return_val_if_fail(OSINFO_IS_FILTER(filter), NULL);

    tmp = deployment->priv->deviceLinks;

    // For each device in section list, apply filter. If filter passes, return device.
    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);

        if (!filter || osinfo_filter_matches(filter, OSINFO_ENTITY(devlink))) {
           return devlink;
        }

       tmp = tmp->next;
    }

    // If no devices pass filter, return NULL.
    return NULL;
}


/**
 * osinfo_deployment_get_devices:
 * @deployment: a deployment entity
 * @filter: (transfer none)(allow-none): an optional filter
 *
 * Retrieve all the associated devices matching the filter.
 * The filter matches against the device, not the link.
 *
 * Returns: (transfer full): a list of #OsinfoDevice entities
 */
OsinfoDeviceList *osinfo_deployment_get_devices(OsinfoDeployment *deployment, OsinfoFilter *filter)
{
    OsinfoDeviceList *newList;
    GList *tmp;

    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    newList = osinfo_devicelist_new();
    tmp = deployment->priv->deviceLinks;

    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);
        OsinfoDevice *dev = osinfo_devicelink_get_target(devlink);
        if (!filter || osinfo_filter_matches(filter, OSINFO_ENTITY(dev)))
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(dev));

        tmp = tmp->next;
    }

    return newList;
}


/**
 * osinfo_deployment_get_device_links:
 * @deployment: a deployment entity
 * @filter: (transfer none)(allow-none): an optional filter
 *
 * Retrieve all the associated devices matching the filter.
 * The filter matches against the link, not the device.
 *
 * Returns: (transfer full): a list of #OsinfoDevice entities
 */
OsinfoDeviceLinkList *osinfo_deployment_get_device_links(OsinfoDeployment *deployment, OsinfoFilter *filter)
{
    OsinfoDeviceLinkList *newList;
    GList *tmp;

    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);
    g_return_val_if_fail(!filter || OSINFO_IS_FILTER(filter), NULL);

    newList = osinfo_devicelinklist_new();
    tmp = deployment->priv->deviceLinks;

    while (tmp) {
        OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(tmp->data);

        if (!filter || osinfo_filter_matches(filter, OSINFO_ENTITY(devlink)))
            osinfo_list_add(OSINFO_LIST(newList), OSINFO_ENTITY(devlink));

        tmp = tmp->next;
    }

    return newList;
}


/**
 * osinfo_deployment_add_device:
 * @deployment: a deployment entity
 * @dev: (transfer none): the device to associate
 *
 * Associate a device with a deployment. The returned #OsinfoDeviceLink
 * can be used to record extra metadata against the link
 *
 * Returns: (transfer none): the device association
 */
OsinfoDeviceLink *osinfo_deployment_add_device(OsinfoDeployment *deployment, OsinfoDevice *dev)
{
    OsinfoDeviceLink *devlink;

    g_return_val_if_fail(OSINFO_IS_DEPLOYMENT(deployment), NULL);
    g_return_val_if_fail(OSINFO_IS_DEVICE(dev), NULL);

    devlink = osinfo_devicelink_new(dev);

    deployment->priv->deviceLinks = g_list_prepend(deployment->priv->deviceLinks, devlink);

    return devlink;
}
