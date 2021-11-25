/*
 * libosinfo: OS installation configuration
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

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_INSTALL_CONFIG (osinfo_install_config_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoInstallConfig,
                                           osinfo_install_config,
                                           OSINFO,
                                           INSTALL_CONFIG,
                                           OsinfoEntity)

#define OSINFO_INSTALL_CONFIG_PROP_HARDWARE_ARCH  "hardware-arch"

#define OSINFO_INSTALL_CONFIG_PROP_L10N_TIMEZONE  "l10n-timezone"
#define OSINFO_INSTALL_CONFIG_PROP_L10N_LANGUAGE  "l10n-language"
#define OSINFO_INSTALL_CONFIG_PROP_L10N_KEYBOARD  "l10n-keyboard"

#define OSINFO_INSTALL_CONFIG_PROP_ADMIN_PASSWORD "admin-password"

#define OSINFO_INSTALL_CONFIG_PROP_USER_PASSWORD  "user-password"
#define OSINFO_INSTALL_CONFIG_PROP_USER_LOGIN     "user-login"
#define OSINFO_INSTALL_CONFIG_PROP_USER_REALNAME  "user-realname"
#define OSINFO_INSTALL_CONFIG_PROP_USER_AUTOLOGIN "user-autologin"
#define OSINFO_INSTALL_CONFIG_PROP_USER_ADMIN     "user-admin"

#define OSINFO_INSTALL_CONFIG_PROP_REG_LOGIN      "reg-login"
#define OSINFO_INSTALL_CONFIG_PROP_REG_PASSWORD   "reg-password"
#define OSINFO_INSTALL_CONFIG_PROP_REG_PRODUCTKEY "reg-product-key"

#define OSINFO_INSTALL_CONFIG_PROP_HOSTNAME    "hostname"
#define OSINFO_INSTALL_CONFIG_PROP_TARGET_DISK "target-disk"
#define OSINFO_INSTALL_CONFIG_PROP_SCRIPT_DISK "script-disk"

#define OSINFO_INSTALL_CONFIG_PROP_AVATAR_LOCATION "avatar-location"
#define OSINFO_INSTALL_CONFIG_PROP_AVATAR_DISK     "avatar-disk"

#define OSINFO_INSTALL_CONFIG_PROP_PRE_INSTALL_DRIVERS_DISK "pre-install-drivers-disk"
#define OSINFO_INSTALL_CONFIG_PROP_PRE_INSTALL_DRIVERS_LOCATION "pre-install-drivers-location"

#define OSINFO_INSTALL_CONFIG_PROP_POST_INSTALL_DRIVERS_DISK "post-install-drivers-disk"
#define OSINFO_INSTALL_CONFIG_PROP_POST_INSTALL_DRIVERS_LOCATION "post-install-drivers-location"

#define OSINFO_INSTALL_CONFIG_PROP_DRIVER_SIGNING "driver-signing"

#define OSINFO_INSTALL_CONFIG_PROP_INSTALLATION_URL "installation-url"

OsinfoInstallConfig *osinfo_install_config_new(const gchar *id);

void osinfo_install_config_set_hardware_arch(OsinfoInstallConfig *config,
                                             const gchar *arch);
const gchar *osinfo_install_config_get_hardware_arch(OsinfoInstallConfig *config);

void osinfo_install_config_set_l10n_keyboard(OsinfoInstallConfig *config,
                                             const gchar *keyboard);
const gchar *osinfo_install_config_get_l10n_keyboard(OsinfoInstallConfig *config);

void osinfo_install_config_set_l10n_language(OsinfoInstallConfig *config,
                                             const gchar *language);
const gchar *osinfo_install_config_get_l10n_language(OsinfoInstallConfig *config);

void osinfo_install_config_set_l10n_timezone(OsinfoInstallConfig *config,
                                             const gchar *tz);
const gchar *osinfo_install_config_get_l10n_timezone(OsinfoInstallConfig *config);




void osinfo_install_config_set_admin_password(OsinfoInstallConfig *config,
                                              const gchar *password);
const gchar *osinfo_install_config_get_admin_password(OsinfoInstallConfig *config);




void osinfo_install_config_set_user_login(OsinfoInstallConfig *config,
                                          const gchar *username);
const gchar *osinfo_install_config_get_user_login(OsinfoInstallConfig *config);


void osinfo_install_config_set_user_password(OsinfoInstallConfig *config,
                                             const gchar *password);
const gchar *osinfo_install_config_get_user_password(OsinfoInstallConfig *config);


void osinfo_install_config_set_user_realname(OsinfoInstallConfig *config,
                                             const gchar *name);
const gchar *osinfo_install_config_get_user_realname(OsinfoInstallConfig *config);


void osinfo_install_config_set_user_autologin(OsinfoInstallConfig *config,
                                              gboolean autologin);
gboolean osinfo_install_config_get_user_autologin(OsinfoInstallConfig *config);


void osinfo_install_config_set_user_administrator(OsinfoInstallConfig *config,
                                                  gboolean admin);
gboolean osinfo_install_config_get_user_administrator(OsinfoInstallConfig *config);



void osinfo_install_config_set_reg_login(OsinfoInstallConfig *config,
                                         const gchar *name);
const gchar *osinfo_install_config_get_reg_login(OsinfoInstallConfig *config);

void osinfo_install_config_set_reg_password(OsinfoInstallConfig *config,
                                            const gchar *password);
const gchar *osinfo_install_config_get_reg_password(OsinfoInstallConfig *config);

void osinfo_install_config_set_reg_product_key(OsinfoInstallConfig *config,
                                               const gchar *key);
const gchar *osinfo_install_config_get_reg_product_key(OsinfoInstallConfig *config);

void osinfo_install_config_set_hostname(OsinfoInstallConfig *config,
                                        const gchar *hostname);
const gchar *osinfo_install_config_get_hostname(OsinfoInstallConfig *config);

void osinfo_install_config_set_target_disk(OsinfoInstallConfig *config,
                                           const gchar *disk);
const gchar *osinfo_install_config_get_target_disk(OsinfoInstallConfig *config);

void osinfo_install_config_set_script_disk(OsinfoInstallConfig *config,
                                           const gchar *disk);
const gchar *osinfo_install_config_get_script_disk(OsinfoInstallConfig *config);

void osinfo_install_config_set_avatar_location(OsinfoInstallConfig *config,
                                               const gchar *location);
const gchar *osinfo_install_config_get_avatar_location(OsinfoInstallConfig *config);

void osinfo_install_config_set_avatar_disk(OsinfoInstallConfig *config,
                                           const gchar *disk);
const gchar *osinfo_install_config_get_avatar_disk(OsinfoInstallConfig *config);

void osinfo_install_config_set_pre_install_drivers_disk(OsinfoInstallConfig *config,
                                                        const gchar *disk);
const gchar *osinfo_install_config_get_pre_install_drivers_disk(OsinfoInstallConfig *config);
void osinfo_install_config_set_pre_install_drivers_location(OsinfoInstallConfig *config,
                                                            const gchar *location);
const gchar *osinfo_install_config_get_pre_install_drivers_location(OsinfoInstallConfig *config);

void osinfo_install_config_set_post_install_drivers_disk(OsinfoInstallConfig *config,
                                                         const gchar *disk);
const gchar *osinfo_install_config_get_post_install_drivers_disk(OsinfoInstallConfig *config);
void osinfo_install_config_set_post_install_drivers_location(OsinfoInstallConfig *config,
                                                             const gchar *location);
const gchar *osinfo_install_config_get_post_install_drivers_location(OsinfoInstallConfig *config);

void osinfo_install_config_set_driver_signing(OsinfoInstallConfig *config,
                                              gboolean signing);
gboolean osinfo_install_config_get_driver_signing(OsinfoInstallConfig *config);

void osinfo_install_config_set_installation_url(OsinfoInstallConfig *config,
                                                const gchar *url);
const gchar *osinfo_install_config_get_installation_url(OsinfoInstallConfig *config);
