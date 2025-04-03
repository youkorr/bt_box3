import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@youkorr"]

bt_box3_ns = cg.esphome_ns.namespace("bt_box3")
BTBox3Component = bt_box3_ns.class_("BTBox3Component", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BTBox3Component),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
