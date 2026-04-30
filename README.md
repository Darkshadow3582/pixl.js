# Pixl.js

[中文](#zh-cn) | [English](#en)

<a id="zh-cn"></a>

这是一个基于原版 [Pixl.js](http://www.espruino.com/Pixl.js) 的复刻版本，主要功能是用来模拟 Amiibo。

## 图片

![image](https://github.com/solosky/pixl.js/blob/main/assets/pixljs-3.jpg)
![image](https://github.com/solosky/pixl.js/blob/main/assets/pixljs-4.jpg)

![image](https://github.com/solosky/pixl.js/blob/main/assets/pixljs-5.jpg)

## 文档

* [中文文档](docs/zh/README.md)
* [英文文档](docs/en/README.md)
* [意大利语文档](docs/it/README.md)

## 内置游戏

由于固件空间和维护成本，主固件不再包含内置游戏。需要自行构建实验性游戏固件的高级用户可以参考 [`game`](https://github.com/solosky/pixl.js/tree/game) 分支。

## 致谢

* [FlipperZero Firmware](https://github.com/flipperdevices/flipperzero-firmware)
* [mlib](https://github.com/P-p-H-d/mlib)
* [TLSF](https://github.com/mattconte/tlsf)
* [cwalk](https://github.com/likle/cwalk)
* [SPIFFS](https://github.com/pellepl/spiffs)
* [ChameleonUltra](https://github.com/RfidResearchGroup/ChameleonUltra)

## 贡献

* 特别感谢 @Caleeeeeeeeeeeee 完善的引导加载程序。
* 特别感谢 @白橙 制作的外壳。
* 特别感谢 @impeeza 提供的文档翻译。

## 讨论群

国内用户可以加入 QQ 群 109761876 进行交流。

## 官方频道

在哪里找到社区？

* [Pixl.js 社区 Discord 服务器](https://discord.gg/4mqeQwcAB2)

## 声明

本项目为开源项目，仅为学习研究用途，请勿用于商业用途。<br />
Amiibo 是任天堂的注册商标，NTAG21X 为 NXP 的注册商标。

内置的 Amiibo 数据库来源分别如下：

* [amiiloop](https://download.amiloop.app/)
* [AmiiboAPI](https://www.amiiboapi.com/)

源代码没有包含任何有任天堂版权的资源（比如相关密钥、Amiibo 原始数据等）。

## 许可证

本项目基于 GPL 2.0 许可证发布，使用请遵循许可证的约定。

* 如果对项目做了修改，需要把改后的源码发布出来
* 发布出来的源码必须要使用相同的许可证发布

## 提示

Amiibo 无限刷需要网上搜索 `key_retail.bin` 文件，然后上传到磁盘根目录后才能使用。

----

<a id="en"></a>

# Pixl.js

This is a recreated version based on the original [Pixl.js](http://www.espruino.com/Pixl.js). Its main purpose is Amiibo emulation.

## Images

![image](https://github.com/solosky/pixl.js/blob/main/assets/pixljs-3.jpg)
![image](https://github.com/solosky/pixl.js/blob/main/assets/pixljs-4.jpg)

![image](https://github.com/solosky/pixl.js/blob/main/assets/pixljs-5.jpg)

## Documentation

* [Chinese Documentation](docs/zh/README.md)
* [English Documentation](docs/en/README.md)
* [Italian Documentation](docs/it/README.md)

## Built-in Games

Built-in games are not part of the main firmware anymore because of firmware size and maintenance trade-offs. Advanced users who want to build an unsupported games firmware can use the [`game`](https://github.com/solosky/pixl.js/tree/game) branch.

## Credits

* [FlipperZero Firmware](https://github.com/flipperdevices/flipperzero-firmware)
* [mlib](https://github.com/P-p-H-d/mlib)
* [TLSF](https://github.com/mattconte/tlsf)
* [cwalk](https://github.com/likle/cwalk)
* [SPIFFS](https://github.com/pellepl/spiffs)
* [ChameleonUltra](https://github.com/RfidResearchGroup/ChameleonUltra)

## Contributions

* Special thanks to @Caleeeeeeeeeeeee for improving the Bootloader.
* Special thanks to @白橙 for making the case.
* Special thanks to @impeeza for the documentation translation.

## Discussion Group

Users in China can join QQ group 109761876 for discussion.

## Official Channel

Where do you find the community?

* [Pixl.js community discord server](https://discord.gg/4mqeQwcAB2)

## Disclaimer

This is an open source project for learning and research only. Do not use it for commercial purposes.<br />
Amiibo is a registered trademark of Nintendo, and NTAG21X is a registered trademark of NXP.

The built-in Amiibo database sources are:

* [amiiloop](https://download.amiloop.app/)
* [AmiiboAPI](https://www.amiiboapi.com/)

The source code does not include any Nintendo-copyrighted resources, such as related keys or raw Amiibo data.

## License

This project is released under the GPL 2.0 License. Please follow the license terms when using it.

* If you modify the project, you need to publish the modified source code
* The published source code must use the same license

## Note

The unlimited Amiibo scan feature requires a `key_retail.bin` file. Search for that file online, then upload it to the disk root before using the feature.
