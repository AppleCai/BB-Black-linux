/*
 * Copyright 2014 Belkin Inc.
 * Copyright 2015 Andrew Lunn <andrew@lunn.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/gpio.h>

#define TLC591XX_MAX_LEDS	16

#define TLC591XX_REG_MODE1	0x00
#define MODE1_RESPON_ADDR_MASK	0xF0
#define MODE1_NORMAL_MODE	(0 << 4)
#define MODE1_SPEED_MODE	(1 << 4)

#define TLC591XX_REG_MODE2	0x01
#define MODE2_DIM		(0 << 5)
#define MODE2_BLINK		(1 << 5)
#define MODE2_OCH_STOP		(0 << 3)
#define MODE2_OCH_ACK		(1 << 3)

#define TLC591XX_REG_PWM(x)	(0x02 + (x))

#define TLC591XX_REG_GRPPWM	0x12
#define TLC591XX_REG_GRPFREQ	0x13

/* LED Driver Output State, determine the source that drives LED outputs */
#define LEDOUT_OFF		0x0	/* Output LOW */
#define LEDOUT_ON		0x1	/* Output HI-Z */
#define LEDOUT_DIM		0x2	/* Dimming */
#define LEDOUT_BLINK		0x3	/* Blinking */
#define LEDOUT_MASK		0x3

#define ldev_to_led(c)		container_of(c, struct tlc591xx_led, ldev)

struct tlc591xx_led {
	bool active;
	unsigned int led_no;
	struct led_classdev ldev;
	struct tlc591xx_priv *priv;
	struct gpio_desc *own_gpiod;
	u8 output_state;
};

struct tlc591xx_priv {
	struct tlc591xx_led leds[TLC591XX_MAX_LEDS];
	struct regmap *regmap;
	unsigned int reg_ledout_offset;
	struct gpio_chip gpo;
};

struct tlc591xx {
	unsigned int max_leds;
	unsigned int reg_ledout_offset;
};

static const struct tlc591xx tlc59116 = {
	.max_leds = 16,
	.reg_ledout_offset = 0x14,
};

static const struct tlc591xx tlc59108 = {
	.max_leds = 8,
	.reg_ledout_offset = 0x0c,
};

static int
tlc591xx_set_mode(struct regmap *regmap, u8 mode)
{
	int err;
	u8 val;

	err = regmap_write(regmap, TLC591XX_REG_MODE1, MODE1_NORMAL_MODE);
	if (err)
		return err;

	val = MODE2_OCH_STOP | mode;

	return regmap_write(regmap, TLC591XX_REG_MODE2, val);
}

static int
tlc591xx_set_ledout(struct tlc591xx_priv *priv, struct tlc591xx_led *led,
		    u8 val)
{
	unsigned int i = (led->led_no % 4) * 2;
	unsigned int mask = LEDOUT_MASK << i;
	unsigned int addr = priv->reg_ledout_offset + (led->led_no >> 2);
	int ret;

	ret = regmap_update_bits(priv->regmap, addr, mask, val << i);
	if (ret == 0)
		led->output_state = val;

	return ret;
}

static int
tlc591xx_set_pwm(struct tlc591xx_priv *priv, struct tlc591xx_led *led,
		 u8 brightness)
{
	u8 pwm = TLC591XX_REG_PWM(led->led_no);

	return regmap_write(priv->regmap, pwm, brightness);
}

static int
tlc591xx_brightness_set(struct led_classdev *led_cdev,
			enum led_brightness brightness)
{
	struct tlc591xx_led *led = ldev_to_led(led_cdev);
	struct tlc591xx_priv *priv = led->priv;
	int err;

	switch (brightness) {
	case 0:
		err = tlc591xx_set_ledout(priv, led, LEDOUT_OFF);
		break;
	case LED_FULL:
		err = tlc591xx_set_ledout(priv, led, LEDOUT_ON);
		break;
	default:
		err = tlc591xx_set_ledout(priv, led, LEDOUT_DIM);
		if (!err)
			err = tlc591xx_set_pwm(priv, led, brightness);
	}

	return err;
}

static int tlc591xx_gpo_get(struct gpio_chip *chip, unsigned offset)
{
	struct tlc591xx_priv *priv = dev_get_drvdata(chip->parent);

	if (priv->leds[offset].output_state == LEDOUT_OFF)
		return 0;
	else
		return 1;
}

static void tlc591xx_gpo_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct tlc591xx_priv *priv = dev_get_drvdata(chip->parent);

	if (WARN_ON(priv->leds[offset].active))
		return;

	tlc591xx_set_ledout(priv, &priv->leds[offset],
			    value ? LEDOUT_ON : LEDOUT_OFF);
}

static int tlc591xx_gpo_get_direction(struct gpio_chip *chip, unsigned offset)
{
	struct tlc591xx_priv *priv = dev_get_drvdata(chip->parent);

	if (priv->leds[offset].active)
		return -EBUSY;

	return GPIOF_DIR_OUT; /* only supported direction is out */
}

static int tlc591xx_gpo_direction_out(struct gpio_chip *chip, unsigned offset,
				      int value)
{
	struct tlc591xx_priv *priv = dev_get_drvdata(chip->parent);

	if (WARN_ON(priv->leds[offset].active))
		return -EBUSY;

	tlc591xx_set_ledout(priv, &priv->leds[offset],
			    value ? LEDOUT_ON : LEDOUT_OFF);
	return 0;
}

static int tlc591xx_gpochip_add(struct device *dev, u16 ngpo)
{
	struct tlc591xx_priv *priv = dev_get_drvdata(dev);
	u32 ngpios;
	int err;

	if (of_property_read_u32(dev->of_node, "ngpios", &ngpios) == 0) {
		if (ngpios > ngpo) {
			dev_err(dev,
				"Too big \"ngpios\" property value %u > %u\n",
				ngpios, ngpo);
			return -EINVAL;
		}
		ngpo = ngpios;
	}

	priv->gpo.label			= "tlc591xx-gpo";
	priv->gpo.base			= -1;
	priv->gpo.ngpio			= ngpo;
	priv->gpo.parent		= dev;
#ifdef CONFIG_OF_GPIO
	priv->gpo.of_node		= dev->of_node;
#endif
	priv->gpo.owner			= THIS_MODULE;
	priv->gpo.get			= tlc591xx_gpo_get;
	priv->gpo.set			= tlc591xx_gpo_set;
	priv->gpo.get_direction		= tlc591xx_gpo_get_direction;
	priv->gpo.direction_output	= tlc591xx_gpo_direction_out;
	priv->gpo.can_sleep		= true;

	err = gpiochip_add(&priv->gpo);
	if (err < 0) {
		dev_err(dev, "could not register gpiochip, %d\n", err);
		priv->gpo.ngpio = 0;
	}
	return err;
}

static void
tlc591xx_destroy_devices(struct tlc591xx_priv *priv, unsigned int j)
{
	int i = j;

	while (--i >= 0) {
		if (priv->leds[i].active) {
			led_classdev_unregister(&priv->leds[i].ldev);
			gpiochip_free_own_desc(priv->leds[i].own_gpiod);
		}
	}

	if (priv->gpo.ngpio)
		gpiochip_remove(&priv->gpo);
}

static int
tlc591xx_configure(struct device *dev,
		   struct tlc591xx_priv *priv,
		   const struct tlc591xx *tlc591xx)
{
	unsigned int i;
	int err = 0;

	tlc591xx_set_mode(priv->regmap, MODE2_DIM);
	for (i = 0; i < TLC591XX_MAX_LEDS; i++) {
		struct tlc591xx_led *led = &priv->leds[i];

		if (!led->active)
			continue;

		err = led_classdev_register(dev, &led->ldev);
		if (err < 0) {
			dev_err(dev, "couldn't register LED %s\n",
				led->ldev.name);
			goto exit;
		}

		if (i < priv->gpo.ngpio) {
			led->own_gpiod =
				gpiochip_request_own_desc(&priv->gpo, i,
							  led->ldev.name);
			if (IS_ERR(led->own_gpiod)) {
				err = PTR_ERR(led->own_gpiod);
				dev_err(dev,
					"Requesting own gpio for '%s'-led failed: %d\n",
					led->ldev.name, err);
				goto exit;
			}
		}
	}

	return 0;

exit:
	tlc591xx_destroy_devices(priv, i);
	return err;
}

static const struct regmap_config tlc591xx_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x1e,
};

static const struct of_device_id of_tlc591xx_leds_match[] = {
	{ .compatible = "ti,tlc59116",
	  .data = &tlc59116 },
	{ .compatible = "ti,tlc59108",
	  .data = &tlc59108 },
	{},
};
MODULE_DEVICE_TABLE(of, of_tlc591xx_leds_match);

static int
tlc591xx_probe(struct i2c_client *client,
	       const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node, *child;
	struct device *dev = &client->dev;
	const struct of_device_id *match;
	const struct tlc591xx *tlc591xx;
	struct tlc591xx_priv *priv;
	int err, reg, i;

	match = of_match_device(of_tlc591xx_leds_match, dev);
	if (!match)
		return -ENODEV;

	tlc591xx = match->data;
	if (!np)
		return -ENODEV;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	for (i = 0; i < TLC591XX_MAX_LEDS; i++) {
		struct tlc591xx_led *led = &priv->leds[i];

		led->priv = priv;
		led->led_no = i;
		led->ldev.brightness_set_blocking = tlc591xx_brightness_set;
		led->ldev.max_brightness = LED_FULL;
	}

	priv->regmap = devm_regmap_init_i2c(client, &tlc591xx_regmap);
	if (IS_ERR(priv->regmap)) {
		err = PTR_ERR(priv->regmap);
		dev_err(dev, "Failed to allocate register map: %d\n", err);
		return err;
	}
	priv->reg_ledout_offset = tlc591xx->reg_ledout_offset;

	i2c_set_clientdata(client, priv);

	for_each_child_of_node(np, child) {
		err = of_property_read_u32(child, "reg", &reg);
		if (err)
			continue;
		if (reg < 0 || reg >= tlc591xx->max_leds)
			return -EINVAL;
		if (priv->leds[reg].active)
			return -EINVAL;
		priv->leds[reg].active = true;
		priv->leds[reg].ldev.name =
			of_get_property(child, "label", NULL) ? : child->name;
		priv->leds[reg].ldev.default_trigger =
			of_get_property(child, "linux,default-trigger", NULL);
	}

	if (of_property_read_bool(np, "gpio-controller")) {
		err = tlc591xx_gpochip_add(dev, tlc591xx->max_leds);
		if (err)
			return err;
	}

	return tlc591xx_configure(dev, priv, tlc591xx);
}

static int
tlc591xx_remove(struct i2c_client *client)
{
	struct tlc591xx_priv *priv = i2c_get_clientdata(client);

	tlc591xx_destroy_devices(priv, TLC591XX_MAX_LEDS);

	return 0;
}

static const struct i2c_device_id tlc591xx_id[] = {
	{ "tlc59116" },
	{ "tlc59108" },
	{},
};
MODULE_DEVICE_TABLE(i2c, tlc591xx_id);

static struct i2c_driver tlc591xx_driver = {
	.driver = {
		.name = "tlc591xx",
		.of_match_table = of_match_ptr(of_tlc591xx_leds_match),
	},
	.probe = tlc591xx_probe,
	.remove = tlc591xx_remove,
	.id_table = tlc591xx_id,
};

module_i2c_driver(tlc591xx_driver);

MODULE_AUTHOR("Andrew Lunn <andrew@lunn.ch>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TLC591XX LED driver");
