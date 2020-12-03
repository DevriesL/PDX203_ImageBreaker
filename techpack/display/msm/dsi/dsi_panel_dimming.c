#include "dsi_panel_dimming.h"
#if 0
#include "sde_expo_dim_layer.h"
#endif

extern int dsi_panel_tx_cmd_set(struct dsi_panel *panel, enum dsi_cmd_set_type type);

static void update_brightness_packet(struct dsi_cmd_desc *packet,
		int *count, struct dsi_panel_cmd_set *tx_cmd)
{
	int loop = 0;

	if (IS_ERR_OR_NULL(packet)) {
		DSI_ERR("%ps no packet\n", __builtin_return_address(0));
		return;
	}

	if (DSI_PANEL_IS_CMDS_NULL(tx_cmd)) {
		DSI_DEBUG("%ps no tx_cmd\n", __builtin_return_address(0));
		return;
	}

	if (*count > (BRIGHTNESS_MAX_PACKET - 1))
		panic("over max brightness_packet size(%d).. !!",
				BRIGHTNESS_MAX_PACKET);

	for (loop = 0; loop < tx_cmd->count; loop++)
		packet[(*count)++] = tx_cmd->cmds[loop];
}

void update_packet_level_key_enable(struct dsi_panel *panel,
		struct dsi_cmd_desc *packet, int *cmd_cnt, int level_key)
{
	if (!level_key)
		return;
	else {
		if (level_key & LEVEL0_KEY)
			update_brightness_packet(packet, cmd_cnt, dsi_panel_get_cmds(panel, TX_LEVEL0_KEY_ENABLE));

		if (level_key & LEVEL1_KEY)
			update_brightness_packet(packet, cmd_cnt, dsi_panel_get_cmds(panel, TX_LEVEL1_KEY_ENABLE));

		if (level_key & LEVEL2_KEY)
			update_brightness_packet(packet, cmd_cnt, dsi_panel_get_cmds(panel, TX_LEVEL2_KEY_ENABLE));
	}
}

void update_packet_level_key_disable(struct dsi_panel *panel,
		struct dsi_cmd_desc *packet, int *cmd_cnt, int level_key)
{
	if (!level_key)
		return;
	else {
		if (level_key & LEVEL0_KEY)
			update_brightness_packet(packet, cmd_cnt, dsi_panel_get_cmds(panel, TX_LEVEL0_KEY_DISABLE));

		if (level_key & LEVEL1_KEY)
			update_brightness_packet(packet, cmd_cnt, dsi_panel_get_cmds(panel, TX_LEVEL1_KEY_DISABLE));

		if (level_key & LEVEL2_KEY)
			update_brightness_packet(packet, cmd_cnt, dsi_panel_get_cmds(panel, TX_LEVEL2_KEY_DISABLE));
	}
}

static struct dsi_panel_cmd_set *ss_acl_on(struct dsi_panel *panel, int *level_key)
{
	struct dsi_panel_cmd_set *pcmds;

	*level_key = LEVEL_KEY_NONE;

	pcmds = dsi_panel_get_cmds(panel, TX_ACL_ON);
	if (DSI_PANEL_IS_CMDS_NULL(pcmds)) {
		DSI_ERR("No cmds for TX_ACL_ON..\n");
		return NULL;
	}

	pcmds->cmds[0].msg.tx_buf[1] = 0x02;	/* 55h ACL 15% */

	return pcmds;
}

static struct dsi_panel_cmd_set *ss_brightness_gamma_mode2_normal(struct dsi_panel *panel, int *level_key,
		u16 brightness)
{
	struct dsi_panel_cmd_set *pcmds;

	*level_key = LEVEL1_KEY;

	pcmds = dsi_panel_get_cmds(panel, TX_GAMMA_MODE2_NORMAL);

	pcmds->cmds[2].msg.tx_buf[1] = get_bit(brightness, 8, 2);
	pcmds->cmds[2].msg.tx_buf[2] = get_bit(brightness, 0, 8);

	return pcmds;
}

static int dsi_panel_brightness_packet_set(struct dsi_panel *panel, u16 brightness)
{
	int cmd_cnt = 0;
	int level_key = 0;
	struct dsi_panel_cmd_set *set;
	struct dsi_cmd_desc *packet = NULL;
	struct dsi_panel_cmd_set *tx_cmd = NULL;

	/* init packet */
	set = dsi_panel_get_cmds(panel, TX_BRIGHT_CTRL);
	if (DSI_PANEL_IS_CMDS_NULL(set)) {
		DSI_ERR("No cmds for TX_BRIGHT_CTRL.. \n");
		return -EINVAL;
	}

	packet = set->cmds;

	level_key = BRIGHTNESS_TOP_LEVEL_KEY;
	update_packet_level_key_enable(panel, packet, &cmd_cnt, level_key);

	/* acl */
	level_key = false;
	tx_cmd = ss_acl_on(panel, &level_key);
	update_packet_level_key_enable(panel, packet, &cmd_cnt, level_key);
	update_brightness_packet(packet, &cmd_cnt, tx_cmd);
	update_packet_level_key_disable(panel, packet, &cmd_cnt, level_key);

	/* gamma */
	level_key = false;
	tx_cmd = ss_brightness_gamma_mode2_normal(panel, &level_key, brightness);
	update_packet_level_key_enable(panel, packet, &cmd_cnt, level_key);
	update_brightness_packet(packet, &cmd_cnt, tx_cmd);
	update_packet_level_key_disable(panel, packet, &cmd_cnt, level_key);

	level_key = BRIGHTNESS_TOP_LEVEL_KEY;
	update_packet_level_key_disable(panel, packet, &cmd_cnt, level_key);

	return cmd_cnt;
}

static int dsi_panel_single_transmission_packet(struct dsi_panel_cmd_set *cmds)
{
	int loop;
	struct dsi_cmd_desc *packet = cmds->cmds;
	int packet_cnt = cmds->count;

	for (loop = 0; (loop < packet_cnt) && (loop < BRIGHTNESS_MAX_PACKET); loop++) {
		if (packet[loop].msg.type == MIPI_DSI_DCS_LONG_WRITE ||
			packet[loop].msg.type == MIPI_DSI_GENERIC_LONG_WRITE) {
			packet[loop].last_command = false;
		} else {
			if (loop > 0)
				packet[loop - 1].last_command = true; /*To ensure previous single tx packet */

			packet[loop].last_command = true;
		}

		/* use null packet for last_command in brightness setting */
		if (packet[loop].msg.tx_buf[0] == 0x0)
			packet[loop].last_command = true;
	}

	if (loop == BRIGHTNESS_MAX_PACKET)
		return false;
	else {
		packet[loop - 1].last_command = true; /* To make last packet flag */
		return true;
	}
}

static bool is_hbm_level(u16 brightness)
{
	return false;
}

#if 0
static void ss_calc_brightness_level(struct samsung_display_driver_data *vdd, int level)
{
	struct backlight_device *bd = GET_SDE_BACKLIGHT_DEVICE(vdd);
	int bl_level, bd_level = bd->props.brightness;
	bool use_current_bl_level =
		level == USE_CURRENT_BL_LEVEL;
	bool should_skip_update =
		use_current_bl_level && (bd_level != vdd->br_info.common_br.bl_level);

	if (use_current_bl_level) {
		bl_level = bd_level;
	} else {
		bl_level = level;
	}

	if (ss_is_panel_on(vdd)) {
		vdd->br_info.common_br.bl_level = 
			expo_map_dim_level(bl_level, GET_DSI_DISPLAY(vdd), should_skip_update);
	} else if (!use_current_bl_level) {
		vdd->br_info.common_br.bl_level = level;
	}
}
#endif

static int dsi_panel_send_cmd(struct dsi_panel *panel,
		enum dsi_cmd_set_type type)
{
	struct dsi_panel_cmd_set *set;
	struct dsi_display *dsi_display = dsi_display_get_main_display();
	int rc = 0;

	if (IS_ERR_OR_NULL(panel)) {
		DSI_ERR("panel is null\n");
		return -ENODEV;
	}

	if (IS_ERR_OR_NULL(dsi_display)) {
		DSI_ERR("dsi_display is null\n");
		return -ENODEV;
	}

	set = dsi_panel_get_cmds(panel, type);

	/* case 03063186:
	 * To prevent deadlock between phandle->phandle_lock and panel->panel_lock,
	 * dsi_display_clk_ctrl() should be called without locking panel_lock.
	 */
	rc = dsi_display_clk_ctrl(dsi_display->dsi_clk_handle,
			DSI_ALL_CLKS, DSI_CLK_ON);
	if (rc) {
		DSI_ERR("[%s] failed to enable DSI core clocks, rc=%d\n",
				dsi_display->name, rc);
		rc = dsi_display_clk_ctrl(dsi_display->dsi_clk_handle,
				DSI_ALL_CLKS, DSI_CLK_OFF);
		goto error;
	}

	dsi_panel_tx_cmd_set(panel, type);

	rc = dsi_display_clk_ctrl(dsi_display->dsi_clk_handle,
			DSI_ALL_CLKS, DSI_CLK_OFF);
	if (rc) {
		DSI_ERR("[%s] failed to disable DSI core clocks, rc=%d\n",
				dsi_display->name, rc);
		goto error;
	}

error:
	return rc;
}

int dsi_panel_brightness_dcs(struct dsi_panel *panel, u16 brightness)
{
	int cmd_cnt = 0;
	int ret = 0;
	struct dsi_panel_cmd_set *brightness_cmds = NULL;

#if 0
	ss_calc_brightness_level(vdd, level);
#endif

	if (is_hbm_level(brightness)) {
		cmd_cnt = 0;
	} else {
		cmd_cnt = dsi_panel_brightness_packet_set(panel, brightness);
	}

	if (cmd_cnt) {
		/* setting tx cmds cmt */
		brightness_cmds = dsi_panel_get_cmds(panel, TX_BRIGHT_CTRL);
		brightness_cmds->count = cmd_cnt;

		/* generate single tx packet */
		ret = dsi_panel_single_transmission_packet(brightness_cmds);

		/* sending tx cmds */
		if (ret) {
			dsi_panel_send_cmd(panel, TX_BRIGHT_CTRL);
		}
	}

	return ret;
}
