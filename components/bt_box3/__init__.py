"""ESP32-S3 Box 3 Bluetooth Audio component for ESPHome."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import component
from esphome.const import CONF_ID

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@your_username"]

CONF_BT_BOX3_ID = "bt_box3_id"

bt_box3_ns = cg.esphome_ns.namespace("bt_box3")
BTBox3Component = bt_box3_ns.class_("BTBox3Component", component.Component)

CONFIG_SCHEMA = component.component_schema(BTBox3Component)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await component.register_component(var, config)
