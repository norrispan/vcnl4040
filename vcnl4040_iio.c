/*
 * VCNL4040.c - Support for Vishay VCNL4040 combined ambient light and
 * proximity sensor
 *
 * Copyright 2018 Norris Pan <pny1989@gmail.com>
 *
 * This file is subject to the terms and conditions of version 2 of
 * the GNU General Public License.  See the file COPYING in the main
 * directory of this archive for more details.
 *
 * IIO driver for VCNL4040 (7-bit I2C slave address 0x60)
 *
 *
 *   allow to adjust IR current
 *   proximity threshold and event handling
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define VCNL4040_DRV_NAME "vcnl4040"
//#define VCNL_4040_PROC_ID    0x0C
#define VCNL_4040_PS         0x08
#define VCNL_4040_ALS        0x09


struct VCNL4040_data {
	struct i2c_client *client;
};

static const struct i2c_device_id VCNL4040_id[] = {
	{ "VCNL4040", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, VCNL4040_id);


static const struct iio_chan_spec VCNL4040_channels[] = {
	{
		.type = IIO_LIGHT,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW)   //  light  ?
	}, {
		.type = IIO_PROXIMITY,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	}
};

static const struct iio_info VCNL4040_info = {
	.read_raw = VCNL4040_read_raw,
	//.write_raw = VCNL4040_write_raw,
	.driver_module = THIS_MODULE,
};

static int VCNL4040_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct VCNL4040_data *data;
	struct iio_dev *indio_dev;
	int ret;



	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev){
		return -ENOMEM;
	}  // allocates a struct iio_dev, along with the private data


	data = iio_priv(indio_dev); //?

	i2c_set_clientdata(client, indio_dev);
	data->client = client;

	// ret = i2c_smbus_read_byte_data(data->client, VCNL4040_PROC_ID);
	// if (ret < 0){
	// 	return ret;
	// }  //???

	//
	// dev_info(&client->dev, "VCNL4040 Ambient light/proximity sensor, Prod %02x, Rev: %02x\n",
	// 	ret >> 4, ret & 0xf);

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &VCNL4040_info;
	indio_dev->channels = VCNL4040_channels;
	indio_dev->num_channels = ARRAY_SIZE(VCNL4040_channels);
	indio_dev->name = VCNL4040_DRV_NAME;
	indio_dev->modes = INDIO_DIRECT_MODE;
	return devm_iio_device_register(&client->dev, indio_dev);
}


// static int vcnl4040_conf(struct vcnl4040_data *data, u8 dev_reg, u16 value){
// 	// configuration
// 	int ret;
// 	ret = i2c_smbus_write_word_data(data->client, u8 dev_reg, value);
// 	if(ret < 0){
// 		return ret;
// 	}
// 	return 0;
// }


static int vcnl4040_measure(struct vcnl4040_data, u8 dev_reg, s32 *val){
	s32 res;
	res = i2c_smbus_read_word_data(data->client, dev_reg);
	// error handling
	if(res < 0){
		return res;
	}

	*val = res;
	return 0;
}

static int vcnl4040_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, s32 *val){
	int ret = -EINVAL;
	struct vcnl4040_data *data = iio_priv(indio_dev);
	switch(chan->type){
	case IIO_LIGHT:
		ret = vcnl4040_measure(data, VCNL4000_ALS, val);
		if(ret < 0){
			return ret;
		}
		ret = IIO_VAL_INT;
		break;

	case IIO_PROXIMITY:
		ret = vcnl4040_measure(data, VCNL_4040_PS, val);
		if(ret < 0){
			return ret;
		}
		ret = IIO_VAL_INT;
		break;
	default:
		break;
	}
	return ret;
}




//read_raw

//write_raw

static struct i2c_driver VCNL4040_driver = {
	.driver = {
		.name   = VCNL4040_DRV_NAME,
		.owner  = THIS_MODULE,
	},
	.probe  = VCNL4040_probe,
	.id_table = VCNL4040_id,
};

module_i2c_driver(VCNL4040_driver);

MODULE_AUTHOR("Norris Pan <pny1989@gmail.com>");
MODULE_DESCRIPTION("Vishay VCNL4040 proximity/ambient light sensor driver");
MODULE_LICENSE("GPL");
