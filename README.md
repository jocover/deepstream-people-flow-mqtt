# 基于deepstream人流识别边缘计算

## 主要特性

- 使用Jetson Nano进行边缘计算
- 使用peoplenet训练模型进行推理
- MsgConvBroker自定义实现
- 数据通过MQTT协议提交到OneNET云平台


## 项目内容
- 项目通过deepstream实现8条rtsp监控视频流推理，通过Gst-nvdsanalytics插件的ROI设置区域内人流计算。
- 修改custom payload实现推理结果传送，通过msgconv和mqtt实现数据上传至云端

* 由于jeston nano性能限制，推理速度只能到10fps左右，使用xavier可以达到300fps


## 参考
- People count application With Deepstream SDK[https://github.com/NVIDIA-AI-IOT/deepstream-occupancy-analytics]
- Deepstream SDK [https://developer.nvidia.com/deepstream-sdk]
- OneNet[https://open.iot.10086.cn/doc/v5/develop/detail/iot_platform]


