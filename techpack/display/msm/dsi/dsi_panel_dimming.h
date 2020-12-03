#ifndef DSI_PANEL_DIMMING_H
#define DSI_PANEL_DIMMING_H

#include <linux/module.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <video/mipi_display.h>

#include "dsi_display.h"
#include "dsi_panel.h"
#include "sde_kms.h"
#include "sde_connector.h"
#include "sde_encoder.h"
#include "sde_encoder_phys.h"

enum PANEL_LEVEL_KEY
{
	LEVEL_KEY_NONE = 0,
	LEVEL0_KEY = BIT(0),
	LEVEL1_KEY = BIT(1),
	LEVEL2_KEY = BIT(2),
};

#define BRIGHTNESS_MAX_PACKET 50
#define BRIGHTNESS_TOP_LEVEL_KEY (LEVEL1_KEY | LEVEL2_KEY)

#define get_bit(value, shift, width)	((value >> shift) & (GENMASK(width - 1, 0)))

static inline bool __must_check DSI_PANEL_IS_CMDS_NULL(struct dsi_panel_cmd_set *set)
{
	return unlikely(!set) || unlikely(!set->cmds);
}

static inline struct dsi_panel_cmd_set *dsi_panel_get_cmds(
	struct dsi_panel *panel,
	enum dsi_cmd_set_type type)
{
	struct dsi_display_mode_priv_info *priv_info;
	struct dsi_panel_cmd_set *set;

	if (!panel->cur_mode || !panel->cur_mode->priv_info)
	{
		DSI_ERR("err: (%d) panel has no valid priv\n", type);
		return NULL;
	}

	priv_info = panel->cur_mode->priv_info;
	set = &priv_info->cmd_sets[type];
	return set;
}

static char acl_on_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x02
};

static char gamma_mode2_normal_tx_cmds[] = {
	0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x53, 0x28,
	0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xB1, 0x00, 0x14,
	0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x51, 0x03, 0xFF,
	0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xF7, 0x03
};

static char level0_key_enable_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x9f, 0xa5, 0xa5
};

static char level0_key_disable_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x9f, 0x5a, 0x5a
};

static char level1_key_enable_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x5a, 0x5a
};

static char level1_key_disable_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x5a, 0x5a
};

static char level2_key_enable_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x5a, 0x5a
};

static char level2_key_disable_tx_cmds[] = {
	0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x5a, 0x5a
};

static char brightness_tx_cmds[] = {
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00
};

int dsi_panel_brightness_dcs(struct dsi_panel *panel, u16 brightness);

#endif