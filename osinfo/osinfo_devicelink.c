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
 * SECTION:osinfo_devicelink
 * @short_description: A hardware devicelink
 * @see_also: #OsinfoOs, #OsinfoPlatform
 *
 * #OsinfoDeviceLink is an entity representing some kind of hardware
 * devicelink. DeviceLinks can be associated with operating systems
 * and platforms.
 */

struct _OsinfoDeviceLinkPrivate
{
    OsinfoDevice *target;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoDeviceLink, osinfo_devicelink, OSINFO_TYPE_ENTITY);

enum {
    PROP_0,

    PROP_TARGET,

    LAST_PROP
};
static GParamSpec *properties[LAST_PROP];

static void
osinfo_devicelink_set_property(GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
    OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(object);

    switch (property_id)
        {
        case PROP_TARGET:
            if (devlink->priv->target)
                g_object_unref(devlink->priv->target);
            devlink->priv->target = g_value_get_object(value);
            if (devlink->priv->target)
                g_object_ref(devlink->priv->target);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}

static void
osinfo_devicelink_get_property(GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec)
{
    OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(object);

    switch (property_id)
        {
        case PROP_TARGET:
            g_value_set_object(value, devlink->priv->target);
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}


static void
osinfo_devicelink_finalize(GObject *object)
{
    OsinfoDeviceLink *devlink = OSINFO_DEVICELINK(object);

    if (devlink->priv->target)
        g_object_unref(devlink->priv->target);

    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_devicelink_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_devicelink_class_init(OsinfoDeviceLinkClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->set_property = osinfo_devicelink_set_property;
    g_klass->get_property = osinfo_devicelink_get_property;
    g_klass->finalize = osinfo_devicelink_finalize;

    /**
     * OsinfoDeviceLink:target:
     *
     * The target of the device link.
     */
    properties[PROP_TARGET] = g_param_spec_object("target",
                                                  "Target",
                                                  _("Target device"),
                                                  OSINFO_TYPE_DEVICE,
                                                  G_PARAM_CONSTRUCT_ONLY |
                                                  G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(g_klass, LAST_PROP, properties);
}

static void
osinfo_devicelink_init(OsinfoDeviceLink *devlink)
{
    devlink->priv = osinfo_devicelink_get_instance_private(devlink);
}


/**
 * osinfo_devicelink_new:
 * @target: the target device
 *
 * Construct a new link for an #OsinfoDevice. The unique ID
 * of the link is set to match the ID of the target device.
 *
 * Returns: (transfer full): the new device link
 */
OsinfoDeviceLink *osinfo_devicelink_new(OsinfoDevice *target)
{
    return g_object_new(OSINFO_TYPE_DEVICELINK,
                        "id", osinfo_entity_get_id(OSINFO_ENTITY(target)),
                        "target", target,
                        NULL);
}

/**
 * osinfo_devicelink_get_target:
 * @link: the device link
 *
 * Retrieve the #OsinfoDevice that the link points to.
 *
 * Returns: (transfer none): the target of the device link
 */
OsinfoDevice *osinfo_devicelink_get_target(OsinfoDeviceLink *devlink)
{
    return devlink->priv->target;
}

const gchar *osinfo_devicelink_get_driver(OsinfoDeviceLink *devlink)
{
    return osinfo_entity_get_param_value(OSINFO_ENTITY(devlink), OSINFO_DEVICELINK_PROP_DRIVER);
}
